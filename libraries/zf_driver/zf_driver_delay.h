#ifndef _zf_driver_delay_h_
#define _zf_driver_delay_h_

#ifdef __cplusplus
extern "C" 
{
#endif

#include "zf_common_typedef.h"

//====================================================延时 基础函数====================================================
void    system_delay                (uint32 time, uint32 num);
void    system_delay_us             (uint32 time);
void    system_delay_ms             (uint32 time);
//====================================================延时 基础函数====================================================

#ifdef __cplusplus
}
#endif

#endif
