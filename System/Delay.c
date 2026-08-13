#include "main.h"

/**
  * 函    数：微秒级延时
  * 参    数：us 延时长度
  * 返 回 值：无
  * 说    明：使用内核DWT计数器实现，不占用SysTick（SysTick留给HAL时基）
  *          72MHz下1个计数 = 1/72us
  */
void Delay_us(uint32_t us)
{
	uint32_t start;
	uint32_t ticks;

	/* 使能DWT内核计数器 */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	start = DWT->CYCCNT;
	ticks = us * (SystemCoreClock / 1000000U);

	while ((DWT->CYCCNT - start) < ticks);
}

/**
  * 函    数：毫秒级延时（使用HAL_Delay）
  */
void Delay_ms(uint32_t ms)
{
	HAL_Delay(ms);
}

/**
  * 函    数：秒级延时
  */
void Delay_s(uint32_t s)
{
	while (s--)
	{
		Delay_ms(1000);
	}
}
