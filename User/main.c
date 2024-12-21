#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "bluetooth.h"
#include "oled.h"
#include "delay.h"
#include "ppm.h"

uint16_t channel_values[8]={0};

int main (void){     

	OLED_Init();
	PPM_Init();
	
	while(1)
	{
		OLED_ShowNum(1,1,channel_values[3],4);
	}
}
