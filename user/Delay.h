#ifndef _DELAY_H
#define _DELAY_H
#include "stm32f4xx.h"

void TIM_DELAY_INIT(void);
void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);
void Delay_s(uint32_t s);

#endif
