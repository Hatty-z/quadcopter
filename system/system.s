	EXPORT myScaterload
 
	AREA |.text|,CODE,READONLY

	IMPORT |Image$$RW_IRAM1$$Base| ;data link addr
	IMPORT |Image$$RW_IRAM1$$Length| ;data length
	IMPORT |Load$$RW_IRAM1$$Base| ;data load addr
	IMPORT |Image$$RW_IRAM1$$ZI$$Base| ;bss
	IMPORT |Image$$RW_IRAM1$$ZI$$Length|
		
myScaterload	PROC
	LDR R0, = |Load$$RW_IRAM1$$Base| 
	LDR R1, = |Image$$RW_IRAM1$$Base| 
	LDR R2, = |Image$$RW_IRAM1$$Length| 
CopyData
	SUB R2, R2, #4 ;copy 4 bytes data
	LDR R3, [R0, R2] 
	STR R3, [R1, R2] 
	CMP R2, #0 
	BNE CopyData 
	
	LDR R0, = |Image$$RW_IRAM1$$ZI$$Base| 
	LDR R1, = |Image$$RW_IRAM1$$ZI$$Length| 
CleanBss
	SUB R1, R1, #4 ;每次清除4个字节的bss段数据
	MOV R3, #0 ;将0存入r3寄存器
	STR R3, [R0, R1] ;把R3寄存器存入到链接地址
	CMP R1, #0 ;将计数和0相比较
	BNE CleanBss ;如果不相等，跳转到CleanBss标签处，相等则往下执行
	
	BX LR
	ALIGN
	ENDP
		
	END
		