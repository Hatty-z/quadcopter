#ifndef APP_CFDG_H
#define APP_CFDG_H

#define  OS_TASK_TMR_PRIO                  (OS_LOWEST_PRIO - 2u)
#ifndef CPU_CFG_KA_IPL_BOUNDARY
#define CPU_CFG_KA_IPL_BOUNDARY     4u 
#endif

#ifndef CPU_CFG_NVIC_PRIO_BITS
#define CPU_CFG_NVIC_PRIO_BITS      4u 
#endif
#endif
