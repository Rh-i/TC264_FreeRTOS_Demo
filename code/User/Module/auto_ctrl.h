#ifndef AUTO_CTRL_H_
#define AUTO_CTRL_H_

#include "FreeRTOS.h"
#include "device_motor.h"
#include "semphr.h"
#include "uart_protocol.h"

/*==============================================================================
 * 枚举定义
 *============================================================================*/

/**
 * @brief 自动控制状态
 */
typedef enum
{
  AUTO_CTRL_STATE_DISABLED = 0, /**< 自动控制未启用 */
  AUTO_CTRL_STATE_ENABLED  = 1, /**< 自动控制已启用 */
} AutoCtrlState;

/*==============================================================================
 * 数据结构
 *============================================================================*/

/**
 * @brief 自动控制结构体
 */
typedef struct
{
  AutoCtrlState state;          /**< 当前状态 */
  uint8         prev_busy;      /**< 上次运动是否进行中（用于检测完成） */
  uint8         is_initialized; /**< 初始化标志 */
} AutoCtrl;

/*==============================================================================
 * 函数接口
 *============================================================================*/

/**
 * @brief 初始化自动控制模块
 */
void auto_ctrl_init(void);

/**
 * @brief 开启自动控制
 * @note 使能后，auto_ctrl 任务会根据协议层数据控制电机
 */
void auto_ctrl_enable(void);

/**
 * @brief 关闭自动控制
 * @note 禁用后，停止电机运动，设置协议层为 IDLE 模式
 */
void auto_ctrl_disable(void);

/**
 * @brief 自动控制更新（信号量触发）
 * @note 由 auto_ctrl_task 调用，读取协议层数据并控制电机
 */
void auto_ctrl_update(void);

/**
 * @brief 查询是否已启用自动控制
 * @return 1 已启用，0 未启用
 */
uint8 auto_ctrl_is_enabled(void);

/*==============================================================================
 * 全局实例
 *============================================================================*/
extern AutoCtrl g_auto_ctrl;

#endif /* AUTO_CTRL_H_ */
