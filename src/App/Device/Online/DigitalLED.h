#ifndef LED_H
#define LED_H

#include "Sys.h"

typedef enum
{
    DIGITAL_LED_ID_CMD = 0,
    DIGITAL_LED_ID_1,
    DIGITAL_LED_ID_2,
    DIGITAL_LED_ID_3,
    DIGITAL_LED_ID_COUNT,
}DigitalLEDId_t;

typedef enum
{
    DIGITAL_FLAG_A = 0x0A,
    DIGITAL_FLAG_B,
    DIGITAL_FLAG_C,
    DIGITAL_FLAG_D,
    DIGITAL_FLAG_E,
    DIGITAL_FLAG_F,
    DIGITAL_FLAG_H,
    DIGITAL_FLAG_L,
    DIGITAL_FLAG_n,
    DIGITAL_FLAG_P,
    DIGITAL_FLAG_R,
    DIGITAL_FLAG_S,
    DIGITAL_FLAG_U,
    DIGITAL_FLAG_NONE,
}DigitalLEDFlag_t;

void DigitalLEDSetChars(DigitalLEDId_t id, uint8_t value, bool point);
void DigitalLEDOff(void);
void DigitalLEDOn(void);
void DigitalLEDInit(void);
#endif

