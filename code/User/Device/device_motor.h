#ifndef DEVICE_MOTOR_H_
#define DEVICE_MOTOR_H_

#include "bsp_encoder.h"
#include "bsp_pwm.h"
#include "pid_speed.h"
#include "zf_common_headfile.h"

/*==============================================================================
 * 电机方向定义
 *============================================================================*/
typedef enum
{
  MOTOR_DIR_REVERSE = 0, /**< 后退 */
  MOTOR_DIR_FORWARD = 1, /**< 前进 */
} MotorDir_enum;

/*==============================================================================
 * 运动模式枚举
 *============================================================================*/
typedef enum
{
  MOTOR_MODE_STOP       = 0, /**< 停止模式 */
  MOTOR_MODE_SPEED      = 1, /**< 纯速度模式 */
  MOTOR_MODE_SPEED_TIME = 2, /**< 速度+时间模式 */
} MotorMode_enum;

/*==============================================================================
 * 运动状态结构体
 *============================================================================*/
typedef struct
{
  MotorMode_enum mode;         /**< 当前运动模式 */
  int32          target_speed; /**< 目标速度 cm/s */
  uint32         target_time;  /**< 目标时间 ms */
  uint32         start_time;   /**< 运动开始时间 ms */
  uint8          is_active;    /**< 运动是否进行中 */
  uint8          is_completed; /**< 运动是否完成 */
} MotorMotionState;

/*==============================================================================
 * 电机设备定义
 * @brief 包含编码器、PWM输出、方向控制、速度环PID控制器的完整电机设备
 *============================================================================*/
typedef struct DeviceMotor
{
  BspEncoder   *encoder; /**< 编码器设备指针 */
  BspPwm       *pwm;     /**< PWM设备指针 */
  gpio_pin_enum dir_pin; /**< 方向控制引脚 */

  SpeedPID speed_pid; /**< 速度环PID */

  MotorMotionState motion; /**< 运动状态 */

  int32  actual_speed; /**< 实际速度 cm/s */
  int32  output;       /**< PWM输出值 */
  uint32 max_output;   /**< 最大输出限幅 */
  uint8  enabled;      /**< 使能标志 */
} DeviceMotor;

/*==============================================================================
 * 公共函数声明
 *============================================================================*/

/**
 * @brief 初始化电机设备
 * @param motor 电机设备指针
 * @param encoder 编码器设备指针
 * @param pwm PWM设备指针
 * @param dir_pin 方向控制引脚
 * @param speed_kp 速度环比例系数
 * @param speed_ki 速度环积分系数
 * @param speed_kd 速度环微分系数
 * @param out_max 输出限幅(PWM占空比)
 */
void device_motor_init(DeviceMotor *motor, BspEncoder *encoder, BspPwm *pwm, gpio_pin_enum dir_pin, float speed_kp, float speed_ki, float speed_kd, uint32 out_max);

/**
 * @brief 设置电机目标速度（纯速度模式）
 * @param motor 电机设备指针
 * @param speed 目标速度 cm/s
 */
void device_motor_set_speed(DeviceMotor *motor, int32 speed);

/**
 * @brief 设置速度+时间模式
 * @param motor 电机设备指针
 * @param speed 目标速度 cm/s
 * @param time_ms 运行时间 ms
 */
void device_motor_set_speed_time(DeviceMotor *motor, int32 speed, uint32 time_ms);

/**
 * @brief 停止电机
 * @param motor 电机设备指针
 */
void device_motor_stop(DeviceMotor *motor);

/**
 * @brief 更新电机状态
 * @param motor 电机设备指针
 * @return 1表示运动完成，0表示运动进行中
 * @note 每20ms调用一次
 */
uint8 device_motor_update(DeviceMotor *motor);

/**
 * @brief 获取电机实际速度
 * @param motor 电机设备指针
 * @return 当前实际速度 cm/s
 */
int32_t device_motor_get_speed(DeviceMotor *motor);

/**
 * @brief 获取运动是否完成
 * @param motor 电机设备指针
 * @return 1完成，0进行中
 */
uint8 device_motor_is_completed(DeviceMotor *motor);

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

/*==============================================================================
 * 全局电机设备实例（外部可访问）
 *============================================================================*/
extern struct DeviceMotor g_motor;

/**
 * @brief 电机总初始化函数
 * @note 初始化TIM2编码器+ATOM1_CH0 PWM+P22_3方向控制
 */
void device_motor_all_init(void);

#endif /* DEVICE_MOTOR_H_ */
