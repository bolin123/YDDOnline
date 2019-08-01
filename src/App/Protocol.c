#include "Protocol.h"
#include "Sys.h"

#define PROTOCOL_DATA_PACKET_LENTH 32
#define PROTOCOL_YEAR_START 2000
#define PROTOCOL_PREAMBLE_CODE 0x02
#define PROTOCOL_DATA_REPORT_DELAY_MS 50

/*34：清空数据， 31：查询数据， 33：开始读取*/
#define PROTOCOL_DATA_CLEAR 0x34
#define PROTOCOL_DATA_QUERY 0x31
#define PROTOCOL_DATA_READ  0x33

#pragma pack(1)
typedef struct
{
    uint8_t head;
    uint16_t length;
    uint8_t cmd;
    uint8_t check;
}Protocol_t;

typedef struct
{
    uint8_t head;
    uint8_t len[3];
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t reserve;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint32_t dLen;
    uint8_t check;
}ProtoQueryAck_t;

typedef struct
{
    uint8_t head;
    uint8_t len[3];    //长度，全0xff
    uint16_t pid;      //包序号
    /*
    struct{
        uint8_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
    }collectionDate;
    */
    struct
    {
        uint8_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
    }date;
    uint16_t threshold; //阈值
    uint16_t duration;  //采集时长
    //uint8_t data[16];
}ProtoDataHead_t;
#pragma pack()

typedef struct
{
    uint16_t id;    //当前发送包序号
    uint16_t count; //总包数
    uint16_t size;  //当前发送的字节数
    uint16_t total; //当前包数据大小
    uint32_t address; //flash起始地址
    uint32_t lastTransTime;
}ProtocolDataTransfer_t;

static bool g_dataTransferStart = false;
static ProtocolDataTransfer_t g_currentTransInfo;

static void protocolDataSend(uint8_t *data, uint16_t len)
{
    HalUartWrite(HAL_UART_PC_PORT, data, len);
}

static void protocolQueryAck(uint32_t size)
{
    ProtoQueryAck_t queryAck;    
    SysDateTime_t *date = SysDateTime();
    
    queryAck.head = PROTOCOL_PREAMBLE_CODE;
    queryAck.len[0] = sizeof(ProtoQueryAck_t);
    queryAck.len[1] = 0;
    queryAck.len[2] = 0;
    queryAck.year   = date->year - PROTOCOL_YEAR_START;
    queryAck.month  = date->month;
    queryAck.day    = date->day;
    queryAck.hour   = date->hour;
    queryAck.minute = date->minute;
    queryAck.second = date->second;
    queryAck.dLen = size;
    queryAck.reserve = 0;
    queryAck.check = 0xff;
    protocolDataSend((uint8_t *)&queryAck, sizeof(ProtoQueryAck_t));
}

static void protocolParse(Protocol_t *proto)
{
    SysDataRecord_t record;
    uint8_t eraseDone[5] = {0x02, 0x05, 0x00, 0x34, 0x33};

    
    if(proto->cmd == PROTOCOL_DATA_CLEAR)
    {
        SysArgsClear();
        protocolDataSend(eraseDone, sizeof(eraseDone));
    }else if(proto->cmd == PROTOCOL_DATA_QUERY)
    {
        SysArgsGetRecord(&record);
        protocolQueryAck(record.size + record.num * sizeof(ProtoDataHead_t));
        
    }else if(proto->cmd == PROTOCOL_DATA_READ)
    {
        SysArgsGetRecord(&record);
        if(record.num > 0)
        {
            g_dataTransferStart = true;
            memset(&g_currentTransInfo, 0, sizeof(ProtocolDataTransfer_t));
            g_currentTransInfo.count = record.num;
        }
    }else{
        //do nothing
    }
}

static void protocolDataRecv(uint8_t *data, uint16_t len)
{    
    static volatile uint32_t lastRecvTime = 0;
    static uint8_t recvBuff[32];
    static uint8_t recvCount = 0;
    Protocol_t *proto = (Protocol_t *)recvBuff;
    uint8_t i;

    if(SysTimeHasPast(lastRecvTime, 500))
    {
        recvCount = 0;
    }
    lastRecvTime = SysTime();
    
    for(i = 0; i < len; i++)
    {
        recvBuff[recvCount++] = data[i];
        if(recvCount == 1)
        {
            if(data[i] != PROTOCOL_PREAMBLE_CODE)
            {
                recvCount = 0;
            }
        }
        else if(recvCount == sizeof(Protocol_t))
        {
            protocolParse(proto);
            recvCount = 0;
        }
    }
    
}

static void protocolSendDataHead(uint16_t id, SysDataInfo_t *info)
{
    ProtoDataHead_t dataHead;

    if(info != NULL)
    {
        dataHead.head = PROTOCOL_PREAMBLE_CODE;
        memset(dataHead.len, 0xff, sizeof(dataHead.len));
        dataHead.pid = id;
        dataHead.date.year   = (uint8_t)(info->date.year - PROTOCOL_YEAR_START);
        dataHead.date.month  = info->date.month;
        dataHead.date.day    = info->date.day;
        dataHead.date.hour   = info->date.hour;
        dataHead.date.minute = info->date.minute;
        dataHead.date.second = info->date.second;
        dataHead.threshold = info->threshold;
        dataHead.duration = info->times;
        protocolDataSend((uint8_t *)&dataHead, sizeof(ProtoDataHead_t));
    }
}

static void protocolDataTransfer(void)
{
    uint8_t buff[PROTOCOL_DATA_PACKET_LENTH];
    SysDataInfo_t dataInfo;
    ProtocolDataTransfer_t *currentTranfer = &g_currentTransInfo;
    
    if(g_dataTransferStart)
    {
        if(currentTranfer->size >= currentTranfer->total) //do next
        {
            if(currentTranfer->id < currentTranfer->count)
            {
                currentTranfer->id++;
                currentTranfer->size = 0;
                SysArgsGetPointInfo(currentTranfer->id, &dataInfo);
                currentTranfer->total = dataInfo.size;
                currentTranfer->address = dataInfo.startAddr;
                // transfer head
                protocolSendDataHead(currentTranfer->id, &dataInfo);
            }
            else
            {
                g_dataTransferStart = false;
            }
        }
        else
        {
            if(SysTimeHasPast(currentTranfer->lastTransTime, PROTOCOL_DATA_REPORT_DELAY_MS))
            {
                if(currentTranfer->total - currentTranfer->size >= PROTOCOL_DATA_PACKET_LENTH)
                {
                    SysRawDataRead(currentTranfer->address + currentTranfer->size, buff, PROTOCOL_DATA_PACKET_LENTH);
                    currentTranfer->size += PROTOCOL_DATA_PACKET_LENTH;
                    //transfer data
                    protocolDataSend(buff, PROTOCOL_DATA_PACKET_LENTH);
                    currentTranfer->lastTransTime = SysTime();
                }
                else
                {
                    uint16_t lastDatalen = currentTranfer->total - currentTranfer->size;
                    if(lastDatalen)
                    {
                        SysRawDataRead(currentTranfer->address + currentTranfer->size, buff, lastDatalen);
                        currentTranfer->size += lastDatalen;
                        //transfer data
                        protocolDataSend(buff, lastDatalen);
                        currentTranfer->lastTransTime = SysTime();
                    }                
                }
            }
        }
    }
}

static void protocolUartInit(void)
{
    HalUartConfig_t config;
    
    config.baudrate = 9600;
    config.flowControl = 0;
    config.parity = 0;
    config.wordLength = USART_WordLength_8b;
    config.recvCb = protocolDataRecv;
    HalUartConfig(HAL_UART_PC_PORT, &config);
}

void ProtocolInitialize(void)
{
    protocolUartInit();
}

void ProtocolPoll(void)
{
    protocolDataTransfer();
}

