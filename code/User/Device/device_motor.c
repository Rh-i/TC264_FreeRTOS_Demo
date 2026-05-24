/**
 * @file device_motor.c
 * @brief 电机设备驱动实现
 * @note 支持两种运动模式：纯速度、速度+时间
 * @date 2026-05-24
 */

#include "device_motor.h"
#include "FreeRTOS.h"
#include "bsp_encoder.h"
#include "bsp_pwm.h"
#include "isr_config.h"
#include "task.h"
#include "uart_protocol.h" /* 用于运动完成时主动上报 */

#pragma section all "cpu0_dsram"

/*==============================================================================
 * 全局电机设备实例
 *============================================================================*/

/* 全局电机设备 - TIM2 编码器 + ATOM1_CH0 PWM */
struct DeviceMotor g_motor;

/*==============================================================================
 * 私有辅助函数
 *============================================================================*/

/**
 * @brief 获取系统运行时间（毫秒）
 * @note 使用FreeRTOS的TickCount，每毫秒一个tick
 */
static uint32 get_system_time_ms(void)
{
  return (uint32)xTaskGetTickCount();
}

/**
 * @brief PWM输出和方向控制
 */
static void motor_output_pwm(DeviceMotor *motor)
{
  /* 方向控制 */
  if (motor->output > 0)
  {
    gpio_set_level(motor->dir_pin, MOTOR_DIR_REVERSE);
  }
  else if (motor->output < 0)
  {
    gpio_set_level(motor->dir_pin, MOTOR_DIR_FORWARD);
    motor->output = -motor->output;
  }
  else
  {
    gpio_set_level(motor->dir_pin, MOTOR_DIR_REVERSE);
  }

  /* 限幅处理 */
  if (motor->output > (int32)motor->max_output)
  {
    motor->output = motor->max_output;
  }

  /* 输出PWM */
  bsp_pwm_set_duty(motor->pwm, (uint32)motor->output);
}

/**
 * @brief 检查运动是否完成
 */
static uint8 check_motion_completed(DeviceMotor *motor)
{
  uint8 completed = 0;

  switch (motor->motion.mode)
  {
    case MOTOR_MODE_STOP:
      completed = 1;
      break;

    case MOTOR_MODE_SPEED:
      /* 纯速度模式持续运行，直到被停止 */
      completed = 0;
      break;

    case MOTOR_MODE_SPEED_TIME:
      /* 速度+时间模式：到达指定时间后完成 */
      {
        uint32 elapsed = get_system_time_ms() - motor->motion.start_time;
        if (elapsed >= motor->motion.target_time)
        {
          completed = 1;
        }
      }
      break;

    default:
      completed = 0;
      break;
  }

  return completed;
}

/*==============================================================================
 * 公共函数实现
 *============================================================================*/

/**
 * @brief 初始化电机设备
 */
void device_motor_init(DeviceMotor *motor, BspEncoder *encoder, BspPwm *pwm, gpio_pin_enum dir_pin, float speed_kp, float speed_ki, float speed_kd, uint32 out_max)
{
  motor->encoder      = encoder;
  motor->pwm          = pwm;
  motor->dir_pin      = dir_pin;
  motor->max_output   = out_max;
  motor->enabled      = 0;
  motor->output       = 0;
  motor->actual_speed = 0;

  /* 初始化运动状态 */
  motor->motion.mode         = MOTOR_MODE_STOP;
  motor->motion.target_speed = 0;
  motor->motion.target_time  = 0;
  motor->motion.start_time   = 0;
  motor->motion.is_active    = 0;
  motor->motion.is_completed = 0;

  /* 初始化速度环PID */
  SpeedPID_Init(&motor->speed_pid, speed_kp, speed_ki, speed_kd, (float)out_max);

  /* 初始化方向控制引脚 */
  gpio_init(dir_pin, GPO, GPIO_LOW, GPO_PUSH_PULL);
}

/**
 * @brief 设置电机目标速度（纯速度模式）
 */
void device_motor_set_speed(DeviceMotor *motor, int32 speed)
{
  motor->motion.mode         = MOTOR_MODE_SPEED;
  motor->motion.target_speed = speed;
  motor->motion.is_active    = 1;
  motor->motion.is_completed = 0;
}

/**
 * @brief 设置速度+时间模式
 */
void device_motor_set_speed_time(DeviceMotor *motor, int32 speed, uint32 time_ms)
{
  motor->motion.mode         = MOTOR_MODE_SPEED_TIME;
  motor->motion.target_speed = speed;
  motor->motion.target_time  = time_ms;
  motor->motion.start_time   = get_system_time_ms();
  motor->motion.is_active    = 1;
  motor->motion.is_completed = 0;
}

/**
 * @brief 停止电机
 */
void device_motor_stop(DeviceMotor *motor)
{
  motor->motion.mode         = MOTOR_MODE_STOP;
  motor->motion.target_speed = 0;
  motor->motion.is_active    = 0;
  motor->motion.is_completed = 1;

  motor->output = 0;
  gpio_set_level(motor->dir_pin, MOTOR_DIR_REVERSE);
  bsp_pwm_set_duty(motor->pwm, 0);

  /* 重置速度环PID */
  SpeedPID_Reset(&motor->speed_pid);
}

/**
 * @brief 更新电机状态
 * @note 每20ms调用一次
 */
uint8 device_motor_update(DeviceMotor *motor)
{
  int16 delta_count;

  /* 读取编码器获取增量 */
  delta_count = (int16)bsp_encoder_get_count(motor->encoder);
  bsp_encoder_clear_count(motor->encoder);

  /* 电机未使能则停止 */
  if (motor->enabled == 0)
  {
    device_motor_stop(motor);
    return 1;
  }

  /* 检查运动是否完成 */
  if (motor->motion.is_active)
  {
    if (check_motion_completed(motor))
    {
      /* 运动完成，真正停止电机 */
      device_motor_stop(motor);

      return 1;
    }
  }

  /* 根据运动模式计算PID输出 */
  switch (motor->motion.mode)
  {
    case MOTOR_MODE_STOP:
      motor->output = 0;
      break;

    case MOTOR_MODE_SPEED:
    case MOTOR_MODE_SPEED_TIME:
      /* 速度模式：使用速度环PID */
      {
        float pwm_out = SpeedPID_Calculate(&motor->speed_pid,
                                           (float)motor->motion.target_speed,
                                           delta_count);
        motor->output = (int32)pwm_out;
        /* 更新实际速度 */
        motor->actual_speed = motor->speed_pid.speed_cm_s;
      }
      break;

    default:
      motor->output = 0;
      break;
  }

  /* 输出PWM */
  motor_output_pwm(motor);

  return 0;
}

/**
 * @brief 获取电机实际速度
 */
int32_t device_motor_get_speed(DeviceMotor *motor)
{
  return motor->actual_speed;
}

/**
 * @brief 获取运动是否完成
 */
uint8 device_motor_is_completed(DeviceMotor *motor)
{
  return motor->motion.is_completed;
}

/**
 * @brief 使能电机
 */
void device_motor_enable(DeviceMotor *motor, uint8 enable)
{
  motor->enabled = enable;
}

/**
 * @brief 设置最大输出限幅
 */
void device_motor_set_max_output(DeviceMotor *motor, uint32 max_output)
{
  motor->max_output = max_output;
}

/**
 * @brief 电机总初始化函数
 * @note 初始化TIM2编码器+ATOM1_CH0 PWM+P22_3方向控制
 */
void device_motor_all_init(void)
{
  /* 初始化全局电机设备 - 默认PID参数 */
  device_motor_init(&g_motor,
                    &bsp_encoder_tim2,
                    &bsp_pwm_motor,
                    P22_3,
                    15.0f,    /* speed_kp */
                    3.0f,     /* speed_ki */
                    5.0f,     /* speed_kd */
                    10000);   /* out_max */
  pit_ms_init(CCU61_CH1, 20); // 开启处理电机pid的20ms一次的中断初始化
}

IFX_INTERRUPT(cc61_pit_ch1_isr, 0, CCU6_1_CH1_ISR_PRIORITY)
{
  interrupt_global_enable(0); // 开启中断嵌套

  device_motor_update(&g_motor); // 20ms处理一次电机pid

  pit_clear_flag(CCU61_CH1);
}

#pragma section all restore