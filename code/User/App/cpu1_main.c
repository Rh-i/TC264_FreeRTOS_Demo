/**
 * @file cpu1_main.c
 * @author Rh (qq:750920400)
 * @brief cpu1的主函数
 * @version 0.1
 * @date 2026-05-25
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "zf_common_headfile.h"

#pragma section all "cpu1_dsram"

void core1_main(void)
{
  disable_Watchdog();         // 关闭看门狗
  interrupt_global_enable(0); // 打开全局中断

  cpu_wait_event_ready(); // 等待所有核心初始化完毕
  while (1)
  {
  }
}

#pragma section all restore
