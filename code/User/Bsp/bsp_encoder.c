/**
 * @file bsp_encoder.c
 * @author Rh (qq:750920400)
 * @brief 编码器初始化，很简陋
 * @version 0.1
 * @date 2026-05-25
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "bsp_encoder.h"

#pragma section all "cpu0_dsram"

/*==============================================================================
 * 编码器设备定义
 *============================================================================*/

// 示例编码器设备 - TIM2 正交解码
struct BspEncoder bsp_encoder_tim2;

#pragma section all restore

/*==============================================================================
 * 公共函数实现
 *============================================================================*/

/**
 * @brief 初始化编码器设备
 * @param enc 编码器结构体指针
 * @param encoder_n 编码器模块号
 * @param ch_a A 相引脚
 * @param ch_b B 相引脚
 */
void bsp_encoder_init(BspEncoder *enc, encoder_index_enum encoder_n, encoder_channel1_enum ch_a, encoder_channel2_enum ch_b)
{
  enc->encoder_index = encoder_n;
  enc->channel_a     = ch_a;
  enc->channel_b     = ch_b;
  enc->count         = 0;

  // 初始化正交解码模式
  encoder_quad_init(encoder_n, ch_a, ch_b);
}

/**
 * @brief 获取编码器当前计数值
 * @param enc 编码器结构体指针
 * @return 当前计数值
 */
int16 bsp_encoder_get_count(BspEncoder *enc)
{
  enc->count = encoder_get_count(enc->encoder_index);
  return enc->count;
}

/**
 * @brief 清零编码器计数值
 * @param enc 编码器结构体指针
 */
void bsp_encoder_clear_count(BspEncoder *enc)
{
  encoder_clear_count(enc->encoder_index);
  enc->count = 0;
}

/**
 * @brief 总初始化函数
 * @note 示例初始化 TIM2 编码器（A=P00_7, B=P00_8）
 */
void bsp_encoder_all_init(void)
{
  // TIM2 正交解码编码器初始化
  bsp_encoder_init(&bsp_encoder_tim2,
                   TIM2_ENCODER,
                   TIM2_ENCODER_CH1_P00_7,
                   TIM2_ENCODER_CH2_P00_8);
}