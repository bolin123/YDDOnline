#ifndef HAL_COMMON_H
#define HAL_COMMON_H

#include "HalCtype.h"
#include "HalUart.h"
#include "HalGPIO.h"
#include "HalTimer.h"
#include "HalSPI.h"
#include "HalFlash.h"
#include "HalWait.h"
#include "HalRTC.h"
#include "HalADC.h"
#include "HalDAC.h"
#include "HalExti.h"

void HalCommonWakeup(void);
void HalCommonFallasleep(void);

uint32_t HalGetSysTimeCount(void);
void HalInterruptSet(bool enable);
void HalTimerPast1ms(void);
void HalCommonReboot(void);
uint16_t HalCommonInitialize(void);
void HalCommonPoll(void);

#endif

