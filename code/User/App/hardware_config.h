/**
 * @file hardware_config.h
 * @author Rh (qq:750920400)
 * @brief 调试config内容
 * @version 0.1
 * @date 2026-05-25
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef __HARDWARE_CONFIG_H__
#define __HARDWARE_CONFIG_H__

// 上下位机通讯使用串口，因为串口0 2 3是同样的波特率和配置，故而可以切换
#define NUC_MCU_UART bsp_uart0

// 上下位机通讯串口是否启用响应帧
// #define PROTOCOL_DISABLE_RESPONSE

#endif // __HARDWARE_CONFIG_H__
