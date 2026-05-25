/**
 * @file device_servo.c
 * @author Rh (qq:750920400)
 * @brief 舵机设备驱动实现 - PWM 输出角度控制
 * @version 0.1
 * @date 2026-05-25
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "device_servo.h"
#include "bsp_pwm.h"

#pragma section all "cpu0_dsram"

/*==============================================================================
 * 全局舵机设备实例
 *============================================================================*/

/* 全局舵机设备 - ATOM0_CH3_P21_5 PWM */
struct DeviceServo g_servo;

#pragma section all restore

/*==============================================================================
 * 公共函数实现
 *============================================================================*/

/**
 * @brief 初始化舵机设备
 * @param servo 舵机设备指针
 * @param pwm PWM 设备指针
 * @param duty_min 最小占空比
 * @param duty_max 最大占空比
 * @param duty_mid 中值占空比
 */
void device_servo_init(DeviceServo *servo, BspPwm *pwm, uint32 duty_min, uint32 duty_max, uint32 duty_mid)
{
  servo->pwm       = pwm;
  servo->duty_min  = duty_min;
  servo->duty_max  = duty_max;
  servo->duty_mid  = duty_mid;
  servo->angle     = 0;
  servo->angle_min = -35; /* 角度范围 -35° ~ +35°，以机械中值为基准 */
  servo->angle_max = 35;
  servo->enabled   = 0;

  /* 设置初始占空比为中值 */
  bsp_pwm_set_duty(pwm, duty_mid);

  device_servo_enable(servo, 1);
  device_servo_reset(servo);
}

/**
 * @brief 设置舵机角度
 * @param servo 舵机设备指针
 * @param angle 目标角度
 *
 * @todo ai给的转换有问题，现求又麻烦，所以写死了一个线性坐标转换，
 *
 */
void device_servo_set_angle(DeviceServo *servo, int32 angle)
{
  uint32 duty;

  if (servo->enabled == 0)
  {
    return;
  }

  /* 角度限幅 */
  if (angle > servo->angle_max)
  {
    angle = servo->angle_max;
  }
  else if (angle < servo->angle_min)
  {
    angle = servo->angle_min;
  }

  servo->angle = angle;

  /* 计算占空比 - 线性映射角度到占空比 */
  /* 公式: duty = 4750 + (angle×3079+42) / 84 */
  /* -84° → 1670, 0° → 4750, +84° → 7828 */

  duty = 4750 + (angle * 3079 + 42) / 84;

  /* 占空比限幅 */
  if (duty > servo->duty_max)
  {
    duty = servo->duty_max;
  }
  else if (duty < servo->duty_min)
  {
    duty = servo->duty_min;
  }

  bsp_pwm_set_duty(servo->pwm, duty);
}

/**
 * @brief 设置舵机占空比
 * @param servo 舵机设备指针
 * @param duty 目标占空比
 */
void device_servo_set_duty(DeviceServo *servo, uint32 duty)
{
  /* 占空比限幅 */
  if (duty > servo->duty_max)
  {
    duty = servo->duty_max;
  }
  else if (duty < servo->duty_min)
  {
    duty = servo->duty_min;
  }

  bsp_pwm_set_duty(servo->pwm, duty);
}

/**
 * @brief 获取当前舵机角度
 * @param servo 舵机设备指针
 * @return 当前角度
 */
int32 device_servo_get_angle(DeviceServo *servo)
{
  return servo->angle;
}

/**
 * @brief 使能舵机
 * @param servo 舵机设备指针
 * @param enable 使能标志
 */
void device_servo_enable(DeviceServo *servo, uint8 enable)
{
  servo->enabled = enable;
}

/**
 * @brief 重置舵机到中位
 * @param servo 舵机设备指针
 */
void device_servo_reset(DeviceServo *servo)
{
  servo->angle = 0;
  bsp_pwm_set_duty(servo->pwm, servo->duty_mid);
}

/**
 * @brief 舵机总初始化函数
 * @note 初始化 ATOM0_CH3_P21_5 PWM 舵机, 使用330Hz高精度控制
 *       角度范围: 机械中值4750，左右各35°，总行程70°
 */
void device_servo_all_init(void)
{
  /* 初始化全局舵机设备 - CS-3120舵机参数 */
  /* 330Hz周期 = 1000000/330 ≈ 3030μs */
  /* 机械中值: 4750 */
  /* 角度范围 ±35° */
  /* -35° → duty = 4750 + (-35×3079+42)/84 = 3468 */
  /* +35° → duty = 4750 + (35×3079+42)/84 = 6034 */
  device_servo_init(&g_servo,
                    &bsp_pwm_servo1,
                    3468,  /* 左侧35°占空比 */
                    6034,  /* 右侧35°占空比 */
                    4750); /* 机械中值占空比 */
}
