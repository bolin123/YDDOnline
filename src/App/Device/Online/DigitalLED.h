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

void DigitalLEDSetChars(DigitalLEDId_t id, uint8_t value, bool point);
void DigitalLEDOff(void);
void DigitalLEDOn(void);
void DigitalLEDInit(void);
void DigitalLEDPoll(void);
#endif

