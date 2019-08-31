#include "SNoise.h"

static uint16_t noiseValueExchange(uint16_t value)
{
    uint32_t result = 0;
    uint32_t mv = value * 3300 / 4096;

    printf("noise = %d\n", value);
    if(mv < 1500)
    {
        result = 0;
    }
    else
    {
        result = mv - 1500;
    }
    return result;
}

void SNoiseStart(SensorsNode_t *sensor)
{
    if(sensor)
    {
        sensor->freqCount = 0;
        HalADCMaxValueClear(sensor->id);
    }
}

void SNoiseSampling(SensorsNode_t *sensor, uint16_t *buff, uint16_t index)
{
    //do nothing
}

uint16_t SNoiseGetAmplitude(SensorsNode_t *sensor, uint16_t *buff, uint16_t count)
{
    if(sensor)
    {
        return noiseValueExchange(HalADCGetCollectValue(sensor->id));
    }
    return 0;
}

uint16_t SNoiseGetFrequency(SensorsNode_t *sensor, uint32_t lastTime)
{
    if(sensor)
    {
        return (uint16_t)(sensor->freqCount * 10 / lastTime); //x10 :用整数表示精确到小数点后一位 ,显示时再除去10
    }
    return 0;
}

void SNoiseNext(SensorsNode_t *sensor)
{
    if(sensor)
    {
        sensor->freqCount = 0;
        HalADCMaxValueClear(sensor->id);
    }
}

void SNoiseStop(SensorsNode_t *sensor)
{
}


