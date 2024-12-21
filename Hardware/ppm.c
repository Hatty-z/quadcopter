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
	RCC->AHB1ENR |=RCC_AHB1ENR_GPIOAEN;//ʹ��
	
	GPIOA->MODER &=~GPIO_MODER_MODER6;//����ģʽ
	GPIOA->MODER |= GPIO_MODER_MODER6_1;//����
	GPIOA->PUPDR &=~GPIO_PUPDR_PUPDR6;//��������������
	GPIOA->AFR[0] &= ~(0xF << (6 * 4));      
	GPIOA->AFR[0] |= (2 << (6 * 4)); // AF2
}
*/

//PB4
void PPM_GPIO_Config(void){
	RCC->AHB1ENR |=RCC_AHB1ENR_GPIOAEN;//ʹ��
	
	GPIOA->MODER &=~GPIO_MODER_MODER4;//����ģʽ
	GPIOA->MODER |= GPIO_MODER_MODER4_1;//����
	GPIOA->PUPDR &=~GPIO_PUPDR_PUPDR4;//��������������
	GPIOA->AFR[0] &= ~(0xF << (4 * 4));      
	GPIOA->AFR[0] |= (2 << (4 * 4)); // AF2
}

//TIM3CH1
void TIM3_Config(void){
	RCC->APB1ENR|=RCC_APB1ENR_TIM3EN;//ʹ��ʱ��
	
	TIM3->PSC=84-1;
	TIM3->ARR=0xFFFF;
	TIM3->CNT=0;

	TIM3->CCMR1 |= TIM_CCMR1_CC1S_0;        // ���벶��ģʽ��IC1ӳ�䵽TI1
    TIM3->CCER |= TIM_CCER_CC1E;            // ʹ�ܲ���
    TIM3->DIER |= TIM_DIER_CC1IE;           // ʹ�ܲ����ж�
	
	TIM3->SMCR |= TIM_SMCR_TS_2 | TIM_SMCR_TS_0;  // ����Դ��ͨ�� 1 ��������
    TIM3->SMCR |= TIM_SMCR_SMS_2;               // ��ģʽ����λģʽ
	
	TIM3->CR1 |= TIM_CR1_CEN;     // ������ʱ��
}


void INT_Config(void){
	/*
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	
	SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PA; // �� PA6 ���ӵ� EXTI6
	EXTI->IMR |= EXTI_IMR_IM6;//ʹ���ж���
	EXTI->RTSR |=EXTI_RTSR_TR6;//�����ش����ж�
	*/
	NVIC_EnableIRQ(TIM3_IRQn);              // ����TIM3�ж�
    NVIC_SetPriority(TIM3_IRQn, 1);         // �����ж����ȼ�
}

void PPM_Init(void){
	PPM_GPIO_Config();
	TIM3_Config();
	INT_Config();
}

void TIM3_IRQHandler(void){
	if(TIM3->SR &TIM_SR_CC1IF){
		uint16_t pulse_width=TIM3->CCR1;
		//��־����֡������������һ֡����
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

