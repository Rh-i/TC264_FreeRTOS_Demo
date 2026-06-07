/**
 * @file test.h
 * @author Rh (qq:750920400)
 * @brief 电路板测试模块 — 对外接口
 * @version 0.1
 * @date 2026-06-07
 *
 * @note 由 TEST_BOARD 宏控制编译：
 *       开启 → 测试模式激活，按键触发电机/编码器/舵机测试
 *       关闭 → 所有接口退化为空操作，零开销
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef __TEST_H__
#define __TEST_H__

#include "hardware_config.h"
#include "zf_common_headfile.h"

/*==============================================================================
 * 测试命令枚举（始终可用 — TEST_BOARD 关闭时值作为空宏参数被丢弃）
 *============================================================================*/

/**
 * @brief 按键对应的测试类型
 */
typedef enum
{
  TEST_CMD_MOTOR   = 1, // Key A → 电机 PWM 斜坡测试
  TEST_CMD_ENCODER = 2, // Key B → 编码器读数测试
  TEST_CMD_SERVO   = 3, // Key C → 舵机扫描测试
} TestCmd;

#ifdef TEST_BOARD

#include "FreeRTOS.h"
#include "queue.h"

/*==============================================================================
 * 全局标志（供 isr.c 引用）
 *============================================================================*/

/**
 * @brief 测试模式激活标志
 * @note 当为 1 时，20ms ISR 跳过自动控制和电机 PID，由 test_task 全权接管
 */
extern volatile uint8 g_test_mode_active;

/*==============================================================================
 * 公共接口
 *============================================================================*/

/**
 * @brief 测试模块初始化
 * @note 必须在调度器启动前调用（core0_main 中 user_init 之后、start_freertos 之前）
 *       创建测试命令队列，挂起上位机协议任务
 * @param uart3_task_handle 上位机串口协议任务句柄（用于挂起）
 */
void test_init(TaskHandle_t uart3_task_handle);

/**
 * @brief 按键通知测试命令
 * @note 由 key1/2/3 任务在 LED 翻转后调用
 *       空闲时 → 启动对应测试
 *       测试中 → 触发紧急停止
 * @param cmd 测试命令类型
 */
void test_key_notify(TestCmd cmd);

#else // !TEST_BOARD — 退化为空操作

#define test_init(handle)      ((void)0)
#define test_key_notify(cmd)   ((void)0)

#endif // TEST_BOARD

/*==============================================================================
 * test_task — 两种模式均需声明（定义在 test.c 中）
 *============================================================================*/

/**
 * @brief 测试任务入口
 * @note 由 xTaskCreate 创建，优先级 5
 *       TEST_BOARD 开启 → 打印测试菜单，等待按键命令
 *       TEST_BOARD 关闭 → 空循环
 */
void test_task(void *pvParameters);

#endif // __TEST_H__
