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
    bsp_io_init();

}



#pragma section all restore
