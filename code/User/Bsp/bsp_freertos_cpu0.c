/**
 * @file bsp_freertos_cpu0.c
 * @author Rh (qq:750920400)
 * @brief 启动FreeRTOS,以及相关的定时器、时钟中断配置
 * @version 0.1
 * @date 2026-05-25
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "Compilers.h"
#include "FreeRTOS.h"
#include "IfxStm.h"
#include "Ifx_Types.h"
#include "isr_config.h"
#include "task.h"
#include "zf_common_headfile.h"

/**
 * FreeRTOS Kernel Tick timer setup
 * FreeRTOS needs kernel Tick through an interrupt occurring at
 * configTICK_RATE_HZ rate. The ISR need to call the function
 * vPortSystemTickHandler.
 */
static void OsTasks_setupTimerInterrupt(Ifx_STM *stm)
{
  IfxStm_setCompareControl(stm, IfxStm_Comparator_0, IfxStm_ComparatorOffset_0, IfxStm_ComparatorSize_32Bits, IfxStm_ComparatorInterrupt_ir0);

  IfxStm_clearCompareFlag(stm, IfxStm_Comparator_0);

  {
    volatile Ifx_SRC_SRCR *src;
    src = IfxStm_getSrcPointer(stm, IfxStm_Comparator_0);
    IfxSrc_init(src, (IfxSrc_Tos)IfxSrc_Tos_cpu0, configKERNEL_INTERRUPT_PRIORITY);
    IfxSrc_enable(src);
  }

  {
    uint32 stmCount = IfxStm_getLower(stm);
    IfxStm_updateCompare(&MODULE_STM0, IfxStm_Comparator_0, stmCount + (configSTM_CLOCK_HZ / configTICK_RATE_HZ) * 100);
    IfxStm_enableComparatorInterrupt(stm, IfxStm_Comparator_0);
  }
}

/**
 * FreeRTOS Kernel Tick interrupt provider
 * FreeRTOS needs kernel Tick through an interrupt occurring at
 * configTICK_RATE_HZ rate. The ISR need to call the function
 * vPortSystemTickHandler.
 */
IFX_INTERRUPT(OsTasks_TickProvider, 0, configKERNEL_INTERRUPT_PRIORITY)
{
  uint32 stmCount = IfxStm_getCompare(&MODULE_STM0, IfxStm_Comparator_0);
  IfxStm_updateCompare(&MODULE_STM0, IfxStm_Comparator_0, stmCount + configSTM_CLOCK_HZ / configTICK_RATE_HZ);
  vPortSystemTickHandler();
}

/**
 * @brief 对外暴露接口，开启freertos
 * @note  需要在他前面创建任务
 *
 */
void start_freertos(void)
{
  // Set the Kernel interrupt and timer interrupt
  OsTasks_setupTimerInterrupt(&MODULE_STM0);

  // Start the tasks running.
  vTaskStartScheduler();
}
