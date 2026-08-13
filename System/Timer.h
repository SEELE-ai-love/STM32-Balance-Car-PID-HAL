#ifndef __TIMER_H
#define __TIMER_H

#include <stdint.h>

void Timer_Init(void);

extern volatile uint32_t system_tick;
extern volatile uint32_t Time_Count;
extern volatile uint32_t Time_Flag;

#endif
