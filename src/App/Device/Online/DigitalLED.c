#include "DigitalLED.h"

static uint8_t g_segCode[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};
static uint8_t g_segPin[] = {0x33, 0x16, 0x2c, 0x31, 0x32, 0x34, 0x2b, 0x30};
static uint8_t g_digPin[DIGITAL_LED_ID_COUNT] = {0x40, 0x19, 0x18, 0x17};
static uint8_t g_charValue[DIGITAL_LED_ID_COUNT];
static bool g_ledOn = false;

static void ledDisplayUpdate(void)
{
    uint8_t i, value;
    bool point = false;
    static uint8_t displayPos = 0;
    static uint32_t oldTime;
    
    if(g_ledOn && SysTimeHasPast(oldTime, 2))
    {
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
        oldTime = SysTime();
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
    g_ledOn = true;
}

void DigitalLEDOff(void)
{
    uint8_t i;
    g_ledOn = false;
    for(i = 0; i < sizeof(g_digPin); i++)
    {
        HalGPIOSetLevel(g_digPin[i], 0);
    }
    for(i = 0; i < sizeof(g_segPin); i++)
    {
        HalGPIOSetLevel(g_segPin[i], 0);
    }
}

void DigitalLEDInit(void)
{

    uint8_t i;

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
}

void DigitalLEDPoll(void)
{
    ledDisplayUpdate();
}

