#include "Sys.h"
#include "IR.h"
#include "DataManager.h"
#include "IRDataTransfer.h"

typedef struct IRDataTransferList_st
{
    bool enable;
    uint8_t cmd;
    uint8_t pid;
    uint8_t content[IR_SEND_PACKET_LEN];
    uint8_t reties;
    uint32_t lastTime;
}IRDataTransferData_t;


static bool g_transferStart = false;
static uint32_t g_totalSize, g_transferSize;
static IRDataTransferData_t g_irTransData;
static IRDataTransferEventHandle_t g_dtEventHandle;
static uint8_t g_packetid = 0;
static uint8_t g_poffset = HAL_DEVICE_DATA_PACKET_LENGTH;

static void irLowSend(uint8_t cmd, uint8_t misc, uint8_t *data)
{
    IRDataTransferData_t *transData = &g_irTransData;

    transData->enable = true;
    transData->cmd = cmd;
    transData->pid = misc;
    memcpy(transData->content, data, IR_SEND_PACKET_LEN);
    transData->reties = 0;
    transData->lastTime = 0;
}

static void irSendHandle(void)
{
    IRDataTransferData_t *transData = &g_irTransData;

    if(transData->enable && SysTimeHasPast(transData->lastTime, 2000))
    {
        if(transData->reties >= 3)
        {
            transData->enable = false;
            g_dtEventHandle(IR_DTRANS_EVENT_RESULT, 0);
        }
        else
        {
            Syslog("retry");
            IRSendData((IRKey_t)transData->cmd, transData->pid, transData->content);
            transData->lastTime = SysTime();
            transData->reties++;
        }
    }
}

static uint8_t dataTransferPercent(void)
{
    return (uint8_t)(g_transferSize * 100 / g_totalSize);
}

void IRDataTransferResume(void)
{
    uint8_t buff[IR_SEND_PACKET_LEN];
    uint8_t lastlen;
    static uint8_t packet[HAL_DEVICE_DATA_PACKET_LENGTH];
    
    if(g_transferStart)
    {
        memset(buff, 0xff, sizeof(buff));
        
        if(g_transferSize < g_totalSize)
        {
            if(g_poffset + IR_SEND_PACKET_LEN >= sizeof(packet))
            {
                lastlen = sizeof(packet) - g_poffset;
                if(lastlen)
                {
                    memcpy(buff, packet + g_poffset, lastlen);
                }
                DataManagerLoad(packet, HAL_DEVICE_DATA_PACKET_LENGTH);
                memcpy(buff + lastlen, packet, IR_SEND_PACKET_LEN - lastlen);
                g_poffset = IR_SEND_PACKET_LEN - lastlen;
            }
            else
            {
                memcpy(buff, packet + g_poffset, IR_SEND_PACKET_LEN);
                g_poffset += IR_SEND_PACKET_LEN;
            }

            irLowSend(IR_KEY_READDATA_DATA, g_packetid++, buff);
            g_transferSize += IR_SEND_PACKET_LEN;
            g_dtEventHandle(IR_DTRANS_EVENT_PROCESS, (uint32_t)dataTransferPercent());
        }
        else
        {
            g_dtEventHandle(IR_DTRANS_EVENT_RESULT, 1);
        }
    }
}

bool IRDataTransferIsStart(void)
{
    return g_transferStart;
}

void IRDataTransferStop(void)
{
    g_transferStart = false;
    memset(&g_irTransData, 0, sizeof(IRDataTransferData_t));
}

bool IRDataTransferStart(void)
{   
    if(!g_transferStart)
    {
        g_totalSize = DataManagerGetStoragedLength();
        g_transferSize = 0;
        g_transferStart = true;
        g_packetid = 0;
        g_poffset = HAL_DEVICE_DATA_PACKET_LENGTH;
        irLowSend(IR_KEY_READDATA, HAL_DEVICE_TYPE, (uint8_t *)&g_totalSize);
        return true;
    }
    return false;
}

void IRDataTransferInit(IRDataTransferEventHandle_t eventHandle)
{
    memset(&g_irTransData, 0, sizeof(IRDataTransferData_t));
    g_dtEventHandle = eventHandle;
}

void IRDataTransferPoll(void)
{

    if(g_transferStart)
    {
        irSendHandle();
    }
}

