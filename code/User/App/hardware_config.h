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
#define NUC_MCU_UART bsp_uart3

// 上下位机通讯串口是否启用响应帧
#define PROTOCOL_DISABLE_RESPONSE

/*==============================================================================
 * 电路板测试宏
 * @brief 开启后进入测试模式：屏蔽上位机+遥控器数据，
 *        三个按键分别触发电机PWM/编码器/舵机测试
 *        验证完成后注释此宏，重新编译恢复正常功能
 *============================================================================*/
//#define TEST_BOARD

#endif // __HARDWARE_CONFIG_H__
