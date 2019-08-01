#ifndef SENSORS_H
#define SENSORS_H

#include "Sys.h"

#define SENSORS_SAMPLING_NUM_1S 10

typedef struct
{
    uint16_t amplitude;
    uint16_t frequency;
}SensorsContext_t;

typedef struct SensorsNode_st
{
    HalSensorID_t id;
    uint16_t *buff;
    uint16_t buffNum;
    uint32_t freqCount;
    void (*start)(struct SensorsNode_st *sensor);
    void (*sampling)(struct SensorsNode_st *sensor, uint16_t *buff, uint16_t index);
    uint16_t (*getAmplitude)(struct SensorsNode_st *sensor, uint16_t *buff, uint16_t count);
    uint16_t (*getFrequency)(struct SensorsNode_st *sensor, uint32_t lastTime);
    void (*next)(struct SensorsNode_st *sensor);
    void (*stop)(struct SensorsNode_st *sensor);
}SensorsNode_t;

typedef struct
{
    bool start;
    uint16_t sampleNum;    //���ڳ�������
    uint16_t dataNum;      //��ǰ���������
    uint16_t totalDataNum; //�ܹ�������ĸ���
    uint32_t sampleTime;   //���ڲ���ʱ��
    uint32_t startTime;    //��ʼ�ɼ���ʱ��
    SysDateTime_t startDateTime;
    uint32_t exflashOffset;
    SensorsNode_t node[HAL_SENSOR_ID_COUNT];
    void *private;
}Sensors_t;

typedef enum
{
    SENSORS_EVENT_SAMPLING_DONE = 0,
    SENSORS_EVENT_SAMPLING_UPDATE,
}SensorsEvent_t;

typedef void (* SensorsEvent_cb)(SensorsEvent_t event, uint8_t chn, void *args);

bool SensorsSamplingIsStart(Sensors_t *sensors);
void SensorsFrequencyTrigger(Sensors_t *sensors, HalSensorID_t id);
void SensorsSamplingStop(Sensors_t *sensors);
void SensorsSamplingStart(Sensors_t *sensors, uint32_t num, uint32_t exflashOffset);
Sensors_t *SensorsCreate(void);
void SensorsInitialize(SensorsEvent_cb eventCb);
void SensorsPoll(Sensors_t *sensors);
#endif 

