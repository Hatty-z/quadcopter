#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "oled.h"

// 外部声明汇编中的变量
extern volatile uint32_t PPM[8];
extern volatile uint32_t PPM_CNT;

extern void PPM_GPIO_Config(void);
extern void PPM_EXTI_Config(void);
extern void PPM_TIM3_Config(void);
extern void PPM_ISR(void);
extern void PPM_ResetCounter(void);

// 主函数
int main(void) {
    // 初始化硬件配置
    PPM_GPIO_Config();  // 配置 GPIO
    PPM_EXTI_Config();  // 配置外部中断
    PPM_TIM3_Config();  // 配置定时器

    // 启用全局中断
    __enable_irq();     // 开启中断（需要系统调用）

    OLED_Init();
    while (1) {
        // 显示 PPM 数据
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
