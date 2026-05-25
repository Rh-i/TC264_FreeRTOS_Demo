/**
 * @file FreeRTOSConfig.h
 * @author Rh (qq:750920400)
 * @brief FreeRTOS∆Ù”√ƒ⁄»›…Ë÷√
 * @version 0.1
 * @date 2026-05-25
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef __FREERTOS_CONFIG_H__
#define __FREERTOS_CONFIG_H__

#define configUSE_PREEMPTION				                              1
#define configUSE_IDLE_HOOK					                              0
#define configCPU_CLOCK_HZ				( ( unsigned long ) 200000000UL )
#define configTICK_RATE_HZ				        ( ( TickType_t ) 1000UL )
#define configMAX_PRIORITIES			                           ( 10 )
#define configMINIMAL_STACK_SIZE	       ( ( unsigned short ) 256 )
#define configTOTAL_HEAP_SIZE			   ( ( size_t ) ( 32U * 1024U ) )
#define configMAX_TASK_NAME_LEN		                           ( 16 )
#define configUSE_TRACE_FACILITY			                            0
#define configUSE_16_BIT_TICKS				                            0
#define configIDLE_SHOULD_YIELD				                            0
#define configUSE_MALLOC_FAILED_HOOK 		                          0
#define configCHECK_FOR_STACK_OVERFLOW	               	          0
#define configUSE_TICK_HOOK					                              0
#define configUSE_COUNTING_SEMAPHORES 		                        1
#define configUSE_RECURSIVE_MUTEXES			                          1
#define configUSE_MUTEXES					                                1
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS                   5

// Co-routine definitions.
#define configUSE_CO_ROUTINES 				                            0
#define configMAX_CO_ROUTINE_PRIORITIES 	                    ( 2 )

// Software timer configuration.
#define configUSE_TIMERS					                              (1)
#define configSTM_MODULE					                     (AURIX_STM0)
#define configSTM_CLOCK_HZ					         (configCPU_CLOCK_HZ/2)
#define configTIMER_TASK_PRIORITY			                        ( 9 )
#define configTIMER_QUEUE_LENGTH			                        ( 5 )
#define configTIMER_TASK_STACK_DEPTH	     configMINIMAL_STACK_SIZE

#define configAURIX_GENERATION				          TRICORE_AURIX_TC2XX

#define INCLUDE_vTaskPrioritySet				                          1
#define INCLUDE_uxTaskPriorityGet				                          1
#define INCLUDE_vTaskDelete						                            1
#define INCLUDE_vTaskCleanUpResources		                          1
#define INCLUDE_vTaskSuspend				  	                          1
#define INCLUDE_vTaskDelayUntil					                          1
#define INCLUDE_vTaskDelay					  	                          1

// Interrupt above priority 64 are not effected by critical sections, but cannot call interrupt safe FreeRTOS functions.
#define configMAX_SYSCALL_INTERRUPT_PRIORITY	                   31 
#define configMAX_API_CALL_INTERRUPT_PRIORITY                    31

// This value must not be changed from 1.
#define configKERNEL_INTERRUPT_PRIORITY			                      1  

// Default definition of configASSERT().
#define configASSERT( x ) if( ( x ) == 0 ) 		{ portDISABLE_INTERRUPTS(); TriCore__debug(); vPortLoopForever();}

#endif // __FREERTOS_CONFIG_H__

