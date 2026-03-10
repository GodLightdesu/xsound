#ifndef XSOUND_H
#define XSOUND_H

#include "tim.h"
#include "gpio.h"
#include <stdbool.h>

#define XSOUND_NUM_SENSORS 4

void XSound_Init(uint8_t sensorId, TIM_HandleTypeDef *htim, uint32_t channel1, uint32_t channel2);
void XSound_Trig(uint8_t sensorId, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
float XSound_GetDistance(uint8_t sensorId);
float* XSound_GetAllDistances(void);
void XSound_UpdateAllDistances(void);
bool XSound_DataReady(uint8_t sensorId);
uint32_t XSound_GetCaptureCount(uint8_t sensorId);
int XSound_GetUpEdge(uint8_t sensorId);
int XSound_GetDownEdge(uint8_t sensorId);
int XSound_GetTimeDiff(uint8_t sensorId);

#endif /* XSOUND_H */