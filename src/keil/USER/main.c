#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "motor.h"

int main(void)
{
    NVIC_PriorityGroupConfig(2);
    delay_init();
    uart_init(115200);
    JTAG_Set(SWD_ENABLE);

    PWM_Init(7199, 9);
    printf("QST小车初始化\n");

    while (1)
    {
        Set_Pwm(2500, 2500);  // 前进
        delay_ms(2000);       // 跑 2 秒

        Set_Pwm(-2500, -2500); // 后退
        delay_ms(2000);        // 跑 2 秒
    }
}