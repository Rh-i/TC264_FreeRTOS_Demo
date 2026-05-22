#ifndef __BSP_FREERTOS_CPU0_H__
#define __BSP_FREERTOS_CPU0_H__

#ifdef __cplusplus
extern "C" 
{
#endif

/**
 * @brief 对外暴露接口，开启freertos
 * @note  需要在他前面创建任务
 *
 */
void start_freertos(void);

#ifdef __cplusplus
}
#endif

#endif // __BSP_FREERTOS_CPU0_H__
