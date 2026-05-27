/**
 * @file isr.c
 * @author Rh (qq:750920400)
 * @brief 所有的中断回调函数都在这（除了freertos计数时钟的中断，其优先级为1）
 * @version 0.1
 * @date 2026-05-25
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "app_cfg.h"
#include "isr_config.h"
#include "zf_common_headfile.h"
#include "device_r9ds.h"

/**
 * @brief 电机舵机控制中断 20ms一次
 * @note  根据 r9ds_ctrl 模式分发控制权：
 *        NONE   → 遥控器接管，立即刹停 + 舵机回中
 *        MANUAL → 遥控器接管，CH1 油门 + CH2 转向
 *        AUTO   → 串口协议接管，舵机跟随协议目标角度
 */
IFX_INTERRUPT(cc61_pit_ch1_isr, 0, CCU6_1_CH1_ISR_PRIORITY)
{
  // 开启中断嵌套
  interrupt_global_enable(0);

  R9DS_CtrlMode mode = r9ds_ctrl_get_mode();

  if (mode == R9DS_CTRL_MODE_NONE || mode == R9DS_CTRL_MODE_MANUAL)
  {
    // 遥控器控制 — 内部处理电机速度/停止 + 舵机角度
    r9ds_ctrl_update();
  }
  else // R9DS_CTRL_MODE_AUTO
  {
    // 自动模式 — 舵机跟随串口协议目标角度
    device_servo_set_angle(&g_servo, g_uart_protocol.status.target_angle);
  }

  // 始终运行电机 PID（STOP 模式下 PWM=0，SPEED 模式下 PID 闭环）
  device_motor_update(&g_motor);

  pit_clear_flag(CCU61_CH1);
}


/**
 * @brief key使用到的中断回调 1ms触发一次
 * @note  同时负责 R9DS 遥控器 SBUS 帧解析（与 key 扫描共用 1ms 中断）
 *
 */
IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)
{
  // 开启中断嵌套
  interrupt_global_enable(0);

  key_scan(&key_a_dev); // 扫描按键
  key_scan(&key_b_dev); // 扫描按键
  key_scan(&key_c_dev); // 扫描按键

  device_r9ds_update(&g_r9ds); // R9DS SBUS 帧解析

  pit_clear_flag(CCU61_CH0);
}


/**
 * @brief 串口0接收触发中断
 *
 */
IFX_INTERRUPT(uart0_rx_isr, 0, UART0_RX_INT_PRIO)
{
  interrupt_global_enable(0);
  bsp_uart_rx_isr_handler(&bsp_uart0); // 中断处理函数
}


/**
 * @brief 串口1接收触发中断
 *
 */
IFX_INTERRUPT(uart1_rx_isr, 0, UART1_RX_INT_PRIO)
{
  interrupt_global_enable(0);
  bsp_uart_rx_isr_handler(&bsp_uart1); // 中断处理函数
}


/**
 * @brief 串口2接收触发中断
 *
 */
IFX_INTERRUPT(uart2_rx_isr, 0, UART2_RX_INT_PRIO)
{
  interrupt_global_enable(0);
  bsp_uart_rx_isr_handler(&bsp_uart2); // 中断处理函数
}


/**
 * @brief 串口3接收触发中断
 *
 */
IFX_INTERRUPT(uart3_rx_isr, 0, UART3_RX_INT_PRIO)
{
  interrupt_global_enable(0);
  bsp_uart_rx_isr_handler(&bsp_uart3); // 中断处理函数
}


// 串口0123通讯错误中断
IFX_INTERRUPT(uart0_er_isr, 0, UART0_ER_INT_PRIO)
{
  interrupt_global_enable(0); // 开启中断嵌套
  IfxAsclin_Asc_isrError(&uart0_handle);
}
IFX_INTERRUPT(uart1_er_isr, 0, UART1_ER_INT_PRIO)
{
  interrupt_global_enable(0); // 开启中断嵌套
  IfxAsclin_Asc_isrError(&uart1_handle);
}
IFX_INTERRUPT(uart2_er_isr, 0, UART2_ER_INT_PRIO)
{
  interrupt_global_enable(0); // 开启中断嵌套
  IfxAsclin_Asc_isrError(&uart2_handle);
}
IFX_INTERRUPT(uart3_er_isr, 0, UART3_ER_INT_PRIO)
{
  interrupt_global_enable(0); // 开启中断嵌套
  IfxAsclin_Asc_isrError(&uart3_handle);
}


/***************************** 下面为未使用中断 *****************************/

//// **************************** PIT中断函数 ****************************
//// 类似于定时器触发完成中断
//
// IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)
//{
//  interrupt_global_enable(0); // 开启中断嵌套
//  pit_clear_flag(CCU60_CH0);
//}
//
// IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY)
//{
//  interrupt_global_enable(0); // 开启中断嵌套
//  pit_clear_flag(CCU60_CH1);
//}
//
//// **************************** PIT中断函数 ****************************
//// **************************** 外部中断函数 ****************************
//
// IFX_INTERRUPT(exti_ch0_ch4_isr, 0, EXTI_CH0_CH4_INT_PRIO)
//{
//  interrupt_global_enable(0);            // 开启中断嵌套
//  if (exti_flag_get(ERU_CH0_REQ0_P15_4)) // 通道0中断
//  {
//    exti_flag_clear(ERU_CH0_REQ0_P15_4);
//  }
//
//  if (exti_flag_get(ERU_CH4_REQ13_P15_5)) // 通道4中断
//  {
//    exti_flag_clear(ERU_CH4_REQ13_P15_5);
//  }
//}
//
// IFX_INTERRUPT(exti_ch1_ch5_isr, 0, EXTI_CH1_CH5_INT_PRIO)
//{
//  interrupt_global_enable(0); // 开启中断嵌套
//
//  if (exti_flag_get(ERU_CH1_REQ10_P14_3)) // 通道1中断
//  {
//    exti_flag_clear(ERU_CH1_REQ10_P14_3);
//  }
//
//  if (exti_flag_get(ERU_CH5_REQ1_P15_8)) // 通道5中断
//  {
//    exti_flag_clear(ERU_CH5_REQ1_P15_8);
//  }
//}
//
// IFX_INTERRUPT(exti_ch2_ch6_isr, 0, EXTI_CH2_CH6_INT_PRIO)
//{
//  interrupt_global_enable(0);            // 开启中断嵌套
//  if (exti_flag_get(ERU_CH2_REQ7_P00_4)) // 通道2中断
//  {
//    exti_flag_clear(ERU_CH2_REQ7_P00_4);
//  }
//  if (exti_flag_get(ERU_CH6_REQ9_P20_0)) // 通道6中断
//  {
//    exti_flag_clear(ERU_CH6_REQ9_P20_0);
//  }
//}
//
// IFX_INTERRUPT(exti_ch3_ch7_isr, 0, EXTI_CH3_CH7_INT_PRIO)
//{
//  interrupt_global_enable(0);            // 开启中断嵌套
//  if (exti_flag_get(ERU_CH3_REQ6_P02_0)) // 通道3中断
//  {
//    exti_flag_clear(ERU_CH3_REQ6_P02_0);
//    // camera_vsync_handler(); // 摄像头触发采集统一回调函数 可以抄中断，dma啥的
//  }
//  if (exti_flag_get(ERU_CH7_REQ16_P15_1)) // 通道7中断
//  {
//    exti_flag_clear(ERU_CH7_REQ16_P15_1);
//  }
//}
//// **************************** 外部中断函数 ****************************
//// **************************** DMA中断函数 ****************************
//
//// 此处定义的dma中断不多，不知如何开启
//
// IFX_INTERRUPT(dma_ch5_isr, 0, DMA_INT_PRIO)
//{
// interrupt_global_enable(0);                       // 开启中断嵌套
//   camera_dma_handler();                           // 摄像头采集完成统一回调函数
//}
//
//// **************************** DMA中断函数 ****************************
