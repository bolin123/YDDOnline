#ifndef KEYPAD_H
#define KEYPAD_H

#include "Sys.h"

typedef enum
{
    KEY_STATUS_PRESS = 0,
    KEY_STATUS_RELEASE,
}KeyStatus_t;

typedef void (*KeyEventHandle)(uint8_t keyval, KeyStatus_t status, bool longpress, uint32_t lastTime);

void KeypadInit(KeyEventHandle eventHandle);
void KeypadPoll(void);
#endif

