#include "DispLoop.h"
#include "DigitalLED.h"
#include "Sys.h"

typedef struct
{   
    bool enable;
    DispLoopGetValue_cb getValue;
}DispLoopHandle_t;

static DispLoopHandle_t g_handle[DISPLOOP_ID_COUNT];
static bool g_loopStart = false;
static uint8_t g_loopCount = 0;
static DispLoopID_t g_curLoopID;
static DispLoopDone_cb g_loopDoneHandle;

void DispLoopRegister(DispLoopID_t id, DispLoopGetValue_cb getvalue)
{
    g_handle[id].enable = true;
    g_handle[id].getValue = getvalue;
}

void DispLoopStop(void)
{
    g_loopStart = false;
}

void DispLoopStart(uint8_t loop)
{
    g_loopStart = true;
    g_loopCount = loop;
    g_curLoopID = DISPLOOP_ID_ADDR;
}

static void disploopHandle(void)
{
    DispLoopHandle_t *curLoop;
    //static uint8_t curLoopID = 0;
    static uint32_t lastTime = 0;
    DispLoopValue_t *value;
    
    if(g_loopStart)
    {
        if(lastTime == 0 || SysTimeHasPast(lastTime, 1000))
        {
            curLoop = &g_handle[g_curLoopID];
            if(curLoop->enable && curLoop->getValue)
            {
                value = curLoop->getValue(g_curLoopID);
                if(value)
                {
                    DigitalLEDSetChars(DIGITAL_LED_ID_CMD, value->flag, (value->flag & 0x80) != 0);
                    DigitalLEDSetChars(DIGITAL_LED_ID_1, value->contents[0], (value->contents[0] & 0x80) != 0);
                    DigitalLEDSetChars(DIGITAL_LED_ID_2, value->contents[1], (value->contents[1] & 0x80) != 0);
                    DigitalLEDSetChars(DIGITAL_LED_ID_3, value->contents[2], (value->contents[2] & 0x80) != 0);
                    lastTime = SysTime();
                }
            }
            g_curLoopID++;
            if(g_curLoopID >= DISPLOOP_ID_COUNT)
            {
                g_curLoopID = DISPLOOP_ID_ADDR;
                g_loopCount--;
                if(g_loopCount == 0)
                {
                    g_loopStart = false;
                    g_loopDoneHandle();
                }
            }
        }
    }
}

void DispLoopInit(DispLoopDone_cb loopDone)
{
    g_loopDoneHandle = loopDone;
}

void DispLoopPoll(void)
{
    disploopHandle();
}

