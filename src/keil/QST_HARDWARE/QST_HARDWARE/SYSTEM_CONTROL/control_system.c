#include "control_system.h"

#define K1     1

extern int a1, a2, b1, b2;
float K2;
float Pwm_Max = 7199;
float Angle_X, Angle_Y;

float JSDx = 0;
float JSDy = 0;
float JSDz = 0.0;

extern UARTFrameTypeDef UART2Frame;
extern u8 dir_rec;
extern int Target_MotorA, Target_MotorB;
extern float Target_angle;
u8 t = 0;

void Angle_Calcu(void)
{
    if (mpu_dmp_get_data(&JSDx, &JSDy, &JSDz) == 0)
    {
        put_shuzu(USART3, JSDz);
    }
}

void Set_Pwm(int moto1, int moto2)
{
    if (moto1 > 0)
    {
        AIN = 0;
    }
    else if (moto1 == 0)
    {
        moto1 = 7199;
        AIN = 1;
    }
    else if (moto1 < 0)
    {
        AIN = 1;
        moto1 = 0;
    }
    PWMA = myabs(moto1);

    if (moto2 > 0)
    {
        BIN = 0;
    }
    else if (moto2 == 0)
    {
        moto2 = 7199;
        BIN = 1;
    }
    else if (moto2 < 0)
    {
        BIN = 1;
        moto2 = 0;
    }
    PWMB = myabs(moto2);
}

u32 myabs(long int a)
{
    u32 temp;
    if (a < 0) temp = -a;
    else temp = a;
    return temp;
}

int Incremental_PI_A(int Encoders_A, int Target_A)
{
    static int Bias_A, Pwm_A, Last_bias_A;
    Bias_A = Target_A - (Encoders_A * 50);
    Pwm_A += Velocity_KP * Bias_A + Velocity_KD * (Bias_A - Last_bias_A);
    if (Pwm_A > 7199) Pwm_A = 7199;
    if (Pwm_A < 0) Pwm_A = 0;
    Last_bias_A = Bias_A;
    return Pwm_A;
}

int Incremental_PI_B(int Encoders_B, int Target_B)
{
    static int Bias_B, Pwm_B, Last_bias_B;
    Bias_B = Target_B - (Encoders_B * 50);
    Pwm_B += Velocity_KP * Bias_B + Velocity_KD * (Bias_B - Last_bias_B);
    if (Pwm_B > 7199) Pwm_B = 7199;
    if (Pwm_B < 0) Pwm_B = 0;
    Last_bias_B = Bias_B;
    return Pwm_B;
}

int Position_PID_A(int Encoder, int Target)
{
    static float Bias, Pwm, Integral_bias, Last_Bias;
    Bias = Encoder - Target;
    Integral_bias += Bias;
    if (Integral_bias > 1000) Integral_bias = 1000;
    if (Integral_bias < -1000) Integral_bias = -1000;
    Pwm = Position_KP_A * Bias + Position_KI_A / 100 * Integral_bias + Position_KD_A * (Bias - Last_Bias);
    Last_Bias = Bias;
    return Pwm;
}

int Position_PID_B(int Encoder, int Target)
{
    static float Bias, Pwm, Integral_bias, Last_Bias;
    Bias = Encoder - Target;
    Integral_bias += Bias;
    if (Integral_bias > 1000) Integral_bias = 1000;
    if (Integral_bias < -1000) Integral_bias = -1000;
    Pwm = Position_KP_B * Bias + Position_KI_B / 100 * Integral_bias + Position_KD_B * (Bias - Last_Bias);
    Last_Bias = Bias;
    return Pwm;
}

void System_Control(void)
{
    if (uart_rec_flag)
    {
        if (CAR_buff[3] == 0)
        {
            if (CAR_buff[0] == 0 || CAR_buff[0] == 1)
            {
                Target_MotorA = CAR_buff[1];
                Target_MotorB = CAR_buff[2];
                Key_mode = 1;
                uart_rec_flag = 0;
            }
            else if (CAR_buff[0] == 2)
            {
                Key_mode = 3;
                uart_rec_flag = 0;
            }
        }
        else
        {
            Target_angle = (float)CAR_buff[3];
            Key_mode = 2;
            uart_rec_flag = 0;
        }
        memset(CAR_buff, 0, 4);
    }

    if (1 == Key_mode)
    {
        R_led_CLC();
        Encoder_A = Read_Encoder(2);
        Encoder_B = Read_Encoder(3);
        Motor_A = Incremental_PI_A(Encoder_A, Target_MotorA);
        Motor_B = Incremental_PI_B(Encoder_B, Target_MotorB);
    }
    else if (3 == Key_mode)
    {
        Read_Encoder(0);
        R_led_mode();
        Motor_A = -1;
        Motor_B = -1;
    }
    else if (2 == Key_mode)
    {
        Read_Encoder(0);
        if (Target_angle >= 0)
        {
            Motor_A = -1;
            Motor_B = 7100;
        }
        else
        {
            Motor_A = 7100;
            Motor_B = -1;
        }
    }
    else
    {
        Motor_A = 0;
        Motor_B = 0;
        Read_Encoder(0);
    }

    Set_Pwm(Motor_B, Motor_A);
}

void SysTick_Handler(void)
{
    if (0 != Jiaodu)
        System_Control();
}