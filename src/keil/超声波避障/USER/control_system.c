#include "control_system.h"

// 目标速度
int target_speed_left = 0;
int target_speed_right = 0;

// 声明外部变量（在 usart.c 中定义）
extern u8 CAR_buff[4];
extern u8 uart_rec_flag;

void System_Control(void)
{
    int speedA = 0;
    int speedB = 0;
    int pwmA = 0;
    int pwmB = 0;

    if (uart_rec_flag)
    {
        speedA = CAR_buff[1];
        speedB = CAR_buff[3];

        if (CAR_buff[0] == 1) {
            speedA = -speedA;
        }
        if (CAR_buff[2] == 1) {
            speedB = -speedB;
        }

        if (speedA > 150) speedA = 150;
        if (speedA < -150) speedA = -150;
        if (speedB > 150) speedB = 150;
        if (speedB < -150) speedB = -150;

        target_speed_left = speedA;
        target_speed_right = speedB;

        uart_rec_flag = 0;
    }

    // 速度 0~150 映射到 PWM 0~5000
    pwmA = target_speed_left * 33;
    pwmB = target_speed_right * 33;

    if (pwmA > 5000) pwmA = 5000;
    if (pwmA < -5000) pwmA = -5000;
    if (pwmB > 5000) pwmB = 5000;
    if (pwmB < -5000) pwmB = -5000;

    Set_Pwm(pwmA, pwmB);

    printf("speed: %d, %d, PWM: %d, %d\r\n",
           target_speed_left, target_speed_right, pwmA, pwmB);
}

void Control_Stop(void)
{
    Set_Pwm(0, 0);
}