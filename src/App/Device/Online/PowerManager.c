#include "PowerManager.h"
#include "Sys.h"

static PM_t g_pm[PM_DEVICE_ID_COUNT];
static volatile bool g_startSleep = false;
static volatile uint8_t g_wakeupType;
static uint32_t g_sleepTime = 0;

bool PMIsTypeWakeup(PMWakeupType_t type)
{
    return (g_wakeupType & type) != PM_WAKEUP_TYPE_NONE;
}

void PMWakeup(PMWakeupType_t type)
{
#if 1
    int i;

    g_startSleep = false;

    if(!PMIsTypeWakeup(type))
    {
        g_wakeupType |= type;
        g_sleepTime = SysTime();
        
        for(i = PM_DEVICE_ID_COUNT; i > 0; i--)
        {
            if(g_pm[i - 1].enable)
            {
                g_pm[i - 1].wakeup(&g_pm[i - 1], type);
            }
        }
    }
    
#endif
}

void PMStartSleep(uint32_t after)
{
#if 1
    Syslog("after %dms", after);
    g_startSleep = true;
    if((SysTime() + after) > g_sleepTime)
    {
        g_sleepTime = SysTime() + after;
    }
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
        //HalGPIOSetLevel(HAL_SENSORS_POWER_PIN, HAL_SENSORS_POWER_DISABLE_LEVEL);
        //HalGPIOSetLevel(HAL_485_POWER_PIN,     1);
        g_wakeupType = PM_WAKEUP_TYPE_NONE;
        pm->status = PM_STATUS_SLEEP;        
        HalCommonFallasleep();
    }
}

static void powerup(PM_t *pm, PMWakeupType_t type)
{
    if(pm && pm->status == PM_STATUS_SLEEP)
    {
        HalCommonWakeup();
        //HalGPIOSetLevel(HAL_SENSORS_POWER_PIN, HAL_SENSORS_POWER_ENABLE_LEVEL);
        //HalGPIOSetLevel(HAL_485_POWER_PIN,     0);
 
        pm->status = PM_STATUS_WAKEUP;
    }
}
/*
static void powerPinInit(void)
{
    HalGPIOConfig(HAL_SENSORS_POWER_PIN, HAL_IO_OUTPUT);//PC13
    HalGPIOSetLevel(HAL_SENSORS_POWER_PIN, HAL_SENSORS_POWER_ENABLE_LEVEL);
}
*/
void PMInit(void)
{
    PM_t pm;
    pm.status = PM_STATUS_WAKEUP;
    pm.sleep = lowSleep;
    pm.wakeup = powerup;
    PMRegist(&pm, PM_DEVICE_ID_HAL);

    //powerPinInit();
}

void PMPoll(void)
{
    sleepPoll();
}

