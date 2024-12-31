#include "stm32f4xx.h"                  // Device header
#include "GY86.h"
#include "MOTOR.h"
#include "BLUETOOTH.h"
#include "OLED.h"
#include "PPM.h"
#include "Delay.h"
#include <stdio.h>
#include <stdbool.h>

uint16_t PPM[8];
uint8_t RxData;

MPU6050_DataTypeDef MPU6050_Data;
HMC5883L_DataTypeDef HMC_Data;

void ALL_Init(void)
{
	MOTOR_Init();
	Unlock_ALL_ESC();
	GY86_Init();
	OLED_Init();
	PPM_Init();
	USART_Config(9600);
}

void GY86_Show(void)	
{
	MPU6050_GetData(&MPU6050_Data);
	HMC5883L_GetData(&HMC_Data);
	switch(RxData)
	{
		case 01:
		{
			OLED_ShowString(1, 1, "AccX: ");
			OLED_ShowString(2, 1, "AccY: ");
			OLED_ShowString(3, 1, "AccZ: ");
			OLED_ShowSignedNum(1, 7, MPU6050_Data.Acc_X, 5);
			OLED_ShowSignedNum(2, 7, MPU6050_Data.Acc_Y, 5);
			OLED_ShowSignedNum(3, 7, MPU6050_Data.Acc_Z, 5);
			break;
		}
		case 02:
		{
			OLED_ShowString(1, 1, "GyroX:");
			OLED_ShowString(2, 1, "GyroY:");
			OLED_ShowString(3, 1, "GyroZ:");
			OLED_ShowSignedNum(1, 7, MPU6050_Data.Gyro_X, 5);
			OLED_ShowSignedNum(2, 7, MPU6050_Data.Gyro_Y, 5);
			OLED_ShowSignedNum(3, 7, MPU6050_Data.Gyro_Z, 5);
			break;
		}
		case 03:
		{
			OLED_ShowString(1, 1, "MagX: ");
			OLED_ShowString(2, 1, "MagY: ");
			OLED_ShowString(3, 1, "MagZ: ");
			OLED_ShowSignedNum(1, 7, HMC_Data.Mag_X, 5);
			OLED_ShowSignedNum(2, 7, HMC_Data.Mag_Y, 5);
			OLED_ShowSignedNum(3, 7, HMC_Data.Mag_Z, 5);
			break;
		}
	}
}

void test(void)
{
	GY86_Show();
	PWM_SetCompare(1, PPM[2]);
	PWM_SetCompare(2, PPM[2]);
	PWM_SetCompare(3, PPM[2]);
	PWM_SetCompare(4, PPM[2]);
}
