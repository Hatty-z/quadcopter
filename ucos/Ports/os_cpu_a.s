.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb

// 导出符号
.global mySystemInit
.global OSCtxSw
.global OSIntCtxSw
.global PendSV_Handler
.global OSStartHighRdy
.global OS_CPU_SR_Save
.global OS_CPU_SR_Restore

// 外部符号
.extern OSTCBCur
.extern OSTCBHighRdy
.extern OSPrioCur
.extern OSPrioHighRdy
.extern OSRunning
.extern OSTaskSwHook

.section .text
.align 2

OS_CPU_SR_Save:
    cpsid   i                   // 关闭中断
    push    {r1}                // 保存 R1（后面使用到）
    mrs     r1, basepri         // 保存当前的 BASEPRI（中断屏蔽优先级）
    msr     basepri, r0         // 设置新的 BASEPRI —— 参数 R0 是新的屏蔽级别（更高的优先级）
    dsb                         // 数据同步屏障，确保指令顺序和内存访问顺序
    isb                         // 指令同步屏障，刷新流水线，确保后续执行的是新状态
    mov     r0, r1              // 返回旧的 BASEPRI（临界区前的中断状态）
    pop     {r1}                // 恢复 R1
    cpsie   i                   // 开启中断
    bx      lr                  // 返回

OS_CPU_SR_Restore:
    cpsid   i                   // 关闭中断
    msr     basepri, r0         // 恢复旧的 BASEPRI 值
    dsb
    isb
    cpsie   i                   // 开启中断
    bx      lr                  // 返回

// 常量定义
.equ SCB_SHPR3, 0xE000ED20

OSStartHighRdy:
    cpsid   i                   // 关闭中断
    
    // OSTCBCur = OSTCBHighRdy
    ldr     r0, =OSTCBCur
    ldr     r1, =OSTCBHighRdy
    ldr     r2, [r1]
    str     r2, [r0]
    
    // OSPrioCur = OSPrioHighRdy
    ldr     r0, =OSPrioCur
    ldr     r1, =OSPrioHighRdy
    ldrb    r2, [r1]            // INT8U类型
    strb    r2, [r0]
    
    // PENDSV PRIO(Pri14)
    ldr     r0, =SCB_SHPR3
    ldr     r1, [r0, #2]
    orr     r1, r1, #0xFF
    strb    r1, [r0, #2]
    
    // OSRunning = 1
    ldr     r0, =OSRunning
    mov     r1, #1
    strb    r1, [r0]
    
    // PSP init
    movs    r0, #0
    msr     psp, r0
    
    ldr     r0, =OSTCBCur
    ldr     r1, [r0]
    ldr     r2, [r1]
    msr     psp, r2
    
    // use PSP
    mrs     r1, control
    orr     r1, r1, #2
    msr     control, r1
    isb
    
    // TASK1 启动
    ldmfd   sp!, {r4-r11, lr}
    ldmfd   sp!, {r0-r3, r12, lr}
    ldmfd   sp!, {r1, r2}
    msr     xpsr, r2
    cpsie   i                   // 开启中断
    bx      r1                  // 跳转到任务

// 常量定义
.equ SCB_ICSR, 0xE000ED04

OSCtxSw:
    // pendsvset
    ldr     r0, =SCB_ICSR
    ldr     r1, [r0]
    orr     r1, r1, #(1<<28)
    str     r1, [r0]
    bx      lr

OSIntCtxSw:
    ldr     r0, =SCB_ICSR
    ldr     r1, [r0]
    orr     r1, r1, #(1<<28)
    str     r1, [r0]
    bx      lr

PendSV_Handler:
    // SAVE R4-R11,LR
    mrs     r0, psp
    stmfd   r0!, {r4-r11, lr}
    
    // SAVE SP
    ldr     r1, =OSTCBCur
    ldr     r2, [r1]
    str     r0, [r2]
    
    // UPDATE OSTCBCur
    ldr     r0, =OSTCBCur
    ldr     r1, =OSTCBHighRdy
    ldr     r2, [r1]
    str     r2, [r0]
    
    // UPDATE OSPrioCur
    ldr     r0, =OSPrioCur
    ldr     r1, =OSPrioHighRdy
    ldrb    r2, [r1]
    strb    r2, [r0]
    
    bl      OSTaskSwHook        // 调用任务切换钩子函数
    
    // NEW R4-R11,LR
    ldr     r1, =OSTCBCur
    ldr     r2, [r1]
    ldr     r2, [r2]
    
    ldmfd   r2!, {r4-r11, lr}
    msr     psp, r2
    bx      lr                  // Exception return will restore remaining context

// 寄存器基地址定义
.equ RCC_BASE,      0x40023800
.equ FLASH_BASE,    0x40023C00
.equ RCC_CR,        RCC_BASE+0x00
.equ RCC_PLLCFGR,   RCC_BASE+0x04
.equ RCC_CFGR,      RCC_BASE+0x08
.equ FLASH_ACR,     FLASH_BASE + 0x00
.equ STK_BASE,      0xE000E010
.equ STK_CTRL,      STK_BASE+0x00
.equ STK_LOAD,      STK_BASE+0x04
.equ STK_VAL,       STK_BASE+0x08
.equ STK_CALIB,     STK_BASE+0x0C

mySystemInit:
    // HSEON - 开启HSE
    ldr     r0, =RCC_CR
    ldr     r1, [r0]
    orr     r1, r1, #(1<<16)    // HSE ON
    orr     r1, r1, #(1<<18)    // 打开了HSE旁路
    str     r1, [r0]
    
    // HSERDY - 等待HSE稳定
hse_wait:
    ldr     r1, [r0]
    tst     r1, #(1<<17)
    beq     hse_wait
    
    // FLASH等待状态设置
    ldr     r0, =FLASH_ACR
    mov     r1, #0x02
    str     r1, [r0]
    
    // PLL配置：PLLM=8, PLLN=336, PLLP=4
    ldr     r0, =RCC_PLLCFGR
    mov     r1, #0x00000000
    orr     r1, r1, #8              // PLLM = 8
    orr     r1, r1, #(336 << 6)     // PLLN = 336
    orr     r1, r1, #(1 << 16)      // PLLP = 4
    orr     r1, r1, #(1 << 22)      // PLL Source = HSE
    str     r1, [r0]
    
    // PLLON - 开启PLL
    ldr     r0, =RCC_CR
    ldr     r1, [r0]
    orr     r1, r1, #(1<<24)
    str     r1, [r0]
    
    // PLL LOCK - 等待PLL锁定
pll_lock:
    ldr     r1, [r0]
    tst     r1, #(1<<25)
    beq     pll_lock
    
    // AHB, APB1/2 分频设置
    ldr     r0, =RCC_CFGR
    ldr     r1, [r0]
    bic     r1, r1, #(0xF << 4)     // Clear HPRE[7:4]  AHB
    bic     r1, r1, #(0x7 << 10)    // Clear PPRE1[12:10]
    bic     r1, r1, #(0x7 << 13)    // Clear PPRE2[15:13] APB2
    orr     r1, r1, #(4<<10)        // APB1分频
    str     r1, [r0]
    
    // PLL AS CLOCK SOURCE - 设置PLL为系统时钟源
    ldr     r1, [r0]
    orr     r1, r1, #0x02
    str     r1, [r0]
    
    // WAIT PLL AS CLOCK SOURCE - 等待PLL成为系统时钟源
pll_wait:
    ldr     r1, [r0]
    and     r1, r1, #0xC            // Mask SWS
    cmp     r1, #0x8                // SWS = 0b10 << 2
    bne     pll_wait
    
    // SYSTICK配置
    // Counter enable + exception disable + clock AHB
    ldr     r0, =STK_CTRL
    mov     r1, #0x05
    str     r1, [r0]
    
    // load重载值
    ldr     r0, =STK_LOAD
    ldr     r1, =84000-1
    str     r1, [r0]
    
    // CLEAR VAL
    ldr     r0, =STK_VAL
    mov     r1, #0
    str     r1, [r0]
    
    // Counter enable + exception enable + clock AHB
    ldr     r0, =STK_CTRL
    mov     r1, #0x07
    str     r1, [r0]
    
    // SysTick优先级设置
    ldr     r0, =SCB_SHPR3
    mov     r1, #(5<<24)
    str     r1, [r0]
    
    cpsie   i                   // 打开全局中断
    
    bx      lr                  // 返回

.end

//	EXPORT mySystemInit
//	EXPORT OSCtxSw
//	EXPORT OSIntCtxSw
//	EXPORT PendSV_Handler
//	EXPORT OSStartHighRdy                                      
//    EXPORT OS_CPU_SR_Save
//    EXPORT OS_CPU_SR_Restore
//		
//	EXTERN OSTCBCur
//	EXTERN OSTCBHighRdy
//	EXTERN OSPrioCur
//	EXTERN OSPrioHighRdy
//	EXTERN OSRunning
//
//
//	AREA |.text|,CODE,READONLY
//	PRESERVE8
//OS_CPU_SR_Save
//    CPSID   I         
//    PUSH   {R1}        ; 保存 R1（后面使用到）
//    MRS     R1, BASEPRI ; 保存当前的 BASEPRI（中断屏蔽优先级）
//    MSR     BASEPRI, R0 ; 设置新的 BASEPRI —— 参数 R0 是新的屏蔽级别（更高的优先级）
//    DSB               ; 数据同步屏障，确保指令顺序和内存访问顺序
//    ISB               ; 指令同步屏障，刷新流水线，确保后续执行的是新状态
//    MOV     R0, R1     ; 返回旧的 BASEPRI（临界区前的中断状态）
//    POP    {R1}        ; 恢复 R1
//    CPSIE   I        
//    BX      LR       
//
//
//OS_CPU_SR_Restore
//    CPSID   I          
//    MSR     BASEPRI, R0 ; 恢复旧的 BASEPRI 值
//    DSB
//    ISB
//    CPSIE   I
//    BX      LR
//	
//	
//SCB_SHPR3	   EQU 0XE000ED20
//OSStartHighRdy
//	CPSID I
//	; OSTCBCur=OSTCBHighRdy
//	LDR R0,=OSTCBCur
//	LDR R1,=OSTCBHighRdy
//	LDR R2,[R1]
//	STR R2,[R0]
//	;OSPrioCur=OSPrioHighRdy
//	LDR R0,=OSPrioCur
//	LDR R1,=OSPrioHighRdy
//	LDRB R2,[R1] 		;INT8U类型
//	STRB R2,[R0]
//	
//	;PENDSV PRIO(Pri14)
//	LDR R0,=SCB_SHPR3
//	LDR R1,[R0,#2]
//	ORR R1,R1,#0XFF
//	STRB R1,[R0,#2]
//	
//	;OSRUNNING
//	LDR R0,=OSRunning
//	MOV R1,#1
//	STRB R1,[R0]
//	
//	;PSP init
//	MOVS R0,#0
//	MSR PSP,R0
//	
//	LDR R0,=OSTCBCur
//	LDR R1,[R0]
//	LDR R2,[R1]
//	MSR PSP,R2
//	;use PSP
//	MRS R1,CONTROL
//	ORR R1,R1,#2
//	MSR CONTROL, R1
//	ISB 
//	
//	;TASK1 
//	LDMFD SP!,{R4-R11,LR}
//	LDMFD SP!,{R0-R3,R12,LR}
//;	LDMFD SP!,{R0-R3}
//;	LDMFD SP!,{R12,LR}
//	LDMFD SP!,{R1,R2}
//	MSR XPSR,R2
//	CPSIE    I
//    BX       R1
//	
//
//
//SCB_ICSR 	  EQU 0XE000ED04
//
//OSCtxSw	
//	;pendsvset
//	LDR R0,=SCB_ICSR
//	LDR R1,[R0]
//	ORR R1,R1,#(1<<28)
//	STR R1,[R0]
//	BX LR
//
//OSIntCtxSw
//	LDR R0,=SCB_ICSR
//	LDR R1,[R0]
//	ORR R1,R1,#(1<<28)
//	STR R1,[R0]
//	BX LR
//
//	EXTERN OSTaskSwHook
//PendSV_Handler
//	;SAVE R4-R11,LR
//	MRS R0,psp
//	STMFD R0!,{R4-R11,LR}
//	
//	;SAVE SP
//	LDR R1,=OSTCBCur
//	LDR R2,[R1]
//	STR R0,[R2]
//	;UPDATE OSTCBCur
//	LDR R0,=OSTCBCur
//	LDR R1,=OSTCBHighRdy
//	LDR R2,[R1]
//	STR R2,[R0]
//	;UPDATE OSPrioCur
//	LDR R0,=OSPrioCur
//	LDR R1,=OSPrioHighRdy
//	LDRB R2,[R1]				
//	STRB R2,[R0]	
//
//	BL      OSTaskSwHook 
//
//	;NEW R4-R11,LR
//	LDR R1,=OSTCBCur
//	LDR R2,[R1]
//	LDR R2,[R2]
//	
//	
//	LDMFD R2!,{R4-R11,LR}	
//	MSR psp,R2
//	BX LR
//
//                                                ; Exception return will restore remaining context
//
//RCC_BASE       EQU 0x40023800
//FLASH_BASE     EQU 0x40023C00	
//RCC_CR         EQU RCC_BASE+0x00
//RCC_PLLCFGR    EQU RCC_BASE+0x04
//RCC_CFGR       EQU RCC_BASE+0x08
//FLASH_ACR      EQU FLASH_BASE + 0x00
//STK_BASE   	   EQU 0XE000E010
//STK_CTRL   	   EQU STK_BASE+0X00
//STK_LOAD   	   EQU STK_BASE+0X04
//STK_VAL   	   EQU STK_BASE+0X08
//STK_CALIB      EQU STK_BASE+0X0C	
//
//
//mySystemInit
//	;HSEON
//	LDR R0,=RCC_CR
//	LDR R1,[R0]
//	ORR R1,R1,#(1<<16)
//	ORR R1,R1,#(1<<18) ;打开了HSE旁路
//	STR R1,[R0]
//	
//	;HSERDY
//hse_wait
//	LDR R1,[R0]
//	TST R1,#(1<<17)
//	BEQ hse_wait
//
//	;FLASH
//	LDR R0, =FLASH_ACR
//	MOV R1, #0x02           
//	STR R1, [R0]
//
//	;PLLM=16,PLLN=336,PLLP=4
//	LDR R0,=RCC_PLLCFGR
//	MOV R1,#0X00000000
//    ORR R1, R1, #8        	    ; PLLM = 8
//    ORR R1, R1, #(336 << 6)     ; PLLN = 336
//    ORR R1, R1, #(1 << 16)      ; PLLP = 4
//    ORR R1, R1, #(1 << 22)      ; PLL Source = HSE
//	STR R1,[R0]
//
//	;PLLON
//	LDR R0,=RCC_CR
//	LDR R1,[R0]
//	ORR R1,R1,#(1<<24)
//	STR R1,[R0]
//	
//	;PLL LOCK
//pll_lock
//	LDR R1,[R0]
//	TST R1,#(1<<25)
//	BEQ pll_lock	
//	
//	;AHB,APB1/2
//	LDR R0,=RCC_CFGR
//	LDR R1,[R0]
//	BIC R1, R1, #(0xF << 4)       ; Clear HPRE[7:4]  AHB
//	BIC R1, R1, #(0x7 << 10)      ; Clear PPRE1[12:10]
//	BIC R1, R1, #(0x7 << 13)      ; Clear PPRE2[15:13] APB2
//	ORR R1,R1,#(4<<10) ;APB1
//	STR R1,[R0]
//	
//	;PLL AS CLOCK SOURCE
//	LDR R1,[R0]
//	ORR R1,R1,#0x02
//	STR R1,[R0]
//	
//	;WAIT PLL AS CLOCK SOURCE
//pll_wait
//	LDR R1,[R0]
//	AND R1, R1, #0xC            ; Mask SWS
//    CMP R1, #0x8                ; SWS = 0b10 << 2
//	BNE pll_wait
//	
//	;SYSTICK
//	;Counter enable+exception disable+clock AHB
//	LDR R0,=STK_CTRL
//	MOV R1,#0X05
//	STR R1,[R0]
//	;load
//	LDR R0,=STK_LOAD
//	LDR R1,=84000-1
//	STR R1,[R0]
//	;CLEAR VAL
//	LDR R0,=STK_VAL
//	MOV R1,#0
//	STR R1,[R0]
//	;Counter enable+exception enable+clock AHB
//	LDR R0,=STK_CTRL
//	MOV R1,#0X07
//	STR R1,[R0]
//	
//	;PRI
//	LDR R0,=SCB_SHPR3
//	MOV R1,#(5<<24)
//	STR R1,[R0]
//	
//	CPSIE I	;打开全局中断
//	
//	BX LR
//	
//	ALIGN
//
//
//	END
