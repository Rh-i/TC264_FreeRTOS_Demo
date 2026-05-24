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

  bsp_pwm_all_init();
  device_servo_all_init();

  bsp_encoder_all_init();
  device_motor_all_init();
  device_motor_enable(&g_motor, 1);

  auto_ctrl_init();
  auto_ctrl_enable();
}

int fputc(int ch, FILE *stream)
{
  uart_write_byte(UART_0, (char)ch);
  return (ch);
}


#pragma section all restore
