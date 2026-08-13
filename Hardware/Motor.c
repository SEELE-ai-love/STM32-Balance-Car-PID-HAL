#include "main.h"
#include "PWM.h"

/**
  * 函    数：直流电机初始化
  * 参    数：无
  * 返 回 值：无
  */
void Motor_Init(void)
{
	/* 开启GPIOB时钟 */
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);			/* PB12/PB13/PB14/PB15推挽输出 */

	PWM_Init();											/* 初始化直流电机的底层PWM */
}

/**
  * 函    数：直流电机设置PWM
  * 参    数：n 1=左电机，2=右电机
  * 参    数：PWM 范围-100~100（负数为反转）
  * 返 回 值：无
  */
void Motor_SetSpeed(uint8_t n, int8_t PWM)
{
	if (n == 1)							/* 左电机 */
	{
		if (PWM >= 0)					/* 正转 */
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
			PWM_SetCompare1(PWM);		/* 设置PWM占空比 */
		}
		else							/* 反转 */
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
			PWM_SetCompare1(-PWM);
		}
	}
	else if (n == 2)						/* 右电机 */
	{
		if (PWM >= 0)					/* 正转 */
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
			PWM_SetCompare2(PWM);
		}
		else							/* 反转 */
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
			PWM_SetCompare2(-PWM);
		}
	}
}
