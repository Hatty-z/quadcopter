#ifndef __DWT_TIME_H__
#define __DWT_TIME_H__

#include "stm32f4xx.h"

void DWT_Init(void);
uint32_t DWT_GetTime(void);

#endif
