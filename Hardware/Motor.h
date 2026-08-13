#ifndef __MOTOR_H
#define __MOTOR_H

#include <stdint.h>

void Motor_Init(void);
void Motor_SetSpeed(uint8_t n, int8_t PWM);

#endif
