#include "ucos_ii.h"
#include "led.h"
#include "mytask.h"
#include "FINAL.h"

OS_STK led1stk[128];
OS_STK led2stk[128];
OS_STK RC_Stk[128];
OS_STK DIP_Stk[128];

void  mymain(void){
	led_init();
	ALL_Init();
	OSInit();
	OSTaskCreate(Task1,(void*)0,&led1stk[127],6);
	OSTaskCreate(Task2,(void*)0,&led2stk[127],7);
	OSTaskCreate(Task_RCControl,(void*)0,&RC_Stk[127],4);
  OSTaskCreate(Task_Display,(void*)0,&DIP_Stk[127],5);
	OSStart();
}
