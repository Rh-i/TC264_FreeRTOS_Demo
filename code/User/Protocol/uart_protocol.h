/**
 * @file uart_protocol.h
 * @brief UART串口协议 - 从机实现，基于固定16字节帧格式
 */

#ifndef UART_PROTOCOL_H_
#define UART_PROTOCOL_H_

#include "bsp_uart.h"

/*==============================================================================
 * 常量定义
 *============================================================================*/

/* 帧格式常量 */
#define PROTOCOL_FRAME_SIZE 16 /**< 固定帧长度 */
#define PROTOCOL_DATA_MAX 9    /**< 最大数据长度 */

/* 帧头帧尾 */
#define PROTOCOL_HEAD_0 0xAA /**< 帧头字节1 */
#define PROTOCOL_HEAD_1 0x55 /**< 帧头字节2 */
#define PROTOCOL_TAIL_0 0xBB /**< 帧尾字节1 */
#define PROTOCOL_TAIL_1 0x66 /**< 帧尾字节2 */

/* 帧内偏移 */
#define PROTOCOL_OFF_CMD 2    /**< 命令码偏移 */
#define PROTOCOL_OFF_LEN 3    /**< 数据长度偏移 */
#define PROTOCOL_OFF_DATA 4   /**< 数据区偏移 */
#define PROTOCOL_OFF_CHECK 13 /**< 校验字节偏移 */
#define PROTOCOL_OFF_TAIL 14  /**< 帧尾偏移 */

/* 默认值 */
#define PROTOCOL_DEFAULT_SPEED 40 /**< 默认速度 cm/s */

/*==============================================================================
 * 命令码定义
 *============================================================================*/
typedef enum
{
  PROTOCOL_CMD_SET_SPEED   = 0x01, /**< 设置目标速度 */
  PROTOCOL_CMD_QUERY_SPEED = 0x02, /**< 查询当前速度 */
  PROTOCOL_CMD_SPEED_TIME  = 0x03, /**< 速度-时间模式 */
  PROTOCOL_CMD_STOP        = 0x04, /**< 紧急停止 */
} ProtocolCmd;

/*==============================================================================
 * 控制模式定义
 *============================================================================*/
typedef enum
{
  PROTOCOL_MODE_IDLE       = 0x00, /**< 空闲模式 */
  PROTOCOL_MODE_SPEED      = 0x01, /**< 速度模式 */
  PROTOCOL_MODE_SPEED_TIME = 0x03, /**< 速度-时间模式 */
} ProtocolMode;

/*==============================================================================
 * 状态码定义
 *============================================================================*/
typedef enum
{
  PROTOCOL_STATUS_OK  = 0x00, /**< 成功 */
  PROTOCOL_STATUS_ERR = 0x01, /**< 通用错误 */
} ProtocolStatus;

/*==============================================================================
 * 协议状态机状态
 *============================================================================*/
typedef enum
{
  PROTOCOL_STATE_IDLE   = 0, /**< 等待帧头0xAA */
  PROTOCOL_STATE_HEADER = 1, /**< 等待帧头0x55 */
  PROTOCOL_STATE_DATA   = 2, /**< 接收数据 */
} ProtocolState;

/*==============================================================================
 * 数据结构
 *============================================================================*/

/**
 * @brief 从机状态结构
 * @note 用于存储从机的运动控制参数，供外部运动控制模块使用
 */
typedef struct
{
  int32        target_speed; /**< 目标速度 cm/s */
  int32        target_time;  /**< 目标时间 ms */
  ProtocolMode mode;         /**< 当前控制模式 */
} SlaveStatus;

/**
 * @brief 协议层结构
 * @note 管理协议解析状态和缓冲区
 */
typedef struct
{
  ProtocolState state;                          /**< 状态机状态 */
  uint8         rx_buffer[PROTOCOL_FRAME_SIZE]; /**< 接收缓冲区 */
  uint8         rx_index;                       /**< 接收索引 */
  SlaveStatus   status;                         /**< 从机状态 */
} UartProtocol;

/*==============================================================================
 * 函数接口
 *============================================================================*/

/**
 * @brief 初始化协议层
 * @param protocol 协议结构体指针
 */
void uart_protocol_init(UartProtocol *protocol);

/**
 * @brief 协议轮询处理（主循环或FreeRTOS任务调用）
 * @param protocol 协议结构体指针
 */
uint8_t uart_protocol_poll(UartProtocol *protocol);

/**
 * @brief 获取从机状态指针
 * @param protocol 协议结构体指针
 * @return 从机状态指针
 */
SlaveStatus *uart_protocol_get_status(UartProtocol *protocol);

/*==============================================================================
 * 全局实例（使用UART3）
 *============================================================================*/
extern UartProtocol g_uart_protocol;

#endif /* UART_PROTOCOL_H_ */
