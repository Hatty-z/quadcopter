#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "misc.h"
#include "BLUETOOTH.h"
#include "ucos_ii.h"


void USART_GPIO_Init(void)
{ 
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;		//����GPIOBʱ��
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;		//����USART1ʱ��

	GPIOB->MODER &= ~(0xF << 12);
	GPIOB->MODER |= GPIO_Mode_AF<<12 | GPIO_Mode_AF<<14;		//AF
	GPIOB->AFR[0] |= GPIO_AF_USART1<<24 | GPIO_AF_USART1<<28;		//AF:USART1
	GPIOB->OTYPER  |= GPIO_OType_PP<<6 | GPIO_OType_PP<<7;			//PP
	GPIOB->OSPEEDR |= GPIO_High_Speed<<12 | GPIO_High_Speed<<14;	
}

void USART_Config(uint16_t bps)
{
	
	USART1->CR1 &= ~(USART_CR1_UE);		//USART�ر�
	USART1->CR1 &= ~USART_CR1_OVER8;		//16��������
	USART1->CR1 &= ~USART_CR1_M;		//8λ�ֳ�
	USART1->CR1 &= ~USART_CR1_PCE;		//��ֹ��żУ��λ
	USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;		//���͡�����ʹ��
	
	USART1->CR2 &= ~USART_CR2_STOP;		//1��ֹͣλ
	
	float USARTDIV;
	static uint16_t DIV_M;			//��������
	uint16_t DIV_F;			//С������
	if((USART1->CR1 & USART_CR1_OVER8) == SET)			
	{
		USARTDIV = 84000000.0/8/bps;
		DIV_M = (uint32_t)USARTDIV;
		DIV_F = (USARTDIV - DIV_M)*8 + 0.5f;		//OVER8Ϊ1
	}
	else
	{
		USARTDIV = 84000000.0/16/bps;
		DIV_M = (uint32_t)USARTDIV;
		DIV_F = (USARTDIV - DIV_M)*16 + 0.5f;		//OVER8Ϊ0
	}
	USART1->BRR = DIV_M<<4 | DIV_F;
	
	USART1->CR1 |= USART_CR1_RXNEIE;		//����USART�ж�
	
	SCB->AIRCR |= 5<<SCB_AIRCR_PRIGROUP_Pos;		//����NVIC�ж����ȼ�����
	NVIC->ISER[1] |= 1<<5;		//ʹ��USART1��NVIC�ж�
	NVIC->IP[36] = 5;		//����USART1�ж���Ӧ���ȼ�����ռ���ȼ�
	
	USART1->CR1 |= USART_CR1_UE;		//����USART
}


void bluetooth_Init(uint16_t bps){
	USART_GPIO_Init();
	USART_Config(bps);
}

void USART_SendByte(uint8_t Byte)
{
	while(!(USART1->SR & USART_SR_TC));		//�ȴ���һ�η������
	USART1->DR = Byte;
}

//uint8_t USART_ReceiveByte(void)
//{
//	uint8_t RxData;
//	while((USART1->SR & USART_SR_RXNE) == RESET);		//�ȴ����յ�����
//	RxData = USART1->DR;
//	return RxData;
//}

void USART1_IRQHandler(void)
{
	OSIntEnter();
	if(USART1->SR & USART_SR_RXNE)		//DR�Ĵ�����Ϊ��
	{
		RxData = USART1->DR;		//��ȡDR
		USART1->SR &= ~USART_SR_RXNE;		//���RXNE��־λ
	}		
	OSIntExit();
}
