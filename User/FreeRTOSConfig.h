/*
 * FreeRTOS Kernel V10.4.4+ (202212.00) configuration for STM32F103C8 + STM32Cube HAL
 * AC5 (ARMCC) + RVDS/ARM_CM3 port
 */
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f1xx_hal.h"

extern uint32_t SystemCoreClock;

/*-----------------------------------------------------------
 * Kernel behaviour
 *----------------------------------------------------------*/
#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      ( SystemCoreClock )
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                    ( 7 )
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 12 * 1024 ) )
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_16_BIT_TICKS                  0
#define INCLUDE_xTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                        1
#define INCLUDE_vTaskDelete                        1
#define INCLUDE_vTaskSuspend                       1
#define configIDLE_SHOULD_YIELD                 1

/*-----------------------------------------------------------
 * Synchronisation primitives
 *----------------------------------------------------------*/
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_TASK_NOTIFICATIONS            1

/*-----------------------------------------------------------
 * Software timers
 *----------------------------------------------------------*/
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            ( configMINIMAL_STACK_SIZE )

/*-----------------------------------------------------------
 * Co-routines (not used)
 *----------------------------------------------------------*/
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         ( 2 )

/*-----------------------------------------------------------
 * Memory allocation
 *----------------------------------------------------------*/
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configAPPLICATION_ALLOCATED_HEAP        0

/*-----------------------------------------------------------
 * Debug / hooks
 *----------------------------------------------------------*/
#define configUSE_TRACE_FACILITY                1
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1

/*-----------------------------------------------------------
 * Interrupt nesting behaviour
 * STM32F1: 4 priority bits, full preemption (NVIC_PRIORITYGROUP_4)
 *----------------------------------------------------------*/
#define configPRIO_BITS                         4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )

/*-----------------------------------------------------------
 * Assert
 *----------------------------------------------------------*/
#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/*-----------------------------------------------------------
 * Map FreeRTOS port interrupt handlers to CMSIS names
 * (SVC_Handler / PendSV_Handler are removed from stm32f1xx_it.c)
 *----------------------------------------------------------*/
#define vPortSVCHandler         SVC_Handler
#define xPortPendSVHandler      PendSV_Handler

#endif /* FREERTOS_CONFIG_H */
