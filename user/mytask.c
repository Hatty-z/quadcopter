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
//			 OSTimeDly(1000);
    }
}

void Task2(void *p_arg)
{
    (void)p_arg;  

    while (1)
    {
       led_off();
//			 OSTimeDly(1800);
    }
}

OS_EVENT *UartMutex;    // UART mutex semphore
OS_EVENT *PPMSem;       // PPM semphore
INT8U err; //error code

OS_STK RC_Stk[128];
OS_STK DIP_Stk[128];
OS_STK led1stk[128];
OS_STK led2stk[128];

//to create other tasks
void StartTask(void *p_arg) {
    (void)p_arg;
	  INT8U RCControl_err,Attitude_err;
//	  INT8U ledon_err,ledoff_err;
    
    // init hardware
    ALL_Init();
//	 	led_init();
    
    UartMutex = OSMutexCreate(1, &err);  
    PPMSem = OSSemCreate(0);             
    
    if (UartMutex == (OS_EVENT *)0 || PPMSem == (OS_EVENT *)0) {
        // handle error
        while(1);
    }
    
    OSTaskCreate(Task_RCControl, (void*)0, &RC_Stk[127], 4);
    OSTaskCreate(Task_Attitude, (void*)0, &DIP_Stk[127], 5);
    
    OSTaskNameSet(4, (INT8U*)(void*)"RCControl", &RCControl_err);
    OSTaskNameSet(5, (INT8U*)(void*)"Attitude", &Attitude_err);
		
//		OSTaskCreate(Task1,(void*)0,&led1stk[127],6);
//		OSTaskCreate(Task2,(void*)0,&led2stk[127],7);
//    OSTaskNameSet(6,(INT8U*)(void*)"ledon",&ledon_err);
//		OSTaskNameSet(7,(INT8U*)(void*)"ledoff",&ledoff_err);
		
    // delete StartTask
    OSTaskDel(OS_PRIO_SELF);
}

//void Task_RCControl(void *p_arg){
//		(void)p_arg;  
//		while(1){
//			PWM_TEST();
//			OSTimeDly(30);
//		}
//}

void Task_RCControl(void *p_arg){
    (void)p_arg;
    while(1){
        OSSemPend(PPMSem, 100, &err);  // 100ms timeout
        
        if (err == OS_ERR_NONE) {
            PWM_TEST();
        }
        
//        OSTimeDly(30);  
    }
}

//void Task_Attitude(void *p_arg){
//		(void)p_arg;  
//		while(1){
//			GY86_Show();
//			OSTimeDly(51);
//		}
//}

void Task_Attitude(void *p_arg){
    (void)p_arg;
    
    while(1){
        //pend UartMutex
        OSMutexPend(UartMutex, 0, &err);
				//pend successively
        if (err == OS_ERR_NONE) {
            GY86_Show();
            OSMutexPost(UartMutex);
        }
        
//        OSTimeDly(51);  
    }
}



