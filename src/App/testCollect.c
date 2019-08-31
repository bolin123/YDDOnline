#include "Sys.h"

void TestCollectPoll(void)
{
    static uint32_t oldTime = 0;
    static uint8_t count = 0;
    uint16_t data[8];

    if(count < 120)
    {
        data[0] = 188;
        data[1] = 15;
        data[2] = 14;
        data[3] = 189;
        
        data[4] = 199;
        data[5] = 12;
        data[6] = 11;
        data[7] = 201;

        if(SysTimeHasPast(oldTime, 1000))
        {
            SysRawDataWrite(count * sizeof(data), (uint8_t *)data, sizeof(data));
            count++;
            printf("raw data: %d\n", count);
            oldTime = SysTime();
        }
    }
    
}

void TestCollectInit(void)
{
#if 0
    SysDataRecord_t record;
    SysDataInfo_t info;
    
    record.num = 1;
    record.size = 16 * 120; //120s
    SysArgsSetRecord(&record);

    info.threshold = 150;
    info.times = 120;
    info.startAddr = 0;
    info.size = 16 * 120;
    info.date.year = 2019;
    info.date.month = 5;
    info.date.day = 23;
    info.date.hour = 16;
    info.date.minute = 30;
    info.date.second = 39;
    SysArgsSetPointInfo(1, &info);
#endif
    SysDeviceArgs_t args;
    SysDeviceArgsGet(&args);
    Syslog(" threshold:%d, intensityAlarm:%d, ringAlarm:%d, runTime:%d", args.signalThreshold, args.intensityAlarm, args.ringAlarm, args.runTime);
}

