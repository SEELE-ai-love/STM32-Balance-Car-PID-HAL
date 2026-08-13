#include "main.h"
#include <stdio.h>
#include <stdarg.h>
#include "USB.h"

UART_HandleTypeDef huart2;

char BlueSerial_RxPacket[100];
uint8_t BlueSerial_RxFlag;
static uint8_t BlueSerial_RxByte;

/* 来自USB.c（USART1）的句柄与接收字节，用于合并接收回调 */
extern UART_HandleTypeDef huart1;
extern uint8_t Usart1_RxByte;

/**
  * 函    数：蓝牙串口初始化（USART2，PA2=TX，PA3=RX，9600bps）
  */
void BlueSerial_Init(void)
{
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Pin = GPIO_PIN_2;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.Pin = GPIO_PIN_3;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	huart2.Instance = USART2;
	huart2.Init.BaudRate = 9600;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	HAL_UART_Init(&huart2);

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);  /* FreeRTOS: full preemption, 16 levels */

	HAL_NVIC_SetPriority(USART2_IRQn, 1, 1);
	HAL_NVIC_EnableIRQ(USART2_IRQn);

	HAL_UART_Receive_IT(&huart2, &BlueSerial_RxByte, 1);
}

void BlueSerial_SendByte(uint8_t Byte)
{
	HAL_UART_Transmit(&huart2, &Byte, 1, 100);
}

void BlueSerial_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i++)
	{
		BlueSerial_SendByte(Array[i]);
	}
}

void BlueSerial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		BlueSerial_SendByte(String[i]);
	}
}

uint32_t BlueSerial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

void BlueSerial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)
	{
		BlueSerial_SendByte(Number / BlueSerial_Pow(10, Length - i - 1) % 10 + '0');
	}
}

void BlueSerial_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	BlueSerial_SendString(String);
}

/**
  * 函    数：串口接收完成回调（合并处理USART1和USART2）
  * 说    明：USART2=蓝牙调参数据包，USART1=USB串口单字节接收
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)
	{
		USART_RX = Usart1_RxByte;
		USART_RXFlag = 1;
		HAL_UART_Receive_IT(&huart1, &Usart1_RxByte, 1);
	}
	else if (huart->Instance == USART2)
	{
		static uint8_t RxState = 0;
		static uint8_t pRxPacket = 0;
		uint8_t RxData = BlueSerial_RxByte;

		if (RxState == 0)
		{
			if (RxData == '[' && BlueSerial_RxFlag == 0)
			{
				RxState = 1;
				pRxPacket = 0;
			}
		}
		else if (RxState == 1)
		{
			if (RxData == ']')
			{
				RxState = 0;
				BlueSerial_RxPacket[pRxPacket] = '\0';
				BlueSerial_RxFlag = 1;
			}
			else
			{
				BlueSerial_RxPacket[pRxPacket] = RxData;
				pRxPacket++;
			}
		}

		HAL_UART_Receive_IT(&huart2, &BlueSerial_RxByte, 1);
	}
}
