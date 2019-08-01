#ifndef HAL_TIMER_H
#define HAL_TIMER_H

void HalLEDUpdateTimerEnable(char enable);
void HalTimerStop(void);
void HalTimerStart(void);
void HalTimerInitialize(void);
void HalTimerPoll(void);

#endif

