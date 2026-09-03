#ifndef CAR_UART_H
#define CAR_UART_H

#include <stdbool.h>
#include <stdint.h>

bool CarUart_Init(void);
bool CarUart_SendCommand(uint8_t command);
bool CarUart_SendByte(uint8_t byte);              // ← 添加
bool CarUart_SendProtocolCommand(uint8_t cmd);    // ← 添加


// 命令定义（和STM32保持一致）
#define CAR_COMMAND_FORWARD  'F'
#define CAR_COMMAND_REVERSE  'B'
#define CAR_COMMAND_LEFT     'L'
#define CAR_COMMAND_RIGHT    'R'
#define CAR_COMMAND_STOP     'S'

#endif
