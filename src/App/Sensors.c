#include "Sensors.h"
#include "SGeomagnetism.h"
#include "SPress.h"
#include "SNoise.h"
#include "PowerManager.h"

#define SENSORS_SAMPLING_NUM_MIN 20  //最小采样时间20s，20s保存一次flash
#define SENSORS_SAMPLING_INTERVAL (SYS_TIME_COUNT_1S / SENSORS_SAMPLING_NUM_1S)

static SensorsEvent_cb g_seventCb = NULL;
static SensorsContext_t g_context[SENSORS_SAMPLING_NUM_MIN][HAL_SENSOR_ID_COUNT];
static uint8_t g_contextCount = 0;

static void sensorsEventEmitt(SensorsEvent_t event, uint8_t chn, void *args)
{
    if(g_seventCb)
    {
        g_seventCb(event, chn, args);
    }
}

static void sensorSamplingPoll(Sensors_t *sensors)
{
    uint8_t i;
    uint32_t interval;
    if(sensors && sensors->start)
    {
        if(SysTimeHasPast(sensors->sampleTime , SENSORS_SAMPLING_INTERVAL)) //100ms采样一次
        {
            for (i = 0; i < HAL_SENSOR_ID_COUNT; i++)
            {
                sensors->node[i].sampling(&sensors->node[i], sensors->node[i].buff, sensors->sampleNum);
            }
            sensors->sampleNum++;

            if(sensors->sampleNum >= SENSORS_SAMPLING_NUM_1S) //获取一个采样点
            {
                interval = SysTime() - sensors->startTime; //时间间隔，用于计算频率
                for(i = 0;  i < HAL_SENSOR_ID_COUNT; i++)
                {
                    g_context[g_contextCount][i].amplitude = sensors->node[i].getAmplitude(&sensors->node[i], sensors->node[i].buff, sensors->sampleNum);
                    g_context[g_contextCount][i].frequency = sensors->node[i].getFrequency(&sensors->node[i], interval);
                    sensors->node[i].next(&sensors->node[i]);
                    
                    sensorsEventEmitt(SENSORS_EVENT_SAMPLING_UPDATE, i, (void *)&g_context[g_contextCount][i]);
                }
                
                g_contextCount++;
                if(g_contextCount >= SENSORS_SAMPLING_NUM_MIN) //SENSORS_SAMPLING_NUM_MIN个点保存一次flash
                {
                    SysRawDataWrite(sensors->exflashOffset, (uint8_t *)g_context, sizeof(g_context));
                    sensors->exflashOffset += sizeof(g_context);
                    g_contextCount = 0;
                }
                
                sensors->dataNum++;
                if(sensors->dataNum >= sensors->totalDataNum) //采样结束
                {
                    if(g_contextCount) //保存剩余数据
                    {
                        uint16_t lastnum = g_contextCount * sizeof(SensorsContext_t) * HAL_SENSOR_ID_COUNT;
                        SysRawDataWrite(sensors->exflashOffset, (uint8_t *)g_context, lastnum);
                    }
                    SensorsSamplingStop(sensors);
                    sensors->dataNum = 0;
                    sensorsEventEmitt(SENSORS_EVENT_SAMPLING_DONE, 0xff, (void *)sensors);
                }
                sensors->sampleNum = 0;
                sensors->startTime = SysTime();
            }
            sensors->sampleTime = SysTime();
        }
    }
}

void SensorsFrequencyTrigger(Sensors_t *sensors, HalSensorID_t id)
{
    if(sensors && sensors->start && id < HAL_SENSOR_ID_COUNT)
    {
        sensors->node[id].freqCount++;
    }
}

bool SensorsSamplingIsStart(Sensors_t *sensors)
{
    if(sensors)
    {
        return sensors->start;
    }
    return false;
}

void SensorsSamplingStop(Sensors_t *sensors)
{

    uint8_t i;
    if(sensors)
    {
        //HalADCStop();
        sensors->start = false;
        for(i = 0; i < HAL_SENSOR_ID_COUNT; i++)
        {
            sensors->node[i].stop(&sensors->node[i]);
        }
    }
    g_contextCount = 0;
}

void SensorsSamplingStart(Sensors_t *sensors, uint32_t num, uint32_t exflashOffset)
{
    uint8_t i;
    if(sensors && !sensors->start)
    {
        Syslog("");
        sensors->start = true;
        sensors->sampleNum = 0;
        sensors->dataNum = 0;
        sensors->totalDataNum = num;
        sensors->sampleTime = SysTime();
        sensors->startTime = SysTime();
        sensors->startDateTime = *SysDateTime();
        sensors->exflashOffset = exflashOffset;

        for(i = 0; i < HAL_SENSOR_ID_COUNT; i++)
        {
            sensors->node[i].start(&sensors->node[i]);
        }
        g_contextCount = 0;
    }
}

Sensors_t *SensorsCreate(void)
{
    static Sensors_t sensors;
    sensors.start = false;
    
    return &sensors;
}
#if 1
static void sensorsSleep(PM_t *pm)
{
    if(pm)
    {
        HalADCStop();
        HalDACEnable(false);
        //W25Q64PowerDown();
        pm->status = PM_STATUS_SLEEP;
    }
}

static void sensorsWakeup(PM_t *pm)
{
    
    if(pm)
    {
        //W25Q64Wakeup();
        HalADCStart();
        HalDACEnable(true);
        pm->status = PM_STATUS_WAKEUP;
    }
}

static void sensorsPowerInit(void)
{
    PM_t pm;
    
    pm.sleep = sensorsSleep;
    pm.wakeup = sensorsWakeup;
    pm.status = PM_STATUS_WAKEUP;
    PMRegist(&pm, PM_DEVICE_ID_SENSORS);
}
#endif
void SensorsInitialize(SensorsEvent_cb eventCb)
{   
    sensorsPowerInit();
    g_seventCb = eventCb;
}

void SensorsPoll(Sensors_t *sensors)
{
    sensorSamplingPoll(sensors);
}

