.syntax unified
.cpu cortex-m4
.thumb

.section .text
.align 2
.global myScaterload
.type myScaterload, %function

myScaterload:
    // 复制初始化数据段 (.data section)
    // 从 FLASH 加载地址复制到 RAM 运行地址
    ldr r0, =_sidata        // 数据段在FLASH中的加载地址
    ldr r1, =_sdata         // 数据段在RAM中的起始地址  
    ldr r2, =_edata         // 数据段在RAM中的结束地址
    
    // 计算数据段长度
    sub r2, r2, r1          // r2 = _edata - _sdata (数据段长度)
    
CopyData:
    cmp r2, #0              // 检查是否还有数据需要复制
    beq InitBss             // 如果数据复制完成，跳转到BSS初始化
    
    sub r2, r2, #4          // 每次复制4字节，长度减4
    ldr r3, [r0, r2]        // 从FLASH加载地址读取数据
    str r3, [r1, r2]        // 存储到RAM运行地址
    b CopyData              // 继续复制下一个4字节

InitBss:
    // 清零BSS段
    ldr r0, =_sbss          // BSS段起始地址
    ldr r1, =_ebss          // BSS段结束地址
    sub r1, r1, r0          // r1 = BSS段长度
    mov r3, #0              // 用于清零的值

CleanBss:
    cmp r1, #0              // 检查是否还有BSS需要清零
    beq Done                // 如果BSS清零完成，跳转到结束
    
    sub r1, r1, #4          // 每次清零4字节，长度减4
    str r3, [r0, r1]        // 将0写入BSS段
    b CleanBss              // 继续清零下一个4字节

Done:
    bx lr                   // 返回调用者

.size myScaterload, .-myScaterload
.end

	
//	EXPORT myScaterload
// 
//	AREA |.text|,CODE,READONLY
//
//	IMPORT |Image$$RW_IRAM1$$Base| ;data link addr
//	IMPORT |Image$$RW_IRAM1$$Length| ;data length
//	IMPORT |Load$$RW_IRAM1$$Base| ;data load addr
//	IMPORT |Image$$RW_IRAM1$$ZI$$Base| ;bss
//	IMPORT |Image$$RW_IRAM1$$ZI$$Length|
//		
//myScaterload	PROC
//	LDR R0, = |Load$$RW_IRAM1$$Base| 
//	LDR R1, = |Image$$RW_IRAM1$$Base| 
//	LDR R2, = |Image$$RW_IRAM1$$Length| 
//CopyData
//	SUB R2, R2, #4 ;copy 4 bytes data
//	LDR R3, [R0, R2] 
//	STR R3, [R1, R2] 
//	CMP R2, #0 
//	BNE CopyData 
//	
//	LDR R0, = |Image$$RW_IRAM1$$ZI$$Base| 
//	LDR R1, = |Image$$RW_IRAM1$$ZI$$Length| 
//CleanBss
//	SUB R1, R1, #4 ;每次清除4个字节的bss段数据
//	MOV R3, #0 ;将0存入r3寄存器
//	STR R3, [R0, R1] ;把R3寄存器存入到链接地址
//	CMP R1, #0 ;将计数和0相比较
//	BNE CleanBss ;如果不相等，跳转到CleanBss标签处，相等则往下执行
//	
//	BX LR
//	ALIGN
//	ENDP
//		
//	END
		