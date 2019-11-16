#ifndef HAL_EXTI_H
#define HAL_EXTI_H

#include "HalCtype.h"

typedef enum
{
    HAL_EXIT_GEOM_FREQ_CAPTRUE = 0,
    HAL_EXIT_NOISE_FREQ_CAPTRUE,
    HAL_EXIT_IR_INPUT,
    HAL_EXIT_433MODULE_WAKEUP,
    HAL_EXIT_LIGHT_WAKEUP,
    HAL_EXIT_COUNT,
}HalExit_t;

#if 0
void HalExtiIRRecvEnable(bool enable);
void HalExtiLightEnable(bool enable);
void HalExtiWakeupSet(bool enable);
void HalExtiFreqStart(void);
void HalExtiFreqStop(void);
#endif
void HalExitSet(HalExit_t exitNo, bool enable);
void HalExtiInitialize(void);
void HalExtiPoll(void);
#endif

