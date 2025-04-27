#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "misc.h"
#include "BLUETOOTH.h"
#include "ucos_ii.h"


void USART_GPIO_Init(void)
{ 
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;		//开启GPIOB时钟
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;		//开启USART1时钟

	GPIOB->MODER &= ~(0xF << 12);
	GPIOB->MODER |= GPIO_Mode_AF<<12 | GPIO_Mode_AF<<14;		//AF
	GPIOB->AFR[0] |= GPIO_AF_USART1<<24 | GPIO_AF_USART1<<28;		//AF:USART1
	GPIOB->OTYPER  |= GPIO_OType_PP<<6 | GPIO_OType_PP<<7;			//PP
	GPIOB->OSPEEDR |= GPIO_High_Speed<<12 | GPIO_High_Speed<<14;	
}

void USART_Config(uint16_t bps)
{
	
	USART1->CR1 &= ~(USART_CR1_UE);		//USART关闭
	USART1->CR1 &= ~USART_CR1_OVER8;		//16倍过采样
	USART1->CR1 &= ~USART_CR1_M;		//8位字长
	USART1->CR1 &= ~USART_CR1_PCE;		//禁止奇偶校验位
	USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;		//发送、接收使能
	
	USART1->CR2 &= ~USART_CR2_STOP;		//1个停止位
	
	float USARTDIV;
	static uint16_t DIV_M;			//整数部分
	uint16_t DIV_F;			//小数部分
	if((USART1->CR1 & USART_CR1_OVER8) == SET)			
	{
		USARTDIV = 84000000.0/8/bps;
		DIV_M = (uint32_t)USARTDIV;
		DIV_F = (USARTDIV - DIV_M)*8 + 0.5f;		//OVER8为1
	}
	else
	{
		USARTDIV = 84000000.0/16/bps;
		DIV_M = (uint32_t)USARTDIV;
		DIV_F = (USARTDIV - DIV_M)*16 + 0.5f;		//OVER8为0
	}
	USART1->BRR = DIV_M<<4 | DIV_F;
	
	USART1->CR1 |= USART_CR1_RXNEIE;		//开启USART中断
	
	SCB->AIRCR |= 5<<SCB_AIRCR_PRIGROUP_Pos;		//设置NVIC中断优先级分组
	NVIC->ISER[1] |= 1<<5;		//使能USART1到NVIC中断
	NVIC->IP[36] = 5;		//设置USART1中断响应优先级和抢占优先级
	
	USART1->CR1 |= USART_CR1_UE;		//开启USART
}


void bluetooth_Init(uint16_t bps){
	USART_GPIO_Init();
	USART_Config(bps);
}

void USART_SendByte(uint8_t Byte)
{
	while(!(USART1->SR & USART_SR_TC));		//等待上一次发送完成
	USART1->DR = Byte;
}

//uint8_t USART_ReceiveByte(void)
//{
//	uint8_t RxData;
//	while((USART1->SR & USART_SR_RXNE) == RESET);		//等待接收到数据
//	RxData = USART1->DR;
//	return RxData;
//}

void USART1_IRQHandler(void)
{
	OSIntEnter();
	if(USART1->SR & USART_SR_RXNE)		//DR寄存器不为空
	{
		RxData = USART1->DR;		//读取DR
		USART1->SR &= ~USART_SR_RXNE;		//清除RXNE标志位
	}		
	OSIntExit();
}
