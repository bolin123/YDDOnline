#include "Wireless.h"
#include "RFModule.h"

#define WIRELESS_WAIT_TIMES 0xE803D007 //设置外设开机时间1000ms+响应超时2000ms
#define WIRELESS_ACT_OUTPUT_LEVEL 0  //act管脚中断输出低电平

typedef struct
{
    char preamble;
    char mac[3];
    //char mac;
    char cmd[2];
    char length[4];
}WirelessHead;

typedef struct
{
    WirelessHead head;
    char devType[2];
    char err[2];
    char power[3];
    char data[];
}WirelessReport_t;

static WirelessDataEvent_cb g_eventHandle;

static uint8_t crc8Check(uint8_t *data, uint16_t len)
{
    uint8_t i;
    uint8_t value;
    
    value = 0;
    while(len--)
    {
        value ^= *data++;
        for(i = 0; i < 8; i++)
        {
            if(value & 0x01)
            {
                value = (value >> 1) ^ 0x8C;
            }
            else 
            {
                value >>= 1;
            }
        }
    }
    return value;
}

void WirelessReportData(uint8_t err, uint8_t power, uint16_t *data, uint16_t dataNum)
{
    char buff[255] = {0};
    uint8_t i;
    WirelessReport_t *report = (WirelessReport_t *)buff;

    report->head.preamble = '~';
    sprintf(report->head.mac, "%03d", SysRfAddressGet());
    //report->head.mac = SysRfAddressGet();
    
    report->head.cmd[0] = '0'; //
    report->head.cmd[1] = '1'; //report cmd
    
    sprintf(report->head.length, "%04d", sizeof(WirelessReport_t) - sizeof(WirelessHead) + dataNum * 4);
    
    sprintf(report->devType, "%02d", SysDeviceTypeGet());
    sprintf(report->err, "%02d", err);
    sprintf(report->power, "%03d", power);
    
    for(i = 0; i < dataNum; i++)
    {
        sprintf(&report->data[i*4], "%04d", data[i]);
    }
    uint8_t bufflen = sizeof(WirelessReport_t) + dataNum * 4;
    uint8_t crc = crc8Check((uint8_t *)buff, bufflen);
    sprintf(&buff[bufflen], "%02x", crc);
    buff[bufflen + 2] = '\r';
    RFModuleSendData((uint8_t *)buff, strlen(buff));
}

void WirelessDataParse(char *data)
{
    Syslog("%s", data);
    g_eventHandle(WIRELESS_EVENT_QUERY, NULL);
}

void WirelessSetChannel(uint8_t chn)
{
    RFMoudleSetChannel(chn);
}

static void rfEventHandle(RFModuleEvent_t event, void *arg)
{
    uint8_t chnl;
    uint32_t times;
    uint32_t actmode;
    
    if(event == RFMODULE_EVENT_GET_RFCHNL)
    {
        chnl = (uint8_t)(uint32_t)arg;
        SysPrint("RF chnl = %d\n", chnl);
        if(chnl != SysRfChannelGet())
        {
            RFMoudleSetChannel(SysRfChannelGet());
        }
    }
    else if(event == RFMODULE_EVENT_GET_WAITTIME)//RFMODULE_EVENT_GET_WAITTIME
    {
        times = (uint32_t)arg;
        SysPrint("RF wait times = %04x, %04x\n", times, (uint32_t)arg);
        if(times != WIRELESS_WAIT_TIMES)
        {
            RFModuleSetWaitTimes(WIRELESS_WAIT_TIMES);
        }
    }
    else //RFMODULE_EVENT_GET_ACTMODE
    {
        actmode = (uint32_t)arg;
        SysPrint("ACT mode: output [%s]\n", actmode ? "high":"low");
        if(actmode != WIRELESS_ACT_OUTPUT_LEVEL)
        {
            RFModuleSetActMode(WIRELESS_ACT_OUTPUT_LEVEL);
        }
    }
}

void WirelessInit(WirelessDataEvent_cb eventHandle)
{
    g_eventHandle = eventHandle;
    RFModuleInit(rfEventHandle);
    RFModuleGetChannel();
    RFModuleGetWaitTimes();
    RFModuleGetActMode();
}

void WirelessPoll(void)
{
    RFModulePoll();
}

