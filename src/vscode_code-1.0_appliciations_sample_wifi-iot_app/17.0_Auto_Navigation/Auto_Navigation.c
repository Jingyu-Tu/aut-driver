#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include "wifiiot_uart.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "hi_io.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_watchdog.h"
#include "hi_time.h"

/************** 引脚定义 **************/
#define GPIO_IR_LEFT    12
#define GPIO_IR_CENTER  13
#define GPIO_IR_RIGHT   14
#define MOTOR_UART      WIFI_IOT_UART_IDX_2

/************** 速度配置 **************/
#define SPEED_BASE      40
#define SPEED_TURN      28

/************** 红外极性（根据你的传感器实际值修改） **************/
#define BLACK_VALUE     1
#define WHITE_VALUE     0

typedef enum {
    DIR_FORWARD,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_STOP
} CarDirection;

static uint8_t uart_sendbuf[20];

/************** 电机控制 **************/
void stm32motor_control(int motorA, int motorB)
{
    uint8_t A_dir = 0, B_dir = 0;
    if (motorA < 0) { A_dir = 1; motorA = -motorA; }
    if (motorB < 0) { B_dir = 1; motorB = -motorB; }
    if (motorA > 150) motorA = 150;
    if (motorB > 150) motorB = 150;

    uart_sendbuf[0] = 0xFC;
    uart_sendbuf[1] = A_dir;
    uart_sendbuf[2] = motorA;
    uart_sendbuf[3] = B_dir;
    uart_sendbuf[4] = motorB;
    uart_sendbuf[5] = 0xFD;
    UartWrite(MOTOR_UART, (unsigned char *)uart_sendbuf, 6);
}

void car_forward(void)  { stm32motor_control(SPEED_BASE, SPEED_BASE); }
void car_left(void)     { stm32motor_control(-SPEED_TURN, SPEED_TURN); }
void car_right(void)    { stm32motor_control(SPEED_TURN, -SPEED_TURN); }
void car_stop(void)     { stm32motor_control(0, 0); }

/************** 读取三路传感器 **************/
void GetIRStatus(int *left, int *center, int *right)
{
    WifiIotGpioValue l, c, r;
    GpioGetInputVal(GPIO_IR_LEFT, &l);
    GpioGetInputVal(GPIO_IR_CENTER, &c);
    GpioGetInputVal(GPIO_IR_RIGHT, &r);
    *left = l;
    *center = c;
    *right = r;
}

/************** 循迹判断（精简版，适配窄黑线） **************/
CarDirection GetLineDirection(void)
{
    int left, center, right;
    GetIRStatus(&left, &center, &right);

    // 打印调试
    // printf("L=%d C=%d R=%d\r\n", left, center, right);

    // ★★★ 核心：中间传感器必须一直对准黑线 ★★★

    // 1. 完美居中：中间黑，左右白 → 直走
    if (center == BLACK_VALUE && left == WHITE_VALUE && right == WHITE_VALUE) {
        return DIR_FORWARD;
    }

    // 2. 左边偏了（左边也变成黑）→ 左转调回中间
    if (center == BLACK_VALUE && left == BLACK_VALUE && right == WHITE_VALUE) {
        return DIR_LEFT;
    }

    // 3. 右边偏了（右边也变成黑）→ 右转调回中间
    if (center == BLACK_VALUE && left == WHITE_VALUE && right == BLACK_VALUE) {
        return DIR_RIGHT;
    }

    // 4. 中间变成白（黑线脱了）→ 左右传感器判断方向
    if (center == WHITE_VALUE) {
        // 左边是黑 → 黑线在左边 → 左转找线
        if (left == BLACK_VALUE) {
            return DIR_LEFT;
        }
        // 右边是黑 → 黑线在右边 → 右转找线
        if (right == BLACK_VALUE) {
            return DIR_RIGHT;
        }
        // 左右都是白 → 完全脱线
        if (left == WHITE_VALUE && right == WHITE_VALUE) {
            return DIR_STOP;
        }
    }

    // 5. 全是白（完全脱线）→ 停止
    if (left == WHITE_VALUE && center == WHITE_VALUE && right == WHITE_VALUE) {
        return DIR_STOP;
    }

    return DIR_FORWARD;
}

/************** 循迹主任务 **************/
static void line_follow_task(void)
{
    CarDirection dir;
    int lost_counter = 0;
    int stable_counter = 0;
    CarDirection last_dir = DIR_FORWARD;

    printf("[LINE] Follow started! (narrow black line mode)\n");

    while (1)
    {
        dir = GetLineDirection();

        // 去抖动
        if (dir == last_dir) {
            stable_counter++;
        } else {
            stable_counter = 0;
            last_dir = dir;
        }

        if (stable_counter < 2) {
            osDelay(1);
            continue;
        }

        switch (dir)
        {
            case DIR_FORWARD:
                car_forward();
                lost_counter = 0;
                break;

            case DIR_LEFT:
                car_left();
                lost_counter = 0;
                break;

            case DIR_RIGHT:
                car_right();
                lost_counter = 0;
                break;

            case DIR_STOP:
            default:
                lost_counter++;
                if (lost_counter < 30) {
                    car_left();
                } else {
                    car_stop();
                    printf("[LINE] Lost track!\n");
                }
                break;
        }

        osDelay(2);
    }
}

/************** 初始化 **************/
static void line_follow_init(void)
{
    WatchDogDisable();
    GpioInit();

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_GPIO);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_GPIO);
    GpioSetDir(GPIO_IR_LEFT, WIFI_IOT_GPIO_DIR_IN);
    GpioSetDir(GPIO_IR_CENTER, WIFI_IOT_GPIO_DIR_IN);
    GpioSetDir(GPIO_IR_RIGHT, WIFI_IOT_GPIO_DIR_IN);

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_UART2_TXD);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_UART2_RXD);
    WifiIotUartAttribute uart_attr2 = {
        .baudRate = 115200,
        .dataBits = 8,
        .stopBits = 1,
        .parity = 0,
    };
    UartInit(MOTOR_UART, &uart_attr2, NULL);

    osThreadAttr_t attr = {0};
    attr.stack_size = 1024 * 8;
    attr.name = "line_follow";
    attr.priority = 25;
    if (osThreadNew((osThreadFunc_t)line_follow_task, NULL, &attr) == NULL) {
        printf("[LINE] Failed to create task!\n");
    }
}

APP_FEATURE_INIT(line_follow_init);
