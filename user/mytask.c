#include "mytask.h"
#include "led.h"
#include "final.h"
#include "ucos_ii.h"
#include "stm32f4xx.h"
#include "oled.h"
#include "motor.h"

void Task1(void *p_arg)
{
    (void)p_arg;  

    while (1)
    {
       led_on();
			 OSTimeDly(1000);
    }
}

void Task2(void *p_arg)
{
    (void)p_arg;  

    while (1)
    {
       led_off();
			 OSTimeDly(1800);
    }
}



void Task_RCControl(void *p_arg){
		(void)p_arg;  
		while(1){
			PWM_TEST();
			OSTimeDly(5);
		}
}




void Task_Sensor(void *p_arg){
		(void)p_arg;  
		while(1){
			
		}
}


void Task_Display(void *p_arg){
		(void)p_arg;  
		while(1){
			GY86_Show();
			OSTimeDly(5);
		}
}

void Task_Comm(void *p_arg){
		(void)p_arg;  
	   
		while(1){
			
		}
}

