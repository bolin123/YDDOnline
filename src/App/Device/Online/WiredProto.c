#include "WiredProto.h"
#include "Sys.h"

#define WIRED_PROTO_UART_PORT HAL_UART_PORT_1
#define WIRED_PROTO_CONTEXT_HEAD 0xf5
#define WIRED_PROTO_CMD_QUERY 0x01

#define WIRED_PROTO_485DE_PIN 0x0b //pa11
#define WIRED_PROTO_RECV_MODE() HalGPIOSetLevel(WIRED_PROTO_485DE_PIN, 0)
#define WIRED_PROTO_SEND_MODE() HalGPIOSetLevel(WIRED_PROTO_485DE_PIN, 1)

typedef struct
{
    uint8_t head;
    uint8_t addr;
    uint8_t cmd;
    uint8_t dlen;
    uint8_t data[];
}WiredProtoContext_t;

typedef struct
{
    uint8_t address;
    uint8_t devType;
    uint8_t errcode;
    uint8_t power;
    uint8_t data[];
}WiredProtoReportData_t;

static uint8_t g_wiredBuff[128];
static volatile bool g_wiredFrameGot = false;
static WiredProtoQuery_cb g_queryHandle;

static uint8_t checkSum(uint8_t *data, uint8_t len)
{
    uint8_t i;
    uint8_t sum = 0;

    for(i = 0; i < len; i++)
    {
        sum += data[i];
    }
    return sum;
}

static void wiredDataRecv(uint8_t *data, uint16_t len)
{
    uint16_t i;
    static uint32_t lastTime;
    static uint8_t wiredBuffCount = 0;
    static uint8_t dataLength = 0;

    if(g_wiredFrameGot)
    {
        return;
    }

    for (i = 0; i < len; i++)
    {
        if(SysTimeHasPast(lastTime, 200))
        {
            wiredBuffCount = 0;
        }
        lastTime = SysTime();

        g_wiredBuff[wiredBuffCount++] = data[i];

        if(wiredBuffCount == 1)
        {
            if(data[i] != WIRED_PROTO_CONTEXT_HEAD)
            {
                wiredBuffCount = 0;
            }
        }
        else if(wiredBuffCount == 2)
        {
            if(data[i] != SysRfAddressGet() && data[i] != 0xff)
            {
                wiredBuffCount = 0;
            }
        }
        else if(wiredBuffCount == 4)
        {
            dataLength = data[i];
        }
        else if(sizeof(WiredProtoContext_t) + dataLength + 1 == wiredBuffCount)
        {
            WiredProtoContext_t *context = (WiredProtoContext_t *)g_wiredBuff;
            if(checkSum(context->data, context->dlen) == g_wiredBuff[sizeof(WiredProtoContext_t) + dataLength])
            {
                g_wiredFrameGot = true;
            }
            dataLength = 0;
            wiredBuffCount = 0;
        }
    }
    
}

static void wiredFramePrase(void)
{
    WiredProtoContext_t *context;
    
    if(g_wiredFrameGot)
    {
        context = (WiredProtoContext_t *)g_wiredBuff;
        if(context->cmd == WIRED_PROTO_CMD_QUERY)
        {
            g_queryHandle();
        }
        g_wiredFrameGot = false;
    }
}

void WiredProtoReportData(uint8_t err, uint8_t power, uint16_t *data, uint16_t dataNum)
{
    uint8_t i = 0;
    uint8_t buff[128];
    WiredProtoContext_t *context = (WiredProtoContext_t *)buff;
    uint8_t dlen = dataNum * sizeof(uint16_t);

    Syslog("");
    
    WIRED_PROTO_SEND_MODE();
    buff[i++] = WIRED_PROTO_CONTEXT_HEAD;
    buff[i++] = 0; //master addr
    buff[i++] = WIRED_PROTO_CMD_QUERY;
    buff[i++] = dlen + sizeof(WiredProtoReportData_t); //length
    buff[i++] = SysRfAddressGet();
    buff[i++] = SysDeviceTypeGet();
    buff[i++] = err;
    buff[i++] = power;
    memcpy(&buff[i], data, dlen);
    i += dlen;
    buff[i++] = checkSum(context->data, dlen + sizeof(WiredProtoReportData_t));
    HalUartWrite(WIRED_PROTO_UART_PORT, buff, i);
    WIRED_PROTO_RECV_MODE();
}

void WiredProtoInit(WiredProtoQuery_cb queryCallback)
{
    HalUartConfig_t config;
    
    g_queryHandle = queryCallback;

    config.enble = true;
    config.baudrate = 9600;
    config.flowControl = 0;
    config.parity = 0;
    config.wordLength = USART_WordLength_8b;
    config.recvCb = wiredDataRecv;
    HalUartConfig(WIRED_PROTO_UART_PORT, &config);

    //485 poweron
    HalGPIOConfig(HAL_485_POWER_PIN, HAL_IO_OUTPUT);//pa12
    HalGPIOSetLevel(HAL_485_POWER_PIN, HAL_485_POWER_ENABLE_LEVEL);

    //485 recv mode default
    HalGPIOConfig(WIRED_PROTO_485DE_PIN, HAL_IO_OUTPUT);//pa11
    WIRED_PROTO_RECV_MODE();
}

void WiredProtoPoll(void)
{
    wiredFramePrase();
}

