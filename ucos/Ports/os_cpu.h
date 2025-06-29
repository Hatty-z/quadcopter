#ifndef  OS_CPU_H
#define  OS_CPU_H

#ifndef  OS_CPU_EXCEPT_STK_SIZE
#define  OS_CPU_EXCEPT_STK_SIZE      256u        /* Default exception stack size is 256 OS_STK entries */
#endif

//#ifndef  __TARGET_FPU_SOFTVFP
//#define  OS_CPU_ARM_FP_EN              1u
//#else
//#define  OS_CPU_ARM_FP_EN              0u
//#endif


#ifndef CPU_CFG_KA_IPL_BOUNDARY
#error  "CPU_CFG_KA_IPL_BOUNDARY        not #define'd in 'app_cfg.h'    "   /* See Note # 1 & 2        */
#else
#if (CPU_CFG_KA_IPL_BOUNDARY == 0u)
#error  "CPU_CFG_KA_IPL_BOUNDARY        should be > 0 "
#endif
#endif

#ifndef CPU_CFG_NVIC_PRIO_BITS
#error  "CPU_CFG_NVIC_PRIO_BITS         not #define'd in 'app_cfg.h'    "   /* See Note # 3            */
#else
#if (CPU_CFG_KA_IPL_BOUNDARY >= (1u << CPU_CFG_NVIC_PRIO_BITS))
#error  "CPU_CFG_KA_IPL_BOUNDARY        should not be set to higher than max programable priority level "
#endif
#endif

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned int   INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   int   INT32S;                   /* Signed   32 bit quantity                           */
//typedef float          FP32;                     /* Single precision floating point                    */
//typedef double         FP64;                     /* Double precision floating point                    */

typedef unsigned int   OS_STK;                   /* Each stack entry is 32-bit wide                    */
typedef unsigned int   OS_CPU_SR;                /* Define size of CPU status register (PSR = 32 bits) */


#define  OS_STK_GROWTH  1u
#define  OS_CRITICAL_METHOD   3u

#if OS_CRITICAL_METHOD == 3u
#define  OS_ENTER_CRITICAL()  do { cpu_sr = OS_CPU_SR_Save();} while (0)
#define  OS_EXIT_CRITICAL()   do { OS_CPU_SR_Restore(cpu_sr);} while (0)
#endif

  
#if OS_CRITICAL_METHOD == 3u                      /* See OS_CPU_A.ASM                                  */
OS_CPU_SR  OS_CPU_SR_Save         (void);
void       OS_CPU_SR_Restore      (OS_CPU_SR  cpu_sr);
#endif
void       OSCtxSw             (void);
void       OSIntCtxSw             (void);
void       OSStartHighRdy         (void);

#endif
