#include "i2c_slave.h"

static I2C_HandleTypeDef* m_hi2c;
static uint8_t Rxbuffer[SLAVE_RX_BUF_SIZE] = {0};
static uint8_t Txbuffer[SLAVE_TX_BUF_SIZE] = {0};

// 初始化 I2C 從設備
void I2C_Slave_Init(I2C_HandleTypeDef *hi2c) {
  m_hi2c = hi2c;
  HAL_I2C_EnableListen_IT(m_hi2c);
}

void updateTxBuffer(uint8_t *data, uint16_t size) {
  if (!data) {
    return; // 防止空指標
  }
  if (size > SLAVE_TX_BUF_SIZE) {
    size = SLAVE_TX_BUF_SIZE; // 限制大小以防止溢出
  }
  memcpy(Txbuffer, data, size);
}

void setTxBufferByte(uint16_t index, uint8_t value) {
  if (index < SLAVE_TX_BUF_SIZE) {
    Txbuffer[index] = value;
  }
}

// 取得接收 buffer 指標
uint8_t* I2C_Slave_GetRxBuffer(void) {
  return Rxbuffer;
}

// 取得傳送 buffer 指標
uint8_t *I2C_Slave_GetTxBuffer(void) { return Txbuffer; }

/* I2C event callback */
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode) {
  // if (hi2c->Instance != m_hi2c->Instance) { return; }

  // master 將寫資料到 slave → slave 應準備接收
  if (TransferDirection == I2C_DIRECTION_TRANSMIT) {
    // master 將寫資料到 slave → slave 應準備接收 (使用中斷)
    HAL_I2C_Slave_Seq_Receive_IT(
      m_hi2c, 
      Rxbuffer, 
      SLAVE_RX_BUF_SIZE,
      I2C_FIRST_AND_LAST_FRAME
    );
  } else {
    // master 從 slave 讀取資料 → slave 應準備傳送 (使用中斷)
    HAL_I2C_Slave_Seq_Transmit_IT(
      m_hi2c, 
      Txbuffer, 
      SLAVE_TX_BUF_SIZE,
      I2C_FIRST_AND_LAST_FRAME
    );
  }
}



void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c) {
  // if (hi2c->Instance != m_hi2c->Instance) { return; }

  // 重新啟用 Listen 模式
  HAL_I2C_EnableListen_IT(m_hi2c);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c) {
  if (hi2c->Instance != m_hi2c->Instance) { return; }

  // 在此處處理接收到的資料，資料存儲在 Rxbuffer 中
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c) {
  if (hi2c->Instance != m_hi2c->Instance) { return; }

  // 在此處準備下一批要傳送的資料，資料存儲在 Txbuffer 中
  // 例如：更新感測器數值、狀態資訊等
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
  if (hi2c->Instance != m_hi2c->Instance) { return; }

  // 清除錯誤狀態和緩衝區
  memset(Rxbuffer, 0, SLAVE_RX_BUF_SIZE);
  memset(Txbuffer, 0, SLAVE_TX_BUF_SIZE);
  
  // 重新啟用 Listen 模式
  HAL_I2C_EnableListen_IT(m_hi2c);
}