/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_it.h"
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS 移植层提供的 SysTick 处理函数（定义于 port.c） */
extern void xPortSysTickHandler(void);

/* 外设句柄外部声明（定义在对应驱动文件中） */
extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{
	}
}

void MemManage_Handler(void)
{
	while (1)
	{
	}
}

void BusFault_Handler(void)
{
	while (1)
	{
	}
}

void UsageFault_Handler(void)
{
	while (1)
	{
	}
}

/* SVC_Handler / PendSV_Handler: 由 FreeRTOS 内核移植文件提供（见 FreeRTOSConfig.h 宏映射） */

void DebugMon_Handler(void)
{
}


void SysTick_Handler(void)
{
	/* HAL 时基（HAL_Delay 依赖） */
	HAL_IncTick();

	/* FreeRTOS 系统节拍（调度器启动后才允许调用） */
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
		xPortSysTickHandler();
	}
}

/******************************************************************************/
/*                 STM32F1xx Peripherals Interrupt Handlers                   */
/******************************************************************************/

/**
  * @brief  This function handles TIM1 update interrupt request.
  *         1ms系统时间基准（system_tick + 按键扫描）
  */
void TIM1_UP_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim1);
}

/**
  * @brief  This function handles USART1 global interrupt request.（USB串口）
  */
void USART1_IRQHandler(void)
{
	HAL_UART_IRQHandler(&huart1);
}

/**
  * @brief  This function handles USART2 global interrupt request.（蓝牙串口）
  */
void USART2_IRQHandler(void)
{
	HAL_UART_IRQHandler(&huart2);
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
