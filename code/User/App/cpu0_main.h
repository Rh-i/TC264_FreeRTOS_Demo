#ifndef CPU0_MAIN_H
#define CPU0_MAIN_H

// 这里面的东西是给Clock使用的，保留

#include "Cpu/Std/Ifx_Types.h"

typedef struct
{
  float32 sysFreq; /**< \brief Actual SPB frequency */
  float32 cpuFreq; /**< \brief Actual CPU frequency */
  float32 pllFreq; /**< \brief Actual PLL frequency */
  float32 stmFreq; /**< \brief Actual STM frequency */
} AppInfo;

typedef struct
{
  AppInfo info; /**< \brief Info object */
} App_Cpu0;


IFX_EXTERN App_Cpu0 g_AppCpu0;

#endif
