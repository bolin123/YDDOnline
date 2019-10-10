#include "Wireless.h"
#include "RFModule.h"

#define WIRELESS_WAIT_TIMES 0xE803D007 //设置外设开机时间1000ms+响应超时2000ms

typedef struct
{
    char preamble;
    char mac[SYS_MAC_ADDR_LEN];
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

void WirelessReportData(uint8_t err, uint8_t power, uint16_t *data, uint16_t dataNum)
{
    char buff[255] = {0};
    uint8_t i;
    WirelessReport_t *report = (WirelessReport_t *)buff;

    report->head.preamble = '~';
    sprintf(report->head.mac, "%03d", SysRfAddressGet());
    //memcpy(report->head.mac, SysMacAddrGet(), SYS_MAC_ADDR_LEN);
    
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
    buff[sizeof(WirelessReport_t) + dataNum * 4] = '\r';
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
    if(event == RFMODULE_EVENT_GET_RFCHNL)
    {
        chnl = (uint8_t)(uint32_t)arg;
        Syslog("got chnl = %d", chnl);
        if(chnl != SysRfChannelGet())
        {
            RFMoudleSetChannel(SysRfChannelGet());
        }
    }
    else //RFMODULE_EVENT_GET_WAITTIME
    {
        times = (uint32_t)arg;
        Syslog("got wait times = %04x, %04x", times, (uint32_t)arg);
        if(times != WIRELESS_WAIT_TIMES)
        {
            RFModuleSetWaitTimes(WIRELESS_WAIT_TIMES);
        }
    }
}

void WirelessInit(WirelessDataEvent_cb eventHandle)
{
    g_eventHandle = eventHandle;
    RFModuleInit(rfEventHandle);
    RFModuleGetChannel();
    RFModuleGetWaitTimes();
}

void WirelessPoll(void)
{
    RFModulePoll();
}

