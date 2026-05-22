/**
 * @file app_cfg.c
 * @brief 应用配置实现
 */

#include "zf_common_headfile.h"
#include "app_cfg.h"

#pragma section all "cpu0_dsram"

/**
 * @brief 用户所有的初始化
 */
void user_init(void)
{
    /* IO设备初始化 */
    bsp_io_init();
    
    /* 串口初始化 - 使用默认配置示例 */
    /* UART0: 波特率115200, TX=P14_0, RX=P14_1, 接收缓冲区256字节 */
    bsp_uart_init(&bsp_uart0, UART_0, 115200, UART0_TX_P14_0, UART0_RX_P14_1, 256);
    
    /* UART1: 波特率115200, TX=P15_0, RX=P15_1, 接收缓冲区256字节 */
    bsp_uart_init(&bsp_uart1, UART_1, 115200, UART1_TX_P15_0, UART1_RX_P15_1, 256);
    
    /* UART2: 波特率115200, TX=P02_0, RX=P02_1, 接收缓冲区256字节 */
    bsp_uart_init(&bsp_uart2, UART_2, 115200, UART2_TX_P02_0, UART2_RX_P02_1, 256);
    
    /* UART3: 波特率115200, TX=P00_0, RX=P00_1, 接收缓冲区256字节 */
    bsp_uart_init(&bsp_uart3, UART_3, 115200, UART3_TX_P00_0, UART3_RX_P00_1, 256);

}



#pragma section all restore
