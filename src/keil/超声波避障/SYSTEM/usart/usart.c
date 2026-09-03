#include "sys.h"
#include "usart.h"
#include "motor.h"
// 串口接收缓冲区
u8 USART_RX_BUF[200];
u8 USART_RX_COUNT = 0;
u8 CAR_buff[4] = {0};
u8 uart_rec_flag = 0;

#if 1
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
};

FILE __stdout;

_sys_exit(int x)
{
    x = x;
}

int fputc(int ch, FILE *f)
{
    while ((USART1->SR & 0X40) == 0);
    USART1->DR = (u8)ch;
    return ch;
}
#endif



#if EN_USART1_RX

void uart_init(u32 bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
}
void USART1_IRQHandler(void)
{
    u8 Res;
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        Res = USART_ReceiveData(USART1);
        
        // ★★★ 直接调用小车动作函数 ★★★
        switch (Res)
        {
            case 'W':
                Set_Pwm(2000, 2000);   // 前进
                printf("Forward\r\n");
                break;
            case 'S':
                Set_Pwm(-2000, -2000); // 后退
                printf("Backward\r\n");
                break;
            case 'A':
                Set_Pwm(1000, 3000);   // 左转（左轮慢，右轮快）
                printf("Left\r\n");
                break;
            case 'D':
                Set_Pwm(3000, 1000);   // 右转（左轮快，右轮慢）
                printf("Right\r\n");
                break;
            case 'O':
                Set_Pwm(0, 0);         // 停止
                printf("Stop\r\n");
                break;
            case 'I':
                Set_Pwm(100, 100);     // 速度100
                printf("Speed 100\r\n");
                break;
            case 'K':
                Set_Pwm(150, 150);     // 速度150
                printf("Speed 150\r\n");
                break;
            default:
                printf("Unknown: %c\r\n", Res);
                break;
        }
    }
    USART_ClearFlag(USART1, USART_FLAG_RXNE);
}
 
#endif