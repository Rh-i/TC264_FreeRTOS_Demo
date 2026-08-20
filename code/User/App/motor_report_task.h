/**
 * @file motor_report_task.h
 * @author Rh
 * @brief 电机数据周期上报任务 - vofa JustFloat 协议
 * @version 0.1
 * @date 2026-08-20
 *
 * @copyright Copyright (c) 2026
 */

#ifndef __MOTOR_REPORT_TASK_H__
#define __MOTOR_REPORT_TASK_H__

/**
 * @brief 电机数据上报任务函数
 * @param pvParameters 任务参数（未使用）
 * @note 周期性通过 UART3 以 vofa JustFloat 协议上报：
 *       目标速度、实际速度、PWM输出，供上位机看波形调 PID
 */
void motor_report_task(void *pvParameters);

#endif // __MOTOR_REPORT_TASK_H__
