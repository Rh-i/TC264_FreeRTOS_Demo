/**
 * @file app_cfg.h
 * @brief 应用配置头文件
 */

#ifndef _APP_CFG_H_
#define _APP_CFG_H_

#include "bsp_io.h"
#include "bsp_uart.h"

/* 串口设备声明 */
extern BspUart bsp_uart0;
extern BspUart bsp_uart1;
extern BspUart bsp_uart2;
extern BspUart bsp_uart3;

/**
 * @brief 用户所有的初始化
 */
void user_init(void);

#endif // _APP_CFG_H_
