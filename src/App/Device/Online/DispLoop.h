#ifndef DISPLOOP_H
#define DISPLOOP_H

#include "Sys.h"

typedef enum
{
    DISPLOOP_ID_ADDR = 0, //无线地址
    DISPLOOP_ID_RFCHN,    //无线通道
    DISPLOOP_ID_DEVTYPE,  //设备类型
    DISPLOOP_ID_ERRCODE,  //异常代码
    DISPLOOP_ID_POWER,    //电量
//    DISPLOOP_ID_TEMP,     //温度
    DISPLOOP_ID_PRESS,    //应力
    DISPLOOP_ID_THRESHOLD,//阈值
    DISPLOOP_ID_COUNT,
}DispLoopID_t;

typedef struct
{
    uint8_t flag;
    uint8_t contents[3];
}DispLoopValue_t;

typedef DispLoopValue_t *(*DispLoopGetValue_cb)(DispLoopID_t id);
typedef void (*DispLoopDone_cb)(void);

void DispLoopRegister(DispLoopID_t id, DispLoopGetValue_cb getvalue);
void DispLoopStop(void);
void DispLoopStart(uint8_t num);
void DispLoopInit(DispLoopDone_cb loopDone);
void DispLoopPoll(void);

#endif

