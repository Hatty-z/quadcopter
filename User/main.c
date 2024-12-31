#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "oled.h"

// �ⲿ��������еı���
extern volatile uint32_t PPM[8];
extern volatile uint32_t PPM_CNT;

extern void PPM_GPIO_Config(void);
extern void PPM_EXTI_Config(void);
extern void PPM_TIM3_Config(void);
extern void PPM_ISR(void);
extern void PPM_ResetCounter(void);

// ������
int main(void) {
    // ��ʼ��Ӳ������
    PPM_GPIO_Config();  // ���� GPIO
    PPM_EXTI_Config();  // �����ⲿ�ж�
    PPM_TIM3_Config();  // ���ö�ʱ��

    // ����ȫ���ж�
    __enable_irq();     // �����жϣ���Ҫϵͳ���ã�

    OLED_Init();
    while (1) {
        // ��ʾ PPM ����
        OLED_ShowNum(1, 1, PPM[0], 4);
        OLED_ShowNum(1, 5, PPM[1], 4);
        OLED_ShowNum(2, 1, PPM[2], 4);
        OLED_ShowNum(2, 5, PPM[3], 4);
        OLED_ShowNum(3, 1, PPM[4], 4);
        OLED_ShowNum(3, 5, PPM[5], 4);
        OLED_ShowNum(4, 1, PPM[6], 4);
        OLED_ShowNum(4, 5, PPM[7], 4);		
    }
}
