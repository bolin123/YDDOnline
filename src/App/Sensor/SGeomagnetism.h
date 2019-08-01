#ifndef SGEOMAGNETISM_H
#define SGEOMAGNETISM_H

#include "Sensors.h"
void SGeomagnetismStart(SensorsNode_t *sensor);
void SGeomagnetismSampling(SensorsNode_t *sensor, uint16_t *buff, uint16_t index);
uint16_t SGeomagnetismGetAmplitude(SensorsNode_t *sensor, uint16_t *buff, uint16_t count);
uint16_t SGeomagnetismGetFrequency(SensorsNode_t *sensor, uint32_t lastTime);
void SGeomagnetismNext(SensorsNode_t *sensor);
void SGeomagnetismStop(SensorsNode_t *sensor);
#endif

