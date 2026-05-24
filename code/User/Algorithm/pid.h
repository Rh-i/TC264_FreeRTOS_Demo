/**
 * @file pid.h
 * @brief 基础单环PID控制器
 * @note 最简易稳定的PID实现
 */

#ifndef PID_H_
#define PID_H_

#include "zf_common_headfile.h"

/*==============================================================================
 * PID结构体定义
 * @brief 位置式PID控制器
 *============================================================================*/
typedef struct
{
  float kp; /**< 比例系数 */
  float ki; /**< 积分系数 */
  float kd; /**< 微分系数 */

  float target;   /**< 目标值 */
  float feedback; /**< 反馈值 */
  float error;    /**< 误差 */

  float integral;     /**< 积分累计 */
  float integral_max; /**< 积分限幅 */

  float output;     /**< PID输出 */
  float output_max; /**< 输出限幅 */

  float last_error; /**< 上次误差 */
} PID;

/*==============================================================================
 * 函数声明
 *============================================================================*/

/**
 * @brief PID初始化
 * @param pid PID结构体指针
 * @param kp 比例系数
 * @param ki 积分系数
 * @param kd 微分系数
 * @param out_max 输出限幅
 * @param i_max 积分限幅
 */
void PID_Init(PID *pid, float kp, float ki, float kd, float out_max, float i_max);

/**
 * @brief PID计算（位置式）
 * @param pid PID结构体指针
 * @param target 目标值
 * @param feedback 反馈值
 * @return PID输出
 */
float PID_Calculate(PID *pid, float target, float feedback);

/**
 * @brief PID重置
 * @param pid PID结构体指针
 */
void PID_Reset(PID *pid);

#endif /* PID_H_ */