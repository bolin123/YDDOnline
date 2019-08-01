#include "Keypad.h"
#include "HalGPIO.h"

#define KEYPAD_COLUMN_NUM 3
#define KEYPAD_RAW_NUM  2
#define KEYPAD_KEY_NUM  (KEYPAD_COLUMN_NUM * KEYPAD_RAW_NUM)
#define KEYPAD_LONG_PRESS_TRIGGER_TIME 300 // ms


typedef struct
{
    uint8_t status;
    uint8_t lastStatus;
    uint32_t lastTriggerTime;
    
//    bool supportLongPress;
//    uint16_t longPressInterval; //长按间隔触发时间
    uint32_t longpressIntervalTime;
}Keypad_t;

static const uint8_t g_columnPin[KEYPAD_COLUMN_NUM] = {KEYPAD_SCAN1_PIN, KEYPAD_SCAN2_PIN, KEYPAD_SCAN3_PIN};
static const uint8_t g_rawPin[KEYPAD_RAW_NUM] = {KEYPAD_CODE1_PIN, KEYPAD_CODE2_PIN};
static Keypad_t g_key[KEYPAD_KEY_NUM];
static KeyEventHandle g_eventHandle = NULL;

static void delay(void)
{
    uint16_t i;
    for(i = 0; i < 50; i++);
}

static void eventHandle(uint8_t keyval, KeyStatus_t status, bool longpress, uint32_t lastTime)
{
    Syslog("key = %d, status = %d, longpress = %d, time = %d", keyval, status, longpress, lastTime);
    
    if(g_eventHandle != NULL)
    {
        g_eventHandle(keyval, status, longpress, lastTime);
    }
    
    if(status == KEY_STATUS_RELEASE)
    {
        HalBeepSet(20);
    }
}

static void keypadScan(void)
{
    uint8_t i, j;
    uint8_t val;
    Keypad_t *key;

    for(i = 0; i < KEYPAD_COLUMN_NUM; i++)
    {
        HalGPIOSetLevel(g_columnPin[i], 0);
        delay();
        for(j = 0; j < KEYPAD_RAW_NUM; j++)
        {
            val = j * KEYPAD_COLUMN_NUM + i;
            key = &g_key[j * KEYPAD_COLUMN_NUM + i];
            if(HalGPIOGetLevel(g_rawPin[j]) == 0) 
            {
                if(key->lastStatus != 0)
                {
                    eventHandle(val, KEY_STATUS_PRESS, false, 0);
                    key->lastStatus = 0;
                    key->lastTriggerTime = SysTime();
                }
                else
                {
                    if(SysTimeHasPast(key->lastTriggerTime, 2000)) //长按触发时间 2s
                    {
                        if(SysTimeHasPast(key->longpressIntervalTime, KEYPAD_LONG_PRESS_TRIGGER_TIME))
                        {
                            eventHandle(val, KEY_STATUS_PRESS, true, SysTime() - key->lastTriggerTime);
                            key->longpressIntervalTime = SysTime();
                            key->lastStatus = 0;
                        }
                    }
                }
            }
            else
            {
                if(key->lastStatus != 1) //release
                {
                    eventHandle(val, KEY_STATUS_RELEASE, false, SysTime() - key->lastTriggerTime);
                    key->lastTriggerTime = SysTime();
                    key->lastStatus = 1;
                }
            }
        }
        
        HalGPIOSetLevel(g_columnPin[i], 1);
    }
}

static void getKeyStatus(void)
{
    uint8_t i, j;
    Keypad_t *key;

    for(i = 0; i < KEYPAD_COLUMN_NUM; i++)
    {
        HalGPIOSetLevel(g_columnPin[i], 0);
        for(j = 0; j < KEYPAD_RAW_NUM; j++)
        {
            key = &g_key[j * KEYPAD_COLUMN_NUM + i];
            key->lastStatus = HalGPIOGetLevel(g_rawPin[j]);
        }
    }
}

void KeypadInit(KeyEventHandle eventHandle)
{
    HalGPIOConfig(KEYPAD_SCAN1_PIN, HAL_IO_OUTPUT);
    HalGPIOConfig(KEYPAD_SCAN2_PIN, HAL_IO_OUTPUT);
    HalGPIOConfig(KEYPAD_SCAN3_PIN, HAL_IO_OUTPUT);
    
    HalGPIOConfig(KEYPAD_CODE1_PIN, HAL_IO_INPUT);
    HalGPIOConfig(KEYPAD_CODE2_PIN, HAL_IO_INPUT);

    getKeyStatus();
	
    HalGPIOSetLevel(KEYPAD_SCAN1_PIN, 1);
    HalGPIOSetLevel(KEYPAD_SCAN2_PIN, 1);
    HalGPIOSetLevel(KEYPAD_SCAN3_PIN, 1);
    g_eventHandle = eventHandle;
}

void KeypadPoll(void)
{
    keypadScan();
}

