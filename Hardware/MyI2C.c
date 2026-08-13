#include "main.h"
#include "Delay.h"

/* 软件I2C：SCL=PB10，SDA=PB11（开漏输出） */

void I2C_SCL_Write(uint8_t BiteVaule)	/* 改SCL */
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, (GPIO_PinState)BiteVaule);
	Delay_us(10);
}

void I2C_SDA_Write(uint8_t BiteVaule)	/* 改SDA */
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, (GPIO_PinState)BiteVaule);
	Delay_us(10);
}

uint8_t Read_SDA(void)					/* 读SDA */
{
	uint8_t Bitvalue;
	Bitvalue = (uint8_t)HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11);
	Delay_us(10);
	return Bitvalue;
}

void I2c_Init(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;	/* 开漏输出 */
	GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_11;
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10 | GPIO_PIN_11, GPIO_PIN_SET);
}

void I2C_start(void)
{
	I2C_SDA_Write(1);
	I2C_SCL_Write(1);
	I2C_SDA_Write(0);
	I2C_SCL_Write(0);
}

void I2C_stop(void)
{
	I2C_SDA_Write(0);
	I2C_SCL_Write(1);
	I2C_SDA_Write(1);
}

/* 发送一个字节 */
void I2C_SendByte(uint8_t Bite)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		I2C_SCL_Write(0);					/* 拉低SCL，准备改SDA */
		I2C_SDA_Write(Bite & 0x80 >> i);	/* 从最高位开始，依次放到SDA */
		I2C_SCL_Write(1);					/* 拉高SCL，从机采样 */
	}
	I2C_SCL_Write(0);						/* 拉低SCL，准备接收字节 */
}

/* 接收一个字节 */
uint8_t I2C_ReceiveByte(void)
{
	I2C_SDA_Write(1);						/* 释放SDA，让从机控制 */
	uint8_t i, Bite = 0x00;
	for (i = 0; i < 8; i++)
	{
		I2C_SCL_Write(1);					/* 拉高SCL，从机放数据到SDA */
		if (Read_SDA() == 1)				/* 读SDA */
		{
			Bite |= (0x80 >> i);			/* 对应位置1 */
		}
		I2C_SCL_Write(0);					/* 拉低SCL，从机准备下一位 */
	}
	return Bite;
}

void I2C_SendACK(uint8_t ACK)				/* 发送应答 */
{
	I2C_SDA_Write(ACK);
	I2C_SCL_Write(1);
	I2C_SCL_Write(0);
}

uint8_t I2C_ReceiveACK(void)				/* 接收应答 */
{
	I2C_SDA_Write(1);
	uint8_t ACK;
	I2C_SCL_Write(1);
	ACK = Read_SDA();
	I2C_SCL_Write(0);
	return ACK;
}
