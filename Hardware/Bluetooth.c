#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include <stdio.h>

uint8_t Rxdata;

//PB6-TX-复用推挽输出，PB7-RX-复用上拉输入
void USART_GPIO_Config(void){
	RCC->AHB1ENR |=RCC_AHB1ENR_GPIOBEN;//使能
	
	GPIOB ->MODER &=~(GPIO_MODER_MODER6|GPIO_MODER_MODER7);//清零
	GPIOB->MODER |=GPIO_MODER_MODER6_1|GPIO_MODER_MODER7_1;//复用
	GPIOB->OTYPER &=~GPIO_OTYPER_OT_6;//推挽
	GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR6|GPIO_OSPEEDER_OSPEEDR7); // 清零
	GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR6_1 | GPIO_OSPEEDER_OSPEEDR7_1); // 中速
	GPIOB->PUPDR &=~GPIO_PUPDR_PUPDR7;
	GPIOB->PUPDR |=GPIO_PUPDR_PUPDR7_0;//上拉
	GPIOB->AFR[0] &= ~((0xF << 24) | (0xF << 28));
	GPIOB->AFR[0] |= (7 << (4 * 6)) | (7 << (4 * 7)) ; // 设置为 AF7（UISART）
}

//UASRT1
void USART_Config(void){
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;//使能
	
	USART1->CR1&=~USART_CR1_UE;
	USART1->BRR=546<<4|7<<0;//16倍采样频率，84MHz时钟
	USART1->CR3&=~USART_CR3_CTSE;//不需要硬件流控制
	USART1->CR1|=USART_CR1_TE|USART_CR1_RE;
	USART1->CR1 |= USART_CR1_TXEIE|USART_CR1_RXNEIE;// 启用接收中断，发送中断
	USART1->CR1&=~(USART_CR1_M|USART_CR1_PCE);//8bit,无校验
	USART1->CR2&=~USART_CR2_STOP;//1个停止位
	USART1->CR1|= USART_CR1_UE;//启动
}


void USART_NVIC_Config(void){
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn, 1);  // 设置抢占优先级1
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
