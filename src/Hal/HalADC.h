#ifndef HAL_ADC_H
#define HAL_ADC_H

#include "HalCtype.h"

typedef enum
{
    HAL_ADC_CHNL_GEOM = 0,
    HAL_ADC_CHNL_NOISE,
    HAL_ADC_CHNL_PRESS1,
    HAL_ADC_CHNL_PRESS2,
    HAL_ADC_CHNL_POWER,
    HAL_ADC_CHNL_COUNT,
}HalADCChannel_t;

uint16_t HalADCGetCollectValue(uint8_t ch);
uint16_t HalADCGetPowerValue(void);
void HalADCMaxValueClear(uint8_t ch);

void HalADCStop(void);
void HalADCStart(void);

void HalADCInitialize(void);
#endif

