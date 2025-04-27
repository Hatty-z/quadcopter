#ifndef _MOTOR_H
#define _MOTOR_H

#include "stm32f4xx.h"

void MOTOR_GPIO_Config(void);
void TIM1_Config(void);
void PWM_SetCompare(int motor, uint16_t compare);
void MOTOR_Init(void);
void MOTOR_SetSpeed(int moter,int8_t Speed);
void Unlock_ESC(int motor);
void Unlock_ALL_ESC(void);

#endif
