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
  // 默认积分分离阈值：输出限幅的25%（|error| < 该值才积分）
  pid->integral_sep_threshold = out_max * 0.25f;

  pid->target     = 0.0f;
  pid->feedback   = 0.0f;
  pid->error      = 0.0f;
  pid->integral   = 0.0f;
  pid->output     = 0.0f;
  pid->last_error = 0.0f;
}

/**
 * @brief PID计算（位置式）
 * @note 带积分分离与抗积分饱和
 *       - 积分分离：|error| < integral_sep_threshold 时才累加积分，偏差大时只用 PD，防止超调
 *       - 抗积分饱和：积分达到限幅后，若误差方向与当前饱和方向一致则不再累加，保留限幅值
 */
float PID_Calculate(PID *pid, float target, float feedback)
{
  pid->target   = target;
  pid->feedback = feedback;

  // 计算误差
  pid->error = target - feedback;

  // 比例项
  float p_out = pid->kp * pid->error;

  // 微分项（对误差差分）
  float d_out     = pid->kd * (pid->error - pid->last_error);
  pid->last_error = pid->error;

  // 积分项（积分分离 + 抗积分饱和）
  float err_abs = (pid->error >= 0.0f) ? pid->error : -pid->error;

  // 积分分离：误差小于阈值才允许积分（threshold<=0 时视为始终积分）
  if (pid->integral_sep_threshold <= 0.0f || err_abs < pid->integral_sep_threshold)
  {
    // 抗积分饱和：到达上限且误差方向一致（继续增大）→ 停止累加
    if (!(pid->integral >= pid->integral_max && pid->error > 0.0f) &&
        !(pid->integral <= -pid->integral_max && pid->error < 0.0f))
    {
      pid->integral += pid->ki * pid->error;
    }

    // 积分限幅
    if (pid->integral > pid->integral_max)
      pid->integral = pid->integral_max;
    if (pid->integral < -pid->integral_max)
      pid->integral = -pid->integral_max;
  }

  // 计算总输出
  pid->output = p_out + pid->integral + d_out;

  // 输出限幅
  if (pid->output > pid->output_max)
    pid->output = pid->output_max;
  if (pid->output < -pid->output_max)
    pid->output = -pid->output_max;

  return pid->output;
}

/**
 * @brief 设置积分分离阈值
 */
void PID_SetIntegralSeparate(PID *pid, float threshold)
{
  pid->integral_sep_threshold = threshold;
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