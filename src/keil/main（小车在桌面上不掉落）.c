#include "stm32f10x.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "motor.h"

#define TRIG_PIN GPIO_Pin_0
#define ECHO_PIN GPIO_Pin_1

#define SAMPLE_COUNT 5      // 连续采样次数
#define TRIGGER_THRESHOLD 3 // 连续多少次超过阈值才触发

void Ultrasonic_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = TRIG_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = ECHO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void Timer2_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler = 71;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
}

float GetDistance(void)
{
    uint32_t time = 0;
    uint32_t timeout = 0;
    
    GPIO_SetBits(GPIOA, TRIG_PIN);
    delay_us(10);
    GPIO_ResetBits(GPIOA, TRIG_PIN);
    
    timeout = 0;
    while (GPIO_ReadInputDataBit(GPIOA, ECHO_PIN) == 0)
    {
        timeout++;
        if (timeout > 100000) return 999;
    }
    
    TIM_SetCounter(TIM2, 0);
    TIM_Cmd(TIM2, ENABLE);
    timeout = 0;
    
    while (GPIO_ReadInputDataBit(GPIOA, ECHO_PIN) == 1)
    {
        timeout++;
        if (timeout > 100000) break;
    }
    
    time = TIM_GetCounter(TIM2);
    TIM_Cmd(TIM2, DISABLE);
    
    return time * 0.01715;
}

// 平均值滤波
float GetAverageDistance(void)
{
    int i;
    float sum = 0;
    float valid_samples[5];
    int valid_count = 0;
    
    for(i = 0; i < 5; i++)
    {
        float d = GetDistance();
        if (d < 999)  // 有效数据
        {
            valid_samples[valid_count] = d;
            valid_count++;
        }
        delay_ms(5);
    }
    
    if (valid_count == 0) return 999;
    
    for(i = 0; i < valid_count; i++)
    {
        sum += valid_samples[i];
    }
    
    return sum / valid_count;
}

int main(void)
{
    float dist;
    int trigger_count = 0;
    
    Stm32_Clock_Init(9);
    delay_init();
    uart_init(115200);
    JTAG_Set(JTAG_SWD_DISABLE);
    JTAG_Set(SWD_ENABLE);
    
    Ultrasonic_Init();
    Timer2_Init();
    PWM_Init(7199, 9);
    
    printf("防跌落启动（滤波版）\r\n");
    
    while (1)
    {
        // ★★★ 用平均值，避免波动误触发 ★★★
        dist = GetAverageDistance();
        printf("距离: %.1f cm\r\n", dist);
        
        // ★★★ 连续 3 次超过阈值才触发掉头 ★★★
        if (dist > 25 && dist < 999)
        {
            trigger_count++;
            printf("触发计数: %d/3\r\n", trigger_count);
            
            if (trigger_count >= 3)
            {
                // 掉头
                Set_Pwm(-1500, 1500);
                delay_ms(300);
                Set_Pwm(0, 0);
                delay_ms(200);
                trigger_count = 0;  // 重置
            }
        }
        else
        {
            trigger_count = 0;  // 安全，重置计数
            // 正常前进
            Set_Pwm(1500, 1500);
        }
        
        delay_ms(50);
    }
}
