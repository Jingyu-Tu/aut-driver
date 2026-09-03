#ifndef __SOFT_UART_RX_H
#define __SOFT_UART_RX_H

#include "stm32f10x.h"

void SoftUartRx_Init(void);
uint8_t SoftUartRx_ReadByte(uint8_t *data);
uint32_t SoftUartRx_GetByteCount(void);
uint8_t SoftUartRx_IsReceiving(void);

#endif