#include "RFModule.h"
#include "Wireless.h"
#include "PowerManager.h"
#include "VTList.h"
#include "VTStaticQueue.h"

#define RFMODULE_UART_PORT HAL_UART_PORT_3 //uart3
#define RFMODULE_REPLY_TIMEOUT 300
#define RFMODULE_RETRY_MAX_NUM 5

typedef enum
{
    RFMODULE_CMD_GET_RFCHN = 0,
    RFMODULE_CMD_SET_RFCHN,
    RFMODULE_CMD_GET_WAITTIME,
    RFMODULE_CMD_SET_WAITTIME,
}RFModuleCmd_t;

typedef struct RFSendList_st
{
    uint8_t retries;
    RFModuleCmd_t cmd;
    char *data;
    uint32_t lastTime;
    VTLIST_ENTRY(struct RFSendList_st);
}RFModuleSendList_t;

static const char *g_chnlist[HAL_RF_CHANNEL_NUM] = {
                    "ATCM002BC88E10\r",  //430.5
                    "ATCM002B9E9810\r",  //431.5
                    "ATCM002B77A210\r",  //432.5
                    "ATCM002B50AC10\r",  //433.5
                    "ATCM002B29B610\r",  //434.5
                    "ATCM002B00C010\r",  //435.5
                    "ATCM002BD9C910\r",  //436.5
                    "ATCM002BB2D310\r",  //437.5
                    "ATCM002B8BDD10\r",  //438.5
                    "ATCM002B64E710\r",  //439.5
                    "ATCM002B3DF110\r",  //440.5
                    "ATCM002B16FB10\r",  //441.5
                    "ATCM002BEF0411\r",  //442.5
                    "ATCM002BC80E11\r",  //443.5
                    "ATCM002BA11811\r",  //444.5
                    "ATCM002B762211\r",  //445.5
                    "ATCM002BB35311\r",  //450.5
                    "ATCM002B8C5D11\r",  //451.5
                    "ATCM002B656711\r",  //452.5
                    "ATCM002B3E7111\r",  //453.5
                    "ATCM002B177B11\r",  //454.5
                    "ATCM002BF08411\r",  //455.5
                    "ATCM002B2BB611\r",  //460.5
                    "ATCM002B66E711\r",  //465.5
                    "ATCM002BA11812\r",  //470.5
                    "ATCM002BDCC90F\r",  //410.5
                    "ATCM002B17FB0F\r",  //415.5
                    "ATCM002B522C10\r",  //420.5
                    "ATCM002B8D5D10\r",  //425.5
};


static bool g_rfModuleDetected = false;
static uint8_t g_rfRecvBuff[256];
static uint16_t g_rfBuffCount = 0;
static VTSQueueDef(uint8_t, g_uartBuff, 512);
static RFModuleSendList_t g_rfmoduleList;
static RFModuleEvent_cb g_rfModuleEventHandle;

static void dataRecvByte(uint8_t byte)
{   
    if(g_rfBuffCount == 1)
    {
        if(byte != '~')
        {
            g_rfBuffCount = 0;
        }
    }
    else
    {
        if(g_rfBuffCount >= sizeof(g_rfRecvBuff))
        {
            g_rfBuffCount = 0;
        }
        
        if(byte == '\r')
        {
            g_rfRecvBuff[g_rfBuffCount] = '\0';
            WirelessDataParse((char *)g_rfRecvBuff);
            g_rfBuffCount = 0;
        }

    }
}

static int sendlistInsert(char *data, RFModuleCmd_t cmd)
{
    RFModuleSendList_t *rfsend = (RFModuleSendList_t *)malloc(sizeof(RFModuleSendList_t));

    if(rfsend)
    {
        rfsend->data = (char *)malloc(strlen(data) + 1);
        if(rfsend->data)
        {
            memset(rfsend->data, 0, strlen(data) + 1);
            strcpy(rfsend->data, data);
            rfsend->cmd = cmd;
            rfsend->lastTime = 0;
            rfsend->retries = 0;
            VTListAdd(&g_rfmoduleList, rfsend);
            return 0;
        }
        else
        {
            free(rfsend);
        }
    }
    return -1;
}

static void sendlistDel(RFModuleSendList_t *node)
{
    if(node)
    {
        VTListDel(node);
        if(node->data)
        {
            free(node->data);
        }
        free(node);
    }
}

static void rfSendlistHandle(void)
{
    RFModuleSendList_t *rfsend = VTListFirst(&g_rfmoduleList);
    if(rfsend != NULL)
    {
        if(SysTimeHasPast(rfsend->lastTime, RFMODULE_REPLY_TIMEOUT))
        {
            if(rfsend->retries < RFMODULE_RETRY_MAX_NUM)
            {
                HalUartWrite(RFMODULE_UART_PORT, (const uint8_t *)rfsend->data, strlen(rfsend->data));
                rfsend->retries++;
                rfsend->lastTime = SysTime();
            }
            else //out of range
            {
                sendlistDel(rfsend);
            }
        }
    }
}

static void atcmdParse(char *atcmd)
{
    RFModuleSendList_t *rfsend;
    char *value = &atcmd[4];
    Syslog("%s", atcmd);
    
    g_rfModuleDetected = true;

    if(strstr(atcmd, "ATCb"))
    {
        rfsend = VTListFirst(&g_rfmoduleList);
        if(rfsend)
        {
            if(rfsend->cmd == RFMODULE_CMD_GET_RFCHN)
            {
                for(uint32_t i = 0; i < HAL_RF_CHANNEL_NUM; i++)
                {
                    if(strstr(g_chnlist[i], value))
                    {
                        g_rfModuleEventHandle(RFMODULE_EVENT_GET_RFCHNL, (void *)(i + 1));
                        break;
                    }
                }
            }
            else //RFMODULE_CMD_GET_WAITTIME
            {
                //ATCbD007B80B
                uint32_t time = (uint32_t)strtoul(value, NULL, 16);
                g_rfModuleEventHandle(RFMODULE_EVENT_GET_WAITTIME, (void *)time);
            }
            
            sendlistDel(rfsend);
        }
    }
    else if(strstr(atcmd, "ATOK"))
    {
        rfsend = VTListFirst(&g_rfmoduleList);
        if(rfsend)
        {
            sendlistDel(rfsend);
        }
    }
    else
    {
        //ignore
    }
}

static void atcmdRecvByte(uint8_t byte)
{
    if(g_rfBuffCount == 1)
    {
        if(byte != 'A')
        {
            g_rfBuffCount = 0;
        }
    }
    else if(g_rfBuffCount == 2)
    {
        if(byte != 'T')
        {
            g_rfBuffCount = 0;
        }
    }
    else
    {
        if(g_rfBuffCount >= sizeof(g_rfRecvBuff))
        {
            g_rfBuffCount = 0;
        }

        if(byte == '\r')
        {
            g_rfRecvBuff[g_rfBuffCount] = '\0';
            atcmdParse((char *)g_rfRecvBuff);
            g_rfBuffCount = 0;
        }
    }
}

static void uartDataRecv(uint8_t *data, uint16_t len)
{
    uint16_t i;
    for(i = 0; i < len; i++)
    {
        if(VTSQueueHasSpace(g_uartBuff))
        {
            VTSQueuePush(g_uartBuff, data[i]);
        }
    }
}

static void frameParsePoll(void)
{
//    uint8_t i;
    uint8_t data;
    static uint32_t recvTime;
    static bool atcmdRecved = false;
    
    while(VTSQueueCount(g_uartBuff))
    {
        HalInterruptSet(false);
        data = VTSQueueFront(g_uartBuff);
        HalInterruptSet(true);
        VTSQueuePop(g_uartBuff);
        if(SysTimeHasPast(recvTime, 100))
        {
            g_rfBuffCount = 0;
        }
        recvTime = SysTime();

        g_rfRecvBuff[g_rfBuffCount++] = data;

        if(g_rfBuffCount == 1)
        {
            if(data == '~')
            {
                atcmdRecved = false;
            }
            else
            {
                atcmdRecved = true;
            }
        }
        
        if(atcmdRecved)
        {
            atcmdRecvByte(data);
        }
        else
        {
            dataRecvByte(data);
        }
    }
}

static void uartInit(void)
{
    HalUartConfig_t config;

    config.enble = true;
    config.baudrate = 115200;
    config.flowControl = 0;
    config.parity = 0;
    config.wordLength = USART_WordLength_8b;
    config.recvCb = uartDataRecv;
    HalUartConfig(RFMODULE_UART_PORT, &config);
}

static void uartDeinit(void)
{
    HalUartConfig_t config;

    config.enble = false;
    config.recvCb = NULL;
    HalUartConfig(RFMODULE_UART_PORT, &config);
}

void RFModuleGetChannel(void)
{
    char *getId = "ATCb002B03\r";
    sendlistInsert(getId, RFMODULE_CMD_GET_RFCHN);
}

void RFModuleGetWaitTimes(void)
{
    char *cmd = "ATCb003C04\r"; //读取开机和响应超时时间
    sendlistInsert(cmd, RFMODULE_CMD_GET_WAITTIME);    
}

void RFModuleSetWaitTimes(uint32_t time)
{
    char cmd[20] = "";
    
    Syslog("%04x", time);
    sprintf(cmd, "ATCM003C%04x\r", time);
    sendlistInsert(cmd, RFMODULE_CMD_SET_WAITTIME);
}

void RFModuleSendData(uint8_t *data, uint16_t len)
{
    if(g_rfModuleDetected)
    {
        Syslog("%s", data);
        HalUartWrite(RFMODULE_UART_PORT, (const unsigned char *)data, len);
    }
}

bool RFModuleDetected(void)
{
    return g_rfModuleDetected;
}

void RFMoudleSetChannel(uint8_t chn)
{   
    uint8_t rfchnl = chn - 1;

    Syslog("%d", chn);
    if(rfchnl < HAL_RF_CHANNEL_NUM)
    {
        sendlistInsert((char *)g_chnlist[rfchnl], RFMODULE_CMD_SET_RFCHN);
    }
}

static void rfModuleSleep(PM_t *pm)
{
    if(pm)
    {
        uartDeinit();
        pm->status = PM_STATUS_SLEEP;
    }
}

static void rfModuleWakeup(PM_t *pm, PMWakeupType_t type)
{
    if(pm && pm->status == PM_STATUS_SLEEP)
    {
        uartInit();
        pm->status = PM_STATUS_WAKEUP;
    }
}

static void rfModulePowerInit(void)
{
    PM_t pm;
    pm.sleep = rfModuleSleep;
    pm.wakeup = rfModuleWakeup;
    pm.status = PM_STATUS_WAKEUP;
    PMRegist(&pm, PM_DEVICE_ID_RFMODULE);
}

void RFModuleInit(RFModuleEvent_cb eventcb)
{   
    uartInit();
    HalExitSet(HAL_EXIT_433MODULE_WAKEUP, true);
    rfModulePowerInit();
    //rfModuleDetect();
    g_rfModuleEventHandle = eventcb;
}

void RFModulePoll(void)
{
    rfSendlistHandle();
    frameParsePoll();
}

