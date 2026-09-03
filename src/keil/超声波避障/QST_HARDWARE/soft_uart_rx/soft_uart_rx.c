#include "soft_uart_rx.h"
#include "delay.h"

#define SOFT_RX_PIN   GPIO_Pin_8
#define SOFT_RX_PORT  GPIOB
#define BAUD_RATE     2400
#define BIT_TIME_US   (1000000 / BAUD_RATE)  // ≈ 416us

static uint32_t rx_byte_count = 0;
static uint8_t is_receiving = 0;

void SoftUartRx_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = SOFT_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(SOFT_RX_PORT, &GPIO_InitStructure);
}

uint8_t SoftUartRx_ReadByte(uint8_t *data)
{
    uint8_t byte = 0;
    uint8_t bit;
    
    // 等待起始位（低电平）
    uint32_t timeout = 0;
    while (GPIO_ReadInputDataBit(SOFT_RX_PORT, SOFT_RX_PIN) == 1)
    {
        timeout++;
        if (timeout > 100000) return 0;
    }
    
    is_receiving = 1;
    
    // 等待起始位中间
    delay_us(BIT_TIME_US / 2);
    
    // 读取 8 位数据
    for (bit = 0; bit < 8; bit++)
    {
        delay_us(BIT_TIME_US);
        if (GPIO_ReadInputDataBit(SOFT_RX_PORT, SOFT_RX_PIN) == 1)
        {
            byte |= (1 << bit);
        }
    }
    
    // 等待停止位
    delay_us(BIT_TIME_US);
    
    *data = byte;
    rx_byte_count++;
    is_receiving = 0;
    
    return 1;
}

uint32_t SoftUartRx_GetByteCount(void)
{
    return rx_byte_count;
}

uint8_t SoftUartRx_IsReceiving(void)
{
    return is_receiving;
}