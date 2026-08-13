#include "main.h"
#include "MyI2C.h"
#include "MPU_Reg.h"

void MPU_Wrinte(uint8_t Address, uint8_t *Bite, uint8_t Lengh)	/* 在MPU指定地址写数据 */
{
	I2C_start();
	I2C_SendByte(0x68 << 1);			/* 设备地址，I2C通信左移一位空出读写位 */
	I2C_ReceiveACK();					/* 接收应答 */

	I2C_SendByte(Address);
	I2C_ReceiveACK();					/* 接收应答 */

	for (uint8_t i = 0; i < Lengh; i++)
	{
		I2C_SendByte(Bite[i]);
		I2C_ReceiveACK();				/* 接收应答 */
	}
	I2C_stop();
}

void Receive_Bite(uint8_t Address, uint8_t *Bite, uint8_t Lengh)	/* 在MPU指定地址读数据 */
{
	I2C_start();
	I2C_SendByte(0x68 << 1);
	I2C_ReceiveACK();					/* 接收应答 */

	I2C_SendByte(Address);				/* 指定寄存器 */
	I2C_ReceiveACK();					/* 接收应答 */

	I2C_start();
	I2C_SendByte(0x68 << 1 | 0x01);		/* 读操作 */
	I2C_ReceiveACK();					/* 接收应答 */

	for (uint8_t i = 0; i < Lengh - 1; i++)
	{
		Bite[i] = I2C_ReceiveByte();
		I2C_SendACK(0);					/* 读写位是0，表示继续 */
	}
	Bite[Lengh - 1] = I2C_ReceiveByte();
	I2C_SendACK(1);
	I2C_stop();
}

void MPU_Init(void)
{
	I2c_Init();

	uint8_t Data = 0x01;
	MPU_Wrinte(PWR_MGMT_1, &Data, 1);

	uint8_t Bite = 0x00;
	MPU_Wrinte(PWR_MGMT_2, &Bite, 1);

	uint8_t BBC = 0x06;
	MPU_Wrinte(MPU6050_SMPLRT_DIV, &BBC, 1);

	uint8_t BBA = 0x07;
	MPU_Wrinte(MPU6050_CONFIG, &BBA, 1);

	uint8_t ABC = 0x00;
	MPU_Wrinte(MPU6050_GYRO_CONFIG, &ABC, 1);

	uint8_t AAD = 0x18;
	MPU_Wrinte(MPU6050_ACCEL_CONFIG, &AAD, 1);
}

void MPU_Read_All(int16_t *accel_x, int16_t *accel_y, int16_t *accel_z,
                 int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z)
{
	uint8_t accel_buf[6], gyro_buf[6];	/* 存放角速度和加速度 */

	Receive_Bite(MPU6050_ACCEL_XOUT_H, accel_buf, 6);
	*accel_x = (accel_buf[0] << 8) | accel_buf[1];
	*accel_y = (accel_buf[2] << 8) | accel_buf[3];
	*accel_z = (accel_buf[4] << 8) | accel_buf[5];

	Receive_Bite(MPU6050_GYRO_XOUT_H, gyro_buf, 6);
	*gyro_x = (gyro_buf[0] << 8) | gyro_buf[1];
	*gyro_y = (gyro_buf[2] << 8) | gyro_buf[3];
	*gyro_z = (gyro_buf[4] << 8) | gyro_buf[5];
}
