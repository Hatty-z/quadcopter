#include "stm32f4xx.h"

//TIM2
void TIM_DELAY_INIT(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseInitTypeDef TimeBase_InitStructure;
	TimeBase_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TimeBase_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TimeBase_InitStructure.TIM_Prescaler = 84 - 1;		//PSC
	TimeBase_InitStructure.TIM_Period = 0xFFFFFFFF;		//ARR
	TimeBase_InitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TimeBase_InitStructure);
	
	TIM_Cmd(TIM2, ENABLE);
}

static inline uint32_t GetMicros(void) {
    return TIM2->CNT;
}

void Delay_us(uint32_t us){
//	SysTick->LOAD=84*us-1;  
//	SysTick->VAL=0;
//	SysTick->CTRL=SysTick_CTRL_CLKSOURCE_Msk|SysTick_CTRL_ENABLE_Msk;
//	while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
//	SysTick->CTRL=0;
	
	uint32_t start=GetMicros();
	while ((GetMicros() - start) < us) {
        __NOP(); //  avoid compiler optimize
  }
}

void Delay_ms(uint32_t ms){
	while(ms--){
		Delay_us(1000);
	}
}

void Delay_s(uint32_t s){
	while(s--){
		Delay_ms(1000);
	}
}
