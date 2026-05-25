/**
 * @file pid_speed.c
 * @author Rh (qq:750920400)
 * @brief 基于pid计算的，速度环封装
 * @version 0.1
 * @date 2026-05-25
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "pid_speed.h"

#pragma section all "cpu0_dsram"

/*==============================================================================
 * 函数实现
 *============================================================================*/

/**
 * @brief 速度环PID初始化
 */
void SpeedPID_Init(SpeedPID *sp, float kp, float ki, float kd, float out_max)
{
  // 初始化基础PID
  // 积分限幅设为输出限幅的20%
  PID_Init(&sp->pid, kp, ki, kd, out_max, out_max * 0.35f);

  sp->last_count   = 0;
  sp->delta_count  = 0;
  sp->speed_cm_s   = 0.0f;
  sp->target_speed = 0.0f;
}

/**
 * @brief 速度环PID计算
 * @note 每20ms调用一次
 *
 * 速度计算公式:
 * speed_cm_s = delta_count × WHEEL_PERIMETER_CM / ENCODER_RESOLUTION / (CONTROL_PERIOD_MS/1000)
 *            = delta_count × 19.48 / 2000 / 0.02
 *            = delta_count × 0.487 cm/s
 *
 * @param sp SpeedPID结构体指针
 * @param target_speed 目标速度 cm/s
 * @param delta_count 编码器增量（外部传入，20ms周期内的脉冲数）
 */
float SpeedPID_Calculate(SpeedPID *sp, float target_speed, int16 delta_count)
{
  // 直接使用外部传入的增量
  sp->delta_count = delta_count;

  // 计算当前速度 cm/s
  // speed = delta × 周长 / 分辨率 / 时间
  sp->speed_cm_s = (float)sp->delta_count * WHEEL_PERIMETER_CM / ENCODER_RESOLUTION / (CONTROL_PERIOD_MS / 1000.0f);

  // 保存目标速度
  sp->target_speed = target_speed;

  // 调用基础PID计算
  // 输出为PWM占空比(0-10000)
  return PID_Calculate(&sp->pid, target_speed, sp->speed_cm_s);
}

/**
 * @brief 获取当前速度
 */
float SpeedPID_GetSpeed(SpeedPID *sp)
{
  return sp->speed_cm_s;
}

/**
 * @brief 速度环PID重置
 */
void SpeedPID_Reset(SpeedPID *sp)
{
  PID_Reset(&sp->pid);
  sp->last_count   = 0;
  sp->delta_count  = 0;
  sp->speed_cm_s   = 0.0f;
  sp->target_speed = 0.0f;
}

#pragma section all restore
