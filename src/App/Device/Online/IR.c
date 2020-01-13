#include "IR.h"
#include "Sys.h"
#include "PowerManager.h"

#define IR_DATA_FACTORY_CODE 'T'
#define IR_PREAMBLE_CODE_LEN 2
//#define IR_RECV_DATA_LEN (2 + IR_SEND_PACKET_LEN)

typedef struct
{
    uint8_t head;
    uint8_t cmd;
    uint8_t pid;
    uint8_t content[IR_SEND_PACKET_LEN];
}IRData_t;

#define IR_TX_EN_ENABLE_LEVEL 1

static volatile uint32_t g_irTimerCount = 0;
static IRKeyHandle_t g_keyEventHandle;
static volatile bool g_gotIRPacket = false;
static uint8_t g_recvBuff[sizeof(IRData_t)];

void IRRecvTimerHandle(void)
{
    g_irTimerCount++;
}

static void resetTimerCount(void)
{
    g_irTimerCount = 0;
}

static void irPacketHandle(void)
{
    IRData_t *recvData = (IRData_t *)g_recvBuff; 
    if(g_gotIRPacket)
    {
        if(recvData->head == IR_DATA_FACTORY_CODE)
        {
            g_keyEventHandle(recvData->cmd, recvData->content);
        }
        g_gotIRPacket = false;
    }
}

void IRRecvBits(void)
{
    static uint8_t recvnum = 0;
    static uint32_t lastTime;
    uint32_t bit;

    if(g_gotIRPacket)
    {
        return ;
    }
    
    if(SysTimeHasPast(lastTime, 150))
    {
        recvnum = 0;
    }
    lastTime = SysTime();

    recvnum++;
    if(recvnum == 1)
    {
        resetTimerCount();
        //irValue = 0;
    }
    else if(recvnum == 2)
    {
        if(g_irTimerCount < 127 || g_irTimerCount > 140) //13.5ms
        {
            recvnum = 0;
        }
        resetTimerCount();
    }
    else
    {
        if(g_irTimerCount < 14 && g_irTimerCount > 9) //1.125ms
        {
            bit = 0x0;
        }
        else if(g_irTimerCount < 25 && g_irTimerCount > 19) //2.25ms
        {
            bit = 0x1;
        }
        else
        {
            recvnum = 0;
        }
        resetTimerCount();

        g_recvBuff[(recvnum - IR_PREAMBLE_CODE_LEN - 1) / 8] = (g_recvBuff[(recvnum - IR_PREAMBLE_CODE_LEN - 1) / 8] << 1) + bit;
        
        if(recvnum == sizeof(g_recvBuff) * 8 + IR_PREAMBLE_CODE_LEN)
        {
            g_gotIRPacket = true;
            recvnum = 0;
        }
    }
}


void IRSendData(IRKey_t key, uint8_t pid, uint8_t data[IR_SEND_PACKET_LEN])
{
    uint8_t i, j;
    IRData_t irData;
    uint8_t *txData = (uint8_t *)&irData;

    irData.head = IR_DATA_FACTORY_CODE;
    irData.cmd = (uint8_t)key;
    irData.pid = pid;
    memcpy(irData.content, data, sizeof(irData.content));
    
HalInterruptSet(false);
    HalPWMEnable(true);
    HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 0);
    HalWaitMs(9);
    HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 1);
    HalWaitUs(4500);

    for(j = 0; j < sizeof(IRData_t); j++)
    {
        for(i = 0; i < 8; i++)
        {
            if(txData[j] & (0x80 >> i))
            {
                HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 0);
                HalWaitUs(560);
                
                HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 1);
                HalWaitUs(1690);
            }
            else
            {
                HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 0);
                HalWaitUs(560);
                
                HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 1);
                HalWaitUs(560);
            }
        }
    }
    HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 0);
    HalWaitUs(560);
HalInterruptSet(true);
    HalPWMEnable(false);
}


static void irPMSleep(struct PM_st *pm)
{
    if(pm)
    {
        HalExitSet(HAL_EXIT_IR_INPUT, false);
        HalIRRecvTimerEnable(false);
        HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 0);
        pm->status = PM_STATUS_SLEEP;
    }
}

static void irPMWakeup(struct PM_st *pm, PMWakeupType_t type)
{
    if(pm && pm->status == PM_STATUS_SLEEP && PM_WAKEUP_TYPE_LIGHT == type)
    {
        HalExitSet(HAL_EXIT_IR_INPUT, true);
        HalIRRecvTimerEnable(true);
        HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 1);
        pm->status = PM_STATUS_WAKEUP;
    }
}

static void irPMRegist(void)
{
    PM_t pm;

    pm.enable = true;
    pm.status = PM_STATUS_WAKEUP;
    pm.sleep = irPMSleep;
    pm.wakeup = irPMWakeup;
    PMRegist(&pm, PM_DEVICE_ID_IR);
}

void IRInit(IRKeyHandle_t handle)
{
    HalGPIOConfig(HAL_IR_TX_EN_PIN, HAL_IO_OUTPUT);
    HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 1);
    HalExitSet(HAL_EXIT_IR_INPUT, true);
    HalIRRecvTimerEnable(true);
    irPMRegist();
    g_keyEventHandle = handle;
}

void IRPoll(void)
{
    irPacketHandle();
}

