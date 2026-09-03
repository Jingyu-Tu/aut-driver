#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"
#include "motor.h"
#include "soft_uart_rx.h"

// 指令定义
#define CMD_FORWARD 'F'
#define CMD_REVERSE 'B'
#define CMD_LEFT    'L'
#define CMD_RIGHT   'R'
#define CMD_STOP    'S'

// 速度定义
#define SPEED_FORWARD  2500
#define SPEED_REVERSE  2200
#define SPEED_TURN     2000

void ExecuteCommand(uint8_t cmd)
{
    switch(cmd)
    {
        case CMD_FORWARD:
            Set_Pwm(SPEED_FORWARD, SPEED_FORWARD);
            break;
        case CMD_REVERSE:
            Set_Pwm(-SPEED_REVERSE, -SPEED_REVERSE);
            break;
        case CMD_LEFT:
            Set_Pwm(-SPEED_TURN, SPEED_TURN);
            break;
        case CMD_RIGHT:
            Set_Pwm(SPEED_TURN, -SPEED_TURN);
            break;
        case CMD_STOP:
        default:
            Set_Pwm(0, 0);
            break;
    }
}

int main(void)
{
    uint8_t cmd;
    
    Stm32_Clock_Init(9);
    MY_NVIC_PriorityGroupConfig(2);
    delay_init();
    JTAG_Set(JTAG_SWD_DISABLE);
    JTAG_Set(SWD_ENABLE);
    
    PWM_Init(7199, 9);
    SoftUartRx_Init();      // ★★★ 波特率在 soft_uart_rx.c 里改 ★★★
    Control_Stop();
    
    while (1)
    {
        if (SoftUartRx_ReadByte(&cmd))
        {
            ExecuteCommand(cmd);
        }
        delay_ms(10);
    }
}