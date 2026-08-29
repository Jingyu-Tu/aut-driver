#ifndef HAL_BSP_SHT20_H
#define HAL_BSP_SHT20_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHT20_I2C_IDX   0   // I2C 索引
#define SHT20_I2C_ADDR  0x40 // SHT20 设备地址

// SHT20 初始化
uint32_t SHT20_Init(void);

// 读取温湿度数据
uint32_t SHT20_ReadData(float *temp, float *humi);

#ifdef __cplusplus
}
#endif

#endif /* HAL_BSP_SHT20_H */
