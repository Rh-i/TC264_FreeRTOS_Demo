#include "SysSe/Bsp/Bsp.h"
#include "zf_common_interrupt.h"
#include "zf_driver_pit.h"
#include "zf_driver_pwm.h"
#include "zf_driver_exti.h"

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     全局中断使能
// 参数说明     void
// 返回参数     void
// 使用示例     interrupt_global_enable(1);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void interrupt_global_enable (uint32 primask)
{
    if(primask == 0)
    {
        enableInterrupts();
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     全局中断屏蔽
// 参数说明     void
// 返回参数     void
// 使用示例     uint32 interrupt_num = interrupt_global_disable();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint32 interrupt_global_disable (void)
{
    return (uint32)(!disableInterrupts());
}


//-------------------------------------------------------------------------------------------------------------------
// 函数简介     断言中断配置
// 参数说明     void
// 返回参数     void
// 使用示例     assert_interrupt_config();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void assert_interrupt_config (void)
{
    interrupt_global_disable();                 // 全局中断失能
    pit_all_close();                            // 关闭所有pit中断
    pwm_all_channel_close();                    // 关闭PWM所有通道输出
    exti_all_close();                           // 关闭所有外部触发中断
    interrupt_global_enable(0);                 // 全局中断使能
}
