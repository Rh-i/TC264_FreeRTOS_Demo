/**
 * @file auto_ctrl.c
 * @brief 自动控制模块实现 - 根据串口协议数据控制电机
 * @note 从 uart_protocol 获取命令，驱动 device_motor 执行运动
 * @date 2026-05-24
 */

#include "auto_ctrl.h"

#pragma section all "cpu0_dsram"

/*==============================================================================
 * 全局实例
 *============================================================================*/
AutoCtrl g_auto_ctrl;

/*==============================================================================
 * 函数实现
 *============================================================================*/

/**
 * @brief 初始化自动控制模块
 */
void auto_ctrl_init(void)
{
  g_auto_ctrl.state          = AUTO_CTRL_STATE_DISABLED;
  g_auto_ctrl.prev_busy      = 0;
  g_auto_ctrl.is_initialized = 1;
}

/**
 * @brief 开启自动控制
 */
void auto_ctrl_enable(void)
{
  if (!g_auto_ctrl.is_initialized)
  {
    auto_ctrl_init();
  }
  g_auto_ctrl.state = AUTO_CTRL_STATE_ENABLED;
}

/**
 * @brief 关闭自动控制
 */
void auto_ctrl_disable(void)
{
  g_auto_ctrl.state = AUTO_CTRL_STATE_DISABLED;

  /* 停止电机 */
  device_motor_stop(&g_motor);

  /* 设置协议层为 IDLE 模式 */
  SlaveStatus *status = uart_protocol_get_status(&g_uart_protocol);
  if (status != NULL)
  {
    status->mode         = PROTOCOL_MODE_IDLE;
    status->target_speed = 0;
    status->target_time  = 0;
  }
}

/**
 * @brief 自动控制更新，解包成功后触发一次
 */
void auto_ctrl_update(void)
{
  SlaveStatus *status;

  /* 未启用则不处理 */
  if (g_auto_ctrl.state != AUTO_CTRL_STATE_ENABLED)
  {
    return;
  }

  /* 获取协议层状态 */
  status = uart_protocol_get_status(&g_uart_protocol);
  if (status == NULL)
  {
    return;
  }

  /* 根据模式控制电机 */
  switch (status->mode)
  {
    case PROTOCOL_MODE_IDLE:
      /* 空闲模式，停止电机 */
      device_motor_stop(&g_motor);
      break;

    case PROTOCOL_MODE_SPEED:
      /* 速度模式，设置目标速度 */
      device_motor_set_speed(&g_motor, status->target_speed);
      break;

    case PROTOCOL_MODE_SPEED_TIME:
      /* 速度-时间模式，设置速度和时间 */
      device_motor_set_speed_time(&g_motor, status->target_speed, (uint32)status->target_time);
      break;

    default:
      break;
  }

  /* 更新 prev_busy 状态 */
  g_auto_ctrl.prev_busy = (uint8)(device_motor_is_completed(&g_motor) == 0);
}

/**
 * @brief 查询是否已启用自动控制
 * @return 1 已启用，0 未启用
 */
uint8 auto_ctrl_is_enabled(void)
{
  return (uint8)(g_auto_ctrl.state == AUTO_CTRL_STATE_ENABLED);
}

#pragma section all restore
