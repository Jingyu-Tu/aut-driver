#include "hal_bsp_ssd1306.h"
#include <stdio.h>
#include <string.h>
#include "wifiiot_i2c.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

// OLED 显示屏尺寸
#define OLED_WIDTH  128
#define OLED_HEIGHT 64

// I2C 配置
#define OLED_I2C_IDX 0
#define OLED_I2C_ADDR 0x3C

// 显存缓冲区
static uint8_t OLED_Buffer[1024];

// 发送命令
static void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd};
    WifiIotI2cData i2cData;
    i2cData.sendBuf = buf;
    i2cData.sendLen = 2;
    i2cData.receiveBuf = NULL;
    i2cData.receiveLen = 0;
    I2cWrite(OLED_I2C_IDX, OLED_I2C_ADDR, &i2cData);
}

// 发送数据
static void OLED_WriteData(uint8_t data)
{
    uint8_t buf[2] = {0x40, data};
    WifiIotI2cData i2cData;
    i2cData.sendBuf = buf;
    i2cData.sendLen = 2;
    i2cData.receiveBuf = NULL;
    i2cData.receiveLen = 0;
    I2cWrite(OLED_I2C_IDX, OLED_I2C_ADDR, &i2cData);
}

// 更新整个显存到 OLED
static void OLED_Update(void)
{
    for (uint8_t page = 0; page < 8; page++) {
        OLED_WriteCmd(0xB0 + page);
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);
        for (uint8_t col = 0; col < 128; col++) {
            OLED_WriteData(OLED_Buffer[page * 128 + col]);
        }
    }
}

// OLED 初始化
void SSD1306_Init(void)
{
    I2cInit(OLED_I2C_IDX, 400000);
    
    // 初始化序列
    OLED_WriteCmd(0xAE);
    OLED_WriteCmd(0xD5);
    OLED_WriteCmd(0x80);
    OLED_WriteCmd(0xA8);
    OLED_WriteCmd(0x3F);
    OLED_WriteCmd(0xD3);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0x8D);
    OLED_WriteCmd(0x14);
    OLED_WriteCmd(0x20);
    OLED_WriteCmd(0x02);
    OLED_WriteCmd(0xA1);
    OLED_WriteCmd(0xC8);
    OLED_WriteCmd(0xDA);
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(0x81);
    OLED_WriteCmd(0xCF);
    OLED_WriteCmd(0xD9);
    OLED_WriteCmd(0xF1);
    OLED_WriteCmd(0xDB);
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0xA4);
    OLED_WriteCmd(0xA6);
    OLED_WriteCmd(0x2E);
    OLED_WriteCmd(0xAF);
    
    SSD1306_CLS();
    printf("I2C SSD1306 Init is succeeded!!!\r\n");
}

// 清屏
void SSD1306_CLS(void)
{
    memset(OLED_Buffer, 0, sizeof(OLED_Buffer));
    OLED_Update();
}

// 设置像素点
static void OLED_SetPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    uint16_t index = (y / 8) * 128 + x;
    if (color) {
        OLED_Buffer[index] |= (1 << (y % 8));
    } else {
        OLED_Buffer[index] &= ~(1 << (y % 8));
    }
}

// 显示一个方块（代替汉字）
static void OLED_ShowBlock(uint8_t x, uint8_t y)
{
    for (uint8_t row = 0; row < 16; row++) {
        for (uint8_t col = 0; col < 16; col++) {
            OLED_SetPixel(x + col, y + row, 1);
        }
    }
}

// 显示字符串
void SSD1306_ShowStr(uint8_t x, uint8_t y, uint8_t *str, uint8_t size)
{
    (void)size;
    (void)x;
    (void)y;
    
    // 打印到串口调试
    printf("OLED: %s\n", str);
    
    // 在 OLED 上显示方块
    uint8_t index = 0;
    while (*str && index < 8) {
        OLED_ShowBlock(index * 16, y);
        index++;
        str++;
    }
    OLED_Update();
}
