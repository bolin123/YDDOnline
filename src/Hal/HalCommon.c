#include "HalCommon.h"
#include "stm32f10x_pwr.h"
#include "W25Q64.h"

static volatile uint32_t g_sysTimeCount = 0;
uint8_t g_commonBuff[4096];

static void halIOUartSendbyte(uint8_t val);

//redirect "printf()"
int fputc(int ch, FILE *f)
{
	halIOUartSendbyte((uint8_t )ch);
	return ch;
}

static void halIOUartSendbyte(uint8_t val)
{
    uint16_t delay = 95;
    HalGPIOSetLevel(HAL_IO_UART_PIN, 0);
    HalWaitUs(delay); //8.6us,115200bps

    for (int i = 0; i < 8; i++)
    {
        HalGPIOSetLevel(HAL_IO_UART_PIN, (val >> i) & 0x01);
        HalWaitUs(delay);
        //val >>= 1;
    }
    HalGPIOSetLevel(HAL_IO_UART_PIN, 1);
    HalWaitUs(delay);
}

void HalTimerPast1ms(void)
{
    g_sysTimeCount++;
}

uint32_t HalGetSysTimeCount(void)
{
    return g_sysTimeCount;
}

void HalInterruptSet(bool enable)
{
    if(enable)
    {
        __enable_irq();
    }
    else
    {
        __disable_irq();
    }
}

static void periphClockInit(bool enable)
{
    FunctionalState state = enable ? ENABLE : DISABLE;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, state);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, state);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, state);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, state);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, state);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, state);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, state);
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, state);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, state);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, state);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, state);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, state);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, state);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, state);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, state);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, state);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC , state);
}

void HalCommonWakeup(void)
{
    SystemInit();
    periphClockInit(true);
    HalTimerStart();
    HalExtiWakeupSet(false);
    HalExtiFreqStart();
    W25Q64Wakeup();
}

void HalCommonFallasleep(void)
{
    W25Q64PowerDown();
    HalExtiFreqStop();
    HalTimerStop();
    HalExtiWakeupSet(true);
#if 1
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    /*
    PWR->CR |= PWR_CR_CWUF;
    #if defined ( __CC_ARM )
        __force_stores();
    #endif
    __WFI();
    */
#endif
}

void HalCommonReboot(void)
{
    __set_FAULTMASK(1); //÷’÷πÀ˘”–÷–∂œ
    NVIC_SystemReset();
}

static void halInit(void)
{
    HalGPIOConfig(HAL_IO_UART_PIN, HAL_IO_OUTPUT);    
    HalTimerStart();
    HalExtiFreqStart(); 
}

uint16_t HalCommonInitialize(void)
{
    uint16_t err;
    SystemInit();
    periphClockInit(true);
    HalGPIOInitialize();
    HalUartInitialize();
    HalTimerInitialize();
    HalSPIInitialize();
    HalADCInitialize();
    HalDACInitialize();
    HalExtiInitialize();
    HalPWMInit();
    err = HalRTCInit();
    halInit();
    return err;
}

void HalCommonPoll(void)
{
    HalUartPoll();
    HalGPIOPoll();
    HalTimerPoll();
    HalSPIPoll();
}

