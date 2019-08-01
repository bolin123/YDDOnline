#include "Sys.h"
#include "Protocol.h"
#include "W25Q64.h"
#include "YDDOnline.h"
//#include "Display.h"
#include "PowerManager.h"

#define SYS_FIRMWARE_VERSION "1.0.0.1"
#define SYS_POWER_ADC_NUM 10
static uint16_t g_power[SYS_POWER_ADC_NUM];
static uint16_t g_powerAverage = 0;

uint8_t SysPowerPercent(void)
{
    int16_t max = 3240, min = 2296; //µç³ØµçÑ¹£º2.61v ~ 1.85v
    int8_t percent = 0;

    if(g_powerAverage > max)
    {
        return 100;
    }
    else if(g_powerAverage < min)
    {
        return 0;
    }
    else
    {
        percent = (int8_t)((g_powerAverage - min) * 100 / (max - min));
    }
    
    return (uint8_t)percent;
    //return 87;
}

static void powerValueUpdate(void)
{
    static uint8_t powerNum = 0;
    static uint32_t lastTime;
    uint8_t i;
    uint32_t valuecount = 0;

    if(SysTimeHasPast(lastTime, 50))
    {
        g_power[powerNum++] = HalADCGetPowerValue();
        if(powerNum >= SYS_POWER_ADC_NUM)
        {
            for(i = 0; i < powerNum; i++)
            {
                valuecount += g_power[i];
            }
            g_powerAverage = valuecount / powerNum;
            powerNum = 0;
        }
        lastTime = SysTime();
    }
}

int SysDateTimeSet(SysDateTime_t *dateTime)
{
    return HalRTCSetTime((HalRTCTime_t *)dateTime);
}

void SysArgsGetRecord(SysDataRecord_t *record)
{
    if(record != NULL)
    {
        HalFlashRead(HAL_DATA_RECORD_ADDR, (uint8_t *)record, sizeof(SysDataRecord_t));
        if(record->num == 0xffff || record->size == 0xffffffff)
        {
            record->num = 0;
            record->size = 0;
            HalFlashWrite(HAL_DATA_RECORD_ADDR, (const uint8_t *)record, sizeof(SysDataRecord_t));
        }
    }
}

void SysArgsSetRecord(SysDataRecord_t *record)
{
    if(record != NULL)
    {
        Syslog("num = %d, size = %d", record->num, record->size);
        HalFlashWrite(HAL_DATA_RECORD_ADDR, (const uint8_t *)record, sizeof(SysDataRecord_t));
    }
}

//id start 1,2,3...
void SysArgsSetPointInfo(uint16_t id, SysDataInfo_t *info)
{
    uint32_t addr;
    //HAL_DATA_POINT_INFO_ADDR
    if(info != NULL)
    {
        Syslog("id = %d, threshold = %d, times = %d, size = %d, address = %d", id, info->threshold, 
                                                                    info->times, info->size, info->startAddr);
        addr = HAL_DATA_POINT_INFO_ADDR + ((id - 1) * sizeof(SysDataInfo_t));
        HalFlashWrite(addr, (const uint8_t *)info, sizeof(SysDataInfo_t));
    }
}

void SysArgsGetPointInfo(uint16_t id, SysDataInfo_t *info)
{
    uint32_t addr;
    //HAL_DATA_POINT_INFO_ADDR
    if(info != NULL)
    {
        addr = HAL_DATA_POINT_INFO_ADDR + ((id - 1) * sizeof(SysDataInfo_t));
        HalFlashRead(addr, (uint8_t *)info, sizeof(SysDataInfo_t));
    }
}

void SysSignalThresholdSet(uint16_t value)
{
    //soft step 50, from 50~300
    uint16_t daValue = (value / 50) * HAL_DAC_STEP_VALUE + HAL_DAC_BASE_VALUE;
    HalDACSetValue(daValue);
}

void SysBeepEnable(bool enable)
{
//    HalBeepEnable(enable);
}

void SysArgsClear(void)
{
    SysDataRecord_t record;

    Syslog("");
    record.num = 0;
    record.size = 0;
    SysArgsSetRecord(&record);
}

void SysCollectArgsGet(SysCollectArgs_t *args)
{
    if(args)
    {
        HalFlashRead(HAL_DEVICE_ARGS_ADDR, (uint8_t *)args, sizeof(SysCollectArgs_t));
        if(args->signalThreshold == 0xffff ||
            args->runTime == 0xffff ||
            args->intensityAlarm == 0xffff ||
            args->ringAlarm == 0xffff)
        {
            args->beep = 1;
            args->brightness = 50;
            args->signalThreshold = 150;
            args->runTime = 120;
            args->intensityAlarm = 500;
            args->ringAlarm = 200;
            HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (const uint8_t *)args, sizeof(SysCollectArgs_t));
        }
    }
}

void SysCollectArgsSet(SysCollectArgs_t *args)
{
    if(args)
    {
        Syslog("threshold = %d, runTime = %d, alarm = %d, ring = %d", args->signalThreshold, args->runTime, 
                                                                args->intensityAlarm, args->ringAlarm);
        HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (const uint8_t *)args, sizeof(SysCollectArgs_t));
    }
}

void SysRawDataRead(uint32_t addresss, uint8_t *buff, uint16_t length)
{
    if(addresss != HAL_FLASH_INVALID_ADDR)
    {
        W25Q64Read(buff, addresss, length);
    }
}

void SysRawDataWrite(uint32_t addresss, uint8_t *buff, uint16_t length)
{
    if(addresss != HAL_FLASH_INVALID_ADDR)
    {
        W25Q64Write(buff, addresss, length);
    }
}

static uint8_t g_mac[SYS_MAC_ADDR_LEN];
void SysMacAddrSet(uint8_t *mac)
{
    Syslog("");
    for(int i = 0; i < SYS_MAC_ADDR_LEN; i++)
    {
        printf("%02x ", mac[i]);
    }
    printf("\n");
    memcpy(g_mac, mac, SYS_MAC_ADDR_LEN);
}

uint8_t *SysMacAddrGet(void)
{
    return g_mac;
}

static void startupInit(void)
{
    SysDataRecord_t record;
    SysCollectArgs_t args;

    SysArgsGetRecord(&record);
    SysCollectArgsGet(&args);
//    HalBeepEnable(args.beep);
    SysSignalThresholdSet(args.signalThreshold);

    //log
    printf("\r\n-----------------------------------------------------------\r\n");
    printf("--Firmware version:%s\r\n", SYS_FIRMWARE_VERSION);
    printf("--Compile date:%s %s\r\n", __DATE__, __TIME__);
    printf("--Data record: num = %d, size = %d\r\n", record.num, record.size);
    printf("--XHFZ = %d, CSSJ = %d, QDBJ = %d, ZLBJ = %d\r\n", args.signalThreshold, args.runTime, 
                                                                args.intensityAlarm, args.ringAlarm);
    SysDateTime_t *time = SysDateTime();
    printf("--Now: %d-%02d-%02d %02d:%02d:%02d\r\n", time->year, time->month, time->day, 
                                          time->hour, time->minute, time->second);
    printf("-----------------------------------------------------------\r\n");
    
//    HalBeepSet(100);
}

void SysReboot(void)
{
    Syslog("");
    HalCommonReboot();
}

void SysInitalize(void)
{
    uint16_t errcode;
    errcode = HalCommonInitialize();
    //ProtocolInitialize();
    printf(".....Hardware init....\n");
    errcode |= W25Q64Initialize();
    printf("errcode = %d\r\n", errcode);
    //YDDInitialize(errcode);
    PMInit();
    YDDOnlineInit();
    startupInit();
}

extern void TestCollectPoll(void);
void SysPoll(void)
{
    HalCommonPoll();
    //ProtocolPoll();
    W25Q64Poll();
    //TestCollectPoll();
    //YDDPoll();
    PMPoll();
    YDDOnlinePoll();
    powerValueUpdate();
}

