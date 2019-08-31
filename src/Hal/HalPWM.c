#include "HalPWM.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"

void HalPWMEnable(char enable)
{
    if(enable)
    {
    	TIM_Cmd(TIM3, ENABLE);		//打开定时器的外设
    }
    else
    {
    	TIM_Cmd(TIM3, DISABLE);		//打开定时器的外设
    }
}

void HalPWMInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure; 
	
	//初始化PC8
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 	//配置为复用模式(PWM输出)
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	// 定时器初始化
	TIM_TimeBaseStructure.TIM_Prescaler = 72 - 3;		// 预分频器，输入值会自动加1
	TIM_TimeBaseStructure.TIM_Period = 26;		// 初值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	// 时钟分割
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	// 向上计数
	TIM_TimeBaseInit(TIM3, & TIM_TimeBaseStructure);

    // PWM 初始化
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_Pulse = 13;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;		// 输出为低
	TIM_OC3Init(TIM3, & TIM_OCInitStructure);
	
	// 使能或者失能TIMx在CCR2上的预装载寄存器
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
	
}


