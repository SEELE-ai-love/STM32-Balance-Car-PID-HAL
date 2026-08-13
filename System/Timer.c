#include "main.h"
#include "Key.h"
#include "Timer.h"

TIM_HandleTypeDef htim1;

volatile uint32_t Time_Count = 0;
volatile uint32_t Time_Flag = 0;

/**
  * 函    数：定时器中断初始化（TIM1，1ms系统时基）
  * 参    数：无
  * 返 回 值：无
  */
void Timer_Init(void)
{
	/* 开启TIM1时钟（APB2） */
	__HAL_RCC_TIM1_CLK_ENABLE();

	/* 时基单元初始化 */
	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 72 - 1;				/* 72MHz / 72 = 1MHz */
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 1000 - 1;				/* 1MHz / 1000 = 1ms */
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&htim1);

	/* NVIC中断分组（与标准库工程保持一致：2位抢占+2位响应） */
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);  /* FreeRTOS: full preemption, 16 levels */

	/* NVIC配置 */
	HAL_NVIC_SetPriority(TIM1_UP_IRQn, 2, 1);
	HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);

	/* 启动定时器并开启更新中断 */
	HAL_TIM_Base_Start_IT(&htim1);
}

/**
  * 函    数：定时器更新中断回调函数
  * 参    数：htim 触发回调的定时器句柄
  * 返 回 值：无
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM1)
	{
		/* 1ms系统时间 */
		system_tick++;

		/* 按键扫描 */
		Key_Tick();
	}
}
