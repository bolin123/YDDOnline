#include "YDDOnline.h"
#include "Sys.h"
#include "DigitalLED.h"
#include "RFModule.h"
#include "Sensors.h"
#include "Sensor/SGeomagnetism.h"
#include "Sensor/SPress.h"
#include "Sensor/SNoise.h"
#include "PowerManager.h"
#include "Temperature.h"

#define YDD_FALL_SLEEP_DELAY 1000

typedef struct
{
    char head;
    char mac[6];
    char devType[2];
    char cmd[2];
    char dlen[4];
    char data[];
}YDDRfData_t;

static Sensors_t *g_sensors = NULL;
static SensorsContext_t g_sensorContext[HAL_SENSOR_ID_COUNT];
//static bool g_fallSleep = false;
//static uint32_t g_sleepTime = 0;


static void yddWakeup(PM_t *pm)
{
    //DigitalLEDOn();
    if(pm)
    {
        HalGPIOSetLevel(HAL_LED1_PIN, 0);
        
        HalGPIOSetLevel(HAL_SENSORS_POWER_PIN, 1);
        HalGPIOSetLevel(HAL_485_POWER_PIN,     0);
        HalGPIOSetLevel(HAL_IR_POWER_PIN,      0);
        //HalCommonWakeup();
        //RFModuleWakeup();
        SensorsSamplingStart(g_sensors, 1, HAL_FLASH_INVALID_ADDR);
        Syslog("");
        pm->status = PM_STATUS_WAKEUP;
    }
    
}

static void yddSleep(PM_t *pm)
{
    if(pm)
    {
        Syslog("");
        DigitalLEDOff();
        HalGPIOSetLevel(HAL_LED1_PIN, 1);
        
        HalGPIOSetLevel(HAL_SENSORS_POWER_PIN, 0);
        HalGPIOSetLevel(HAL_485_POWER_PIN, 1);
        HalGPIOSetLevel(HAL_IR_POWER_PIN, 1);
        
        pm->status = PM_STATUS_SLEEP;
    }
    
    //RFModuleSleep();
    //HalCommonFallasleep();
}

static void reportSensorsValue(SensorsContext_t *context, uint8_t num)
{
    //char *buff = "~000012{\"devType\":1,\"values\":[{\"ch1\":[\"856\", \"21\"]},{\"ch2\":[\"546\", \"23\"]},{\"ch3\":[\"123\"]},{\"ch4\":[\"0\"]}]}\r";
    char buff[255] = {0};
    uint8_t i;
    YDDRfData_t *rfdata = (YDDRfData_t *)buff;

    rfdata->head = '~';
    memcpy(rfdata->mac, SysMacAddrGet(), SYS_MAC_ADDR_LEN);
    sprintf(rfdata->devType, "%02d", HAL_DEVICE_TYPE);
    
    rfdata->cmd[0] = '0'; //update
    rfdata->cmd[1] = '1'; //update
    sprintf(rfdata->dlen, "%04d", num * 8);
    for(i = 0; i < num; i++)
    {
        sprintf(&rfdata->data[i*8], "%04d", context[i].amplitude);
        sprintf(&rfdata->data[i*8 + 4], "%04d", context[i].frequency);
    }
    buff[sizeof(YDDRfData_t) + num * 8] = '\r';
    RFModuleSendData((uint8_t *)buff, strlen(buff));
}

static void sensorsEventHandle(SensorsEvent_t event, uint8_t chn, void *args)
{
    //SensorsContext_t *context;
    Syslog("event = %d", event);
    if(event == SENSORS_EVENT_SAMPLING_UPDATE)
    {
        if(chn < HAL_SENSOR_ID_COUNT)
        {
            memcpy((void *)&g_sensorContext[chn], args, sizeof(SensorsContext_t));
        }
    }
    else if(event == SENSORS_EVENT_SAMPLING_DONE)
    {
        if(RFModuleDetected())
        {
            reportSensorsValue(g_sensorContext, HAL_SENSOR_ID_COUNT);
            //sleepSet(true, YDD_FALL_SLEEP_DELAY);
            PMStartSleep(YDD_FALL_SLEEP_DELAY);
        }
    }
}

#if 0
static void sleepHandle(void)
{
    if(RFModuleDetected() && g_fallSleep && SysTime() > g_sleepTime)
    {
        fallSleep();
    }
}
#endif

void YDDOnlineSensorFreqTrigger(uint8_t ch)
{
    SensorsFrequencyTrigger(g_sensors, (HalSensorID_t)ch);
}


static void sensorsHandleInit(Sensors_t *sensors)
{
    //HAL_SENSOR_ID_GEOMAGNETISM1
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM].buff         = NULL;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM].buffNum      = 0;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM].start        = SGeomagnetismStart;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM].stop         = SGeomagnetismStop;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM].getAmplitude = SGeomagnetismGetAmplitude;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM].getFrequency = SGeomagnetismGetFrequency;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM].next         = SGeomagnetismNext;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM].sampling     = SGeomagnetismSampling;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM].id           = HAL_SENSOR_ID_GEOMAGNETISM;

    //HAL_SENSOR_ID_NOISE
    sensors->node[HAL_SENSOR_ID_NOISE].buff         = NULL;
    sensors->node[HAL_SENSOR_ID_NOISE].buffNum      = 0;
    sensors->node[HAL_SENSOR_ID_NOISE].start        = SNoiseStart;
    sensors->node[HAL_SENSOR_ID_NOISE].stop         = SNoiseStop;
    sensors->node[HAL_SENSOR_ID_NOISE].getAmplitude = SNoiseGetAmplitude;
    sensors->node[HAL_SENSOR_ID_NOISE].getFrequency = SNoiseGetFrequency;
    sensors->node[HAL_SENSOR_ID_NOISE].next         = SNoiseNext;
    sensors->node[HAL_SENSOR_ID_NOISE].sampling     = SNoiseSampling;
    sensors->node[HAL_SENSOR_ID_NOISE].id           = HAL_SENSOR_ID_NOISE;

    //HAL_SENSOR_ID_PRESS
    static uint16_t press1Buff[SENSORS_SAMPLING_NUM_1S];
    sensors->node[HAL_SENSOR_ID_PRESS1].buff           = press1Buff;
    sensors->node[HAL_SENSOR_ID_PRESS1].buffNum        = SENSORS_SAMPLING_NUM_1S;
    sensors->node[HAL_SENSOR_ID_PRESS1].start          = SPressStart;
    sensors->node[HAL_SENSOR_ID_PRESS1].stop           = SPressStop;
    sensors->node[HAL_SENSOR_ID_PRESS1].getAmplitude   = SPressGetAmplitude;
    sensors->node[HAL_SENSOR_ID_PRESS1].getFrequency   = SPressGetFrequency;
    sensors->node[HAL_SENSOR_ID_PRESS1].next           = SPressNext;
    sensors->node[HAL_SENSOR_ID_PRESS1].sampling       = SPressSampling;
    sensors->node[HAL_SENSOR_ID_PRESS1].id             = HAL_SENSOR_ID_PRESS1;

    //HAL_SENSOR_ID_PRESS
    static uint16_t press2Buff[SENSORS_SAMPLING_NUM_1S];
    sensors->node[HAL_SENSOR_ID_PRESS2].buff           = press2Buff;
    sensors->node[HAL_SENSOR_ID_PRESS2].buffNum        = SENSORS_SAMPLING_NUM_1S;
    sensors->node[HAL_SENSOR_ID_PRESS2].start          = SPressStart;
    sensors->node[HAL_SENSOR_ID_PRESS2].stop           = SPressStop;
    sensors->node[HAL_SENSOR_ID_PRESS2].getAmplitude   = SPressGetAmplitude;
    sensors->node[HAL_SENSOR_ID_PRESS2].getFrequency   = SPressGetFrequency;
    sensors->node[HAL_SENSOR_ID_PRESS2].next           = SPressNext;
    sensors->node[HAL_SENSOR_ID_PRESS2].sampling       = SPressSampling;
    sensors->node[HAL_SENSOR_ID_PRESS2].id             = HAL_SENSOR_ID_PRESS2;
}

static void yddPowerInit(void)
{
    PM_t pm;
    pm.sleep = yddSleep;
    pm.wakeup = yddWakeup;
    pm.status = PM_STATUS_WAKEUP;
    PMRegist(&pm, PM_DEVICE_ID_APP);
}

static void init(void)
{   
    HalGPIOConfig(HAL_LED1_PIN, HAL_IO_OUTPUT);
    HalGPIOSetLevel(HAL_LED1_PIN, 0);

    HalGPIOConfig(HAL_485_POWER_PIN, HAL_IO_OUTPUT);//pa12
    HalGPIOSetLevel(HAL_485_POWER_PIN, 1);

    HalGPIOConfig(HAL_SENSORS_POWER_PIN, HAL_IO_OUTPUT);//PC13
    HalGPIOSetLevel(HAL_SENSORS_POWER_PIN, 1);

    HalGPIOConfig(HAL_IR_POWER_PIN, HAL_IO_OUTPUT);//PC13
    HalGPIOSetLevel(HAL_IR_POWER_PIN, 1);
}

void YDDOnlineInit(void)
{
    init();
    yddPowerInit();
    DigitalLEDInit();
    RFModuleInit();
    TemperatureInit();
    SensorsInitialize(sensorsEventHandle);
    g_sensors = SensorsCreate();
    sensorsHandleInit(g_sensors);

    DigitalLEDSetChars(DIGITAL_LED_ID_CMD, 0x0E, false);
    DigitalLEDOn();
    //PMStartSleep(5000);
}

void YDDOnlinePoll(void)
{
    DigitalLEDPoll();
    SensorsPoll(g_sensors);
    RFModulePoll();
    TemperaturePoll();
    //sleepHandle();
}

