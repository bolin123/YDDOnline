#include "YDDOnline.h"
#include "Sys.h"
#include "SysTimer.h"
#include "DigitalLED.h"
//#include "RFModule.h"
#include "Sensors.h"
#include "Sensor/SGeomagnetism.h"
#include "Sensor/SPress.h"
#include "Sensor/SNoise.h"
#include "PowerManager.h"
#include "Temperature.h"
#include "Wireless.h"
#include "WiredProto.h"
#include "DispLoop.h"
#include "Menu.h"
#include "DataManager.h"
#include "IRDataTransfer.h"

#define YDD_FALL_SLEEP_DELAY 1000

#pragma pack(1)
typedef struct 
{
    uint8_t address;
    uint8_t type;
    uint8_t errcode;
    uint8_t power;
    uint32_t utc;
    struct
    {
        uint16_t temperate;
        uint16_t press1;
        uint16_t press2;
    }data;
}DeviceStorageData_t;
#pragma pack()


static Sensors_t *g_sensors = NULL;
static SensorsContext_t g_sensorContext[HAL_SENSOR_ID_COUNT];
static bool g_queryRecved = false;
static volatile bool g_startLightDetect = false;
static uint32_t g_lightDetectTime;
static volatile uint8_t g_lostQueryCount = 0;

static bool needStorageData(void)
{
    return g_lostQueryCount > 2 ? true : false;
}

static void yddWakeup(PM_t *pm, PMWakeupType_t type)
{
    //printf("type = %d\n", type);
    if(pm)
    {
        if(pm->status != PM_STATUS_WAKEUP)
        {
            TemperaturePowerOn();
        }

        if(type == PM_WAKEUP_TYPE_LIGHT)
        {
            g_startLightDetect = true;
            g_lightDetectTime = SysTime();
        }
        else if(type == PM_WAKEUP_TYPE_WIRELESS)
        {
            HalGPIOSetLevel(HAL_STATUS_LED_PIN, HAL_STATUS_LED_ENABLE_LEVEL);
            PMStartSleep(2000);
        }
        else if(type == PM_WAKEUP_TYPE_RTC)
        {
            if(needStorageData())
            {
                SensorsSamplingStart(g_sensors, 1, HAL_FLASH_INVALID_ADDR);
                PMStartSleep(2000);
            }
            else
            {
                g_lostQueryCount++;
                PMStartSleep(500);
            }
        }
        else
        {
            PMStartSleep(2000);
        }
        pm->status = PM_STATUS_WAKEUP;
    }
    
}

static void lightActiveDetect(void)
{
    if(g_startLightDetect)
    {
        if(HalGPIOGetLevel(HAL_LIGHT_IRQ_PIN))
        {
            g_startLightDetect = false;
            PMStartSleep(500);
        }
        else
        {
  
            if(SysTimeHasPast(g_lightDetectTime, 2000))
            {
                DispLoopStart(2);
                //SensorsSamplingStart(g_sensors, 1, HAL_FLASH_INVALID_ADDR);
                g_startLightDetect = false;
            }
        }
    }
}

static void yddSleep(PM_t *pm)
{
    if(pm)
    {
        Syslog("");
        DispLoopStop();
        MenuDeactive();
        if(PMIsTypeWakeup(PM_WAKEUP_TYPE_WIRELESS))
        {
            HalRTCAlarmSet(SysQueryIntervalGet() / 2 + SysQueryIntervalGet());
        }else if(PMIsTypeWakeup(PM_WAKEUP_TYPE_RTC))
        {
            HalRTCAlarmSet(SysQueryIntervalGet());
        }
        
        HalGPIOSetLevel(HAL_STATUS_LED_PIN, HAL_STATUS_LED_DISABLE_LEVEL);
        
        pm->status = PM_STATUS_SLEEP;
    }
}

static void sensorsEventHandle(SensorsEvent_t event, uint8_t chn, void *args)
{
    uint8_t i = 0;
    uint16_t reportData[16];
    float temperature;

    if(event == SENSORS_EVENT_SAMPLING_UPDATE)
    {
        if(chn < HAL_SENSOR_ID_COUNT)
        {
            memcpy((void *)&g_sensorContext[chn], args, sizeof(SensorsContext_t));
        }
    }
    else if(event == SENSORS_EVENT_SAMPLING_DONE)
    {

        HalInterruptSet(false);
            temperature = TemperatureValueExchange(TemperatureGetValue());
        HalInterruptSet(true);
            printf("temperature=%.1f\n", temperature);
            
            reportData[i++] = (uint16_t)(temperature * 10);
            if(SysDeviceTypeGet() == HAL_DEVICE_TYPE_PRESS)
            {
                reportData[i++] = g_sensorContext[HAL_SENSOR_ID_PRESS1].amplitude;
                reportData[i++] = g_sensorContext[HAL_SENSOR_ID_PRESS2].amplitude;
            }
            else
            {
                reportData[i++] = g_sensorContext[HAL_SENSOR_ID_GEOMAGNETISM].amplitude;
                reportData[i++] = g_sensorContext[HAL_SENSOR_ID_GEOMAGNETISM].frequency;
                reportData[i++] = g_sensorContext[HAL_SENSOR_ID_NOISE].amplitude;
                reportData[i++] = g_sensorContext[HAL_SENSOR_ID_NOISE].frequency;
            }
            

            if(g_queryRecved)
            {
                WirelessReportData(SysErrorCode(), SysPowerPercent(), reportData, i);
                g_queryRecved = false;
            }

            if(PMIsTypeWakeup(PM_WAKEUP_TYPE_RTC) && needStorageData())
            {
                DeviceStorageData_t storage;
                printf("storage\n");
                storage.address = SysRfAddressGet();
                storage.type    = SysDeviceTypeGet();
                storage.errcode = SysErrorCode();
                storage.power   = SysPowerPercent();
                storage.utc     = HalRTCGetUtc();
                storage.data.temperate = (uint16_t)(temperature * 10);
                storage.data.press1    = g_sensorContext[HAL_SENSOR_ID_PRESS1].amplitude;
                storage.data.press2    = g_sensorContext[HAL_SENSOR_ID_PRESS2].amplitude;
                DataManagerStorage((uint8_t *)&storage, sizeof(DeviceStorageData_t));
            }

    }
}

void YDDOnlineSensorFreqTrigger(uint8_t ch)
{
    SensorsFrequencyTrigger(g_sensors, (HalSensorID_t)ch);
}

static void updateQueryTime(void)
{
    static uint32_t lastQueryTime = 0;
    int newInterval, oldInterval;

    if(lastQueryTime != 0)
    {
        newInterval = HalRTCGetUtc() - lastQueryTime;
        oldInterval = SysQueryIntervalGet();
        
        if(abs(oldInterval - newInterval) > 5)
        {
            SysQueryIntervalSet(newInterval);
        }
    }
    lastQueryTime = HalRTCGetUtc();    
}

static void wirelessEventHandle(WirelessEvent_t event, void *args)
{
    if(event == WIRELESS_EVENT_QUERY)
    {
        if(!IRDataTransferIsStart())
        {
            g_queryRecved = true;
            g_lostQueryCount = 0;
            Syslog("sensor sampling...");
            SensorsSamplingStart(g_sensors, 1, HAL_FLASH_INVALID_ADDR);
            PMStartSleep(2000);
        }
        
        updateQueryTime();
    }
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

static void sensorsRegist(void)
{
    SensorsInitialize(sensorsEventHandle);
    g_sensors = SensorsCreate();
    sensorsHandleInit(g_sensors);
}

static void disploopDoneHandle(void)
{
    Syslog("");
    PMStartSleep(YDD_FALL_SLEEP_DELAY);
}

static void updateUTCTimer(uint8_t *utc)
{
    int setUTC, realUTC;
    memcpy(&setUTC, utc, 4);
    realUTC = (int)HalRTCGetUtc();
    if(abs(setUTC - realUTC) > 10)
    {
        HalRTCSetUtc(setUTC);
    }
}

static void irDataTransferPercentShow(uint8_t percent)
{
    DigitalLEDSetChars(DIGITAL_LED_ID_CMD, DIGITAL_FLAG_S, false);
    DigitalLEDSetChars(DIGITAL_LED_ID_1, percent / 100, false);
    DigitalLEDSetChars(DIGITAL_LED_ID_2, percent % 100 / 10, false);
    DigitalLEDSetChars(DIGITAL_LED_ID_3, percent % 10, false);
}

static void irDataTransEventHandle(IRDataTransferEvent_t event, uint32_t value)
{
    if(event == IR_DTRANS_EVENT_PROCESS)
    {
        irDataTransferPercentShow((uint8_t)value);
    }
    else if(event == IR_DTRANS_EVENT_RESULT)
    {
        Syslog("result, success = %d", value);
        DigitalLEDOff();
        IRDataTransferStop();
        PMStartSleep(500);
    }
}

static void irKeyEventHandle(uint8_t key, uint8_t *contents)
{
    Syslog("key = %d", key);
    DispLoopStop();
    PMStartSleep(5000);
    if(IR_KEY_READDATA == key)
    {
        updateUTCTimer(contents);
        if(IRDataTransferStart())
        {
            MenuDeactive();
            DigitalLEDOn();
            irDataTransferPercentShow(0);
        }
    }
    else if(IR_KEY_READDATA_ACK == key)
    {
        IRDataTransferResume();
    }
    else
    {
        if(IR_KEY_MENU == key)
        {
            updateUTCTimer(contents);
        }
        MenuKeyHandle((IRKey_t)key);
        PMStartSleep(20000);
    }
}

static DispLoopValue_t *disploopGetValue(DispLoopID_t id)
{
    static DispLoopValue_t displayValue;
    uint8_t flag;
    uint16_t value;
    
    switch (id)
    {
    case DISPLOOP_ID_ADDR:
        flag = DIGITAL_FLAG_A;
        value = SysRfAddressGet();
        break;
    case DISPLOOP_ID_RFCHN:
        flag = DIGITAL_FLAG_C;
        value = SysRfChannelGet();
        break;
    case DISPLOOP_ID_DEVTYPE:
        flag = DIGITAL_FLAG_D;
        value = SysDeviceTypeGet();
        break;
    case DISPLOOP_ID_ERRCODE:
        flag = DIGITAL_FLAG_E;
        value = SysErrorCode();
        break;
    case DISPLOOP_ID_POWER:
        flag = DIGITAL_FLAG_P;
        value = SysPowerPercent();
        break;
    case DISPLOOP_ID_PRESS1:
        flag = 0x01;
        value = g_sensorContext[HAL_SENSOR_ID_PRESS1].amplitude;
        break;
    case DISPLOOP_ID_PRESS2:
        flag = 0x02;
        value = g_sensorContext[HAL_SENSOR_ID_PRESS2].amplitude;
        break;
    default:
        return NULL;
    }

    displayValue.flag = flag;
    displayValue.contents[0] = value / 100;
    displayValue.contents[1] = value % 100 / 10;
    if(id == DISPLOOP_ID_PRESS1 || id == DISPLOOP_ID_PRESS2)//小数点
    {
        displayValue.contents[1] |= 0x80;
    }
    displayValue.contents[2] = value % 10;
    return &displayValue;
}

void YDDOnlineLightActive(void)
{
    HalExitSet(HAL_EXIT_LIGHT_WAKEUP, false);
    PMWakeup(PM_WAKEUP_TYPE_LIGHT);
}

static void lowInit(void)
{   
    HalGPIOConfig(HAL_STATUS_LED_PIN, HAL_IO_OUTPUT);
    HalGPIOSetLevel(HAL_STATUS_LED_PIN, HAL_STATUS_LED_ENABLE_LEVEL);
    
    yddPowerInit();
    sensorsRegist();
    TemperaturePowerOn();
}

static void menuEventHandle(MenuEvent_t event)
{   
    Syslog("%d", event);
    switch (event)
    {
    case MENU_EVENT_ACTIVE:
        DigitalLEDOn();
    break;
    case MENU_EVENT_DEACTIVE:
    //break;
    case MENU_EVENT_TIMEOUT:
        MenuDeactive();
        DigitalLEDOff();
        PMStartSleep(500);//无按键，30秒后休眠
    break;
    }
}

static unsigned short menuGetValue(MenuID_t id)
{
    uint16_t value;
    
    switch (id)
    {
    case MENU_ID_ADDR:
        value = SysRfAddressGet();
    break;
    case MENU_ID_RFCHN:
        value = SysRfChannelGet();
    break;
    default:
    break;
    }

    return value;
}

static void menuSetValue(MenuID_t id, uint16_t value)
{   
    Syslog("id = %d, value = %d", id, value);
    switch (id)
    {
    case MENU_ID_ADDR:
        SysRfAddressSet((uint8_t)value);
    break;
    case MENU_ID_RFCHN:
        SysRfChannelSet((uint8_t)value);
        WirelessSetChannel((uint8_t)value);
    break;
    default:
    break;
    }
}

static void displayInit(void)
{
    DigitalLEDInit();
    DispLoopInit(disploopDoneHandle);
    DispLoopRegister(DISPLOOP_ID_ADDR,    disploopGetValue);
    DispLoopRegister(DISPLOOP_ID_RFCHN,   disploopGetValue);
    if(SysDeviceTypeGet() == HAL_DEVICE_TYPE_PRESS)
    {
        DispLoopRegister(DISPLOOP_ID_PRESS1, disploopGetValue);
        DispLoopRegister(DISPLOOP_ID_PRESS2, disploopGetValue);
    }

    MenuInit(menuEventHandle);
    MenuItem_t item;
    //MENU_ID_ADDR
    item.flag     = DIGITAL_FLAG_A;
    item.digitNum = DIGITAL_LED_ID_1;
    item.step     = 1;
    item.max      = 254;
    item.min      = 1;
    item.getValue = menuGetValue;
    item.setValue = menuSetValue;
    MenuRegister(MENU_ID_ADDR, &item);
    
    //MENU_ID_RFCHN
    item.flag     = DIGITAL_FLAG_C;
    item.digitNum = DIGITAL_LED_ID_2;
    item.step     = 1;
    item.max      = HAL_RF_CHANNEL_NUM;
    item.min      = 1;
    item.getValue = menuGetValue;
    item.setValue = menuSetValue;
    MenuRegister(MENU_ID_RFCHN, &item);

    DigitalLEDOn();
    DispLoopStart(2);
    SensorsSamplingStart(g_sensors, 1, HAL_FLASH_INVALID_ADDR);
}

void YDDOnlineInit(void)
{
    lowInit();
    displayInit();
    WirelessInit(wirelessEventHandle);
    IRInit(irKeyEventHandle);
    DataManagerInit();
    IRDataTransferInit(irDataTransEventHandle);
    HalRTCAlarmSet(SysQueryIntervalGet() / 2 + SysQueryIntervalGet());
}

void YDDOnlinePoll(void)
{
    SensorsPoll(g_sensors);
    WirelessPoll();
    DispLoopPoll();
    MenuPoll();
    IRPoll();
    IRDataTransferPoll();
	lightActiveDetect();
}

