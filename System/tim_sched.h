#ifndef __TIM_SCHED_H__
#define __TIM_SCHED_H__

#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "usart.h"

extern uint8_t attitude_flag, inner_flag, outer_flag, com_flag;

void TIM2_Init(void);

#endif
