#include "PID.h"

#define GYRO_LSB 65.5f

extern uint16_t PPM[8];
float gyro_bias[3] = {-3.807f, -0.254f, -0.756f};
extern float euler_bias[2];

// Gyro LPF state
static float gyro_lpf_x = 0.0f;
static float gyro_lpf_y = 0.0f;
static float gyro_lpf_z = 0.0f;

// Gyro LPF filter
static inline float lpf_filter(float input, float *state, float cutoff, float dt)
{
    // RC = 1/(2*pi*fc)
    float RC = 1.0f / (2.0f * 3.14159265358979f * cutoff);
    float alpha = RC / (RC + dt); // numerically stable for small dt
    *state = alpha * (*state) + (1.0f - alpha) * input;
    return *state;
}

// PID参数初始化
void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd, float maxOutput, float maxIntegral)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->target = 0.0f;
    pid->output = 0.0f;
    pid->error = 0.0f;
    pid->derivative = 0.0f;
    pid->dState = 0.0f;
    pid->lastError = 0.0f;
    pid->lastfilteredError = 0.0f;
    pid->eState = 0.0f;
    pid->integral = 0.0f;
    pid->maxOutput = maxOutput;
    pid->maxIntegral = maxIntegral;
}

// PID计算函数
float PID_Calculate(PID_TypeDef *pid, float current, float target, float dt)
{
    // 计算当前误差
    pid->error = target - current;
    
    // 积分项计算和限幅
    pid->integral += pid->error * dt;
    if (pid->integral > pid->maxIntegral) {
        pid->integral = pid->maxIntegral;
    } else if (pid->integral < -pid->maxIntegral) {
        pid->integral = -pid->maxIntegral;
    }
    
    // 微分项计算
    pid->derivative = lpf_filter((pid->error - pid->lastError) / dt, &pid->dState, 20.0f, dt);
    pid->lastError = pid->error;

    // if(pid->derivative > 400.0f) pid->derivative = 400.0f;
    // if(pid->derivative < -400.0f) pid->derivative = -400.0f;

    // printf("%f\n", derivative);
    
    // PID输出计算
    pid->output = pid->Kp * pid->error + pid->Ki * pid->integral + pid->Kd * pid->derivative;
    
    // 输出限幅
    if (pid->output > pid->maxOutput) {
        pid->output = pid->maxOutput;
    } else if (pid->output < -pid->maxOutput) {
        pid->output = -pid->maxOutput;
    }
    
    // 保存当前误差为下次计算使用
    pid->lastError = pid->error;
    
    return pid->output;
}

// PID重置函数
void PID_Reset(PID_TypeDef *pid)
{
    pid->target = 0.0f;
    pid->output = 0.0f;
    pid->error = 0.0f;
    pid->lastError = 0.0f;
    pid->integral = 0.0f;
}

// 四轴PID初始化
void QuadPID_Init(Quad_TypeDef *quad)
{
    // 外环-姿态角PID初始化
    // 注意：这些参数需要根据实际飞行器特性进行调整
    PID_Init(&quad->OuterPID.roll, 4.5f, 0.0f, 0.0f, 500.0f, 500.0f);
    PID_Init(&quad->OuterPID.pitch, 4.5f, 0.0f, 0.0f, 500.0f, 500.0f);
    PID_Init(&quad->OuterPID.yaw, 0.0f, 0.0f, 0.0f, 500.0f, 200.0f);
    
    // 内环-角速度PID初始化
    PID_Init(&quad->InnerPID.roll, 1.05f, 0.0f, 0.065f, 1000.0f, 500.0f);
    PID_Init(&quad->InnerPID.pitch, 1.05f, 0.0f, 0.065f, 1000.0f, 500.0f);
    PID_Init(&quad->InnerPID.yaw, 5.0f, 0.0f, 0.0f, 500.0f, 200.0f);
    
    // 初始化姿态和遥控数据
    quad->roll = 0.0f;
    quad->pitch = 0.0f;
    quad->yaw = 0.0f;
    quad->rollRate = 0.0f;
    quad->pitchRate = 0.0f;
    quad->yawRate = 0.0f;
    
    quad->rcRoll = 0.0f;
    quad->rcPitch = 0.0f;
    quad->rcYaw = 0.0f;
    quad->rcThrottle = 0.0f;
    
    // 初始化电机输出
    quad->motor1 = 0.0f;
    quad->motor2 = 0.0f;
    quad->motor3 = 0.0f;
    quad->motor4 = 0.0f;

    // 初始化外环PID输出
    quad->rollRate = 0.0f;
    quad->pitchRate = 0.0f;
    quad->yawRate = 0.0f;

    // 初始化目标角速度
    quad->targetRollRate = 0.0f;
    quad->targetPitchRate = 0.0f;
    quad->targetYawRate = 0.0f;
}

// 四轴PID更新函数
// void QuadPID_Update(Quad_TypeDef *quad, float roll, float pitch, float yaw, 
//                    float rollRate, float pitchRate, float yawRate, 
//                    uint16_t *ppm, float dt)
// {
//     // 更新当前姿态数据
//     quad->roll = roll;
//     quad->pitch = pitch;
//     quad->yaw = yaw;
//     quad->rollRate = rollRate;
//     quad->pitchRate = pitchRate;
//     quad->yawRate = yawRate;
    
//     // 更新遥控器输入
//     // 直接使用PPM值，横滚俯仰偏航转换为-500到500的范围
//     quad->rcRoll = ppm[0] - 1500;
//     quad->rcPitch = ppm[1] - 1500;
//     quad->rcYaw = ppm[3] - 1500;
//     quad->rcThrottle = ppm[2]; // 油门直接使用原始PPM值(1000-2000)
    
//     // 如果油门低于最低值，重置所有PID
//     if (quad->rcThrottle < 1100) {
//         PID_Reset(&quad->OuterPID.roll);
//         PID_Reset(&quad->OuterPID.pitch);
//         PID_Reset(&quad->OuterPID.yaw);
//         PID_Reset(&quad->InnerPID.roll);
//         PID_Reset(&quad->InnerPID.pitch);
//         PID_Reset(&quad->InnerPID.yaw);
//         return;
//     }
    
//     // 外环PID计算：将目标姿态角转换为目标角速度
//     float targetRollRate = PID_Calculate(&quad->OuterPID.roll, quad->roll, quad->rcRoll * 0.06f, dt);
//     float targetPitchRate = PID_Calculate(&quad->OuterPID.pitch, quad->pitch, quad->rcPitch * 0.06f, dt);
//     float targetYawRate = quad->rcYaw * 0.2f; // 偏航角速度直接由遥控器控制
    
//     // 内环PID计算：将目标角速度转换为电机控制量
//     float rollOutput = PID_Calculate(&quad->InnerPID.roll, quad->rollRate, targetRollRate, dt);
//     float pitchOutput = PID_Calculate(&quad->InnerPID.pitch, quad->pitchRate, targetPitchRate, dt);
//     float yawOutput = PID_Calculate(&quad->InnerPID.yaw, quad->yawRate, targetYawRate, dt);
    
//     // 混合控制输出到四个电机
//     // 假设电机布局：
//     //    4   2
//     //      *
//     //    3   1
//     quad->motor1 = quad->rcThrottle - rollOutput + pitchOutput - yawOutput;
//     quad->motor2 = quad->rcThrottle + rollOutput + pitchOutput + yawOutput;
//     quad->motor3 = quad->rcThrottle - rollOutput - pitchOutput + yawOutput;
//     quad->motor4 = quad->rcThrottle + rollOutput - pitchOutput - yawOutput;
// }

void RCtoRATE(Quad_TypeDef *quad, uint16_t *ppm)
{
    // 将遥控器输入转换为期望的角速度
    quad->rcThrottle = ppm[2]; // 油门直接使用原始PPM值(1000-2000)
    quad->targetRollRate = (ppm[0] - 1500) * 0.1f;
    quad->targetPitchRate = (ppm[1] - 1512) * 0.1f;
    quad->targetYawRate = (ppm[3] - 1500) * 0.1f;
}

void OuterLoop_Update(Quad_TypeDef *quad, float roll, float pitch, float yaw, uint16_t *ppm, float dt)
{
    // 更新当前姿态数据
    quad->roll = roll;
    quad->pitch = pitch;
    quad->yaw = yaw;

    // 更新遥控器输入
    // 直接使用PPM值，横滚俯仰转换为-500到500的范围
    quad->rcRoll = ppm[0] - 1500;
    quad->rcPitch = ppm[1] - 1512;
    quad->rcYaw = ppm[3] - 1500;
    quad->rcThrottle = ppm[2]; // 油门直接使用原始PPM值(1000-2000)

    // 将摇杆值线性映射到 ±180 度
    quad->targetRollRate = PID_Calculate(&quad->OuterPID.roll, quad->roll, quad->rcRoll * 0.06f, dt);
    quad->targetPitchRate = PID_Calculate(&quad->OuterPID.pitch, quad->pitch, quad->rcPitch * 0.06f, dt);

    quad->targetYawRate = quad->rcYaw * 0.2f; // 偏航角速度直接由遥控器控制

    // printf("%f\n", quad->targetPitchRate);
}

void InnerLoop_Update(Quad_TypeDef *quad, MPU6050_DataTypeDef *Gyro, float dt)
{
	static float gyro[3] = {0, 0, 0};
    gyro[0] = Gyro->Gyro_X / GYRO_LSB;
    gyro[1] = Gyro->Gyro_Y / GYRO_LSB;
    gyro[2] = Gyro->Gyro_Z / GYRO_LSB;
    Gyro_Process(gyro, gyro_bias, dt);

    // printf("%f\n", gyro[1]);
    quad->rollRate = gyro[0];
    quad->pitchRate = gyro[1];
    quad->yawRate = gyro[2];

    // 内环PID计算：将目标角速度转换为电机控制量
    float rollOutput = PID_Calculate(&quad->InnerPID.roll, quad->rollRate, quad->targetRollRate, dt);
    float pitchOutput = PID_Calculate(&quad->InnerPID.pitch, quad->pitchRate, quad->targetPitchRate, dt);
    float yawOutput = PID_Calculate(&quad->InnerPID.yaw, quad->yawRate, quad->targetYawRate, dt);

    // printf("%f\n", pitchOutput);

    // 混合控制输出到四个电机
    // 电机布局：
    //    1(CCW)   4(CW)
    //          *
    //    2(CW)   3(CCW)
    quad->motor1 = quad->rcThrottle + rollOutput - pitchOutput - yawOutput;
    quad->motor2 = quad->rcThrottle + rollOutput + pitchOutput + yawOutput;
    quad->motor3 = quad->rcThrottle - rollOutput + pitchOutput - yawOutput;
    quad->motor4 = quad->rcThrottle - rollOutput - pitchOutput + yawOutput;
}

// 电机控制函数
void QuadPID_MotorControl(Quad_TypeDef *quad)
{
    // 限制电机输出在1000-2000范围内
    if (quad->motor1 > 2000) quad->motor1 = 2000;
    else if (quad->motor1 < 1000) quad->motor1 = 1000;
    
    if (quad->motor2 > 2000) quad->motor2 = 2000;
    else if (quad->motor2 < 1000) quad->motor2 = 1000;
    
    if (quad->motor3 > 2000) quad->motor3 = 2000;
    else if (quad->motor3 < 1000) quad->motor3 = 1000;
    
    if (quad->motor4 > 2000) quad->motor4 = 2000;
    else if (quad->motor4 < 1000) quad->motor4 = 1000;
    
    // 直接使用电机输出值作为PWM值
    uint16_t motor1PWM = (uint16_t)quad->motor1;
    uint16_t motor2PWM = (uint16_t)quad->motor2;
    uint16_t motor3PWM = (uint16_t)quad->motor3;
    uint16_t motor4PWM = (uint16_t)quad->motor4;
    
    // 更新电机PWM值
    PWM_SetCompare(1, motor1PWM);
    PWM_SetCompare(2, motor2PWM);
    PWM_SetCompare(3, motor3PWM);
    PWM_SetCompare(4, motor4PWM);
}

void Gyro_Process(float gyro[3], float gyroBias[3], float dt)
{
    float gx = gyro[0] - gyroBias[0];
    float gy = gyro[1] - gyroBias[1];
    float gz = gyro[2] - gyroBias[2];

    // 使用低通滤波器处理陀螺仪数据
    const float cutoff = 50.0f; // Cutoff frequency in Hz
    gx = lpf_filter(gx, &gyro_lpf_x, cutoff, dt);
    gy = lpf_filter(gy, &gyro_lpf_y, cutoff, dt);
    gz = lpf_filter(gz, &gyro_lpf_z, cutoff, dt);

    gyro[0] = gx;
    gyro[1] = gy;
    gyro[2] = gz;
}
