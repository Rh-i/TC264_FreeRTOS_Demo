/**
 * @file pid.c
 * @author Rh (qq:750920400)
 * @brief 简单单环pid实现
 * @version 0.1
 * @date 2026-05-25
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "pid.h"

#pragma section all "cpu0_dsram"

/*==============================================================================
 * 函数实现
 *============================================================================*/

/**
 * @brief PID初始化
 */
void PID_Init(PID *pid, float kp, float ki, float kd, float out_max, float i_max)
{
  pid->kp = kp;
  pid->ki = ki;
  pid->kd = kd;

  pid->output_max   = out_max;
  pid->integral_max = i_max;

  pid->target     = 0.0f;
  pid->feedback   = 0.0f;
  pid->error      = 0.0f;
  pid->integral   = 0.0f;
  pid->output     = 0.0f;
  pid->last_error = 0.0f;
}

/**
 * @brief PID计算（位置式）
 * @note 最简易稳定的实现
 */
float PID_Calculate(PID *pid, float target, float feedback)
{
  pid->target   = target;
  pid->feedback = feedback;

  // 计算误差
  pid->error = target - feedback;

  // 比例项
  float p_out = pid->kp * pid->error;

  // 积分项（带积分限幅）
  pid->integral += pid->ki * pid->error;
  if (pid->integral > pid->integral_max)
    pid->integral = pid->integral_max;
  if (pid->integral < -pid->integral_max)
    pid->integral = -pid->integral_max;

  // 微分项
  float d_out     = pid->kd * (pid->error - pid->last_error);
  pid->last_error = pid->error;

  // 计算输出
  pid->output = p_out + pid->integral + d_out;

  // 输出限幅
  if (pid->output > pid->output_max)
    pid->output = pid->output_max;
  if (pid->output < -pid->output_max)
    pid->output = -pid->output_max;

  return pid->output;
}

/**
 * @brief PID重置
 */
void PID_Reset(PID *pid)
{
  pid->error      = 0.0f;
  pid->integral   = 0.0f;
  pid->output     = 0.0f;
  pid->last_error = 0.0f;
}

#pragma section all restore