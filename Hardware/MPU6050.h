#ifndef __MPU_H
#define __MPU_H

#include <stdint.h>

void MPU_Init(void);
void Receive_Bite(uint8_t Address, uint8_t *Bite, uint8_t Lengh);	/* 在MPU指定地址读数据 */
void MPU_Wrinte(uint8_t Address, uint8_t *Bite, uint8_t Lengh);		/* 在MPU指定地址写数据 */
void MPU_Read_All(int16_t *accel_x, int16_t *accel_y, int16_t *accel_z,
                 int16_t *gyro_x, int16_t *gyro_y, int16_t *gyro_z);

#endif
