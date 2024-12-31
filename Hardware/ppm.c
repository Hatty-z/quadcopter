#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_exti.h"
#include "misc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_syscfg.h"

uint16_t PPM_Time = 0;
uint8_t PPM_CNT = 0;
extern uint16_t PPM[8];

void PPM_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);        //ѡ��GPIOB_6ΪEXTIԴ����
    
    TIM_TimeBaseInitTypeDef TimeBase_InitStructure;
    TimeBase_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TimeBase_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TimeBase_InitStructure.TIM_Period = 65536 - 1;        //ARR
    TimeBase_InitStructure.TIM_Prescaler = 100 - 1;                //PSC
    TimeBase_InitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TimeBase_InitStructure);
    
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line4;        //����EXTI��6�ж�
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;                //�ж�ģʽ
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;                //�½��ش���
    EXTI_Init(&EXTI_InitStructure);
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);        //����NVIC���ȼ�����
    
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;        //����EXTI5-9��NVIC�ж�ͨ��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;        //��Ӧ���ȼ�
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        //��ռ���ȼ�
    NVIC_Init(&NVIC_InitStructure);
    
    TIM_Cmd(TIM2, ENABLE);                //������ʱ��
}

void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        PPM_Time = TIM2->CNT;        //��ȡ������ֵ
        TIM2->CNT = 0;
        if(PPM_Time <= 2050)        //ʱ��С��2ms,��ͨ��ֵ
        {
            PPM[PPM_CNT++] = PPM_Time;
        }
        else
        {
            PPM_CNT = 0;
        }
    }
    EXTI_ClearITPendingBit(EXTI_Line4);
}