/**
 * @file app_cfg.c
 * @brief 应用配置实现
 */

#include "app_cfg.h"
#include "zf_common_headfile.h"

#pragma section all "cpu0_dsram"

/**
 * @brief 用户所有的初始化
 */
void user_init(void)
{
  /* IO设备初始化 */
  bsp_io_init();
  bsp_uart_all_init();
  uart_protocol_init(&g_uart_protocol);
}


#pragma section all restore
