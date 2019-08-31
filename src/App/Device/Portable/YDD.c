#include "YDD.h"
#include "Keypad.h"
#include "Sensors.h"
#include "Display.h"
#include "DisplayCoordinate.h"
#include "Menu/MenuSettings.h"
#include "Menu/MenuContact.h"
#include "Menu/MenuDetail.h"
#include "Sensor/SGeomagnetism.h"
#include "Sensor/SPress.h"
#include "Sensor/SNoise.h"

typedef enum
{
    YDD_MENU_ID_DETAIL = 0,
    YDD_MENU_ID_SETTINGS,
    YDD_MENU_ID_CONTACTUS,
    YDD_MENU_ID_COUNT,
}YDDMenuId_t;

typedef struct
{
    void (*hide)(void);
    void (*keyHandle)(HalKeyValue_t key);
    void (*show)(void);
}YDDMenuHandle_t;

static Sensors_t *g_sensors = NULL;
static YDDMenuId_t g_menuId = YDD_MENU_ID_DETAIL;
static YDDMenuHandle_t g_menuHandle[YDD_MENU_ID_COUNT];
static uint16_t g_lastCollectNum = 0;

static void updateTimeAndPower(void)
{
    static uint32_t oldTime = 0;
    
    if(SysTimeHasPast(oldTime, 10000))
    {
        Syslog("update");
        DisplayDateTimeUpdate(); //时间显示
        DisplayPowerPercent();
        oldTime = SysTime();
    }
}

static void idleDisplay(void)
{
    g_menuId = YDD_MENU_ID_DETAIL;
    MenuDetailShow();
    //updateTimeAndPower(true);
}

static void lastCollectNumDisplay(void)
{    
    char buff[4] = "";
    SysDisplayPosition_t pos, bottom;
    pos.x = 30;
    pos.y = 235;

    bottom.x = pos.x + 12 * 3;
    bottom.y = pos.y + 24;
    DisplayDrawRect(DISPLAY_COLOR_BOTTOM, &pos, &bottom);
    
    sprintf(buff, "%03d", g_lastCollectNum);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &pos, DISPLAY_CHAR_SIZE_NORMAL);
}

static void collectDisplayShow(void)
{   

    DisplayPictureShow(SYS_PICTURE_CAPTURE_ID);
    DisplayDateTimeUpdate(); //时间显示
    DisplayPowerPercent();

    lastCollectNumDisplay();
}


static void yddKeyEventHandle(uint8_t keyval, KeyStatus_t status, bool longpress, uint32_t lastTime)
{
    SysDataRecord_t record;
    SysDeviceArgs_t args;
    HalKeyValue_t value = (HalKeyValue_t)keyval;

    if(lastTime > 20)//防抖
    {
        switch (value)
        {
            case HAL_KEY_VALUE_CONFIG:
            case HAL_KEY_VALUE_UP:
            case HAL_KEY_VALUE_DOWN:
                //Syslog("HAL_KEY_VALUE_CONFIG");
                
                if(!SensorsSamplingIsStart(g_sensors))
                {
                    g_menuHandle[g_menuId].keyHandle(value);
                }
                break;
            case HAL_KEY_VALUE_PAGEDOWN:
                //Syslog("HAL_KEY_VALUE_PAGEDOWN");
                if(status == KEY_STATUS_RELEASE && !SensorsSamplingIsStart(g_sensors))
                {
                    g_menuHandle[g_menuId].hide();
                    g_menuId++;
                    if(g_menuId >= YDD_MENU_ID_COUNT)
                    {
                        g_menuId = YDD_MENU_ID_DETAIL;
                    }
                    g_menuHandle[g_menuId].show();
                    //updateTimeAndPower(true);
                }
                break;
            case HAL_KEY_VALUE_START:
                //Syslog("HAL_KEY_VALUE_START");
                if(status == KEY_STATUS_RELEASE && !SensorsSamplingIsStart(g_sensors))
                {
                    g_menuHandle[g_menuId].hide();
                    SysArgsGetRecord(&record);
                    SysDeviceArgsGet(&args);
                    g_lastCollectNum = args.runTime;
                    SensorsSamplingStart(g_sensors, args.runTime, record.size);
                    collectDisplayShow();
                }
                break;
            case HAL_KEY_VALUE_TOGGLE:
                //Syslog("HAL_KEY_VALUE_TOGGLE");
                if(SensorsSamplingIsStart(g_sensors)) //cancle
                {
                    SensorsSamplingStop(g_sensors);
                    idleDisplay();
                }
                else
                {
                    g_menuHandle[g_menuId].keyHandle(value);
                }
                break;
            default:
                break;
        }
    }
}

static void updateCollectDisplay(uint8_t chn, SensorsContext_t *value)
{
    SysDisplayPosition_t top, bottom;
    char buff[8] = "";
    uint16_t integer, decimal;

    if(chn < 3)
    {
        sprintf(buff, "%d", value->amplitude);
    }
    else //应力通道0~26MPa
    {
        integer = value->amplitude / 10;
        decimal = value->amplitude % 10;
        sprintf(buff, "%d.%d", integer, decimal); //x.x MPa
    }
    top.x = g_capturePos[chn].vltg.x;
    top.y = g_capturePos[chn].vltg.y;

    bottom.x = g_capturePos[chn].vltg.x + 12 * 4;
    bottom.y = g_capturePos[chn].vltg.y + 24;
    DisplayDrawRect(DISPLAY_COLOR_BOTTOM, &top, &bottom);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &top, DISPLAY_CHAR_SIZE_NORMAL);
            
    top.x = g_capturePos[chn].freq.x;
    top.y = g_capturePos[chn].freq.y;

    bottom.x = g_capturePos[chn].freq.x + 12 * 4;
    bottom.y = g_capturePos[chn].freq.y + 24;

    buff[0] = '\0';
    integer = value->frequency / 10;
    decimal = value->frequency % 10;
    sprintf(buff, "%d.%d", integer, decimal);
    //sprintf(buff, "%d", value->frequency);
    DisplayDrawRect(DISPLAY_COLOR_BOTTOM, &top, &bottom);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &top, DISPLAY_CHAR_SIZE_NORMAL);

    if(chn == (HAL_SENSOR_ID_COUNT - 1))
    {
        g_lastCollectNum--;
        lastCollectNumDisplay();
    }
}

static void updateSysArgs(Sensors_t *sensors)
{
    //...update record & args
    SysDataRecord_t record;
    SysDataInfo_t info;
    SysDeviceArgs_t args;
    
    SysArgsGetRecord(&record);
    SysDeviceArgsGet(&args);
    info.size = args.runTime * HAL_SENSOR_ID_COUNT * sizeof(SensorsContext_t);
    info.threshold = args.signalThreshold;
    info.times = args.runTime;
    info.startAddr = record.size;
    info.date = sensors->startDateTime;
    SysArgsSetPointInfo(record.num + 1, &info);
    
    record.num++;
    record.size += args.runTime * HAL_SENSOR_ID_COUNT * sizeof(SensorsContext_t);
    SysArgsSetRecord(&record);
}

static void sensorsEventHandle(SensorsEvent_t event, uint8_t chn, void *args)
{
    SensorsContext_t *context;
    SysDeviceArgs_t cargs;
    if(event == SENSORS_EVENT_SAMPLING_DONE)

    {
        updateSysArgs((Sensors_t *)args);
        idleDisplay();
        HalBeepSet(200);
    }
    else if(event == SENSORS_EVENT_SAMPLING_UPDATE)
    {
        context = (SensorsContext_t *)args;
        SysDeviceArgsGet(&cargs);
        if(chn != 3 && (context->amplitude >= cargs.intensityAlarm || context->frequency >= cargs.ringAlarm * 10))
        {
            HalBeepSet(100);
        }
        updateCollectDisplay(chn, context);
    }
    else
    {

    }
}

static void exceptionShow(uint16_t errcode)
{
    SysDisplayPosition_t pos;
    char buff[8] = "";
    DisplayPictureShow(SYS_PICTURE_EXCEPTION_ID);
    pos.x = 165;
    pos.y = 105;
    sprintf(buff, "%d", errcode);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &pos, DISPLAY_CHAR_SIZE_LARGE);
    HalGPIOSetLevel(KEYPAD_SCAN1_PIN, 0);
    HalGPIOSetLevel(KEYPAD_SCAN2_PIN, 0);
    HalGPIOSetLevel(KEYPAD_SCAN3_PIN, 0);
    while(1)
    {
        if(HalGPIOGetLevel(KEYPAD_CODE1_PIN) == 0 ||
            HalGPIOGetLevel(KEYPAD_CODE2_PIN) == 0)
        {
            break;
        }
        HalWaitMs(100);
    }
}

void YDDSensorFreqTrigger(uint8_t ch)
{
    SensorsFrequencyTrigger(g_sensors, (HalSensorID_t)ch);
}

static void sensorsHandleInit(Sensors_t *sensors)
{
    //HAL_SENSOR_ID_GEOMAGNETISM1
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM1].buff         = NULL;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM1].buffNum      = 0;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM1].start        = SGeomagnetismStart;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM1].stop         = SGeomagnetismStop;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM1].getAmplitude = SGeomagnetismGetAmplitude;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM1].getFrequency = SGeomagnetismGetFrequency;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM1].next         = SGeomagnetismNext;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM1].sampling     = SGeomagnetismSampling;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM1].id           = HAL_SENSOR_ID_GEOMAGNETISM1;
    
    //HAL_SENSOR_ID_GEOMAGNETISM2
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM2].buff         = NULL;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM2].buffNum      = 0;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM2].start        = SGeomagnetismStart;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM2].stop         = SGeomagnetismStop;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM2].getAmplitude = SGeomagnetismGetAmplitude;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM2].getFrequency = SGeomagnetismGetFrequency;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM2].next         = SGeomagnetismNext;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM2].sampling     = SGeomagnetismSampling;
    sensors->node[HAL_SENSOR_ID_GEOMAGNETISM2].id           = HAL_SENSOR_ID_GEOMAGNETISM2;

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
    static uint16_t pressBuff[SENSORS_SAMPLING_NUM_1S];
    sensors->node[HAL_SENSOR_ID_PRESS].buff         = pressBuff;
    sensors->node[HAL_SENSOR_ID_PRESS].buffNum      = SENSORS_SAMPLING_NUM_1S;
    sensors->node[HAL_SENSOR_ID_PRESS].start        = SPressStart;
    sensors->node[HAL_SENSOR_ID_PRESS].stop         = SPressStop;
    sensors->node[HAL_SENSOR_ID_PRESS].getAmplitude = SPressGetAmplitude;
    sensors->node[HAL_SENSOR_ID_PRESS].getFrequency = SPressGetFrequency;
    sensors->node[HAL_SENSOR_ID_PRESS].next         = SPressNext;
    sensors->node[HAL_SENSOR_ID_PRESS].sampling     = SPressSampling;
    sensors->node[HAL_SENSOR_ID_PRESS].id           = HAL_SENSOR_ID_PRESS;
}

void YDDInitialize(uint16_t errcode)
{
    DisplayInitialize();
    KeypadInit(yddKeyEventHandle);
    SensorsInitialize(sensorsEventHandle);
    g_sensors = SensorsCreate();
    sensorsHandleInit(g_sensors);

    g_menuHandle[YDD_MENU_ID_DETAIL].show = MenuDetailShow;
    g_menuHandle[YDD_MENU_ID_DETAIL].keyHandle = MenuDetailKeyHanlde;
    g_menuHandle[YDD_MENU_ID_DETAIL].hide = MenuDetailHide;
    g_menuHandle[YDD_MENU_ID_SETTINGS].show = MenuSettingsShow;
    g_menuHandle[YDD_MENU_ID_SETTINGS].keyHandle = MenuSettingsKeyHanlde;
    g_menuHandle[YDD_MENU_ID_SETTINGS].hide = MenuSettingsHide;
    g_menuHandle[YDD_MENU_ID_CONTACTUS].show = MenuContactShow;
    g_menuHandle[YDD_MENU_ID_CONTACTUS].keyHandle = MenuContactKeyHanlde;
    g_menuHandle[YDD_MENU_ID_CONTACTUS].hide = MenuContactHide;

    if(errcode)
    {
        exceptionShow(errcode);
    }
    //displayPictureSet(SYS_PICTURE_DETAILS_ID);
    idleDisplay();
}

void YDDPoll(void)
{
    DisplayPoll();
    KeypadPoll();
    SensorsPoll(g_sensors);
    updateTimeAndPower();
}

