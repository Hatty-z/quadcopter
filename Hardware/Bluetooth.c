#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include <stdio.h>

uint8_t Rxdata;

//PB6-TX-�������������PB7-RX-������������
void USART_GPIO_Config(void){
	RCC->AHB1ENR |=RCC_AHB1ENR_GPIOBEN;//ʹ��
	
	GPIOB ->MODER &=~(GPIO_MODER_MODER6|GPIO_MODER_MODER7);//����
	GPIOB->MODER |=GPIO_MODER_MODER6_1|GPIO_MODER_MODER7_1;//����
	GPIOB->OTYPER &=~GPIO_OTYPER_OT_6;//����
	GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR6|GPIO_OSPEEDER_OSPEEDR7); // ����
	GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR6_1 | GPIO_OSPEEDER_OSPEEDR7_1); // ����
	GPIOB->PUPDR &=~GPIO_PUPDR_PUPDR7;
	GPIOB->PUPDR |=GPIO_PUPDR_PUPDR7_0;//����
	GPIOB->AFR[0] &= ~((0xF << 24) | (0xF << 28));
	GPIOB->AFR[0] |= (7 << (4 * 6)) | (7 << (4 * 7)) ; // ����Ϊ AF7��UISART��
}

//UASRT1
void USART_Config(void){
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;//ʹ��
	
	USART1->CR1&=~USART_CR1_UE;
	USART1->BRR=546<<4|7<<0;//16������Ƶ�ʣ�84MHzʱ��
	USART1->CR3&=~USART_CR3_CTSE;//����ҪӲ��������
	USART1->CR1|=USART_CR1_TE|USART_CR1_RE;
	USART1->CR1 |= USART_CR1_TXEIE|USART_CR1_RXNEIE;// ���ý����жϣ������ж�
	USART1->CR1&=~(USART_CR1_M|USART_CR1_PCE);//8bit,��У��
	USART1->CR2&=~USART_CR2_STOP;//1��ֹͣλ
	USART1->CR1|= USART_CR1_UE;//����
}


void USART_NVIC_Config(void){
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn, 1);  // ������ռ���ȼ�1
}

void Bluetooth_Init(void){
	USART_GPIO_Config();
	USART_Config();
	USART_NVIC_Config();
}


void Serial_SendByte(uint8_t Byte){
	while(USART1->SR & USART_SR_TC)
		USART1->DR=Byte;
}

int fputc(int ch, FILE *f)
{
    Serial_SendByte(ch);
	return ch;
}

void USART1_IRQHandler(void){
	if(USART1->SR & USART_SR_RXNE)
		Rxdata=USART1->DR;
		USART1->SR&=~USART_SR_RXNE;
}
