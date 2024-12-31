#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "FINAL.h"
#include "oled.h"
/*
// 外部声明汇编中的变量
extern volatile uint32_t PPM[8];
extern volatile uint32_t PPM_CNT;

extern void PPM_GPIO_Config(void);
extern void PPM_EXTI_Config(void);
extern void PPM_TIM3_Config(void);
extern void PPM_ISR(void);
extern void PPM_ResetCounter(void);

*/

int main(void) {
    ALL_Init();
	OLED_Init();
	
	while(1)
	{
		OLED_ShowNum(1,2,8,1);
		test();		//蓝牙发送01显示mpu6050加速度数据，发送02显示mpu6050角速度数据，发送03显示hmc5883l数据
	}
}
