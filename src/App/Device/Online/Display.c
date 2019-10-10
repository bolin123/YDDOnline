#include "Display.h"
#include "DigitalLED.h"

#define DISPLAY_LOOP_INTERVAL_TIME 1000 //1s

typedef struct
{
    bool display;
    uint8_t symbol;
    uint16_t num;
}DisplayValue_t;

static bool g_start = false;
static uint8_t g_loopCount = 0;
static DisplayValue_t g_loopValue[DISPLAY_ID_COUNT];
static DisplayID_t g_displayId;
static DisplayEvent_cb g_eventCb = NULL;

void DisplayLoopShow(bool start, uint8_t lcount)
{
    g_start = start;
    g_loopCount = lcount;
    g_displayId = DISPLAY_ID_ADDR;
}

void DisplayContentUpdate(DisplayID_t id, uint8_t symbol, uint16_t value)
{
    if(id < DISPLAY_ID_COUNT)
    {
        g_loopValue[id].display = true;
        g_loopValue[id].symbol = symbol;
        g_loopValue[id].num = value;
    }
}

static void displayLoop(void)
{
    uint8_t i;
    static uint32_t lastTime;
    uint16_t value;

    if(g_start && SysTimeHasPast(lastTime, DISPLAY_LOOP_INTERVAL_TIME))
    {
        g_eventCb(false, g_displayId);
        if(g_loopValue[g_displayId].display)
        {
            value = g_loopValue[g_displayId].num;
            DigitalLEDSetChars(DIGITAL_LED_ID_CMD, g_loopValue[g_displayId].symbol, false);
            DigitalLEDSetChars(DIGITAL_LED_ID_1, value / 100, false);
            if(g_displayId == DISPLAY_ID_TEMP || g_displayId == DISPLAY_ID_PRESS)
            {
                DigitalLEDSetChars(DIGITAL_LED_ID_2, value % 100 / 10, true); //Ð¡Êýµã
            }
            else
            {
                DigitalLEDSetChars(DIGITAL_LED_ID_2, value % 100 / 10, false);
            }
            DigitalLEDSetChars(DIGITAL_LED_ID_3, value % 10, false);

            lastTime = SysTime();
        }
        
        g_displayId++;
        if(g_displayId >= DISPLAY_ID_COUNT)
        {
            g_loopCount--;
            g_displayId = 0;
            if(g_loopCount == 0)
            {
                for(i = 0; i < DISPLAY_ID_COUNT; i++)
                {
                    g_loopValue[i].display = false;
                }
                g_start = false;
                g_eventCb(true, DISPLAY_ID_COUNT);
            }
        }
    }
}

void DisplayInit(DisplayEvent_cb cb)
{   
    g_eventCb = cb;
}

void DisplayPoll(void)
{
    displayLoop();
}

