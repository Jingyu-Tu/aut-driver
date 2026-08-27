/**
  ******************************************************************************
  * @file    GPIO/IOToggle/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and peripherals
  *          interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "encoder.h"
#include <stdio.h>
#include "control_system.h"
#include "motor.h"


// 声明外部变量（这些变量在 main.c 中定义）
extern int target_speed_left;
extern int target_speed_right;

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

/*void SysTick_Handler(void)
{
    static uint32_t millis = 0;

    millis++;

    if (millis % 100 == 0)
    {
        millis = 0;
        L_speed = Read_Encoder(2);
        R_speed = Read_Encoder(3);
        printf("left: %d, right: %d\r\n", L_speed, R_speed);
    }
}*/

void SysTick_Handler(void)
{
    static uint32_t tick = 0;
    static uint32_t print_tick = 0;
    tick++;
    print_tick++;
    
    // 每 5ms 执行一次 PID 控制
    if (tick % 5 == 0)
    {
        int left_encoder = Read_Encoder(2);
        int right_encoder = Read_Encoder(3);
        
        int left_pwm = Incremental_PI_A(left_encoder, target_speed_left);
        int right_pwm = Incremental_PI_B(right_encoder, target_speed_right);
        
        // ★★★ 添加调试：打印 PID 计算结果 ★★★
        if (tick % 20 == 0) {  // 每 100ms 打印一次
            printf("Target:%d %d | Encoder:%d %d | PWM:%d %d\r\n", 
                   target_speed_left, target_speed_right,
                   left_encoder, right_encoder,
                   left_pwm, right_pwm);
        }
        
        Set_Pwm(left_pwm, right_pwm);
    }
}
/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/