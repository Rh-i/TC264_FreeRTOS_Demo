/*
 * device_motor.h
 *
 *  Created on: 2024年6月6日
 *      Author: w1445
 */

#ifndef DEVICE_MOTOR_H_
#define DEVICE_MOTOR_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "zf_common_headfile.h"
#include "bsp_encoder.h"
#include "bsp_pwm.h"
#include "pid.h"

/*==============================================================================
 * 电机方向定义
 *============================================================================*/
typedef enum
{
    MOTOR_DIR_REVERSE = 0,  /**< 后退 */
    MOTOR_DIR_FORWARD = 1,   /**< 前进 */
} MotorDir_enum;

/*==============================================================================
 * 电机设备定义
 * @brief 包含编码器、PWM 输出、方向控制、速度PID的完整电机设备
 *============================================================================*/
typedef struct DeviceMotor
{
    BspEncoder        *encoder;          /**< 编码器设备指针 */
    BspPwm            *pwm;               /**< PWM 设备指针 */
    gpio_pin_enum      dir_pin;           /**< 方向控制引脚 */

    BasePID_Object     speed_pid;         /**< 速度PID */
    int32              target_speed;      /**< 目标速度 */
    int32              actual_speed;      /**< 实际速度（编码器反馈） */
    int32              output;            /**< PWM 输出值 */
    uint32             max_output;       /**< 最大输出限幅 */
    MotorDir_enum      direction;        /**< 当前方向 */
    uint8              enabled;           /**< 使能标志 */
} DeviceMotor;

/*==============================================================================
 * 公共函数声明
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
void device_motor_init(DeviceMotor *motor, BspEncoder *encoder, BspPwm *pwm, gpio_pin_enum dir_pin, float kp, float ki, float kd, float detach);

/**
 * @brief 设置电机目标速度
 * @param motor 电机设备指针
 * @param speed 目标速度值
 */
void device_motor_set_speed(DeviceMotor *motor, int32 speed);

/**
 * @brief 更新电机状态（读取编码器并输出PWM）
 * @param motor 电机设备指针
 */
void device_motor_update(DeviceMotor *motor);

/**
 * @brief 获取电机实际速度
 * @param motor 电机设备指针
 * @return 当前实际速度
 */
int32 device_motor_get_speed(DeviceMotor *motor);

/**
 * @brief 使能电机
 * @param motor 电机设备指针
 * @param enable 使能标志
 */
void device_motor_enable(DeviceMotor *motor, uint8 enable);

/**
 * @brief 设置最大输出限幅
 * @param motor 电机设备指针
 * @param max_output 最大输出值
 */
void device_motor_set_max_output(DeviceMotor *motor, uint32 max_output);

/**
 * @brief 停止电机（输出0）
 * @param motor 电机设备指针
 */
void device_motor_stop(DeviceMotor *motor);

/*==============================================================================
 * 全局电机设备实例（外部可访问）
 *============================================================================*/
extern struct DeviceMotor g_motor;

/**
 * @brief 电机总初始化函数
 */
void device_motor_all_init(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_MOTOR_H_ */