#ifndef POWERMANAGER_H
#define POWERMANAGER_H

typedef enum
{
    PM_WAKEUP_TYPE_NONE = 0x00,
    PM_WAKEUP_TYPE_WIRELESS = 0x01, //ÎÞÏß»½ÐÑ
    PM_WAKEUP_TYPE_LIGHT = 0x02,  //µÆ¹â»½ÐÑ
}PMWakeupType_t;

typedef enum
{
    PM_STATUS_SLEEP = 0,
    PM_STATUS_WAKEUP,
}PMStatus_t;

typedef enum
{
    PM_DEVICE_ID_APP = 0,
    PM_DEVICE_ID_SENSORS,
    PM_DEVICE_ID_IR,
    PM_DEVICE_ID_LED,
    PM_DEVICE_ID_RFMODULE,
    PM_DEVICE_ID_HAL,
    PM_DEVICE_ID_COUNT,
}PMDeviceId_t;

typedef struct PM_st
{
    char enable;
    PMStatus_t status;
    void (*sleep)(struct PM_st *pm);
    void (*wakeup)(struct PM_st *pm, PMWakeupType_t type);
}PM_t;

unsigned char PMIsTypeWakeup(PMWakeupType_t type);
void PMWakeup(PMWakeupType_t type);
void PMStartSleep(unsigned int after);
void PMRegist(PM_t *pm, PMDeviceId_t id);

void PMInit(void);
void PMPoll(void);

#endif

