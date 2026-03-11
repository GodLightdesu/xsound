#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

#define SLAVE_RX_BUF_SIZE 16
#define SLAVE_TX_BUF_SIZE 16 // 16 bytes for 4 float distances (4 bytes each)

#include <string.h>
#include "main.h"
#include "i2c.h"

// I2C Slave 初始化
void I2C_Slave_Init(I2C_HandleTypeDef *hi2c);

void updateTxBuffer(uint8_t *data, uint16_t size);
void setTxBufferByte(uint16_t index, uint8_t value);

// 取得接收 buffer 指標（供外部讀取接收到的資料）
uint8_t* I2C_Slave_GetRxBuffer(void);

// 取得傳送 buffer 指標（供外部更新要傳送的資料）
uint8_t* I2C_Slave_GetTxBuffer(void);

#endif /* I2C_SLAVE_H */