/**
 * @file bsp_uart.c
 * @brief UART设备驱动实现 - 串口收发，使用FIFO阻塞发送+中断接收
 */

#include "bsp_uart.h"
#include "isr_config.h"

#pragma section all "cpu0_dsram"

/* 串口设备定义 */
struct BspUart bsp_uart0;
struct BspUart bsp_uart1;
struct BspUart bsp_uart2;
struct BspUart bsp_uart3;

#pragma section all restore

/*==============================================================================
 * 内部函数声明
 *============================================================================*/
static void bsp_uart_rx_isr_handler(BspUart *uart);
static void bsp_uart_rx_interrupt_enable(uart_index_enum uart_n);

/* UART0 接收中断 */
IFX_INTERRUPT(bsp_uart0_rx_isr, 0, UART0_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    bsp_uart_rx_isr_handler(&bsp_uart0);
}

/* UART1 接收中断 */
IFX_INTERRUPT(bsp_uart1_rx_isr, 0, UART1_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    bsp_uart_rx_isr_handler(&bsp_uart1);
}

/* UART2 接收中断 */
IFX_INTERRUPT(bsp_uart2_rx_isr, 0, UART2_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    bsp_uart_rx_isr_handler(&bsp_uart2);
}

/* UART3 接收中断 */
IFX_INTERRUPT(bsp_uart3_rx_isr, 0, UART3_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    bsp_uart_rx_isr_handler(&bsp_uart3);
}

/**
 * @brief UART接收中断处理函数
 * @param uart 串口结构体指针
 */
static void bsp_uart_rx_isr_handler(BspUart *uart)
{
    uint8 dat;
    /* 读取所有可用的数据并写入流缓冲区 */
    while (uart_query_byte(uart->uart_index, &dat))
    {
        xStreamBufferSendFromISR(uart->rx_stream, &dat, 1, NULL);
    }
}

/**
 * @brief 启用串口接收中断
 * @param uart_n 串口模块号
 */
static void bsp_uart_rx_interrupt_enable(uart_index_enum uart_n)
{
    uart_rx_interrupt(uart_n, 1);
}

/*==============================================================================
 * 公共函数实现
 *============================================================================*/

/**
 * @brief 初始化串口成员变量
 * @param uart 串口结构体指针
 * @param uart_index 串口模块号
 * @param baud 波特率
 * @param tx_pin 发送引脚
 * @param rx_pin 接收引脚
 * @param rx_buf_size 流缓冲区大小
 */
void bsp_uart_init(BspUart *uart, uart_index_enum uart_index, uint32 baud, 
                   uart_tx_pin_enum tx_pin, uart_rx_pin_enum rx_pin, size_t rx_buf_size)
{
    uart->uart_index = uart_index;
    uart->tx_pin = tx_pin;
    uart->rx_pin = rx_pin;
    uart->baudrate = baud;
    
    /* 创建流缓冲区用于接收数据 */
    uart->rx_stream = xStreamBufferCreate(rx_buf_size, 1);
    
    /* 初始化逐飞串口驱动 */
    uart_init(uart_index, baud, tx_pin, rx_pin);
    
    /* 启用接收中断 */
    bsp_uart_rx_interrupt_enable(uart_index);
}

/**
 * @brief 析构函数，释放流缓冲区资源
 * @param uart 串口结构体指针
 */
void bsp_uart_deinit(BspUart *uart)
{
    if (uart->rx_stream != NULL)
    {
        vStreamBufferDelete(uart->rx_stream);
        uart->rx_stream = NULL;
    }
}

/**
 * @brief 阻塞发送一个字节
 * @param uart 串口结构体指针
 * @param dat 需要发送的字节
 */
void bsp_uart_send_byte(BspUart *uart, uint8 dat)
{
    uart_write_byte(uart->uart_index, dat);
}

/**
 * @brief 阻塞发送字符串
 * @param uart 串口结构体指针
 * @param str 需要发送的字符串
 */
void bsp_uart_send_string(BspUart *uart, const char *str)
{
    uart_write_string(uart->uart_index, str);
}

/**
 * @brief 阻塞发送缓冲区数据
 * @param uart 串口结构体指针
 * @param buff 需要发送的数据缓冲区
 * @param len 数据长度
 */
void bsp_uart_send_buffer(BspUart *uart, const uint8 *buff, uint32 len)
{
    uart_write_buffer(uart->uart_index, buff, len);
}

/**
 * @brief 非阻塞从流缓冲区读取数据
 * @param uart 串口结构体指针
 * @param buff 接收数据缓冲区
 * @param len 要读取的数据长度
 * @param timeoutTicks 超时时间（portMAX_DELAY表示永久等待）
 * @return 实际读取的字节数
 */
size_t bsp_uart_recv(BspUart *uart, uint8 *buff, size_t len, TickType_t timeoutTicks)
{
    return xStreamBufferReceive(uart->rx_stream, buff, len, timeoutTicks);
}

/**
 * @brief 查询流缓冲区中可读数据的长度
 * @param uart 串口结构体指针
 * @return 流缓冲区中可读数据的字节数
 */
size_t bsp_uart_recv_available(BspUart *uart)
{
    return uxStreamBufferGetSize(uart->rx_stream);
}