#include "zf_common_headfile.h"

#pragma section all "cpu1_dsram"

void core1_main(void)
{
    disable_Watchdog();         // 关闭看门狗
    interrupt_global_enable(0); // 打开全局中断

    cpu_wait_event_ready(); // 等待所有核心初始化完毕
    while (1)
    {
        // printf("ENCODER_QUADDEC counter \t%d .\r\n", encoder_data_quaddec);     // 输出编码器计数信息
    }
}

#pragma section all restore
