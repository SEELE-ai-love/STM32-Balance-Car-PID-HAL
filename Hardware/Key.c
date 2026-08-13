#include "main.h"
#include "Delay.h"

/*全局变量，用于存储按键键码*/
uint8_t Key_Num;

/**
  * 函    数：按键初始化
  * 参    数：无
  * 返 回 值：无
  */
void Key_Init(void)
{
	/*开启时钟*/
	__HAL_RCC_GPIOB_CLK_ENABLE();		/* 开启GPIOB的时钟 */
	__HAL_RCC_GPIOA_CLK_ENABLE();		/* 开启GPIOA的时钟 */

	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_0;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);				/* PB1/PB0上拉输入 */

	GPIO_InitStructure.Pin = GPIO_PIN_5 | GPIO_PIN_4;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);				/* PA5/PA4上拉输入 */
}

/**
  * 函    数：获取全局变量定义的按键键码
  * 返 回 值：按键键码（读后清零）
  */
uint8_t Key_GetNum(void)
{
	uint8_t Temp;
	if (Key_Num)
	{
		Temp = Key_Num;
		Key_Num = 0;
		return Temp;
	}
	return 0;
}

/**
  * 函    数：获取按键状态（非阻塞）
  * 返 回 值：有按键按下返回键码，没有按下返回0
  */
uint8_t Key_GetState(void)
{
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET)	/* PB1为低电平 */
	{
		return 1;
	}
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_RESET)	/* PB0为低电平 */
	{
		return 2;
	}
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET)	/* PA5为低电平 */
	{
		return 3;
	}
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)	/* PA4为低电平 */
	{
		return 4;
	}
	return 0;
}

/**
  * 函    数：按键定时扫描（20ms一次）
  * 注意事项：此函数必须在主程序中每隔1ms自动执行一次
  */
void Key_Tick(void)
{
	static uint8_t Count;
	static uint8_t CurrState, PrevState;

	Count++;
	if (Count >= 20)
	{
		Count = 0;

		PrevState = CurrState;
		CurrState = Key_GetState();

		/* 检测到按键松手瞬间 */
		if (CurrState == 0 && PrevState != 0)
		{
			Key_Num = PrevState;
		}
	}
}
