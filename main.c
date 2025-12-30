#include "stm32f4xx.h"                  // Device header
#include "Service.h"

extern int8_t data[100];

int main()
{
	ALL_Init();
//	Ano_SendData(data, 7, 0x03);
	
	while(1)
	{
		PID_Test();
//		test_1();
		// printf("ch1:%d, ch2:%d, ch3:%d, ch4:%d\n", PPM[0], PPM[1], PPM[2], PPM[3]);
	}
}			
