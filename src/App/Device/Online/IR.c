#include "IR.h"
#include "Sys.h"
#include "PowerManager.h"

#define IR_DATA_FACTORY_CODE 0x80bf

#pragma pack(1)
typedef struct
{
    uint8_t complementCode;
    uint8_t rawCode;
    uint16_t factory;
}IrRxData_t;
#pragma pack()

#define IR_TX_EN_ENABLE_LEVEL 1

static volatile uint32_t g_irTimerCount = 0;
static volatile uint8_t g_irKey = 0;
static IRKeyHandle_t g_keyEventHandle;

void IRRecvTimerHandle(void)
{
    g_irTimerCount++;
}

static void resetTimerCount(void)
{
    g_irTimerCount = 0;
}

static uint8_t getKeyValue(uint32_t value)
{
    IrRxData_t *irData = (IrRxData_t *)&value;
   
    if(irData->factory == IR_DATA_FACTORY_CODE && (uint8_t)~irData->complementCode == irData->rawCode)
    {
        //printf("value = %02x\n", irData->rawCode);
        return irData->rawCode;
    }

    return 0;
}

void IRRecvBits(void)
{
    static uint8_t recvnum = 0;
    static uint32_t lastTime;
    static uint32_t irValue = 0;
    uint32_t bit;
//    uint8_t buff[4];

    if(SysTimeHasPast(lastTime, 150))
    {
        recvnum = 0;
    }
    lastTime = SysTime();

    recvnum++;
    if(recvnum == 1)
    {
        resetTimerCount();
        irValue = 0;
        //HalIRRecvTimerEnable(true);
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

        //g_irValue |= bit << (recvnum - 2); 
        irValue = (irValue << 1) + bit;
        
        if(recvnum == 34)
        {
            //printf("ir value:%04x\n", g_irValue);
            g_irKey = getKeyValue(irValue);
        }
    }
}

static void irKeyHandle(void)
{
    IRKey_t value = IR_KEY_INVALID;
    if(g_irKey)
    {
        switch (g_irKey)
        {
        case 0x6b://menu
            value = IR_KEY_MENU;
            break;
        case 0x53://up
            value = IR_KEY_UP;
            break;
        case 0x4b://down
            value = IR_KEY_DOWN;
            break;
        case 0x99://left
            value = IR_KEY_LEFT;
            break;
        case 0x83://right
            value = IR_KEY_RIGHT;
            break;
        case 0x73://enter
            value = IR_KEY_ENTER;
            break;
        case 0xa3://cancel
            value = IR_KEY_CANCEL;
            break;
        default:
            break;
        }
        g_keyEventHandle(value);
        g_irKey = 0;
    }
}

void IRSendData(uint8_t *data, uint8_t len)
{
    uint8_t i, j;
    
HalInterruptSet(false);
    HalPWMEnable(true);
    HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 0);
    HalWaitMs(9);
    HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 1);
    //HalPWMEnable(false);
    HalWaitUs(4500);

    for(j = 0; j < len; j++)
    {
        for(i = 0; i < 8; i++)
        {
            if(data[j] & (0x80 >> i))
            {
                HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 0);
                //HalPWMEnable(true);
                HalWaitUs(560);
                
                HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 1);
                //HalPWMEnable(false);
                HalWaitUs(1690);
            }
            else
            {
                HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 0);
                //HalPWMEnable(true);
                HalWaitUs(560);
                
                HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 1);
                //HalPWMEnable(false);
                HalWaitUs(560);
            }
        }
    }
    HalGPIOSetLevel(HAL_IR_TX_EN_PIN, 0);
    HalWaitUs(560);
HalInterruptSet(true);
    HalPWMEnable(false);
    //HalGPIOSetLevel(IR_TX_EN_PIN, 0);
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
    irKeyHandle();
}

