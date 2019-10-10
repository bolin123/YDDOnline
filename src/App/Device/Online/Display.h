#ifndef DISPLAY_H
#define DISPLAY_H

#include "Sys.h"

typedef enum
{
    DISPLAY_ID_ADDR = 0, //无线地址
    DISPLAY_ID_RFCHN,    //无线通道
    DISPLAY_ID_DEVTYPE,  //设备类型
    DISPLAY_ID_ERRCODE,  //异常代码
    DISPLAY_ID_POWER,    //电量
    DISPLAY_ID_TEMP,     //温度
    DISPLAY_ID_PRESS,    //应力
    DISPLAY_ID_THRESHOLD,//阈值
    DISPLAY_ID_COUNT,
}DisplayID_t;

typedef void (*DisplayEvent_cb)(bool done, DisplayID_t id);

void DisplayLoopShow(bool start, uint8_t lcount);
void DisplayContentUpdate(DisplayID_t id, uint8_t symbol, uint16_t value);
void DisplayInit(DisplayEvent_cb cb);
void DisplayPoll(void);

#endif
