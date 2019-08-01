#include "HalWait.h"
#include "stm32f10x_tim.h"

void HalWaitUs(uint16_t us)
{
//    uint16_t count = 0;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseInitStruct.TIM_Period = us;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStruct);
    TIM_ClearFlag(TIM6, TIM_FLAG_Update);
    TIM_Cmd(TIM6, ENABLE);

    while(1)
    {
        if((TIM6->SR) & TIM_IT_Update)
    	{
    		TIM6->SR = (uint16_t)~TIM_IT_Update;
    		break;
    	}
    }
    TIM_Cmd(TIM6, DISABLE);
}

void HalWaitMs(uint16_t ms)
{
    uint16_t count = 0;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1;
    TIM_TimeBaseInitStruct.TIM_Period = 1000;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStruct);
    TIM_ClearFlag(TIM6, TIM_FLAG_Update);
    TIM_Cmd(TIM6, ENABLE);

    while(count < ms)
    {
        if((TIM6->SR) & TIM_IT_Update)
        {
            TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
            count++;
        }
    }
    
    TIM_Cmd(TIM6, DISABLE);

}

