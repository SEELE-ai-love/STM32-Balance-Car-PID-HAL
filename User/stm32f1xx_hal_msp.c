/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_hal_msp.c
  * @brief   HAL MSP 初始化（中断分组）
  * @note    各外设的时钟/GPIO已在对应驱动文件中配置，
  *          这里只负责系统级初始化（NVIC分组）。
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

/**
  * @brief  Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
	__HAL_RCC_AFIO_CLK_ENABLE();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);  /* FreeRTOS: full preemption, 16 levels */
}
