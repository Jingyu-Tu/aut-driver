#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "motor.h"
#include "encoder.h"
#include "control_system.h"

// 声明外部变量（在 main.c 中定义）
extern int target_speed_left;
extern int target_speed_right;

// 定义全局目标速度
int target_speed_left = 0;
int target_speed_right = 0;

int main(void)
{
    NVIC_PriorityGroupConfig(2);
    delay_init();
    uart_init(115200);
    JTAG_Set(SWD_ENABLE);

    PWM_Init(7199, 9);
    Encoder_Init_TIM2();
    Encoder_Init_TIM3();
  

	// ★★★ 启动前先给一个初始 PWM 让电机转起来 ★★★
  //  Set_Pwm(1500, 1500);  // 先给一个初始速度
   // delay_ms(500);        // 让电机转起来
	
	
    SysTick_Config(72000000 / 1000);   // 1ms 中断

    printf("QST小车 PID 闭环控制启动\r\n");

    while (1)
    {
        // 前进
        target_speed_left = 2000;
        target_speed_right = 2000;
        delay_ms(1000);
			

        // 后退
        target_speed_left = -2000;
        target_speed_right = -2000;
        delay_ms(1000);
        
    }
}

