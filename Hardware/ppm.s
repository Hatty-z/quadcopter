	AREA PPMData, DATA, READWRITE   ; 定义数据段
		EXPORT PPM
		EXPORT PPM_CNT
PPM        DCD 0, 0, 0, 0, 0, 0, 0, 0    ; 初始化 PPM 数组为 8 个 32 位的 0
PPM_CNT    DCD 0            ; 存储当前的 PPM 通道计数器（最大值为 8）

	AREA PPMCode, CODE, READONLY    ; 定义代码段
		EXPORT PPM_GPIO_Config
		EXPORT PPM_EXTI_Config
		EXPORT PPM_TIM3_Config
		EXPORT PPM_ISR
		EXPORT PPM_SaveData
		EXPORT PPM_ResetCounter
		ENTRY

; 地址宏定义
RCC_BASE     EQU 0x40023800         ; RCC 基地址
RCC_AHB1ENR  EQU RCC_BASE + 0x30    ; RCC AHB1 时钟使能寄存器地址
RCC_APB1ENR  EQU RCC_BASE + 0x40
RCC_APB2ENR  EQU RCC_BASE + 0x44
GPIOA_BASE   EQU 0x40020000         ; GPIOA 基地址
GPIOA_MODER  EQU GPIOA_BASE + 0x00    
GPIOA_PUPDR  EQU GPIOA_BASE + 0x0C   
SYSCFG_BASE  EQU 0x40013800         ; SYSCFG 基地址 
SYSCFG_EXTICR2 EQU SYSCFG_BASE + 0x08 ; 
EXTI_BASE    EQU 0x40013C00         ; EXTI 基地址
EXTI_IMR     EQU EXTI_BASE + 0x00  
EXTI_RTSR    EQU EXTI_BASE + 0x08  
EXTI_PR      EQU EXTI_BASE + 0x14  
TIM3_BASE    EQU 0x40000400
TIM3_CR1     EQU TIM3_BASE+0x00
TIM3_CCMR1   EQU TIM3_BASE+0x18
TIM3_CCER    EQU TIM3_BASE+0x20
TIM3_CNT     EQU TIM3_BASE+0x24    
TIM3_PSC     EQU TIM3_BASE+0x28
TIM3_ARR     EQU TIM3_BASE+0x2C

PPM_GPIO_Config
	; 使能GPIO时钟
	LDR R0,=RCC_AHB1ENR  
	LDR R1,[R0]             
	ORR R1,R1,#0x01        
	STR R1,[R0]             

	; 配置输入模式
	LDR R0,=GPIOA_MODER
	LDR R1,[R0]
	BIC R1,R1,#(0x03<<12)
	STR R1,[R0]

	; 配置无上拉/下拉电阻
	LDR R0,=GPIOA_PUPDR
	LDR R1,[R0]
	BIC R1,R1,#(0x03<<12)
	STR R1,[R0]

PPM_EXTI_Config
	; 使能 SYSCFG 时钟
	LDR R0,=RCC_APB2ENR
	LDR R1,[R0]
	ORR R1,R1,#(1<<14)
	STR R1,[R0]

	; 配置 EXTI6 线路为 PA6
	LDR R0,=SYSCFG_EXTICR2
	LDR R1,[R0]
	BIC R1,R1,#(0x0F<<8)
	STR R1,[R0]

	; 配置 EXTI6 为上升沿触发
	LDR R0,=EXTI_RTSR
	LDR R1,[R0]
	ORR R1,R1,#(0x01<<6)
	STR R1,[R0]

	; 确保 EXTI6 中断不被屏蔽
	LDR R0,=EXTI_IMR
	LDR R1,[R0]
	ORR R1,R1,#(0x01<<6)
	STR R1,[R0]

PPM_TIM3_Config
	; 使能 TIM3 时钟
	LDR R0,=RCC_APB1ENR
	LDR R1,[R0]
	ORR R1,R1,#(0x01<<1)
	STR R1,[R0]

	; 禁用定时器
	LDR R0,=TIM3_CR1
	LDR R1,[R0]
	BIC R1,R1,#(0x01<<0)
	STR R1,[R0]

	; 配置时基单元
	LDR R0,=TIM3_PSC
	MOV R1,#(83)
	STR R1,[R0]

	LDR R0,=TIM3_ARR
	MOV R1,#0xFFFF
	STR R1,[R0]
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
	; 配置 TIM3 通道 1 为输入模式
	LDR R0, =TIM3_CCMR1           
	LDR R1, [R0]                  
	BIC R1, R1, #(0x03 << 0)     
	ORR R1, R1, #(0x01 << 0)      
	STR R1, [R0]                 

	; 配置 TIM3 通道 1 上升沿触发
	LDR R0, =TIM3_CCER           
	LDR R1, [R0]                  
	ORR R1, R1, #(1 << 0)         
	STR R1, [R0]                  

	; 启用定时器
	LDR R0,=TIM3_CR1
	LDR R1,[R0]
	ORR R1,R1,#(0x01<<0)
	STR R1,[R0]

PPM_ISR
	LDR R0, =EXTI_PR      ; 读取 EXTI 中断待处理寄存器
	LDR R1, [R0]
	TST R1, #0x40     ; 检查 EXTI6 中断标志
	BEQ PPM_ISR_EXIT

	; 清除中断标志
	ORR R1,R1,#0x40
	STR R1,[R0]

	; 读取计数器当前计数值
	LDR R0,=TIM3_CNT
	LDR R1, [R0]

	; 计数器清零
	LDR R0,=TIM3_CNT
	MOV R1, #0
	STR R1,[R0]

	; 调用保存数据函数
	BL PPM_SaveData

	; 退出中断服务程序
PPM_ISR_EXIT
	BX LR                   ; 返回中断服务程序

PPM_SaveData
	; 检查数组索引值是否小于8
	LDR R0,=PPM_CNT        
	LDR R1, [R0]           
	CMP R1, #8             
	BGE PPM_ResetCounter   ; 如果达到 8，重置计数器

	; 保存各通道数据
	LDR R0,=PPM           
	ADD R2, R0, R1, LSL #2 ; 计算当前数据保存地址 (每个元素为 4 字节)
	STR R1, [R2]          

	; 更新数组索引值
	ADD R1, R1, #1
	STR R1, [R0]         

	BX LR

PPM_ResetCounter
	LDR R0,=PPM_CNT        
	MOV R1, #0
	STR R1, [R0]           
	BX LR                

	END
