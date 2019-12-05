#include "Sys.h"
#include "Protocol.h"
#include "W25Q64.h"
#include "YDDOnline.h"
#include "PowerManager.h"
#include "SysTimer.h"

#define SYS_FIRMWARE_VERSION "1.0.1.3"
#define SYS_POWER_ADC_NUM 10

typedef struct
{
    uint8_t rfAddr;
    uint8_t rfChnl;
    uint8_t devType;
    uint16_t threshold;
    uint16_t queryTime;
    SysCommunicateType_t commType; //通信方式 1=无线，2=有线
}SysConfig_t;

static uint16_t g_power[SYS_POWER_ADC_NUM];
static uint16_t g_powerAverage = 0;
static uint8_t g_errcode;
static SysConfig_t g_sysConfig;

uint8_t SysErrorCode(void)
{
    return g_errcode;
}

uint8_t SysPowerPercent(void)
{
    int16_t max = 3240, min = 2296; //电池电压：2.61v ~ 1.85v
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

SysCommunicateType_t SysCommunicateTypeGet(void)
{
    return g_sysConfig.commType;
}

void SysCommunicateTypeSet(SysCommunicateType_t type)
{
    g_sysConfig.commType = type;
    HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (const uint8_t *)&g_sysConfig, sizeof(SysConfig_t));
}

uint16_t SysSignalThresholdGet(void)
{
    return g_sysConfig.threshold;
}

void SysSignalThresholdSet(uint16_t value)
{
    //soft step 50, from 50~300
    uint16_t daValue = (value / 50) * HAL_DAC_STEP_VALUE + HAL_DAC_BASE_VALUE;
    HalDACSetValue(daValue);

    g_sysConfig.threshold = value;
    HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (const uint8_t *)&g_sysConfig, sizeof(SysConfig_t));
}

void SysSignalThresholdUpdate(void)
{
    uint16_t daValue = (g_sysConfig.threshold / 50) * HAL_DAC_STEP_VALUE + HAL_DAC_BASE_VALUE;
    HalDACSetValue(daValue);
}

void SysRfAddressSet(uint8_t addr)
{
    g_sysConfig.rfAddr = addr;
    HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (const uint8_t *)&g_sysConfig, sizeof(SysConfig_t));
}

uint16_t SysQueryIntervalGet(void)
{
    return g_sysConfig.queryTime;
}

void SysQueryIntervalSet(uint16_t time)
{
    g_sysConfig.queryTime = time;
    Syslog("time = %d", time);
    HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (const uint8_t *)&g_sysConfig, sizeof(SysConfig_t));
}

uint8_t SysRfAddressGet(void)
{
    return g_sysConfig.rfAddr;
}

uint8_t SysRfChannelGet(void)
{
    return g_sysConfig.rfChnl;
}

void SysRfChannelSet(uint8_t chnl)
{
    g_sysConfig.rfChnl = chnl;
    HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (const uint8_t *)&g_sysConfig, sizeof(SysConfig_t));
}

void SysDeviceTypeSet(HalDeviceType_t type)
{
    g_sysConfig.devType = (uint8_t)type;
    HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (const uint8_t *)&g_sysConfig, sizeof(SysConfig_t));
}

HalDeviceType_t SysDeviceTypeGet(void)
{
    return (HalDeviceType_t)g_sysConfig.devType;
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

static void startupInit(void)
{
    HalFlashRead(HAL_DEVICE_ARGS_ADDR, (uint8_t *)&g_sysConfig, sizeof(SysConfig_t));
    if(g_sysConfig.rfAddr == 0xFF)
    {
        g_sysConfig.rfAddr = 0x01;
        g_sysConfig.rfChnl = 0x01;
        g_sysConfig.devType = HAL_DEVICE_TYPE_PRESS;
        g_sysConfig.threshold = 50;
        g_sysConfig.queryTime = 60; //1min
        g_sysConfig.commType = SYS_COMMUNICATE_TYPE_WIRELESS; //默认为无线通信
        HalFlashWrite(HAL_DEVICE_ARGS_ADDR, (const uint8_t *)&g_sysConfig, sizeof(SysConfig_t));
    }
    SysSignalThresholdSet(g_sysConfig.threshold);
    //log
    printf("\r\n-----------------------------------------------------------\r\n");
    printf("--Firmware version:%s\r\n", SYS_FIRMWARE_VERSION);
    printf("--Device type:%d, threshold = %d\r\n", g_sysConfig.devType, g_sysConfig.threshold);
    printf("--Device address:%d, rf channel:%d\r\n", g_sysConfig.rfAddr, g_sysConfig.rfChnl);
    printf("--Query interval: %d\r\n", g_sysConfig.queryTime);
    printf("--Communication type: %s,%d\r\n", g_sysConfig.commType == SYS_COMMUNICATE_TYPE_WIRELESS ? "WIRELESS":"WIRED", g_sysConfig.commType);
    SysDateTime_t *time = SysDateTime();
    printf("--Now: %d-%02d-%02d %02d:%02d:%02d\r\n", time->year, time->month, time->day, 
                                          time->hour, time->minute, time->second);
    printf("--Compile date:%s %s\r\n", __DATE__, __TIME__);
    printf("-----------------------------------------------------------\r\n");
    
}

void SysReboot(void)
{
    Syslog("");
    HalCommonReboot();
}

void SysInitalize(void)
{
    g_errcode = HalCommonInitialize();
    printf(".....Hardware init....\n");
    g_errcode |= W25Q64Initialize();
    printf("errcode = %d\r\n", g_errcode);
    startupInit();
    PMInit();
    YDDOnlineInit();
}

extern void TestCollectPoll(void);
void SysPoll(void)
{
    HalCommonPoll();
    PMPoll();
    SysTimerPoll();
    YDDOnlinePoll();
    powerValueUpdate();
}

