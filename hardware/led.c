#include "led.h"
#include "stm32f4xx.h"

void led_init(void){
	RCC->AHB1ENR |=1;
	
	GPIOA->MODER|=1<<10; //MODER5:01 OUTPUT
	GPIOA->OTYPER &=~(1<<5); //OUTPUT TYPE:PP
	GPIOA->OSPEEDR |=3<<10;		//Very high speed
	
}

void led_on(void){
	GPIOA->BSRRL |=1<<5;
}

void led_off(void){
	GPIOA->BSRRH |=1<<5;
}
