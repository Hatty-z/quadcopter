	AREA PPMData, DATA, READWRITE   ; �������ݶ�
		EXPORT PPM
		EXPORT PPM_CNT
PPM        DCD 0, 0, 0, 0, 0, 0, 0, 0    ; ��ʼ�� PPM ����Ϊ 8 �� 32 λ�� 0
PPM_CNT    DCD 0            ; �洢��ǰ�� PPM ͨ�������������ֵΪ 8��

	AREA PPMCode, CODE, READONLY    ; ��������
		EXPORT PPM_GPIO_Config
		EXPORT PPM_EXTI_Config
		EXPORT PPM_TIM3_Config
		EXPORT PPM_ISR
		EXPORT PPM_SaveData
		EXPORT PPM_ResetCounter
		ENTRY

; ��ַ�궨��
RCC_BASE     EQU 0x40023800         ; RCC ����ַ
RCC_AHB1ENR  EQU RCC_BASE + 0x30    ; RCC AHB1 ʱ��ʹ�ܼĴ�����ַ
RCC_APB1ENR  EQU RCC_BASE + 0x40
RCC_APB2ENR  EQU RCC_BASE + 0x44
GPIOA_BASE   EQU 0x40020000         ; GPIOA ����ַ
GPIOA_MODER  EQU GPIOA_BASE + 0x00    
GPIOA_PUPDR  EQU GPIOA_BASE + 0x0C   
SYSCFG_BASE  EQU 0x40013800         ; SYSCFG ����ַ 
SYSCFG_EXTICR2 EQU SYSCFG_BASE + 0x08 ; 
EXTI_BASE    EQU 0x40013C00         ; EXTI ����ַ
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
	; ʹ��GPIOʱ��
	LDR R0,=RCC_AHB1ENR  
	LDR R1,[R0]             
	ORR R1,R1,#0x01        
	STR R1,[R0]             

	; ��������ģʽ
	LDR R0,=GPIOA_MODER
	LDR R1,[R0]
	BIC R1,R1,#(0x03<<12)
	STR R1,[R0]

	; ����������/��������
	LDR R0,=GPIOA_PUPDR
	LDR R1,[R0]
	BIC R1,R1,#(0x03<<12)
	STR R1,[R0]

PPM_EXTI_Config
	; ʹ�� SYSCFG ʱ��
	LDR R0,=RCC_APB2ENR
	LDR R1,[R0]
	ORR R1,R1,#(1<<14)
	STR R1,[R0]

	; ���� EXTI6 ��·Ϊ PA6
	LDR R0,=SYSCFG_EXTICR2
	LDR R1,[R0]
	BIC R1,R1,#(0x0F<<8)
	STR R1,[R0]

	; ���� EXTI6 Ϊ�����ش���
	LDR R0,=EXTI_RTSR
	LDR R1,[R0]
	ORR R1,R1,#(0x01<<6)
	STR R1,[R0]

	; ȷ�� EXTI6 �жϲ�������
	LDR R0,=EXTI_IMR
	LDR R1,[R0]
	ORR R1,R1,#(0x01<<6)
	STR R1,[R0]

PPM_TIM3_Config
	; ʹ�� TIM3 ʱ��
	LDR R0,=RCC_APB1ENR
	LDR R1,[R0]
	ORR R1,R1,#(0x01<<1)
	STR R1,[R0]

	; ���ö�ʱ��
	LDR R0,=TIM3_CR1
	LDR R1,[R0]
	BIC R1,R1,#(0x01<<0)
	STR R1,[R0]

	; ����ʱ����Ԫ
	LDR R0,=TIM3_PSC
	MOV R1,#(83)
	STR R1,[R0]

	LDR R0,=TIM3_ARR
	MOV R1,#0xFFFF
	STR R1,[R0]
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
	; ���� TIM3 ͨ�� 1 Ϊ����ģʽ
	LDR R0, =TIM3_CCMR1           
	LDR R1, [R0]                  
	BIC R1, R1, #(0x03 << 0)     
	ORR R1, R1, #(0x01 << 0)      
	STR R1, [R0]                 

	; ���� TIM3 ͨ�� 1 �����ش���
	LDR R0, =TIM3_CCER           
	LDR R1, [R0]                  
	ORR R1, R1, #(1 << 0)         
	STR R1, [R0]                  

	; ���ö�ʱ��
	LDR R0,=TIM3_CR1
	LDR R1,[R0]
	ORR R1,R1,#(0x01<<0)
	STR R1,[R0]

PPM_ISR
	LDR R0, =EXTI_PR      ; ��ȡ EXTI �жϴ�����Ĵ���
	LDR R1, [R0]
	TST R1, #0x40     ; ��� EXTI6 �жϱ�־
	BEQ PPM_ISR_EXIT

	; ����жϱ�־
	ORR R1,R1,#0x40
	STR R1,[R0]

	; ��ȡ��������ǰ����ֵ
	LDR R0,=TIM3_CNT
	LDR R1, [R0]

	; ����������
	LDR R0,=TIM3_CNT
	MOV R1, #0
	STR R1,[R0]

	; ���ñ������ݺ���
	BL PPM_SaveData

	; �˳��жϷ������
PPM_ISR_EXIT
	BX LR                   ; �����жϷ������

PPM_SaveData
	; �����������ֵ�Ƿ�С��8
	LDR R0,=PPM_CNT        
	LDR R1, [R0]           
	CMP R1, #8             
	BGE PPM_ResetCounter   ; ����ﵽ 8�����ü�����

	; �����ͨ������
	LDR R0,=PPM           
	ADD R2, R0, R1, LSL #2 ; ���㵱ǰ���ݱ����ַ (ÿ��Ԫ��Ϊ 4 �ֽ�)
	STR R1, [R2]          

	; ������������ֵ
	ADD R1, R1, #1
	STR R1, [R0]         

	BX LR

PPM_ResetCounter
	LDR R0,=PPM_CNT        
	MOV R1, #0
	STR R1, [R0]           
	BX LR                

	END
