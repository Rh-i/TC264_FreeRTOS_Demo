#include "device_motor.h"
#include "bsp_encoder.h"
#include "bsp_pwm.h"

#pragma section all "cpu0_dsram"

/*==============================================================================
 * 全局电机设备实例
 *============================================================================*/

/* 全局电机设备 - TIM2 编码器 + ATOM1_CH0 PWM */
struct DeviceMotor g_motor;

#pragma section all restore

/*==============================================================================
 * 公共函数实现
 *============================================================================*/

/**
 * @brief 初始化电机设备
 * @param motor 电机设备指针
 * @param encoder 编码器设备指针
 * @param pwm PWM 设备指针
 * @param dir_pin 方向控制引脚
 * @param kp PID比例系数
 * @param ki PID积分系数
 * @param kd PID微分系数
 * @param detach PID积分分离值
 */
void device_motor_init(DeviceMotor *motor, BspEncoder *encoder, BspPwm *pwm, gpio_pin_enum dir_pin, float kp, float ki, float kd, float detach)
{
    motor->encoder      = encoder;
    motor->pwm         = pwm;
    motor->dir_pin     = dir_pin;
    motor->target_speed = 0;
    motor->actual_speed = 0;
    motor->output      = 0;
    motor->max_output  = 5000;
    motor->direction   = MOTOR_DIR_FORWARD;
    motor->enabled     = 0;

    /* 初始化速度PID */
    BasePID_Init(&motor->speed_pid, kp, ki, kd, detach);

    /* 初始化方向控制引脚 */
    gpio_init(dir_pin, GPO, GPIO_LOW, GPO_PUSH_PULL);
}

/**
 * @brief 设置电机目标速度
 * @param motor 电机设备指针
 * @param speed 目标速度值
 */
void device_motor_set_speed(DeviceMotor *motor, int32 speed)
{
    motor->target_speed = speed;
}

/**
 * @brief 更新电机状态
 * @param motor 电机设备指针
 */
void device_motor_update(DeviceMotor *motor)
{
    /* 读取编码器获取实际速度 */
    motor->actual_speed = bsp_encoder_get_count(motor->encoder);
    bsp_encoder_clear_count(motor->encoder);

    if (motor->enabled == 0)
    {
        device_motor_stop(motor);
        return;
    }

    /* 计算PID输出 */
    motor->output = (int32)BasePID_PositionControl(&motor->speed_pid, (float)motor->target_speed, (float)motor->actual_speed);

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

    /* 限幅处理 */
    if (motor->output > (int32)motor->max_output)
    {
        motor->output = motor->max_output;
    }

    /* 输出PWM */
    bsp_pwm_set_duty(motor->pwm, (uint32)motor->output);
}

/**
 * @brief 获取电机实际速度
 * @param motor 电机设备指针
 * @return 当前实际速度
 */
int32 device_motor_get_speed(DeviceMotor *motor)
{
    return motor->actual_speed;
}

/**
 * @brief 使能电机
 * @param motor 电机设备指针
 * @param enable 使能标志
 */
void device_motor_enable(DeviceMotor *motor, uint8 enable)
{
    motor->enabled = enable;
}

/**
 * @brief 设置最大输出限幅
 * @param motor 电机设备指针
 * @param max_output 最大输出值
 */
void device_motor_set_max_output(DeviceMotor *motor, uint32 max_output)
{
    motor->max_output = max_output;
}

/**
 * @brief 停止电机
 * @param motor 电机设备指针
 */
void device_motor_stop(DeviceMotor *motor)
{
    motor->output = 0;
    gpio_set_level(motor->dir_pin, MOTOR_DIR_REVERSE);
    bsp_pwm_set_duty(motor->pwm, 0);
}

/**
 * @brief 电机总初始化函数
 * @note 初始化 TIM2 编码器 + ATOM1_CH0 PWM + P22_3 方向控制
 */
void device_motor_all_init(void)
{
    /* 初始化全局电机设备 - 默认PID参数 */
    device_motor_init(&g_motor, 
                     &bsp_encoder_tim2, 
                     &bsp_pwm_motor, 
                     P22_3,
                     10.0f,   /* Kp */
                     0.0f,    /* Ki */
                     0.0f,    /* Kd */
                     100.0f); /* detach */
}