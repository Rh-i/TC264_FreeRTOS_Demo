#ifndef __BSP_ENCODER_H__
#define __BSP_ENCODER_H__

#include "zf_common_headfile.h"
#include "zf_driver_encoder.h"

/*==============================================================================
 * 编码器设备定义
 * @brief 使用 GPT12 Timer 实现正交解码，支持 AB 相编码器
 *============================================================================*/
typedef struct BspEncoder
{
  encoder_index_enum    encoder_index; // 编码器模块号
  encoder_channel1_enum channel_a;     // A 相引脚
  encoder_channel2_enum channel_b;     // B 相引脚
  int16                 count;         // 当前计数值
} BspEncoder;

extern struct BspEncoder bsp_encoder_tim2;

/*==============================================================================
 * 公共函数声明
 *============================================================================*/

/**
 * @brief 初始化编码器设备
 * @param enc 编码器结构体指针
 * @param encoder_n 编码器模块号 (TIM2_ENCODER ~ TIM6_ENCODER)
 * @param ch_a A 相引脚
 * @param ch_b B 相引脚
 */
void bsp_encoder_init(BspEncoder *enc, encoder_index_enum encoder_n, encoder_channel1_enum ch_a, encoder_channel2_enum ch_b);

/**
 * @brief 获取编码器当前计数值
 * @param enc 编码器结构体指针
 * @return 当前计数值
 */
int16 bsp_encoder_get_count(BspEncoder *enc);

/**
 * @brief 清零编码器计数值
 * @param enc 编码器结构体指针
 */
void bsp_encoder_clear_count(BspEncoder *enc);

/**
 * @brief 总初始化函数（示例）
 */
void bsp_encoder_all_init(void);

#endif // __BSP_ENCODER_H__
