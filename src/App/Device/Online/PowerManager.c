#include "PowerManager.h"
#include "Sys.h"

static PM_t g_pm[PM_DEVICE_ID_COUNT];
static volatile bool g_startSleep = false;
static uint32_t g_sleepTime;

void PMWakeup(void)
{
#if 0
    int i;

    g_startSleep = false;
    
    for(i = PM_DEVICE_ID_COUNT; i > 0; i--)
    {
        if(g_pm[i - 1].enable)
        {
            g_pm[i - 1].wakeup(&g_pm[i - 1]);
        }
    }
#endif
}

void PMStartSleep(uint32_t after)
{
#if 0
    g_startSleep = true;
    g_sleepTime = SysTime() + after;
#endif
}


void PMRegist(PM_t *pm, PMDeviceId_t id)
{
    if(id < PM_DEVICE_ID_COUNT)
    {
        g_pm[id] = *pm;
        g_pm[id].enable = true;
    }
}

static void sleepPoll(void)
{
    int i;
    if(g_startSleep && SysTime() > g_sleepTime)
    {
        for(i = 0; i < PM_DEVICE_ID_COUNT; i++)
        {
            if(g_pm[i].enable && g_pm[i].status == PM_STATUS_WAKEUP)
            {
                g_pm[i].sleep(&g_pm[i]);
                //wait
                break;
            }
        }
    }
}

static void lowSleep(PM_t *pm)
{
    if(pm)
    {
        pm->status = PM_STATUS_SLEEP;
        HalCommonFallasleep();
    }
}

static void powerup(PM_t *pm)
{
    if(pm)
    {
        HalCommonWakeup();
        pm->status = PM_STATUS_WAKEUP;
    }
}

void PMInit(void)
{
    PM_t pm;
    pm.status = PM_STATUS_WAKEUP;
    pm.sleep = lowSleep;
    pm.wakeup = powerup;
    PMRegist(&pm, PM_DEVICE_ID_HAL);
}

void PMPoll(void)
{
    sleepPoll();
}

