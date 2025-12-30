#ifndef __SERVICE_H__
#define __SERVICE_H__

#include "stm32f4xx.h"                  // Device header
#include "GY86.h"
#include "pwm.h"
#include "usart.h"
#include "OLED.h"
#include "PPM.h"
#include "Madgwick.h"
#include "PID.h"
#include "Delay.h"
#include "tim_sched.h"
#include "dwt_time.h"
#include <stdio.h>
#include <stdbool.h>

void ALL_Init(void);
void GY86_Show(void);
void test_1(void);
void Madgwick_Test(void);
void PID_Test(void);
void EulerBias_Correct(void);

#endif
