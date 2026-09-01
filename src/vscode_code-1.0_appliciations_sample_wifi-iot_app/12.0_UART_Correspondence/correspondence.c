#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include "wifiiot_uart.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "hi_io.h"
#include "hi_time.h"
#include "wifiiot_pwm.h"
#include "hi_uart.h"
#include "hi_pwm.h"
#include "wifiiot_gpio_ex.h"

static void thread1(void);
static void thread2(void);

uint8_t uart_sendbuf[20];
osMutexId_t mutex_id;

/*** 通信协议 ***/
/* 函数功能：发送至 STM32 的数据协议 */
/* 参数：电机实际转速的一百倍，例如：设置转速为 1rad/s，则传入 100 */
void stm32motor_control(int motorA, int motorB)
{
    uint8_t A_dir = 0;
    uint8_t B_dir = 0;

    // 小车运动方向：前进（正转）：0，后退（反转）：1
    if (motorA < 0) {
        A_dir = 1;
        motorA = -motorA;
    } else {
        A_dir = 0;
    }
    if (motorB < 0) {
        B_dir = 1;
        motorB = -motorB;
    } else {
        B_dir = 0;
    }

    // 限制幅度 -150 ~ 150
    if (motorA > 150) motorA = 150;
    if (motorB > 150) motorB = 150;

    // 数据协议：帧头 + 方向 + 速度 + 方向 + 速度 + 帧尾
    uart_sendbuf[0] = 0xFC;      // 帧头
    uart_sendbuf[1] = A_dir;     // 左轮方向：0正转，1反转
    uart_sendbuf[2] = motorA; // 左轮速度
    uart_sendbuf[3] = B_dir;     // 右轮方向：0正转，1反转
    uart_sendbuf[4] = motorB; // 右轮速度
    uart_sendbuf[5] = 0xFD;      // 帧尾

    UartWrite(WIFI_IOT_UART_IDX_2, (unsigned char *)uart_sendbuf, 6);
}

// 小车后退
void car_backward(void)
{
    stm32motor_control(-100, -100);
}

// 小车前进
void car_forward(void)
{
    stm32motor_control(100, 100);
}

// 小车左转
void car_left(void)
{
    stm32motor_control(50, 150);
}

// 小车右转
void car_right(void)
{
    stm32motor_control(150, 50);
}

// 小车停止
void car_stop(void)
{
    stm32motor_control(0, 0);
}

/***** 任务一 *****/
static void thread1(void)
{
    while (1)
    {
        osMutexAcquire(mutex_id, osWaitForever);
        car_forward();           // 前进
        usleep(1000000);         // 延时 1s
        osMutexRelease(mutex_id);
    }
}

/***** 任务二 *****/
static void thread2(void)
{
    sleep(1);                    // 休眠 1 秒
    while (1)
    {
        osMutexAcquire(mutex_id, osWaitForever);
        car_left();              // 左转
        usleep(1000000);         // 延时 1s
        osMutexRelease(mutex_id);
    }
}

/****** 任务创建 ******/
static void correspondence(void)
{
    // GPIO 功能初始化
    GpioInit();

    // GPIO_11 复用为 UART2_TXD
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_UART2_TXD);
    // GPIO_12 复用为 UART2_RXD
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_UART2_RXD);

    // 串口参数配置
    WifiIotUartAttribute uart_attr2 = {
        .baudRate = 115200,
        .dataBits = 8,
        .stopBits = 1,
        .parity = 0,
    };
    UartInit(WIFI_IOT_UART_IDX_2, &uart_attr2,NULL);

    osThreadAttr_t attr;
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 4;

    // 创建任务1
    attr.name = "thread1";
    attr.priority = 25;
    if (osThreadNew((osThreadFunc_t)thread1, NULL, &attr) == NULL) {
        printf("Failed to create thread1!\n");
    }

    // 创建任务2
    attr.name = "thread2";
    attr.priority = 25;
    if (osThreadNew((osThreadFunc_t)thread2, NULL, &attr) == NULL) {
        printf("Failed to create thread2!\n");
    }

    // 创建互斥锁
    mutex_id = osMutexNew(NULL);
    if (mutex_id == NULL) {
        printf("Failed to create Mutex!\n");
    }
}

APP_FEATURE_INIT(correspondence);
