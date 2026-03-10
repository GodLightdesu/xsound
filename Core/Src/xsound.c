#include "xsound.h"

// Timer configuration based on ultrasonic sensor requirements
// TIM2 & TIM5: Prescaler = 170-1, with 170MHz clock -> 1µs per tick
#define UTS_TIC_US 1.0f  // Timer tick period in microseconds

typedef struct {
  TIM_HandleTypeDef *htim;
  uint32_t channel1;          // Rising edge capture channel
  uint32_t channel2;          // Falling edge capture channel
  int upEdgeTime;
  int downEdgeTime;
  int timeDiff;
  float distance;
  bool dataReady;
  uint32_t captureCount;
} XSound_Sensor_t;

static XSound_Sensor_t sensors[XSOUND_NUM_SENSORS] = {0};
static float distances[XSOUND_NUM_SENSORS] = {0.0f};  // Array to store all distances

// Delay in timer ticks (1 tick = 1µs with prescaler 170-1)
static void delay_ticks(uint8_t sensorId, uint32_t ticks) {
  if (sensorId >= XSOUND_NUM_SENSORS) return;
  TIM_HandleTypeDef *htim = sensors[sensorId].htim;
  uint32_t start = __HAL_TIM_GET_COUNTER(htim);
  while ((__HAL_TIM_GET_COUNTER(htim) - start) < ticks);
}

void XSound_Init(uint8_t sensorId, TIM_HandleTypeDef *htim, uint32_t channel1, uint32_t channel2) {
  if (sensorId >= XSOUND_NUM_SENSORS) return;
  
  sensors[sensorId].htim = htim;
  sensors[sensorId].channel1 = channel1;
  sensors[sensorId].channel2 = channel2;
  sensors[sensorId].dataReady = false;
  sensors[sensorId].captureCount = 0;
  
  HAL_TIM_Base_Start(htim);
  HAL_TIM_IC_Start(htim, channel1);    // capture rising edge
  HAL_TIM_IC_Start_IT(htim, channel2); // capture falling edge with interrupt
}

// Input Trig Pin
void XSound_Trig(uint8_t sensorId, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
  if (sensorId >= XSOUND_NUM_SENSORS) return;
  
  sensors[sensorId].dataReady = false;
  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
  delay_ticks(sensorId, 10); // 10µs trigger pulse (spec requires >10µs)
  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
  __HAL_TIM_SET_COUNTER(sensors[sensorId].htim, 0); // reset counter
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  uint32_t activeChannel = htim->Channel;
  
  // Find which sensor triggered the interrupt
  for (uint8_t i = 0; i < XSOUND_NUM_SENSORS; i++) {
    if (htim != sensors[i].htim) continue;
    
    // Check if this sensor's falling edge channel triggered the interrupt
    bool isMatch = false;
    switch (sensors[i].channel2) {
      case TIM_CHANNEL_1: isMatch = (activeChannel == HAL_TIM_ACTIVE_CHANNEL_1); break;
      case TIM_CHANNEL_2: isMatch = (activeChannel == HAL_TIM_ACTIVE_CHANNEL_2); break;
      case TIM_CHANNEL_3: isMatch = (activeChannel == HAL_TIM_ACTIVE_CHANNEL_3); break;
      case TIM_CHANNEL_4: isMatch = (activeChannel == HAL_TIM_ACTIVE_CHANNEL_4); break;
    }
    
    if (isMatch) {
      sensors[i].captureCount++;
      
      sensors[i].upEdgeTime = HAL_TIM_ReadCapturedValue(sensors[i].htim, sensors[i].channel1);
      sensors[i].downEdgeTime = HAL_TIM_ReadCapturedValue(sensors[i].htim, sensors[i].channel2);
      
      if (sensors[i].downEdgeTime > sensors[i].upEdgeTime) {
        sensors[i].timeDiff = sensors[i].downEdgeTime - sensors[i].upEdgeTime;
      } else {
        // Handle counter overflow (TIM2/TIM5 are 32-bit, period = 4294967295)
        sensors[i].timeDiff = (4294967295 - sensors[i].upEdgeTime) + sensors[i].downEdgeTime;
      }
      
      // timeDiff is in ticks, 1 tick = 1µs (prescaler = 170-1)
      // Distance (cm) = timeDiff * 1µs * 0.0343cm/µs / 2
      // Simplified: timeDiff * (1 * 0.0343 / 2) = timeDiff * 0.01715
      sensors[i].distance = sensors[i].timeDiff * 0.01715f; // cm
      
      // HC-SR04 max effective range is ~400cm (23530µs round trip)
      // If distance > 450cm, likely invalid echo or timeout
      sensors[i].dataReady = (sensors[i].distance > 0 && sensors[i].distance < 450);
      return;  // Found and processed, exit early
    }
  }
}

float XSound_GetDistance(uint8_t sensorId) {
  if (sensorId >= XSOUND_NUM_SENSORS) return 0.0f;
  sensors[sensorId].dataReady = false; // Clear flag after reading
  distances[sensorId] = sensors[sensorId].distance; // Update array
  return sensors[sensorId].distance;
}

float* XSound_GetAllDistances(void) {
  return distances;
}

void XSound_UpdateAllDistances(void) {
  for (uint8_t i = 0; i < XSOUND_NUM_SENSORS; i++) {
    if (XSound_DataReady(i)) {
      distances[i] = XSound_GetDistance(i);
    }
  }
}

bool XSound_DataReady(uint8_t sensorId) {
  if (sensorId >= XSOUND_NUM_SENSORS) return false;
  return sensors[sensorId].dataReady;
}

uint32_t XSound_GetCaptureCount(uint8_t sensorId) {
  if (sensorId >= XSOUND_NUM_SENSORS) return 0;
  return sensors[sensorId].captureCount;
}

int XSound_GetUpEdge(uint8_t sensorId) {
  if (sensorId >= XSOUND_NUM_SENSORS) return 0;
  return sensors[sensorId].upEdgeTime;
}

int XSound_GetDownEdge(uint8_t sensorId) {
  if (sensorId >= XSOUND_NUM_SENSORS) return 0;
  return sensors[sensorId].downEdgeTime;
}

int XSound_GetTimeDiff(uint8_t sensorId) {
  if (sensorId >= XSOUND_NUM_SENSORS) return 0;
  return sensors[sensorId].timeDiff;
}