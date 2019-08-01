#ifndef DATA_COLLECT_H
#define DATA_COLLECT_H

#include "Sys.h"

#define DATA_COLLECT_PERIOD_NUM 10
#define DATA_COLLECT_TIME_MIN 20 //最小测试时间 20s

typedef struct
{
    uint16_t amplitude;
    uint16_t frequency;
}DataCollectContext_t;

typedef struct 
{
    bool start;
    uint8_t periodCount;   //周期计数（1秒为一个周期）
    uint16_t threshold; //阈值
    uint16_t periodData[DATA_COLLECT_PERIOD_NUM][HAL_ADC_CH_NUM]; //1秒钟采集的数据，然后取平均值
    uint16_t dataCount;       //当前采集到的数据点数
    uint16_t collectCount;    //总共需要采集的点数
    uint32_t flashAddress;    //保存数据的flash地址
    uint32_t lastCollectTime; //上一次采集的时间
    uint32_t periodStartTime; //一秒周期开始的时间，用于计算频率
    SysDateTime_t collectDate;//开始日期
}DataCollect_st;

typedef enum
{
    DATA_COLLECT_EVENT_UPDATE,
    DATA_COLLECT_EVENT_DONE,
}DataCollectEvent_t;

typedef void (* DataCollectEventHandle_t)(DataCollectEvent_t event, uint8_t chn, void *args);

DataCollect_st *DataCollectCreate(void);
bool DataCollectIsStart(DataCollect_st *dcollect);
void DataCollectStop(DataCollect_st *dcollect);
void DataCollectStart(uint32_t address, DataCollect_st *dcollect);
void DataCollectInitialize(DataCollectEventHandle_t eventHandle);
void DataCollectPoll(DataCollect_st *dcollect);

#endif

