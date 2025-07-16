#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#include "stm32f4xx.h"

extern uint8_t RxData;

void USART_GPIO_Init(void);
void USART_Config(uint16_t bps);
void bluetooth_Init(uint16_t bps);
void USART_SendByte(uint8_t Byte);
uint8_t USART_ReceiveByte(void);

#endif
