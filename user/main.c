#include "ucos_ii.h"
#include "led.h"
#include "mytask.h"
#include "FINAL.h"

OS_STK START_Stk[128]; 
INT8U Start_err;

void  mymain(void){
	OS_TRACE_INIT();
	OS_TRACE_START();
	OSInit();
  OSTaskCreate(StartTask, (void*)0, &START_Stk[127], 0);
	OSTaskNameSet(5, (INT8U*)(void*)"StartTask", &Start_err);
	OSStart();
}
