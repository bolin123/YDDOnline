#include "HalDAC.h"

void HalDACEnable(bool enable)
{
    if(enable)
    {   
        DAC_Cmd(DAC_Channel_1, ENABLE);
    }
    else
    {
        DAC_SoftwareTriggerCmd(DAC_Channel_1, DISABLE);
        DAC_Cmd(DAC_Channel_1, DISABLE);
    }
}

void HalDACSetValue(uint16_t value)
{
    DAC_SetChannel1Data(DAC_Align_12b_R, value);     //1892 base = 1.515v, 62 = 0.05v
    DAC_SoftwareTriggerCmd(DAC_Channel_1, ENABLE);   //软件触发,DA值更新
}

void HalDACInitialize(void)
{
    DAC_InitTypeDef	DAC_InitStructure; 
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    DAC_InitStructure.DAC_Trigger = DAC_Trigger_Software; 	         //软件触发,不使用定时器 或者外部中断等触发
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;   //不产生三角波 或者 噪声波
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable; 	  //提高驱动能力可以打开缓冲
    DAC_Init(DAC_Channel_1, &DAC_InitStructure); 

    DAC_DMACmd(DAC_Channel_1, DISABLE);		//不使用DMA
    DAC_Cmd(DAC_Channel_1, ENABLE);

    //HalDACSetValue(HAL_DAC_BASE_VALUE);
}

void HalDACPoll(void)
{
}



