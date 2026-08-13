#ifndef __USB_H
#define __USB_H

#include <stdint.h>
#include <stdio.h>

extern volatile uint8_t USART_RX;
extern uint8_t USART_RXFlag;

void Send_Bite(uint8_t Bite);
void USB_Init(void);
void MyArry_Get(uint8_t *Arry, uint16_t Lengh);	/* ´«ÊäÊý×é */
void MyNumber_Get(char *Number);				/* ×Ö·û´® */
int fputc(int ch, FILE *f);
uint8_t USART_GetRX(void);
uint8_t USART_RXGetFlag(void);

#endif
