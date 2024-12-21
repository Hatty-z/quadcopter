#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "Delay.h"

#define NUM_CHANNELS 8
extern uint16_t channel_values[NUM_CHANNELS]; 
uint8_t channel_index=0; 
uint8_t PPM_OK=0;

/*
//PA6
void PPM_GPIO_Config(void){
	RCC->AHB1ENR |=RCC_AHB1ENR_GPIOAEN;//使能
	
	GPIOA->MODER &=~GPIO_MODER_MODER6;//输入模式
	GPIOA->MODER |= GPIO_MODER_MODER6_1;//复用
	GPIOA->PUPDR &=~GPIO_PUPDR_PUPDR6;//无上拉下拉电阻
	GPIOA->AFR[0] &= ~(0xF << (6 * 4));      
	GPIOA->AFR[0] |= (2 << (6 * 4)); // AF2
}
*/

//PB4
void PPM_GPIO_Config(void){
	RCC->AHB1ENR |=RCC_AHB1ENR_GPIOAEN;//使能
	
	GPIOA->MODER &=~GPIO_MODER_MODER4;//输入模式
	GPIOA->MODER |= GPIO_MODER_MODER4_1;//复用
	GPIOA->PUPDR &=~GPIO_PUPDR_PUPDR4;//无上拉下拉电阻
	GPIOA->AFR[0] &= ~(0xF << (4 * 4));      
	GPIOA->AFR[0] |= (2 << (4 * 4)); // AF2
}

//TIM3CH1
void TIM3_Config(void){
	RCC->APB1ENR|=RCC_APB1ENR_TIM3EN;//使能时钟
	
	TIM3->PSC=84-1;
	TIM3->ARR=0xFFFF;
	TIM3->CNT=0;

	TIM3->CCMR1 |= TIM_CCMR1_CC1S_0;        // 输入捕获模式，IC1映射到TI1
    TIM3->CCER |= TIM_CCER_CC1E;            // 使能捕获
    TIM3->DIER |= TIM_DIER_CC1IE;           // 使能捕获中断
	
	TIM3->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_0;  // 触发源：通道 1 的上升沿
    TIM3->SMCR |= TIM_SMCR_SMS_2;               // 从模式：复位模式
	
	TIM3->CR1 |= TIM_CR1_CEN;     // 启动定时器
}


void INT_Config(void){
	/*
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PA; // 将 PA6 连接到 EXTI6
	EXTI->IMR |= EXTI_IMR_IM6;//使能中断线
	EXTI->RTSR |=EXTI_RTSR_TR6;//上升沿触发中断
	*/
	NVIC_EnableIRQ(TIM3_IRQn);              // 启用TIM3中断
    NVIC_SetPriority(TIM3_IRQn, 1);         // 设置中断优先级
}

void PPM_Init(void){
	PPM_GPIO_Config();
	TIM3_Config();
	INT_Config();
}

void TIM3_IRQHandler(void){
	if(TIM3->SR &TIM_SR_CC1IF){
		uint16_t pulse_width=TIM3->CCR1;
		//标志数据帧结束，开启新一帧捕获
		if(pulse_width>=4000){
			PPM_OK=1;
			channel_index=0;		
		}else if(PPM_OK==1){
			if(pulse_width<=2100){
				if(channel_index<NUM_CHANNELS){
					channel_values[channel_index]=pulse_width;
					channel_index++;
				}else{
					PPM_OK=0;
					channel_index=0;
				}
			}
		}
	}
	TIM3->SR &=~TIM_SR_CC1IF;
}

