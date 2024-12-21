#ifndef _MOTOR_H
#define _MOTOR_H

void MOTOR_GPIO_Config(void);
void TIM1_Config(void);
void PWM_SetCompare(int motor, uint16_t compare);
void MOTOR_Init(void);
void MOTOR_SetSpeed(int moter,int8_t Speed);
void Unlock_ESC(int motor);


#endif