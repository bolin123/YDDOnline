#ifndef HAL_RTC_H
#define HAL_RTC_H

#include "HalCtype.h"

#define HAL_RTC_YEAR_START 1970

typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}HalRTCTime_t;

uint32_t HalRTCGetUtc(void);
int HalRTCSetUtc(uint32_t utc);
void HalRTCAlarmSet(uint16_t seconds);

int HalRTCSetTime(HalRTCTime_t *time);
HalRTCTime_t * HalRTCGetTime(void);
uint16_t HalRTCInit(void);

#endif

