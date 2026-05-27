/**
 * @file app_cfg.c
 * @author Rh (qq:750920400)
 * @brief 应用层init实现
 * @version 0.1
 * @date 2026-05-25
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "app_cfg.h"
#include "hardware_config.h"
#include "zf_common_headfile.h"

#pragma section all "cpu0_dsram"

/**
 * @brief 用户所有的初始化
 */
void user_init(void)
{
  bsp_io_init(); // 初始化io设备 （led key buzzer）

  bsp_uart_all_init();                                 // 初始化四个串口（默认8N1/115200）
  uart_protocol_init(&g_uart_protocol, &NUC_MCU_UART); // 初始化上下位机串口协议
  device_r9ds_all_init();                              // R9DS 遥控器接收机初始化（UART1为SBUS模式100000/9E2）
  r9ds_ctrl_init();                                    // 遥控器控制模块初始化（默认自动模式）

  bsp_pwm_all_init(); // 初始化舵机和电机的pwm

  device_servo_all_init();          // 初始化舵机
  device_servo_enable(&g_servo, 1); // 使能舵机

  bsp_encoder_all_init();           // 初始化编码器
  device_motor_all_init();          // 初始化电机
  device_motor_enable(&g_motor, 1); // 使能电机

  auto_ctrl_init();   // 自动控制初始化
  auto_ctrl_enable(); // 启动自动控制

  pit_ms_init(CCU61_CH1, 20); // 开启处理电机pid的20ms一次的中断初始化
  pit_ms_init(CCU61_CH0, 1);  // 开启处理key_scan的1ms一次的中断初始化
}

int fputc(int ch, FILE *stream)
{
  uart_write_byte(UART_0, (char)ch);
  return (ch);
}

#pragma section all restore
