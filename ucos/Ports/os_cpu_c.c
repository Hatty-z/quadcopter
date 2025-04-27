#include  <ucos_ii.h>

void OSInitHookBegin (void){
}
void OSInitHookEnd (void){
}
void OSTimeTickHook (void){
}
void OSTaskIdleHook (void){
}
void OSTaskStatHook (void){
}
void OSTCBInitHook (OS_TCB *ptcb){
}
void OSTaskCreateHook (OS_TCB *ptcb){
}
OS_STK  *OSTaskStkInit (void    (*task)(void *p_arg),
                        void     *p_arg,
                        OS_STK   *ptos,
                        INT16U    opt)
{
    OS_STK  *p_stk;


    (void)opt;                                                  /* 'opt' is not used, prevent warning                   */
    p_stk      = ptos +1u ;                                     /* Load stack pointer                                   */
                                                                /* Align the stack to 8-bytes.                          */
    p_stk      = (OS_STK *)((OS_STK)(p_stk) & 0xFFFFFFF8u);
                                                                /* Registers stacked as if auto-saved on exception      */
#if (OS_CPU_ARM_FP_EN > 0u)                                     /* FPU auto-saved registers.                            */
     --p_stk;
    *(--p_stk) = (OS_STK)0x02000000u;                           /* FPSCR                                                */
                                                                /* Initialize S0-S15 floating point registers           */
    *(--p_stk) = (OS_STK)0x41700000u;                           /* S15                                                  */
    *(--p_stk) = (OS_STK)0x41600000u;                           /* S14                                                  */
    *(--p_stk) = (OS_STK)0x41500000u;                           /* S13                                                  */
    *(--p_stk) = (OS_STK)0x41400000u;                           /* S12                                                  */
    *(--p_stk) = (OS_STK)0x41300000u;                           /* S11                                                  */
    *(--p_stk) = (OS_STK)0x41200000u;                           /* S10                                                  */
    *(--p_stk) = (OS_STK)0x41100000u;                           /* S9                                                   */
    *(--p_stk) = (OS_STK)0x41000000u;                           /* S8                                                   */
    *(--p_stk) = (OS_STK)0x40E00000u;                           /* S7                                                   */
    *(--p_stk) = (OS_STK)0x40C00000u;                           /* S6                                                   */
    *(--p_stk) = (OS_STK)0x40A00000u;                           /* S5                                                   */
    *(--p_stk) = (OS_STK)0x40800000u;                           /* S4                                                   */
    *(--p_stk) = (OS_STK)0x40400000u;                           /* S3                                                   */
    *(--p_stk) = (OS_STK)0x40000000u;                           /* S2                                                   */
    *(--p_stk) = (OS_STK)0x3F800000u;                           /* S1                                                   */
    *(--p_stk) = (OS_STK)0x00000000u;                           /* S0                                                   */
#endif
    *(--p_stk) = (OS_STK)0x01000000uL;                          /* xPSR                                                 */
    *(--p_stk) = (OS_STK)task;                                  /* Entry Point                                      */
    *(--p_stk) = (OS_STK)OS_TaskReturn;                         /* R14 (LR)                                             */
    *(--p_stk) = (OS_STK)0x12121212uL;                          /* R12                                                  */
    *(--p_stk) = (OS_STK)0x03030303uL;                          /* R3                                                   */
    *(--p_stk) = (OS_STK)0x02020202uL;                          /* R2                                                   */
    *(--p_stk) = (OS_STK)0x01010101uL;                          /* R1                                                   */
    *(--p_stk) = (OS_STK)p_arg;                                 /* R0 : argument                                        */

#if (OS_CPU_ARM_FP_EN > 0u)
    *(--p_stk) = (OS_STK)0xFFFFFFEDuL;                          /* R14: EXEC_RETURN; See Note 5                         */
#else
    *(--p_stk) = (OS_STK)0xFFFFFFFDuL;                          /* R14: EXEC_RETURN; See Note 5                         */
#endif
                                                                /* Remaining registers saved on process stack           */
    *(--p_stk) = (OS_STK)0x11111111uL;                          /* R11                                                  */
    *(--p_stk) = (OS_STK)0x10101010uL;                          /* R10                                                  */
    *(--p_stk) = (OS_STK)0x09090909uL;                          /* R9                                                   */
    *(--p_stk) = (OS_STK)0x08080808uL;                          /* R8                                                   */
    *(--p_stk) = (OS_STK)0x07070707uL;                          /* R7                                                   */
    *(--p_stk) = (OS_STK)0x06060606uL;                          /* R6                                                   */
    *(--p_stk) = (OS_STK)0x05050505uL;                          /* R5                                                   */
    *(--p_stk) = (OS_STK)0x04040404uL;                          /* R4                                                   */

#if (OS_CPU_ARM_FP_EN > 0u)
                                                                /* Initialize S16-S31 floating point registers          */
    *(--p_stk) = (OS_STK)0x41F80000u;                           /* S31                                                  */
    *(--p_stk) = (OS_STK)0x41F00000u;                           /* S30                                                  */
    *(--p_stk) = (OS_STK)0x41E80000u;                           /* S29                                                  */
    *(--p_stk) = (OS_STK)0x41E00000u;                           /* S28                                                  */
    *(--p_stk) = (OS_STK)0x41D80000u;                           /* S27                                                  */
    *(--p_stk) = (OS_STK)0x41D00000u;                           /* S26                                                  */
    *(--p_stk) = (OS_STK)0x41C80000u;                           /* S25                                                  */
    *(--p_stk) = (OS_STK)0x41C00000u;                           /* S24                                                  */
    *(--p_stk) = (OS_STK)0x41B80000u;                           /* S23                                                  */
    *(--p_stk) = (OS_STK)0x41B00000u;                           /* S22                                                  */
    *(--p_stk) = (OS_STK)0x41A80000u;                           /* S21                                                  */
    *(--p_stk) = (OS_STK)0x41A00000u;                           /* S20                                                  */
    *(--p_stk) = (OS_STK)0x41980000u;                           /* S19                                                  */
    *(--p_stk) = (OS_STK)0x41900000u;                           /* S18                                                  */
    *(--p_stk) = (OS_STK)0x41880000u;                           /* S17                                                  */
    *(--p_stk) = (OS_STK)0x41800000u;                           /* S16                                                  */
#endif

    return (p_stk);
}

void OSTaskDelHook (OS_TCB *ptcb){
}
void OSTaskReturnHook (OS_TCB *ptcb){
}

void SysTick_Handler(void)
{
		#if OS_CRITICAL_METHOD == 3u 
			OS_CPU_SR cpu_sr;
		#endif
		OS_ENTER_CRITICAL();
		OSIntEnter(); 
		OS_EXIT_CRITICAL();
		OSTimeTick();
		OSIntExit();
}
