#ifndef __I2C_H
#define __I2C_H

#include <stdint.h>

void I2c_Init(void);
void I2C_start(void);
void I2C_stop(void);
void I2C_SendByte(uint8_t Bite);		/* 发送字节 */
uint8_t I2C_ReceiveByte(void);			/* 接收字节 */
void I2C_SendACK(uint8_t ACK);			/* 发送应答 */
uint8_t I2C_ReceiveACK(void);			/* 接收应答 */

#endif
