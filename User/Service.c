#include "Service.h"

// #define attitude_dt 0.005f
// #define inner_dt 0.005f
// #define outer_dt 0.01f

#define SystemCoreClock 84000000

const float RAD2DEG = 180.0f / 3.14159265359f;

uint16_t PPM[8];
uint8_t RxData;
int8_t data[100];
float vofa_data[4];
float euler_bias[2] = {-2.7f, -6.4f};
// float euler_bias[2] = {-1.7f, -12.4f};

MPU6050_DataTypeDef MPU6050_Data;
HMC5883L_DataTypeDef HMC_Data;
SEQTypeDef q = {1, 0, 0, 0};
float roll, pitch, yaw;

Quad_TypeDef quad;

void ALL_Init(void)
{
	TIM2_Init();
	DWT_Init();
	MOTOR_Init();
//	Unlock_ALL_ESC();
	GY86_Init();
	OLED_Init();
	PPM_Init();
	USART_Config(9600);
	QuadPID_Init(&quad);

	// EulerBias_Correct();

	q = (SEQTypeDef){1, 0, 0, 0};
}

void GY86_Show(void)	
{
	MPU6050_GetData(&MPU6050_Data);
	HMC5883L_GetData(&HMC_Data);
	printf("mx: %d, my: %d, mz: %d\r\n", HMC_Data.Mag_X, HMC_Data.Mag_Y, HMC_Data.Mag_Z);
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

void test_1(void)
{
	GY86_Show();
	PWM_SetCompare(1, PPM[2]);
	PWM_SetCompare(2, PPM[2]);
	PWM_SetCompare(3, PPM[2]);
	PWM_SetCompare(4, PPM[2]);
}

void Madgwick_Test(void)
{
    MPU6050_GetData(&MPU6050_Data);
	HMC5883L_GetData(&HMC_Data);

//	MadgwickUpdate(&MPU6050_Data, &HMC_Data, &q, attitude_dt);


//	Quaternion2Euler(&q, &roll, &pitch, &yaw);

//  printf("roll: %.2f, pitch: %.2f, yaw: %.2f\n", roll, pitch, yaw);

//	int16_t roll_int = roll * 100 * 180.0f / 3.1415926f;
//	int16_t pitch_int = pitch * 100 * 180.0f / 3.1415926f;
//	int16_t yaw_int = yaw * 100 * 180.0f / 3.1415926f;

//	data[0] = (roll_int & 0xff);
//	data[1] = (roll_int >> 8);
//	data[2] = (pitch_int & 0xff);
//	data[3] = (pitch_int >> 8);
//	data[4] = (yaw_int & 0xff);
//	data[5] = (yaw_int >> 8);
//	data[6] = 0;

//	int16_t q1_int = q.q1 * 10000;
//	int16_t q2_int = q.q2 * 10000;
//	int16_t q3_int = q.q3 * 10000;
//	int16_t q4_int = q.q4 * 10000;

//	data[0] = (q1_int & 0xff);
//	data[1] = (q1_int >> 8);
//	data[2] = (q2_int & 0xff);
//	data[3] = (q2_int >> 8);
//	data[4] = (q3_int & 0xff);
//	data[5] = (q3_int >> 8);
//	data[6] = (q4_int & 0xff);
//	data[7] = (q4_int >> 8);
//	data[8] = 0;

//	Ano_SendData(data, 9, 0x04);

//	vofa_data[0] = q.q1;
//	vofa_data[1] = q.q2;
//	vofa_data[2] = q.q3;
//	vofa_data[3] = q.q4;

//	Vofa_SendData(vofa_data);

    printf("%f, %f, %f, %f\n", q.q1, q.q2, q.q3, q.q4);

//		Ano_SendData(data, 7, 0x03);
	
	Delay_ms(10);
}

void PID_Test(void)
{
	if(1){
		// attitude_flag = 0;

		static uint32_t attitude_last_time = 0;
		uint32_t attitude_current_time = DWT_GetTime();
		float attitude_dt = (float)(attitude_current_time - attitude_last_time) / (float)SystemCoreClock;
		attitude_last_time = attitude_current_time;
		// printf("attitude dt: %f\n", attitude_dt);

		if(attitude_dt > 0.01f)
			attitude_dt = 0.01f;

		MPU6050_GetData(&MPU6050_Data);
		HMC5883L_GetData(&HMC_Data);
		MadgwickUpdate(&MPU6050_Data, &HMC_Data, &q, attitude_dt);
		Quaternion2Euler(&q, &roll, &pitch, &yaw);

		roll *= RAD2DEG;
		pitch *= RAD2DEG;
		yaw *= RAD2DEG;

		roll -= euler_bias[0];
		pitch -= euler_bias[1];

		// printf("%f, %f, %f, %f\n", q.q1, q.q2, q.q3, q.q4);
		// printf("%f\n", pitch * 180.0f / 3.1415926f);
	}

	if(1){
		// outer_flag = 0;

		static uint32_t outer_last_time = 0;
		uint32_t outer_current_time = DWT_GetTime();
		float outer_dt = (float)(outer_current_time - outer_last_time) / (float)SystemCoreClock;
		outer_last_time = outer_current_time;
		// printf("outer dt: %f\n", outer_dt);

		if(outer_dt > 0.01f)
			outer_dt = 0.01f;

		if(PPM[2] >= 1700){
			PPM[2] = 1700;
		}
		
		OuterLoop_Update(&quad, roll, pitch, yaw, PPM, outer_dt);

		// 串口调试打印：在外环周期打印关键量，便于地面验证（50Hz）
		{
			// const float RAD2DEG = 180.0f / 3.14159265359f;
			// float roll_deg = roll * RAD2DEG;
			// float pitch_deg = pitch * RAD2DEG;
			// float yaw_deg = yaw * RAD2DEG;
			// printf("%.2f,%.2f,%.2f,%.2f\n", quad.pitch, quad.rcPitch * 0.36f, quad.OuterPID.pitch.error, quad.rcPitch * 0.36f - quad.pitch);
			// printf("%.2f, %.2f\n", pitch_deg, quad.rcPitch * 0.36f);
		}
	}

 	if(1){
 		// inner_flag = 0;

 		static uint32_t inner_last_time = 0;
 		uint32_t inner_current_time = DWT_GetTime();
 		float inner_dt = (float)(inner_current_time - inner_last_time) / (float)SystemCoreClock;
 		inner_last_time = inner_current_time;
 		// printf("inner dt: %f\n", inner_dt);

		if(inner_dt > 0.01f)
			inner_dt = 0.01f;

 		// RCtoRATE(&quad, PPM);
 		InnerLoop_Update(&quad, &MPU6050_Data, inner_dt);
 		// printf("%f, %f\n", quad.pitch, quad.InnerPID.pitch.derivative);
 		// printf("%f, %f, %f, %f\n", quad.targetPitchRate, quad.pitchRate, quad.targetPitchRate - quad.pitchRate, quad.InnerPID.pitch.integral);
 		// printf("%f, %f, %f, %f\n", quad.pitch, quad.rcPitch * 0.18f, quad.OuterPID.pitch.error, quad.OuterPID.pitch.integral);
 		// printf("%f, %f, %f, %f\n", quad.InnerPID.pitch.error, quad.pitch, quad.OuterPID.pitch.output, quad.InnerPID.pitch.output);
 		// printf("%f, %f\n", quad.InnerPID.pitch.derivative, quad.pitch);
 		// printf("%.2f,%.2f,%.2f\n", quad.pitchRate, quad.targetPitchRate,quad.targetPitchRate - quad.pitchRate);
 		// printf("%.2f,%.2f,%.2f\n", quad.pitch, quad.rcRoll * 0.36f, quad.OuterPID.pitch.error);
		// printf("%.2f, %.2f, %.2f\n", MPU6050_Data.Gyro_X / 65.5f, MPU6050_Data.Gyro_Y / 65.5f, MPU6050_Data.Gyro_Z / 65.5f);
 		// printf("m1:%d, m2:%d, m3:%d, m4:%d\n", (uint16_t)quad.motor1, (uint16_t)quad.motor2, (uint16_t)quad.motor3, (uint16_t)quad.motor4);
 		QuadPID_MotorControl(&quad);

		if(PPM[2] <= 1100){
			PWM_SetCompare(1, 1000);
			PWM_SetCompare(2, 1000);
			PWM_SetCompare(3, 1000);
			PWM_SetCompare(4, 1000);
		}
 	}

	

	if(com_flag){
		com_flag = 0;

		// printf("%f\n", quad.pitchRate);
		// printf("%f, %f, %f\n", quad.pitch, quad.roll, quad.yaw);
		// printf("%f\n", quad.yawRate);

		// printf("%f, %f, %f, %f\n", quad.targetPitchRate, quad.pitchRate, quad.targetPitchRate - quad.pitchRate, quad.InnerPID.pitch.integral);
		// printf("%f, %f, %f, %f\n", quad.rcPitch * 0.18f, quad.pitch, quad.OuterPID.pitch.error, quad.OuterPID.pitch.integral);
	}
}

void EulerBias_Correct(void)
{
    float roll_sum = 0.0f;
    float pitch_sum = 0.0f;
    uint32_t last_time = DWT_GetTime(); // 明确初始化
    float dt;
    uint32_t current_time;

    // 1. 预热姿态解算（500次）
    for(int i = 0; i < 100; ++i){
        current_time = DWT_GetTime();
        dt = (float)(current_time - last_time) / (float)SystemCoreClock;
        last_time = current_time; // 更新时间

        MPU6050_GetData(&MPU6050_Data);
        HMC5883L_GetData(&HMC_Data);
        MadgwickUpdate(&MPU6050_Data, &HMC_Data, &q, dt);
    }
    
    // 再次重置时间，确保第二个循环 dt 准确
    last_time = DWT_GetTime(); 

    // 2. 采集姿态角（100次）
    for(int i = 0; i < 200; ++i){
        current_time = DWT_GetTime();
        dt = (float)(current_time - last_time) / (float)SystemCoreClock;
        last_time = current_time; // 更新时间

        MPU6050_GetData(&MPU6050_Data);
        HMC5883L_GetData(&HMC_Data);
        MadgwickUpdate(&MPU6050_Data, &HMC_Data, &q, dt);
        Quaternion2Euler(&q, &roll, &pitch, &yaw);

        roll *= RAD2DEG;
        pitch *= RAD2DEG;

        roll_sum += roll;
        pitch_sum += pitch;
    }
    
    // ... 后续计算不变
    float roll_bias = roll_sum / 100.0f;
    float pitch_bias = pitch_sum / 100.0f;

    euler_bias[0] = roll_bias;
    euler_bias[1] = pitch_bias;

}
