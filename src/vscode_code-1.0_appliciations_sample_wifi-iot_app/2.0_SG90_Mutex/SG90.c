#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include <hi_gpio.h>
#include <hi_io.h>
#include <hi_time.h>

osMutexId_t mutex_id;
#define GPIO2 2
uint8_t flag;    // 舵机旋转角度标志位

// 查阅小车原理图可知，SG90舵机通过GPIO2与3861连接
// SG90舵机的控制需要MCU产生一个周期为20ms的脉冲信号，以0.5ms到2.5ms的高电平来控制舵机转动的角度
// 输出20000微秒的脉冲信号（x微秒高电平，20000-x微秒低电平）
void set_angle(unsigned int duty)
{
    // 设置GPIO2为输出模式
    hi_io_set_func(HI_IO_NAME_GPIO_2, HI_IO_FUNC_GPIO_2_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_2, HI_GPIO_DIR_OUT);

    // GPIO2输出x微秒高电平
    hi_gpio_set_ouput_val(HI_GPIO_IDX_2, HI_GPIO_VALUE1);
    hi_udelay(duty);

    // GPIO2输出20000-x微秒低电平
    hi_gpio_set_ouput_val(HI_GPIO_IDX_2, HI_GPIO_VALUE0);
    hi_udelay(20000 - duty);
}

/*
1、依据角度与脉冲的关系，设置高电平时间为500微秒，控制舵机旋转0度。
2、发送10次脉冲信号。
*/
void engine_run_0(void)
{
    for (int i = 0; i < 10; i++)
    {
        set_angle(500);
    }
}

/*
1、依据角度与脉冲的关系，设置高电平时间为1000微秒，控制舵机旋转45度。
2、发送10次脉冲信号。
*/
void engine_run_45(void)
{
    for (int i = 0; i < 10; i++)
    {
        set_angle(1000);
    }
}

/*
1、依据角度与脉冲的关系，设置高电平时间为1500微秒，控制舵机旋转90度。
2、发送10次脉冲信号。
*/
void engine_run_90(void)
{
    for (int i = 0; i < 10; i++)
    {
        set_angle(1500);
    }
}

/*
1、依据角度与脉冲的关系，设置高电平时间为2000微秒，控制舵机旋转135度。
2、发送10次脉冲信号。
*/
void engine_run_135(void)
{
    for (int i = 0; i < 10; i++)
    {
        set_angle(2000);
    }
}

/*
1、依据角度与脉冲的关系，设置高电平时间为2500微秒，控制舵机向右旋转180度。
2、发送10次脉冲信号。
*/
void engine_run_180(void)
{
    for (int i = 0; i < 10; i++)
    {
        set_angle(2500);
    }
}

/***** 任务一 *****/
static void thread1(void)
{
    osDelay(100U);
    while (1)
    {
        osMutexAcquire(mutex_id, osWaitForever);
        printf("thread1 is running.\r\n");
        flag = 90;
        engine_run_90();
        osDelay(500U);
        osMutexRelease(mutex_id);
    }
}

/***** 任务二 *****/
static void thread2(void)
{
    osDelay(100U);
    while (1)
    {
        printf("thread2 is running.\r\n");
        switch (flag)
        {
            case 90:
                printf("SG90 turn 90 du.\r\n");
                break;
            case 180:
                printf("SG90 turn 180 du.\r\n");
                break;
            default:
                break;
        }
        flag = 0;
        osDelay(100);
    }
}

/***** 任务三 *****/
static void thread3(void)
{
    while (1)
    {
        osMutexAcquire(mutex_id, osWaitForever);
        printf("thread3 is running.\r\n");
        flag = 180;
        engine_run_180();
        osDelay(300U);
        osMutexRelease(mutex_id);
    }
}

/****** 任务创建 *****/
static void SG90(void)
{
    // 初始化GPIO
    hi_io_set_func(HI_IO_NAME_GPIO_2, HI_IO_FUNC_GPIO_2_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_2, HI_GPIO_DIR_OUT);

    osThreadAttr_t attr;
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 4;

    attr.name = "thread1";
    attr.priority = 26;
    if (osThreadNew((osThreadFunc_t)thread1, NULL, &attr) == NULL)
    {
        printf("Failed to create thread1!\n");
    }

    attr.name = "thread2";
    attr.priority = 25;
    if (osThreadNew((osThreadFunc_t)thread2, NULL, &attr) == NULL)
    {
        printf("Failed to create thread2!\n");
    }

    attr.name = "thread3";
    attr.priority = 24;
    if (osThreadNew((osThreadFunc_t)thread3, NULL, &attr) == NULL)
    {
        printf("Failed to create thread3!\n");
    }

    mutex_id = osMutexNew(NULL);
    if (mutex_id == NULL)
    {
        printf("Failed to create Mutex!\n");
    }
}

// 启动任务
APP_FEATURE_INIT(SG90);
