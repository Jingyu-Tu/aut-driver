#ifndef HAL_BSP_SSD1306_H
#define HAL_BSP_SSD1306_H

#include <stdint.h>

void SSD1306_Init(void);
void SSD1306_CLS(void);
void SSD1306_ShowStr(uint8_t x, uint8_t y, uint8_t *str, uint8_t size);

#endif
