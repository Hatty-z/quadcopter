#include  <os.h>
#if (OS_APP_HOOKS_EN > 0)

void  App_TaskCreateHook (OS_TCB *ptcb)
{
    (void)ptcb;
}



void  App_TaskDelHook (OS_TCB *ptcb)
{
    (void)ptcb;
}


#if OS_VERSION >= 251
void  App_TaskIdleHook (void)
{
}
#endif


void  App_TaskStatHook (void)
{
}



#if OS_VERSION >= 289
void  App_TaskReturnHook (OS_TCB  *ptcb)
{
    (void)ptcb;
}
#endif



#if OS_TASK_SW_HOOK_EN > 0
void  App_TaskSwHook (void)
{

}
#endif



#if OS_VERSION >= 204
void  App_TCBInitHook (OS_TCB *ptcb)
{
    (void)ptcb;
}
#endif

#if OS_TIME_TICK_HOOK_EN > 0
void  App_TimeTickHook (void)
{

}
#endif
#endif
