#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

#include <stdio.h>
void USART_GPIO_Config(void);
void USART_Config(void);
void USART_NVIC_Config(void);
void Bluetooth_Init(void);
void Serial_SendByte(uint8_t Byte);
void USART1_IRQHandler(void);

#endif
