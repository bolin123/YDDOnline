#ifndef SNOISE_H
#define SNOISE_H

#include "Sensors.h"
void SNoiseStart(SensorsNode_t *sensor);
void SNoiseSampling(SensorsNode_t *sensor, uint16_t *buff, uint16_t index);
uint16_t SNoiseGetAmplitude(SensorsNode_t *sensor, uint16_t *buff, uint16_t count);
uint16_t SNoiseGetFrequency(SensorsNode_t *sensor, uint32_t lastTime);
//void SNoiseFrequencyTrigger(void);

void SNoiseNext(SensorsNode_t *sensor);
void SNoiseStop(SensorsNode_t *sensor);
#endif

