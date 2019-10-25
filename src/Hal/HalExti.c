#include "HalExti.h"
#include "PowerManager.h"

extern void YDDOnlineSensorFreqTrigger(uint8_t ch);
extern void YDDOnlineLightActive(void);

void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line5) != RESET) //433_ACT
    {
        PMWakeup(PM_WAKEUP_TYPE_WIRELESS);   
        EXTI_ClearITPendingBit(EXTI_Line5);
    }

    if(EXTI_GetITStatus(EXTI_Line6) != RESET) //WAKEUP_EN
    {
        //PMWakeup(PM_WAKEUP_TYPE_LIGHT);
        YDDOnlineLightActive();
        EXTI_ClearITPendingBit(EXTI_Line6);
    }
}

void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line10) != RESET) //F1
	{
        YDDOnlineSensorFreqTrigger(HAL_SENSOR_ID_GEOMAGNETISM);
        EXTI_ClearITPendingBit(EXTI_Line10);
    }

    if(EXTI_GetITStatus(EXTI_Line11) != RESET) //F2
    {
        YDDOnlineSensorFreqTrigger(HAL_SENSOR_ID_NOISE);
        EXTI_ClearITPendingBit(EXTI_Line11);
    }
}

extern void IRRecvBits(void);
void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET) //IR_RX_OUT
	{
	    IRRecvBits();
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void HalExtiLightEnable(bool enable)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    
    if(enable)
    {
        EXTI_InitStructure.EXTI_Line = EXTI_Line6; 
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
        EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
        EXTI_Init(&EXTI_InitStructure);
        EXTI_ClearITPendingBit(EXTI_Line6);
    }
    else
    {
        EXTI_ClearITPendingBit(EXTI_Line6);
        EXTI_InitStructure.EXTI_Line = EXTI_Line6; 
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
        EXTI_InitStructure.EXTI_LineCmd = DISABLE; 
        EXTI_Init(&EXTI_InitStructure);
    }
}

void HalExtiIRRecvEnable(bool enable)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    
    if(enable)
    {
        EXTI_InitStructure.EXTI_Line = EXTI_Line0; 
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
        EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
        EXTI_Init(&EXTI_InitStructure);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
    else
    {
        EXTI_ClearITPendingBit(EXTI_Line0);
        EXTI_InitStructure.EXTI_Line = EXTI_Line0; 
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
        EXTI_InitStructure.EXTI_LineCmd = DISABLE; 
        EXTI_Init(&EXTI_InitStructure);
    }
}


void HalExtiFreqStart(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line10 | EXTI_Line11; 
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);
    EXTI_ClearITPendingBit(EXTI_Line10 | EXTI_Line11);   
}

void HalExtiFreqStop(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line10 | EXTI_Line11; 
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_LineCmd = DISABLE; 
    EXTI_Init(&EXTI_InitStructure);
    EXTI_ClearITPendingBit(EXTI_Line10 | EXTI_Line11);

}

/*433 wakeup exti set*/
void HalExtiWakeupSet(bool enable)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    
    if(enable)
    {
        EXTI_InitStructure.EXTI_Line = EXTI_Line5; 
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
        EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
        EXTI_Init(&EXTI_InitStructure);
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
    else
    {
        EXTI_ClearITPendingBit(EXTI_Line5);
        EXTI_InitStructure.EXTI_Line = EXTI_Line5; 
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
        EXTI_InitStructure.EXTI_LineCmd = DISABLE; 
        EXTI_Init(&EXTI_InitStructure);
    }
}

void HalExtiInitialize(void)
{
    //SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, GPIO_PinSource7 | GPIO_PinSource8 | GPIO_PinSource9);
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStructure;

    //PD10\11 Frequency capture pin, IR input pd0
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource10);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource11);
#if 1
    //433 module wakeup pin pe5
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource5);

    //light irq pin pc6
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource6);

    //light irq enable
/*
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line6; 
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);
*/
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure); 
#endif
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure); 

    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure); 

}

void HalExtiPoll(void)
{
}


