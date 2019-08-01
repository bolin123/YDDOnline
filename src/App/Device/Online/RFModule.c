#include "RFModule.h"
#include "PowerManager.h"

#define RFMODULE_UART_PORT HAL_UART_PORT_3 //uart3

static bool g_rfModuleDetected = false;
static uint8_t g_rfRecvBuff[256];
static uint16_t g_rfBuffCount = 0;

static void rfDataParse(char *cmd)
{
    Syslog("%s", cmd);
}

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
            rfDataParse((char *)g_rfRecvBuff);
            g_rfBuffCount = 0;
        }

    }
}

static void atcmdParse(char *atcmd)
{
    uint8_t i;
    uint8_t mac[SYS_MAC_ADDR_LEN];
    char *macstr = &atcmd[72];
    char tmp[3] = {0};

    Syslog("%s", atcmd);

    //memcpy(macstr, &atcmd[72], 12);
    
    //sscanf(macstr, "%02x%02x%02x%02x%02x%02x", &mac[0],  &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    for(i = 0; i < SYS_MAC_ADDR_LEN; i++)
    {
        memcpy(tmp, macstr + i * 2, 2);
        mac[i] = (uint8_t)strtol(tmp, NULL, 16);
    }
    
    SysMacAddrSet(mac);
    g_rfModuleDetected = true;
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
    else if(g_rfBuffCount == 3)
    {
        if(byte != 'C')
        {
            g_rfBuffCount = 0;
        }
    }
    else if(g_rfBuffCount == 4)
    {
        if(byte != 'i')
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
        }
    }
}

static void uartDataRecv(uint8_t *data, uint16_t len)
{
    uint8_t i;
    static uint32_t recvTime;

    for(i = 0; i < len; i++)
    {
        if(SysTimeHasPast(recvTime, 200))
        {
            g_rfBuffCount = 0; 
        }
        recvTime = SysTime();
        g_rfRecvBuff[g_rfBuffCount++] = data[i];
        if(g_rfModuleDetected)
        {
            dataRecvByte(data[i]);
        }
        else
        {
            atcmdRecvByte(data[i]);
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

static void moduleDetect(void)
{
    char *getId = "ATCi\r";
    static uint32_t getidTime;
    if(!g_rfModuleDetected && SysTimeHasPast(getidTime, 500))
    {
        Syslog("");
        HalUartWrite(RFMODULE_UART_PORT, (const uint8_t *)getId, strlen(getId));
        getidTime = SysTime();
    }
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
/*

void RFModuleSleep(void)
{
    uartDeinit();

}

void RFModuleWakeup(void)
{
    uartInit();
}
*/

static void rfModuleSleep(PM_t *pm)
{
    if(pm)
    {
        uartDeinit();
        //HalExtiWakeupSet(true);
        pm->status = PM_STATUS_SLEEP;
    }
}

static void rfModuleWakeup(PM_t *pm)
{
    if(pm)
    {
        uartInit();
        //HalExtiWakeupSet(false);
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

void RFModuleInit(void)
{   
    uartInit();
    HalExtiWakeupSet(true);
    rfModulePowerInit();
}

void RFModulePoll(void)
{
    moduleDetect();
}

