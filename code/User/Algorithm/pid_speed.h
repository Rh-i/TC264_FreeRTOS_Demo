#ifndef __PID_SPEED_H__
#define __PID_SPEED_H__

#include "pid.h"

/*==============================================================================
 * 速度环PID结构体定义
 * @brief 包含PID控制器和速度换算参数
 *============================================================================*/
typedef struct
{
  PID pid; // 基础PID控制器

  int16 last_count;  // 上次编码器值
  int16 delta_count; // 编码器增量

  float speed_cm_s;   // 当前速度 cm/s
  float target_speed; // 目标速度 cm/s
} SpeedPID;

/*==============================================================================
 * 常量定义
 *============================================================================*/
#define ENCODER_RESOLUTION 2000   // 编码器分辨率 脉冲/圈
#define WHEEL_DIAMETER_MM 62      // 轮子直径 mm
#define WHEEL_PERIMETER_CM 19.48f // 轮子周长 cm (π×62/10)
#define CONTROL_PERIOD_MS 20      // 控制周期 20ms

/*==============================================================================
 * 函数声明
 *============================================================================*/

/**
 * @brief 速度环PID初始化
 * @param sp SpeedPID结构体指针
 * @param kp 比例系数
 * @param ki 积分系数
 * @param kd 微分系数
 * @param out_max 输出限幅(PWM占空比)
 */
void SpeedPID_Init(SpeedPID *sp, float kp, float ki, float kd, float out_max);

/**
 * @brief 速度环PID计算
 * @param sp SpeedPID结构体指针
 * @param target_speed 目标速度 cm/s
 * @param delta_count 编码器增量（外部传入）
 * @return PWM输出值
 * @note 每20ms调用一次
 */
float SpeedPID_Calculate(SpeedPID *sp, float target_speed, int16 delta_count);

/**
 * @brief 获取当前速度
 * @param sp SpeedPID结构体指针
 * @return 当前速度 cm/s
 */
float SpeedPID_GetSpeed(SpeedPID *sp);

/**
 * @brief 速度环PID重置
 * @param sp SpeedPID结构体指针
 */
void SpeedPID_Reset(SpeedPID *sp);

#endif // __PID_SPEED_H__