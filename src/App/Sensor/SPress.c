#include "SPress.h"

static uint16_t pressureValueExchange(uint16_t *data, uint16_t len)
{
    uint16_t i;
    uint32_t result = 0;
    uint32_t count = 0, average = 0;

    for(i = 0; i < len; i++)
    {
        count += data[i];
    }

    average = (count / len);
    result = average * 10 * 25 / 4096; //157.5 = 4096 /25MPa ,10倍存储小数点后一位，显示会自动除10
    return result;
}

void SPressStart(SensorsNode_t *sensor)
{
    if(sensor)
    {
        sensor->freqCount = 0;
    }
}

void SPressSampling(SensorsNode_t *sensor, uint16_t *buff, uint16_t index)
{
    if(sensor)
    {
        if(index < sensor->buffNum)
        {
            buff[index] = HalADCGetCollectValue(sensor->id);
        }
    }
}

uint16_t SPressGetAmplitude(SensorsNode_t *sensor, uint16_t *buff, uint16_t count)
{
    if(sensor)
    {
        return pressureValueExchange(buff, count);
    }
    return 0;
}

uint16_t SPressGetFrequency(SensorsNode_t *sensor, uint32_t lastTime)
{
    return 0;
}

void SPressNext(SensorsNode_t *sensor)
{
    
}

void SPressStop(SensorsNode_t *sensor)
{
    
}

