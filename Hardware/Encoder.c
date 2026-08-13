#include "main.h"

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/**
  * 函    数：编码器初始化
  * 说    明：TIM3编码器模式测速（PA6/PA7），TIM4编码器模式测速（PB6/PB7）
  * 参    数：无
  * 返 回 值：无
  */
void Encoder_Init(void)
{
	/* ---------- TIM3编码器测速 ---------- */
	__HAL_RCC_TIM3_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 1 - 1;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 65536 - 1;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	TIM_Encoder_InitTypeDef TIM_EncoderInitStructure;
	TIM_EncoderInitStructure.EncoderMode = TIM_ENCODERMODE_TI12;
	TIM_EncoderInitStructure.IC1Polarity = TIM_ICPOLARITY_RISING;
	TIM_EncoderInitStructure.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	TIM_EncoderInitStructure.IC1Prescaler = TIM_ICPSC_DIV1;
	TIM_EncoderInitStructure.IC1Filter = 0x0F;
	TIM_EncoderInitStructure.IC2Polarity = TIM_ICPOLARITY_RISING;
	TIM_EncoderInitStructure.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	TIM_EncoderInitStructure.IC2Prescaler = TIM_ICPSC_DIV1;
	TIM_EncoderInitStructure.IC2Filter = 0x0F;
	HAL_TIM_Encoder_Init(&htim3, &TIM_EncoderInitStructure);
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);

	/* ---------- TIM4编码器测速 ---------- */
	__HAL_RCC_TIM4_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure1;
	GPIO_InitStructure1.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStructure1.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure1.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure1);

	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 1 - 1;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 65536 - 1;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	TIM_Encoder_InitTypeDef TIM_EncoderInitStructure1;
	TIM_EncoderInitStructure1.EncoderMode = TIM_ENCODERMODE_TI12;
	TIM_EncoderInitStructure1.IC1Polarity = TIM_ICPOLARITY_RISING;
	TIM_EncoderInitStructure1.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	TIM_EncoderInitStructure1.IC1Prescaler = TIM_ICPSC_DIV1;
	TIM_EncoderInitStructure1.IC1Filter = 0x0F;
	TIM_EncoderInitStructure1.IC2Polarity = TIM_ICPOLARITY_FALLING;
	TIM_EncoderInitStructure1.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	TIM_EncoderInitStructure1.IC2Prescaler = TIM_ICPSC_DIV1;
	TIM_EncoderInitStructure1.IC2Filter = 0x0F;
	HAL_TIM_Encoder_Init(&htim4, &TIM_EncoderInitStructure1);
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
}

/**
  * 函    数：获取编码器的增量值
  * 参    数：n 1=左轮(TIM3)，2=右轮(TIM4)
  * 返 回 值：自上次调用以来的增量
  */
int16_t Encoder_Get(uint8_t n)
{
	int16_t temp;
	if (n == 1)
	{
		temp = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
		__HAL_TIM_SET_COUNTER(&htim3, 0);
	}
	else if (n == 2)
	{
		temp = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);
		__HAL_TIM_SET_COUNTER(&htim4, 0);
	}
	else
	{
		return 0;
	}
	return temp;
}
