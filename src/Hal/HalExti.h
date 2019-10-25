#ifndef HAL_EXTI_H
#define HAL_EXTI_H

#include "HalCtype.h"

void HalExtiIRRecvEnable(bool enable);
void HalExtiLightEnable(bool enable);
void HalExtiWakeupSet(bool enable);
void HalExtiFreqStart(void);
void HalExtiFreqStop(void);
void HalExtiInitialize(void);
void HalExtiPoll(void);
#endif
