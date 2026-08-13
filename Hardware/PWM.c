#include "main.h"

TIM_HandleTypeDef htim2;

/**
  * 函    数：PWM初始化（TIM2通道1/2，PA0/PA1，20kHz）
  * 参    数：无
  * 返 回 值：无
  */
void PWM_Init(void)
{
	/* 开启时钟 */
	__HAL_RCC_TIM2_CLK_ENABLE();			/* 开启TIM2的时钟（APB1） */
	__HAL_RCC_GPIOA_CLK_ENABLE();			/* 开启GPIOA的时钟（APB2） */

	/* GPIO初始化：PA0/PA1复用推挽输出 */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* 时基单元初始化 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 36 - 1;				/* 72MHz / 36 = 2MHz */
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 100 - 1;				/* 2MHz / 100 = 20kHz */
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_PWM_Init(&htim2);

	/* 输出比较初始化 */
	TIM_OC_InitTypeDef TIM_OCInitStructure;
	TIM_OCInitStructure.OCMode = TIM_OCMODE_PWM1;
	TIM_OCInitStructure.Pulse = 0;				/* 初始CCR值 */
	TIM_OCInitStructure.OCPolarity = TIM_OCPOLARITY_HIGH;
	TIM_OCInitStructure.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim2, &TIM_OCInitStructure, TIM_CHANNEL_1);
	HAL_TIM_PWM_ConfigChannel(&htim2, &TIM_OCInitStructure, TIM_CHANNEL_2);

	/* 启动PWM输出 */
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
}

/**
  * 函    数：PWM设置CCR1
  * 说    明：PA0，左轮；占空比Duty = CCR / (ARR+1)
  */
void PWM_SetCompare1(uint16_t Compare)
{
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, Compare);
}

/**
  * 函    数：PWM设置CCR2
  * 说    明：PA1，右轮
  */
void PWM_SetCompare2(uint16_t Compare)
{
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, Compare);
}
