#include "HalTimer.h"
#include "stm32f10x_tim.h"

extern void HalTimerPast1ms(void);

void TIM1_UP_IRQHandler(void)
{
    if(TIM_GetFlagStatus(TIM1, TIM_FLAG_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM1, TIM_FLAG_Update);
        HalTimerPast1ms();
    }
}

extern void DigitalLEDScan(void);
void TIM2_IRQHandler(void)
{
    if(TIM_GetFlagStatus(TIM2, TIM_FLAG_Update) != RESET)
    {
        DigitalLEDScan();
        TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
        //HalTimerPast1ms();
    }
}

extern void IRRecvTimerHandle(void);
void TIM4_IRQHandler(void)
{ 
    if(TIM_GetFlagStatus(TIM4, TIM_FLAG_Update) != RESET)
    {
        IRRecvTimerHandle();
        TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update);
    }
}

void HalTimerStop(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM1, TIM_IT_Update, DISABLE);
    TIM_Cmd(TIM1, DISABLE);
}

void HalTimerStart(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ClearITPendingBit(TIM1, TIM_FLAG_Update);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

void HalLEDUpdateTimerEnable(char enable)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    if(enable)
    {
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
        NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_Init(&NVIC_InitStructure);

        TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
        TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
        TIM_Cmd(TIM2, ENABLE);
    }
    else
    {
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
        NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
        NVIC_Init(&NVIC_InitStructure);

        TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
        TIM_Cmd(TIM2, DISABLE);
    }
    
}

void HalIRRecvTimerEnable(char enable)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    if(enable)
    {
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
        NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_Init(&NVIC_InitStructure);

        TIM_ClearITPendingBit(TIM4, TIM_FLAG_Update);
        TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
        TIM_Cmd(TIM4, ENABLE);
    }
    else
    {
        NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
        NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
        NVIC_Init(&NVIC_InitStructure);

        TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);
        TIM_Cmd(TIM4, DISABLE);
    }
    
}

void HalTimerInitialize(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseInitStruct.TIM_Period = 1000;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStruct);

    TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseInitStruct.TIM_Period = 4000;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);

    TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseInitStruct.TIM_Period = 100; //100us
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);
}

void HalTimerPoll(void)
{
}

