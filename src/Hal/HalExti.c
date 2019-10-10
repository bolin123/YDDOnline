#include "HalExti.h"
#include "PowerManager.h"

extern void YDDOnlineSensorFreqTrigger(uint8_t ch);
extern void YDDOnlineLightActive(void);

void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        YDDOnlineSensorFreqTrigger(HAL_SENSOR_ID_NOISE);
        EXTI_ClearITPendingBit(EXTI_Line5);
    }

    if(EXTI_GetITStatus(EXTI_Line9) != RESET)
    {
        PMWakeup(PM_WAKEUP_TYPE_WIRELESS);   
        EXTI_ClearITPendingBit(EXTI_Line9);
    }

    if(EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        //PMWakeup(PM_WAKEUP_TYPE_LIGHT);
        YDDOnlineLightActive();
        EXTI_ClearITPendingBit(EXTI_Line6);
    }
}

void EXTI2_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
        YDDOnlineSensorFreqTrigger(HAL_SENSOR_ID_GEOMAGNETISM);
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

extern void IRRecvBits(void);
void EXTI3_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line3) != RESET)
	{
	    IRRecvBits();
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void HalExtiIRRecvEnable(bool enable)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    
    if(enable)
    {
        EXTI_InitStructure.EXTI_Line = EXTI_Line3; 
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
        EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
        EXTI_Init(&EXTI_InitStructure);
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
    else
    {
        EXTI_ClearITPendingBit(EXTI_Line3);
        EXTI_InitStructure.EXTI_Line = EXTI_Line3; 
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
        EXTI_InitStructure.EXTI_LineCmd = DISABLE; 
        EXTI_Init(&EXTI_InitStructure);
    }
}


void HalExtiFreqStart(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line2 | EXTI_Line5; 
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);
    EXTI_ClearITPendingBit(EXTI_Line2 | EXTI_Line5);   
}

void HalExtiFreqStop(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line2 | EXTI_Line5; 
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_LineCmd = DISABLE; 
    EXTI_Init(&EXTI_InitStructure);
    EXTI_ClearITPendingBit(EXTI_Line2 | EXTI_Line5);

}

/*433 wakeup exti set*/
void HalExtiWakeupSet(bool enable)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    
    if(enable)
    {
        EXTI_InitStructure.EXTI_Line = EXTI_Line9; 
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
        EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
        EXTI_Init(&EXTI_InitStructure);
        EXTI_ClearITPendingBit(EXTI_Line9);
    }
    else
    {
        EXTI_ClearITPendingBit(EXTI_Line9);
        EXTI_InitStructure.EXTI_Line = EXTI_Line9; 
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

    //PA2\5 Frequency capture pin
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource5);
#if 1
    //433 module wakeup pin 
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource9);

    //light irq pin pe6, IR input pe3
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_6;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource6);

    //light irq enable
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure.EXTI_Line = EXTI_Line6; 
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
    EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure); 
#endif
    
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure); 

    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure); 

}

void HalExtiPoll(void)
{
}


