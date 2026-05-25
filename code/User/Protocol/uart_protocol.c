/**
 * @file uart_protocol.c
 * @author Rh (qq:750920400)
 * @brief UART串口协议从机实现 - 协议解析、命令处理、响应发送
 * @version 0.1
 * @date 2026-05-25
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "uart_protocol.h"
#include "hardware_config.h"

/*==============================================================================
 * 内部函数声明
 *============================================================================*/
static uint32 protocol_calculate_checksum(const uint8 *data, uint32 len);
static uint8  protocol_parse_frame(UartProtocol *protocol);
static void   protocol_send_response(UartProtocol *protocol, uint8 cmd, uint8 status);
static void   protocol_send_query_speed(UartProtocol *protocol);
static void   protocol_send_query_servo_angle(UartProtocol *protocol);

/*==============================================================================
 * 全局实例（使用UART3）
 *============================================================================*/
UartProtocol g_uart_protocol;

/*==============================================================================
 * 公共函数实现
 *============================================================================*/

/**
 * @brief 初始化协议层
 * @param protocol 协议结构体指针
 * @param uart 串口设备指针
 */
void uart_protocol_init(UartProtocol *protocol, struct BspUart *uart)
{
  protocol->uart     = uart;
  protocol->state    = PROTOCOL_STATE_IDLE;
  protocol->rx_index = 0;

  // 初始化从机状态
  protocol->status.target_speed = PROTOCOL_DEFAULT_SPEED;
  protocol->status.target_time  = 0;
  protocol->status.target_angle = PROTOCOL_DEFAULT_SERVO_ANGLE;
  protocol->status.mode         = PROTOCOL_MODE_IDLE;
}

/**
 * @brief 协议轮询处理（主循环或FreeRTOS任务调用）
 * @param protocol 协议结构体指针
 */
uint8_t uart_protocol_poll(UartProtocol *protocol)
{
  uint32 available;

  // 等待接收信号量
  if (bsp_uart_wait(protocol->uart, portMAX_DELAY) != pdTRUE)
  {
    return 0;
  }

  // 检查是否有16字节
  available = bsp_uart_available(protocol->uart);
  if (available < PROTOCOL_FRAME_SIZE)
  {
    return 0;
  }

  // 读取1字节到缓冲区
  bsp_uart_recv(protocol->uart, protocol->rx_buffer, 1);

  // 检查帧头0xAA
  if (protocol->rx_buffer[0] != PROTOCOL_HEAD_0)
  {
    // 帧头不匹配，丢弃这1字节，继续扫描
    return 0;
  }

  // 读取1字节到缓冲区
  bsp_uart_recv(protocol->uart, &protocol->rx_buffer[1], 1);

  // 检查帧头0x55
  if (protocol->rx_buffer[1] != PROTOCOL_HEAD_1)
  {
    // 帧头不匹配，丢弃这1字节，继续扫描
    return 0;
  }

  // 帧头匹配，读取剩余14字节
  bsp_uart_recv(protocol->uart, &protocol->rx_buffer[2], 14);

  // 检查帧尾0xBB 0x66
  if (protocol->rx_buffer[PROTOCOL_OFF_TAIL] != PROTOCOL_TAIL_0 || protocol->rx_buffer[PROTOCOL_OFF_TAIL + 1] != PROTOCOL_TAIL_1)
  {
    // 帧尾不匹配，丢弃并返回
    return 0;
  }

  // 计算并校验校验和（前13字节）
  if (protocol->rx_buffer[PROTOCOL_OFF_CHECK] != protocol_calculate_checksum(protocol->rx_buffer, PROTOCOL_OFF_CHECK))
  {
    // 校验失败，丢弃并返回
    return 0;
  }

  // 帧校验通过，解析数据
  return protocol_parse_frame(protocol);
}

/**
 * @brief 获取从机状态指针
 * @param protocol 协议结构体指针
 * @return 从机状态指针
 */
SlaveStatus *uart_protocol_get_status(UartProtocol *protocol)
{
  return &protocol->status;
}

/*==============================================================================
 * 内部函数实现
 *============================================================================*/

/**
 * @brief 计算校验和（前len字节累加，低8位）
 * @param data 数据缓冲区
 * @param len 要校验的字节数
 * @return 校验和
 */
static uint32 protocol_calculate_checksum(const uint8 *data, uint32 len)
{
  uint32 i;
  uint32 sum = 0;

  for (i = 0; i < len; i++)
  {
    sum += data[i];
  }

  uint32 out = sum & 0xFF;

  // printf("out:%d\n", out);

  return out;
}

/**
 * @brief 解析完整帧
 * @param protocol 协议结构体指针
 */
static uint8_t protocol_parse_frame(UartProtocol *protocol)
{
  uint8 *frame = protocol->rx_buffer;
  uint8  cmd;
  uint8  data_len;

  // 解析命令
  cmd      = frame[PROTOCOL_OFF_CMD];
  data_len = frame[PROTOCOL_OFF_LEN];

  switch (cmd)
  {
    case PROTOCOL_CMD_SET_SPEED:
      // 设置速度：数据长度4字节，int32
      if (data_len >= 4)
      {
        // 小端格式读取：低字节在前
        protocol->status.target_speed = (int32)frame[PROTOCOL_OFF_DATA] | ((int32)frame[PROTOCOL_OFF_DATA + 1] << 8) | ((int32)frame[PROTOCOL_OFF_DATA + 2] << 16) | ((int32)frame[PROTOCOL_OFF_DATA + 3] << 24);
      }
      protocol->status.mode = PROTOCOL_MODE_SPEED;
      protocol_send_response(protocol, cmd, PROTOCOL_STATUS_OK);
      // 通知 auto_ctrl 有新数据
      return 1;
      break;

    case PROTOCOL_CMD_QUERY_SPEED:
      // 查询速度：回复当前速度
      protocol_send_query_speed(protocol);
      return 0;
      break;

    case PROTOCOL_CMD_SPEED_TIME:
      // 速度-时间模式：数据长度8字节，速度4字节+时间4字节
      if (data_len >= 8)
      {
        // 小端格式读取：低字节在前
        protocol->status.target_speed = (int32)frame[PROTOCOL_OFF_DATA] | ((int32)frame[PROTOCOL_OFF_DATA + 1] << 8) | ((int32)frame[PROTOCOL_OFF_DATA + 2] << 16) | ((int32)frame[PROTOCOL_OFF_DATA + 3] << 24);
        protocol->status.target_time  = (int32)frame[PROTOCOL_OFF_DATA + 4] | ((int32)frame[PROTOCOL_OFF_DATA + 5] << 8) | ((int32)frame[PROTOCOL_OFF_DATA + 6] << 16) | ((int32)frame[PROTOCOL_OFF_DATA + 7] << 24);
      }
      protocol->status.mode = PROTOCOL_MODE_SPEED_TIME;
      protocol_send_response(protocol, cmd, PROTOCOL_STATUS_OK);
      // 通知 auto_ctrl 有新数据
      return 1;
      break;

    case PROTOCOL_CMD_STOP:
      // 紧急停止：覆盖式停止所有运动
      protocol->status.target_speed = 0;
      protocol->status.target_time  = 0;
      protocol->status.mode         = PROTOCOL_MODE_IDLE;
      protocol_send_response(protocol, cmd, PROTOCOL_STATUS_OK);
      // 通知 auto_ctrl 有新数据
      return 1;
      break;

    case PROTOCOL_CMD_SET_SERVO_ANGLE:
      // 设置舵机角度：数据长度4字节，int32，范围 -35~+35°
      if (data_len >= 4)
      {
        // 小端格式读取：低字节在前
        protocol->status.target_angle = (int32)frame[PROTOCOL_OFF_DATA] | ((int32)frame[PROTOCOL_OFF_DATA + 1] << 8) | ((int32)frame[PROTOCOL_OFF_DATA + 2] << 16) | ((int32)frame[PROTOCOL_OFF_DATA + 3] << 24);
      }
      protocol_send_response(protocol, cmd, PROTOCOL_STATUS_OK);
      return 0;
      break;

    case PROTOCOL_CMD_QUERY_SERVO_ANGLE:
      // 查询舵机角度：回复当前角度
      protocol_send_query_servo_angle(protocol);
      return 0;
      break;


    default:
      // 未知命令，无响应
      break;
  }
  return 0; // 不需auto_ctrl
}

/**
 * @brief 发送响应帧
 * @param protocol 协议结构体指针
 * @param cmd 命令码
 * @param status 状态码
 */
static void protocol_send_response(UartProtocol *protocol, uint8 cmd, uint8 status)
{
#ifndef PROTOCOL_DISABLE_RESPONSE
  uint8  frame[PROTOCOL_FRAME_SIZE];
  uint32 i;
  uint8  checksum;

  // 组装帧
  frame[0]                 = PROTOCOL_HEAD_0;
  frame[1]                 = PROTOCOL_HEAD_1;
  frame[2]                 = cmd;
  frame[3]                 = 1;      // 数据长度1
  frame[PROTOCOL_OFF_DATA] = status; // 状态码

  // 数据区剩余清零
  for (i = 1; i < PROTOCOL_DATA_MAX; i++)
  {
    frame[PROTOCOL_OFF_DATA + i] = 0;
  }

  // 计算校验和
  checksum                  = protocol_calculate_checksum(frame, PROTOCOL_OFF_CHECK);
  frame[PROTOCOL_OFF_CHECK] = checksum;

  // 帧尾
  frame[PROTOCOL_OFF_TAIL]     = PROTOCOL_TAIL_0;
  frame[PROTOCOL_OFF_TAIL + 1] = PROTOCOL_TAIL_1;

  // 发送
  bsp_uart_send_buffer(protocol->uart, frame, PROTOCOL_FRAME_SIZE);
#else
  (void)protocol;
  (void)cmd;
  (void)status;
#endif
}

/**
 * @brief 发送查询速度响应
 * @param protocol 协议结构体指针
 */
static void protocol_send_query_speed(UartProtocol *protocol)
{
#ifndef PROTOCOL_DISABLE_RESPONSE
  uint8  frame[PROTOCOL_FRAME_SIZE];
  uint32 i;
  uint8  checksum;
  int32  speed;

  // 获取当前速度
  speed = protocol->status.target_speed;

  // 组装帧
  frame[0] = PROTOCOL_HEAD_0;
  frame[1] = PROTOCOL_HEAD_1;
  frame[2] = PROTOCOL_CMD_QUERY_SPEED;
  frame[3] = 4; // 数据长度4

  // 填充速度数据（小端）
  frame[PROTOCOL_OFF_DATA]     = (uint8)(speed & 0xFF);
  frame[PROTOCOL_OFF_DATA + 1] = (uint8)((speed >> 8) & 0xFF);
  frame[PROTOCOL_OFF_DATA + 2] = (uint8)((speed >> 16) & 0xFF);
  frame[PROTOCOL_OFF_DATA + 3] = (uint8)((speed >> 24) & 0xFF);

  // 数据区剩余清零
  for (i = 4; i < PROTOCOL_DATA_MAX; i++)
  {
    frame[PROTOCOL_OFF_DATA + i] = 0;
  }

  // 计算校验和
  checksum                  = protocol_calculate_checksum(frame, PROTOCOL_OFF_CHECK);
  frame[PROTOCOL_OFF_CHECK] = checksum;

  // 帧尾
  frame[PROTOCOL_OFF_TAIL]     = PROTOCOL_TAIL_0;
  frame[PROTOCOL_OFF_TAIL + 1] = PROTOCOL_TAIL_1;

  // 发送
  bsp_uart_send_buffer(protocol->uart, frame, PROTOCOL_FRAME_SIZE);
#else
  (void)protocol;
#endif
}

/**
 * @brief 发送查询舵机角度响应
 * @param protocol 协议结构体指针
 */
static void protocol_send_query_servo_angle(UartProtocol *protocol)
{
#ifndef PROTOCOL_DISABLE_RESPONSE
  uint8  frame[PROTOCOL_FRAME_SIZE];
  uint32 i;
  uint8  checksum;
  int32  angle;

  // 获取当前舵机角度（从协议状态中获取）
  angle = protocol->status.target_angle;

  // 组装帧
  frame[0] = PROTOCOL_HEAD_0;
  frame[1] = PROTOCOL_HEAD_1;
  frame[2] = PROTOCOL_CMD_QUERY_SERVO_ANGLE;
  frame[3] = 4; // 数据长度4

  // 填充角度数据（小端）
  frame[PROTOCOL_OFF_DATA]     = (uint8)(angle & 0xFF);
  frame[PROTOCOL_OFF_DATA + 1] = (uint8)((angle >> 8) & 0xFF);
  frame[PROTOCOL_OFF_DATA + 2] = (uint8)((angle >> 16) & 0xFF);
  frame[PROTOCOL_OFF_DATA + 3] = (uint8)((angle >> 24) & 0xFF);

  // 数据区剩余清零
  for (i = 4; i < PROTOCOL_DATA_MAX; i++)
  {
    frame[PROTOCOL_OFF_DATA + i] = 0;
  }

  // 计算校验和
  checksum                  = protocol_calculate_checksum(frame, PROTOCOL_OFF_CHECK);
  frame[PROTOCOL_OFF_CHECK] = checksum;

  // 帧尾
  frame[PROTOCOL_OFF_TAIL]     = PROTOCOL_TAIL_0;
  frame[PROTOCOL_OFF_TAIL + 1] = PROTOCOL_TAIL_1;

  // 发送
  bsp_uart_send_buffer(protocol->uart, frame, PROTOCOL_FRAME_SIZE);
#else
  (void)protocol;
#endif
}
