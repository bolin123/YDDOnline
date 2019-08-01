#ifndef HAL_ADC_H
#define HAL_ADC_H

#include "HalCtype.h"

uint16_t HalADCGetCollectValue(uint8_t ch);
uint16_t HalADCGetPowerValue(void);
void HalADCMaxValueClear(uint8_t ch);

void HalADCStop(void);
void HalADCStart(void);

void HalADCInitialize(void);
#endif

