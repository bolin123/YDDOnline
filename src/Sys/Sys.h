#ifndef SYS_H
#define SYS_H

#include "HalCommon.h"

#define SYS_TIME_COUNT_1S 1000 //1s
#define SysTime() HalGetSysTimeCount()
#define SysTimeHasPast(oldTime, past) (SysTime() - (oldTime) > past)

#define SysDateTime_t HalRTCTime_t
#define SysDateTime() HalRTCGetTime()

#define Syslog(...) do{printf("[%s]: ", __FUNCTION__); printf(__VA_ARGS__); printf("\r\n");}while(0)

#define SYS_MAC_ADDR_LEN 6

#if 0
typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}SysDateTime_t;
#endif 

typedef struct
{
    uint16_t x;
    uint16_t y;
}SysDisplayPosition_t;


typedef struct
{
    //uint16_t id;      //包序号,从1开始
    uint16_t threshold; //阈值
    uint16_t times;     //持续时间
    SysDateTime_t date; //开始日期
    uint16_t size;      //字节数
    uint32_t startAddr;
}SysDataInfo_t;

typedef struct
{
    uint16_t num;   //包个数
    uint32_t size;  //总长度
}SysDataRecord_t;

//数据采集参数
typedef struct
{
    uint8_t beep; //蜂鸣器开关
    uint8_t brightness;
    uint16_t signalThreshold; //信号阈值 0~300, step 50, default 150
    uint16_t runTime;         //测试时间 20S、60S、120S、240S、480S、600S、800S、999S, default 120
    uint16_t intensityAlarm;  //强度报警值, 500~1500, step 500, default 500
    uint16_t ringAlarm;       //振铃报警值，5K、20K、50K、80K、100K、120K、150K、180K、200K，default 200k
}SysCollectArgs_t; 

//SysDateTime_t *SysDateTime(void);
int SysDateTimeSet(SysDateTime_t *dateTime);
uint8_t SysPowerPercent(void);
void SysReboot(void);

//SysDataInfo_t *SysDataGetInfo(uint16_t id);
void SysArgsGetRecord(SysDataRecord_t *record);
void SysArgsSetRecord(SysDataRecord_t *record);
void SysArgsGetPointInfo(uint16_t id, SysDataInfo_t *info);
void SysArgsSetPointInfo(uint16_t id, SysDataInfo_t *info);
void SysArgsClear(void);
void SysBeepEnable(bool enable);
void SysSignalThresholdSet(uint16_t value);

uint8_t *SysMacAddrGet(void);
void SysMacAddrSet(uint8_t *mac);

void SysCollectArgsGet(SysCollectArgs_t *args);
void SysCollectArgsSet(SysCollectArgs_t *args);

void SysRawDataRead(uint32_t address, uint8_t *buff, uint16_t length);
void SysRawDataWrite(uint32_t addresss, uint8_t *buff, uint16_t length);

void SysInitalize(void);
void SysPoll(void);

#endif

