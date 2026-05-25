#ifndef __CPU0_MAIN_H__
#define __CPU0_MAIN_H__

// 这里面的东西是给Clock使用的，保留

#include "Cpu/Std/Ifx_Types.h"

typedef struct
{
  float32 sysFreq; // Actual SPB frequency
  float32 cpuFreq; // Actual CPU frequency
  float32 pllFreq; // Actual PLL frequency
  float32 stmFreq; // Actual STM frequency
} AppInfo;

typedef struct
{
  // Info object
  AppInfo info;
} App_Cpu0;


IFX_EXTERN App_Cpu0 g_AppCpu0;

#endif // __CPU0_MAIN_H__
