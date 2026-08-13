#include "main.h"
#include <stdio.h>

UART_HandleTypeDef huart1;

volatile uint8_t USART_RX;
uint8_t USART_RXFlag;
uint8_t Usart1_RxByte;

/**
  * 函    数：USB串口初始化（USART1，PA9=TX，PA10=RX，9600bps）
  */
void USB_Init(void)
{
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;			/* 复用推挽输出 */
	GPIO_InitStructure.Pin = GPIO_PIN_9;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;			/* 上拉输入 */
	GPIO_InitStructure.Pin = GPIO_PIN_10;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;						/* 波特率 */
	huart1.Init.WordLength = UART_WORDLENGTH_8B;		/* 8位字长 */
	huart1.Init.StopBits = UART_STOPBITS_1;				/* 1位停止位 */
	huart1.Init.Parity = UART_PARITY_NONE;				/* 无校验 */
	huart1.Init.Mode = UART_MODE_TX_RX;					/* 收发模式 */
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;		/* 无硬件流控 */
	HAL_UART_Init(&huart1);

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);  /* FreeRTOS: full preemption, 16 levels */

	HAL_NVIC_SetPriority(USART1_IRQn, 2, 2);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

	HAL_UART_Receive_IT(&huart1, &Usart1_RxByte, 1);
}

void Send_Bite(uint8_t Bite)							/* 传输数字 */
{
	HAL_UART_Transmit(&huart1, &Bite, 1, 100);
}

void MyArry_Get(uint8_t *Arry, uint16_t Lengh)			/* 传输数组 */
{
	uint16_t i;
	for (i = 0; i < Lengh; i++)
	{
		Send_Bite(Arry[i]);
	}
}

void MyNumber_Get(char *Number)							/* 字符串 */
{
	uint8_t i;
	for (i = 0; Number[i] != 0; i++)
	{
		Send_Bite(Number[i]);
	}
}

int fputc(int ch, FILE *f)								/* printf函数 */
{
	Send_Bite(ch);
	return ch;
}

uint8_t USART_RXGetFlag(void)							/* 判断中断标志位 */
{
	if (USART_RXFlag == 1)
	{
		USART_RXFlag = 0;
		return 1;
	}
	return 0;
}

uint8_t USART_GetRX(void)								/* 传输的数据 */
{
	return USART_RX;
}
