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
//	ALL_Init();
	OS_TRACE_INIT();
	OS_TRACE_START();
//	SEGGER_SYSVIEW_Print("SystemView Initialized OK!");
	OSInit();
//	OSTaskCreate(Task_RCControl,(void*)0,&RC_Stk[127],4);
//  OSTaskCreate(Task_Attitude,(void*)0,&DIP_Stk[127],5);
	OSTaskCreate(Task1,(void*)0,&led1stk[127],4);
	OSTaskCreate(Task2,(void*)0,&led2stk[127],5);
//	INT8U RCControl_err,Attitude_err;
	INT8U ledon_err,ledoff_err;
//	OSTaskNameSet(4,(INT8U*)(void*)"RCControl",&RCControl_err);
//	OSTaskNameSet(5,(INT8U*)(void*)"Attitude",&Attitude_err);
	OSTaskNameSet(4,(INT8U*)(void*)"ledon",&ledon_err);
	OSTaskNameSet(5,(INT8U*)(void*)"ledoff",&ledoff_err);
	OSStart();
}
