#ifndef SPRESS_H
#define SPRESS_H

#include "Sensors.h"
void SPressStart(SensorsNode_t *sensor);
void SPressSampling(SensorsNode_t *sensor, uint16_t *buff, uint16_t index);
uint16_t SPressGetAmplitude(SensorsNode_t *sensor, uint16_t *buff, uint16_t count);
uint16_t SPressGetFrequency(SensorsNode_t *sensor, uint32_t lastTime);
void SPressNext(SensorsNode_t *sensor);
void SPressStop(SensorsNode_t *sensor);
#endif

