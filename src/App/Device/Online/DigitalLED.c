#include "DigitalLED.h"
#include "PowerManager.h"

static uint8_t g_segCode[] = { 
            0x3f, 0x06, 0x5b, 0x4f, 0x66,
            0x6d, 0x7d, 0x07, 0x7f, 0x6f,
            0x77, 0x7c, 0x39, 0x5e, 0x79, 
            0x71, 0x76, 0x38, 0x54, 0x73,
            0x77, 0x6D, 0x3E, 0x00};
/*
LED_D3, LED_D1, LED_D7, LED_D5, LED_D4, LED_D2, LED_D8, LED_D6
*/
#if defined(HAL_OLD_DEVICE)
static uint8_t g_segPin[] = {0x33, 0x16, 0x2c, 0x31, 0x32, 0x34, 0x2b, 0x30};
#else
static uint8_t g_segPin[] = {0x36, 0x13, 0x32, 0x34, 0x35, 0x37, 0x31, 0x33};
#endif
/*
MCU_P97, MCU_P96, MCU_P95, MCU_P93
*/
static uint8_t g_digPin[DIGITAL_LED_ID_COUNT] = {0x40, 0x19, 0x18, 0x17};

static uint8_t g_charValue[DIGITAL_LED_ID_COUNT];
static bool g_ledOn = false;

void DigitalLEDScan(void)
{
    uint8_t i;
    uint8_t value;
    bool point = false;
    static volatile uint8_t displayPos = 0;
    
    for(i = 0; i < DIGITAL_LED_ID_COUNT; i++)
    {
        HalGPIOSetLevel(g_digPin[i], i == displayPos);//en
    }

    value = g_segCode[g_charValue[displayPos] & 0x7f];
    point = g_charValue[displayPos] & 0x80;
    for(i = 0; i < sizeof(g_segPin); i++)
    {
        HalGPIOSetLevel(g_segPin[i], (value & (0x80 >> i)) != 0);
    }
    HalGPIOSetLevel(g_segPin[0], point == 0x80);
    displayPos++;
    if(displayPos >= DIGITAL_LED_ID_COUNT)
    {
        displayPos = 0;
    }
}

void DigitalLEDSetChars(DigitalLEDId_t id, uint8_t value, bool point)
{
    if(id < DIGITAL_LED_ID_COUNT)
    {
        g_charValue[id] = value | (point ? 0x80 : 0x00);
    }
}

void DigitalLEDOn(void)
{
    if(!g_ledOn)
    {
        HalLEDUpdateTimerEnable(true);
        g_ledOn = true;
    }
}

void DigitalLEDOff(void)
{
    uint8_t i;
    if(g_ledOn)
    {
        HalLEDUpdateTimerEnable(false);
        for(i = 0; i < sizeof(g_digPin); i++)
        {
            HalGPIOSetLevel(g_digPin[i], 0);
        }
        for(i = 0; i < sizeof(g_segPin); i++)
        {
            HalGPIOSetLevel(g_segPin[i], 0);
        }
        g_ledOn = false;
    }
}

static void ledPMSleep(PM_t *pm)
{
    if(pm)
    {
        DigitalLEDOff();
        HalGPIOSetLevel(HAL_IR_POWER_PIN, HAL_IR_POWER_DISABLE_LEVEL); //power off
        pm->status = PM_STATUS_SLEEP;
    }
}

static void ledPMWakeup(PM_t *pm, PMWakeupType_t type)
{
    if(pm && pm->status == PM_STATUS_SLEEP && type == PM_WAKEUP_TYPE_LIGHT) //光照才打开遥控和数码管电源
    {
        HalGPIOSetLevel(HAL_IR_POWER_PIN, HAL_IR_POWER_ENABLE_LEVEL); //power on
        DigitalLEDOn();
        for(uint8_t i = 0; i < DIGITAL_LED_ID_COUNT; i++)
        {
            DigitalLEDSetChars((DigitalLEDId_t)i, 23, false);
        }
        pm->status = PM_STATUS_WAKEUP;
    }
}

static void ledPMRegist(void)
{
    PM_t pm;

    pm.enable = true;
    pm.status = PM_STATUS_WAKEUP;
    pm.sleep = ledPMSleep;
    pm.wakeup = ledPMWakeup;
    PMRegist(&pm, PM_DEVICE_ID_LED);
}


void DigitalLEDInit(void)
{

    uint8_t i;
    HalGPIOConfig(HAL_IR_POWER_PIN, HAL_IO_OUTPUT);//PC13
    HalGPIOSetLevel(HAL_IR_POWER_PIN, HAL_IR_POWER_ENABLE_LEVEL);//enable

    for(i = 0; i < sizeof(g_digPin); i++)
    {
        HalGPIOConfig(g_digPin[i], HAL_IO_OUTPUT);
        HalGPIOSetLevel(g_digPin[i], 0);
    }

    for(i = 0; i < sizeof(g_segPin); i++)
    {
        HalGPIOConfig(g_segPin[i], HAL_IO_OUTPUT);
        HalGPIOSetLevel(g_segPin[i], 0);
    }
    ledPMRegist();
}

