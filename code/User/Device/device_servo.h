#ifndef DEVICE_SERVO_H_
#define DEVICE_SERVO_H_

#include "bsp_pwm.h"
#include "zf_common_headfile.h"

/*==============================================================================
 * 舵机设备定义
 * @brief 使用 PWM 占空比控制舵机角度
 *============================================================================*/
typedef struct DeviceServo
{
  BspPwm *pwm;       /**< PWM 设备指针 */
  uint32  duty_min;  /**< 最小占空比 */
  uint32  duty_max;  /**< 最大占空比 */
  uint32  duty_mid;  /**< 中值占空比（对应 0°） */
  int32   angle;     /**< 当前角度值 */
  int32   angle_min; /**< 最小角度 */
  int32   angle_max; /**< 最大角度 */
  uint8   enabled;   /**< 使能标志 */
} DeviceServo;

/*==============================================================================
 * 公共函数声明
 *============================================================================*/

/**
 * @brief 初始化舵机设备
 * @param servo 舵机设备指针
 * @param pwm PWM 设备指针
 * @param duty_min 最小占空比
 * @param duty_max 最大占空比
 * @param duty_mid 中值占空比
 */
void device_servo_init(DeviceServo *servo, BspPwm *pwm, uint32 duty_min, uint32 duty_max, uint32 duty_mid);

/**
 * @brief 设置舵机角度
 * @param servo 舵机设备指针
 * @param angle 目标角度
 */
void device_servo_set_angle(DeviceServo *servo, int32 angle);

/**
 * @brief 设置舵机占空比（直接设置）
 * @param servo 舵机设备指针
 * @param duty 目标占空比
 */
void device_servo_set_duty(DeviceServo *servo, uint32 duty);

/**
 * @brief 获取当前舵机角度
 * @param servo 舵机设备指针
 * @return 当前角度
 */
int32 device_servo_get_angle(DeviceServo *servo);

/**
 * @brief 使能舵机
 * @param servo 舵机设备指针
 * @param enable 使能标志
 */
void device_servo_enable(DeviceServo *servo, uint8 enable);

/**
 * @brief 重置舵机到中位（0°）
 * @param servo 舵机设备指针
 */
void device_servo_reset(DeviceServo *servo);

/*==============================================================================
 * 全局舵机设备实例（外部可访问）
 *============================================================================*/
extern struct DeviceServo g_servo;

/**
 * @brief 舵机总初始化函数
 */
void device_servo_all_init(void);

#endif /* DEVICE_SERVO_H_ */
