#include "HalExti.h"
#include "PowerManager.h"


typedef struct
{
    uint16_t ioPin;
    GPIO_TypeDef *ioGroup;
    uint8_t portSource;
    uint8_t pinSource;
    uint8_t irqChannel;
    uint32_t exitLine;
    EXTITrigger_TypeDef trigger;
}HalExitConfigs_t;

static HalExitConfigs_t HalEXITConfigs[HAL_EXIT_COUNT] = {

#if defined(HAL_OLD_DEVICE)
    /*HAL_EXIT_GEOM_FREQ_CAPTRUE*/
    {
        .ioPin      = GPIO_Pin_2,
        .ioGroup    = GPIOA,
        .portSource = GPIO_PortSourceGPIOA,
        .pinSource  = GPIO_PinSource2,
        .irqChannel = EXTI2_IRQn,
        .exitLine   = EXTI_Line2,
        .trigger    = EXTI_Trigger_Rising,
    },
    /*HAL_EXIT_NOISE_FREQ_CAPTRUE*/
    {
        .ioPin      = GPIO_Pin_5,
        .ioGroup    = GPIOA,
        .portSource = GPIO_PortSourceGPIOA,
        .pinSource  = GPIO_PinSource5,
        .irqChannel = EXTI15_10_IRQn,
        .exitLine   = EXTI_Line5,
        .trigger    = EXTI_Trigger_Rising,
    },
    /*HAL_EXIT_IR_INPUT*/
    {
        .ioPin      = GPIO_Pin_3,
        .ioGroup    = GPIOE,
        .portSource = GPIO_PortSourceGPIOE,
        .pinSource  = GPIO_PinSource3,
        .irqChannel = EXTI3_IRQn,
        .exitLine   = EXTI_Line3,
        .trigger    = EXTI_Trigger_Falling,
    },
    /*HAL_EXIT_433MODULE_WAKEUP*/
    {
        .ioPin      = GPIO_Pin_9,
        .ioGroup    = GPIOC,
        .portSource = GPIO_PortSourceGPIOC,
        .pinSource  = GPIO_PinSource9,
        .irqChannel = EXTI9_5_IRQn,
        .exitLine   = EXTI_Line9,
        .trigger   = EXTI_Trigger_Falling,
    },
    /*HAL_EXIT_LIGHT_WAKEUP*/
    {
        .ioPin      = GPIO_Pin_6,
        .ioGroup    = GPIOE,
        .portSource = GPIO_PortSourceGPIOE,
        .pinSource  = GPIO_PinSource6,
        .irqChannel = EXTI9_5_IRQn,
        .exitLine   = EXTI_Line6,
        .trigger    = EXTI_Trigger_Falling,
    }
#else //new 

    /*HAL_EXIT_GEOM_FREQ_CAPTRUE*/
    {
        .ioPin      = GPIO_Pin_10,
        .ioGroup    = GPIOD,
        .portSource = GPIO_PortSourceGPIOD,
        .pinSource  = GPIO_PinSource10,
        .irqChannel = EXTI15_10_IRQn,
        .exitLine   = EXTI_Line10,
        .trigger    = EXTI_Trigger_Rising,
    },
    /*HAL_EXIT_NOISE_FREQ_CAPTRUE*/
    {
        .ioPin      = GPIO_Pin_11,
        .ioGroup    = GPIOD,
        .portSource = GPIO_PortSourceGPIOD,
        .pinSource  = GPIO_PinSource11,
        .irqChannel = EXTI15_10_IRQn,
        .exitLine   = EXTI_Line11,
        .trigger    = EXTI_Trigger_Rising,
    },
    /*HAL_EXIT_IR_INPUT*/
    {
        .ioPin      = GPIO_Pin_0,
        .ioGroup    = GPIOD,
        .portSource = GPIO_PortSourceGPIOD,
        .pinSource  = GPIO_PinSource0,
        .irqChannel = EXTI0_IRQn,
        .exitLine   = EXTI_Line0,
        .trigger    = EXTI_Trigger_Falling,
    },
    /*HAL_EXIT_433MODULE_WAKEUP*/
    {
        .ioPin      = GPIO_Pin_5,
        .ioGroup    = GPIOE,
        .portSource = GPIO_PortSourceGPIOE,
        .pinSource  = GPIO_PinSource5,
        .irqChannel = EXTI9_5_IRQn,
        .exitLine   = EXTI_Line5,
        .trigger    = EXTI_Trigger_Falling,
    },
    /*HAL_EXIT_LIGHT_WAKEUP*/
    {
        .ioPin      = GPIO_Pin_6,
        .ioGroup    = GPIOC,
        .portSource = GPIO_PortSourceGPIOC,
        .pinSource  = GPIO_PinSource6,
        .irqChannel = EXTI9_5_IRQn,
        .exitLine   = EXTI_Line6,
        .trigger    = EXTI_Trigger_Falling,
    }
#endif
};

extern void YDDOnlineSensorFreqTrigger(uint8_t ch);
extern void YDDOnlineLightActive(void);

#if defined(HAL_OLD_DEVICE)

void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(HalEXITConfigs[HAL_EXIT_NOISE_FREQ_CAPTRUE].exitLine) != RESET)
    {
        YDDOnlineSensorFreqTrigger(HAL_SENSOR_ID_NOISE);
        EXTI_ClearITPendingBit(HalEXITConfigs[HAL_EXIT_NOISE_FREQ_CAPTRUE].exitLine);
    }

    if(EXTI_GetITStatus(HalEXITConfigs[HAL_EXIT_433MODULE_WAKEUP].exitLine) != RESET)
    {
        PMWakeup(PM_WAKEUP_TYPE_WIRELESS);   
        EXTI_ClearITPendingBit(HalEXITConfigs[HAL_EXIT_433MODULE_WAKEUP].exitLine);
    }

    if(EXTI_GetITStatus(HalEXITConfigs[HAL_EXIT_LIGHT_WAKEUP].exitLine) != RESET)
    {
        //PMWakeup(PM_WAKEUP_TYPE_LIGHT);
        YDDOnlineLightActive();
        EXTI_ClearITPendingBit(HalEXITConfigs[HAL_EXIT_LIGHT_WAKEUP].exitLine);
    }
}

void EXTI2_IRQHandler(void)
{
	if(EXTI_GetITStatus(HalEXITConfigs[HAL_EXIT_GEOM_FREQ_CAPTRUE].exitLine) != RESET)
	{
        YDDOnlineSensorFreqTrigger(HAL_SENSOR_ID_GEOMAGNETISM);
        EXTI_ClearITPendingBit(HalEXITConfigs[HAL_EXIT_GEOM_FREQ_CAPTRUE].exitLine);
    }
}

#else //new one

void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(HalEXITConfigs[HAL_EXIT_433MODULE_WAKEUP].exitLine) != RESET) //433_ACT
    {
        PMWakeup(PM_WAKEUP_TYPE_WIRELESS);   
        EXTI_ClearITPendingBit(HalEXITConfigs[HAL_EXIT_433MODULE_WAKEUP].exitLine);
    }

    if(EXTI_GetITStatus(HalEXITConfigs[HAL_EXIT_LIGHT_WAKEUP].exitLine) != RESET) //WAKEUP_EN
    {
        //PMWakeup(PM_WAKEUP_TYPE_LIGHT);
        YDDOnlineLightActive();
        EXTI_ClearITPendingBit(HalEXITConfigs[HAL_EXIT_LIGHT_WAKEUP].exitLine);
    }
}

void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(HalEXITConfigs[HAL_EXIT_GEOM_FREQ_CAPTRUE].exitLine) != RESET) //F1
	{
        YDDOnlineSensorFreqTrigger(HAL_SENSOR_ID_GEOMAGNETISM);
        EXTI_ClearITPendingBit(HalEXITConfigs[HAL_EXIT_GEOM_FREQ_CAPTRUE].exitLine);
    }

    if(EXTI_GetITStatus(HalEXITConfigs[HAL_EXIT_NOISE_FREQ_CAPTRUE].exitLine) != RESET) //F2
    {
        YDDOnlineSensorFreqTrigger(HAL_SENSOR_ID_NOISE);
        EXTI_ClearITPendingBit(HalEXITConfigs[HAL_EXIT_NOISE_FREQ_CAPTRUE].exitLine);
    }
}

#endif

extern void IRRecvBits(void);
#if defined(HAL_OLD_DEVICE)
void EXTI3_IRQHandler(void)
#else
void EXTI0_IRQHandler(void)
#endif
{
    if(EXTI_GetITStatus(HalEXITConfigs[HAL_EXIT_IR_INPUT].exitLine) != RESET) //IR_RX_OUT
    {
        IRRecvBits();
        EXTI_ClearITPendingBit(HalEXITConfigs[HAL_EXIT_IR_INPUT].exitLine);
    }
}

void HalExitSet(HalExit_t exitNo, bool enable)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    if(enable)
    {
        EXTI_InitStructure.EXTI_Line = HalEXITConfigs[exitNo].exitLine; 
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
        EXTI_InitStructure.EXTI_Trigger = HalEXITConfigs[exitNo].trigger; 
        EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
        EXTI_Init(&EXTI_InitStructure);
        EXTI_ClearITPendingBit(HalEXITConfigs[exitNo].exitLine);
    }
    else
    {
        EXTI_ClearITPendingBit(HalEXITConfigs[exitNo].exitLine);
        EXTI_InitStructure.EXTI_Line = HalEXITConfigs[exitNo].exitLine; 
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

    for(int i = 0; i < HAL_EXIT_COUNT; i++)
    {
        GPIO_InitStruct.GPIO_Pin = HalEXITConfigs[i].ioPin;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(HalEXITConfigs[i].ioGroup, &GPIO_InitStruct);

        GPIO_EXTILineConfig(HalEXITConfigs[i].portSource, HalEXITConfigs[i].pinSource);

        NVIC_InitStructure.NVIC_IRQChannel = HalEXITConfigs[i].irqChannel; 
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
        NVIC_Init(&NVIC_InitStructure); 
    }
    

}

void HalExtiPoll(void)
{
}


