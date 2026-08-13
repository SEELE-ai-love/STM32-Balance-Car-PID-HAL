#include "main.h"
#include "Delay.h"
#include "HC_SR04.h"

/**
  * 函    数：HC_SR04超声波模块初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：TRIG配置为推挽输出，ECHO配置为下拉输入
  */
void HC_SR04_Init(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = HC_SR04_TRIG_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(HC_SR04_TRIG_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = HC_SR04_ECHO_PIN;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(HC_SR04_ECHO_GPIO, &GPIO_InitStructure);

	HAL_GPIO_WritePin(HC_SR04_TRIG_GPIO, HC_SR04_TRIG_PIN, GPIO_PIN_RESET);
}

/**
  * 函    数：获取超声波测得的距离
  * 参    数：无
  * 返 回 值：距离，单位cm；测量超时返回-1
  * 说    明：阻塞式测量，一次测量最长约60ms
  *          距离 = 声波往返时间(us) / 58 (cm)
  *          使用DWT内核计数器计时，不占用SysTick
  */
float HC_SR04_GetDistance(void)
{
	uint32_t TimeOut;
	uint32_t StartCnt;
	uint32_t Ticks;
	uint32_t us;

	/* 使能DWT内核计数器（72MHz下1计数 = 1/72us） */
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	/* 1. TRIG输出至少10us的高电平触发测距 */
	HAL_GPIO_WritePin(HC_SR04_TRIG_GPIO, HC_SR04_TRIG_PIN, GPIO_PIN_SET);
	Delay_us(10);
	HAL_GPIO_WritePin(HC_SR04_TRIG_GPIO, HC_SR04_TRIG_PIN, GPIO_PIN_RESET);

	/* 2. 等待ECHO变高，带超时保护（约60ms，超出视为无回波） */
	TimeOut = 0;
	while (HAL_GPIO_ReadPin(HC_SR04_ECHO_GPIO, HC_SR04_ECHO_PIN) == GPIO_PIN_RESET)
	{
		if (++TimeOut > 60000)
		{
			return -1.0f;
		}
	}

	/* 3. 使用DWT测量ECHO高电平时间 */
	DWT->CYCCNT = 0;
	StartCnt = DWT->CYCCNT;
	while (HAL_GPIO_ReadPin(HC_SR04_ECHO_GPIO, HC_SR04_ECHO_PIN) == GPIO_PIN_SET)
	{
		/* 溢出保护，约60ms，超出测量范围 */
		if ((DWT->CYCCNT - StartCnt) > (72U * 60000U))
		{
			break;
		}
	}
	Ticks = DWT->CYCCNT - StartCnt;			/* 高电平持续的总计数 */

	/* 4. 换算距离：us / 58 = cm */
	us = Ticks / 72U;
	return (float)us / 58.0f;
}
