#include "HalADC.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

typedef struct
{
    uint16_t ioPin;
    GPIO_TypeDef *ioGroup;
    uint8_t adChnl;
    uint8_t adRank;
    uint8_t adSampleTime;
}HalADCConfigs_t;

#define HAL_ADC_CHANNEL_COUNT (HAL_SENSOR_ID_COUNT + 1) // 4 capture + 1 voltage
#define HAL_ADC_CH_BUFF_LEN 12
#define ADC1_DR_ADDRESS ((uint32_t)0x4001244C)

static HalADCConfigs_t ADCConfigs[HAL_ADC_CHANNEL_COUNT] = {
#if defined(HAL_OLD_DEVICE)
    /*OUT1*/
    {
        .ioPin   = GPIO_Pin_1,
        .ioGroup = GPIOA,
        .adChnl  = ADC_Channel_1,
        .adRank  = 1,
        .adSampleTime = ADC_SampleTime_1Cycles5,
    },
    /*OUT2*/
    {
        .ioPin   = GPIO_Pin_7,
        .ioGroup = GPIOA,
        .adChnl  = ADC_Channel_7,
        .adRank  = 2,
        .adSampleTime = ADC_SampleTime_1Cycles5,
    },
    /*OUT3*/
    {
        .ioPin   = GPIO_Pin_4,
        .ioGroup = GPIOC,
        .adChnl  = ADC_Channel_14,
        .adRank  = 3,
        .adSampleTime = ADC_SampleTime_71Cycles5,
    },
    /*OUT4*/
    {
        .ioPin   = GPIO_Pin_5,
        .ioGroup = GPIOC,
        .adChnl  = ADC_Channel_15,
        .adRank  = 4,
        .adSampleTime = ADC_SampleTime_71Cycles5,
    },
    /*power*/
    {
        .ioPin   = GPIO_Pin_0,
        .ioGroup = GPIOC,
        .adChnl  = ADC_Channel_10,
        .adRank  = 5,
        .adSampleTime = ADC_SampleTime_71Cycles5,
    }
#else
    /*OUT1*/
    {
        .ioPin   = GPIO_Pin_6,
        .ioGroup = GPIOA,
        .adChnl  = ADC_Channel_6,
        .adRank  = 1,
        .adSampleTime = ADC_SampleTime_1Cycles5,
    },
    /*OUT2*/
    {
        .ioPin   = GPIO_Pin_7,
        .ioGroup = GPIOA,
        .adChnl  = ADC_Channel_7,
        .adRank  = 2,
        .adSampleTime = ADC_SampleTime_1Cycles5,
    },
    /*OUT3*/
    {
        .ioPin   = GPIO_Pin_4,
        .ioGroup = GPIOC,
        .adChnl  = ADC_Channel_14,
        .adRank  = 3,
        .adSampleTime = ADC_SampleTime_71Cycles5,
    },
    /*OUT4*/
    {
        .ioPin   = GPIO_Pin_5,
        .ioGroup = GPIOC,
        .adChnl  = ADC_Channel_15,
        .adRank  = 4,
        .adSampleTime = ADC_SampleTime_71Cycles5,
    },
    /*power*/
    {
        .ioPin   = GPIO_Pin_1,
        .ioGroup = GPIOA,
        .adChnl  = ADC_Channel_1,
        .adRank  = 5,
        .adSampleTime = ADC_SampleTime_71Cycles5,
    }
#endif
}; 

static volatile uint16_t g_adcValue[HAL_ADC_CHANNEL_COUNT];
static volatile uint16_t g_maxValue[2];

uint16_t HalADCGetCollectValue(uint8_t ch)
{
    if(ch < 2)
    {
        return g_maxValue[ch];
    }
    else
    {
        return g_adcValue[ch];
    }
}

void HalADCMaxValueClear(uint8_t ch)
{
    g_maxValue[ch] = 0;
}

uint16_t HalADCGetPowerValue(void)
{
    return g_adcValue[4];
}

void HalADCStop(void)
{   
    ADC_SoftwareStartConvCmd(ADC1, DISABLE);
    ADC_DMACmd(ADC1, DISABLE);
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, DISABLE);
    DMA_Cmd(DMA1_Channel1, DISABLE);
    ADC_Cmd(ADC1, DISABLE);
}

void HalADCStart(void)
{
    HalADCInitialize();
}


void HalADCInitialize(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
#if 0
    /*
    BAT_DET, PA1 --- ADC123_IN1
    OUT1,    PA6 --- ADC12_IN6
    OUT2,    PA7 --- ADC12_IN7
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_6 | GPIO_Pin_7; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*
    OUT3, PC4 --- ADC12_IN14
    OUT4, PC5 --- ADC12_IN15
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif
    uint8_t i;
    
    for(i = 0; i < HAL_ADC_CHANNEL_COUNT; i++)
    {
        GPIO_InitStructure.GPIO_Pin = ADCConfigs[i].ioPin; 
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_Init(ADCConfigs[i].ioGroup, &GPIO_InitStructure);
    }

    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    memset((void *)g_maxValue, 0, sizeof(g_maxValue));
    RCC_ADCCLKConfig(RCC_PCLK2_Div2);

    NVIC_InitTypeDef NVIC_InitStructure;
    
    //设置中断
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    //DMA1通道1配置
    DMA_DeInit(DMA1_Channel1);
    //外设地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_ADDRESS;
    //内存地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&g_adcValue;
    //dma传输方向单向
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    //设置DMA在传输时缓冲区的长度
    //DMA_InitStructure.DMA_BufferSize = HAL_ADC_CH_BUFF_LEN * HAL_ADC_CHANNEL_COUNT;
    DMA_InitStructure.DMA_BufferSize = HAL_ADC_CHANNEL_COUNT;
    //设置DMA的外设递增模式，一个外设
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    //设置DMA的内存递增模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    //外设数据字长
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    //内存数据字长
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    //设置DMA的传输模式：连续不断的循环模式
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    //设置DMA的优先级别
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    //设置DMA的2个memory中的变量互相访问
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

    //使能通道1
    DMA_Cmd(DMA1_Channel1, ENABLE);

    //ADC1配置
    //独立工作模式
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    //扫描方式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    //连续转换
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    //外部触发禁止
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    //数据右对齐
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    //用于转换的通道数
    ADC_InitStructure.ADC_NbrOfChannel = HAL_ADC_CHANNEL_COUNT;
    ADC_Init(ADC1, &ADC_InitStructure);

    //规则模式通道配置
    for(i = 0; i < HAL_ADC_CHANNEL_COUNT; i++)
    {
        ADC_RegularChannelConfig(ADC1, ADCConfigs[i].adChnl,  ADCConfigs[i].adRank, ADCConfigs[i].adSampleTime);
    }
    /*
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6,  1, ADC_SampleTime_1Cycles5);  //PA6
    ADC_RegularChannelConfig(ADC1, ADC_Channel_7,  2, ADC_SampleTime_1Cycles5);  //PA7 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 3, ADC_SampleTime_71Cycles5); //PC4 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 4, ADC_SampleTime_71Cycles5); //PC5
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1,  5, ADC_SampleTime_71Cycles5); //PA1 
    */
    //使能ADC1的DMA
    ADC_DMACmd(ADC1, ENABLE);

    //使能ADC1
    ADC_Cmd(ADC1, ENABLE);

    //使能ADC1复位校准寄存器  
    ADC_ResetCalibration(ADC1);
    //检查校准寄存器是否复位完毕
    while(ADC_GetResetCalibrationStatus(ADC1));

    //开始校准
    ADC_StartCalibration(ADC1);
    //检测是否校准完毕
    while(ADC_GetCalibrationStatus(ADC1));
     
    //开启ADC1的软件转换
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void DMA1_Channel1_IRQHandler(void)
{
    uint8_t i;
    //static uint16_t max = 0;
    if(DMA_GetFlagStatus(DMA1_FLAG_TC1))
    {
        DMA_ClearITPendingBit(DMA1_FLAG_TC1);
        for(i = 0; i < 2; i++)
        {
            if(g_maxValue[i] < g_adcValue[i]) //取最大值
            {
                g_maxValue[i] = g_adcValue[i];
            }
        }
    }
}


