#ifndef HAL_SPI_H
#define HAL_SPI_H

#include "HalCtype.h"

uint8_t HalSPIReadWriteByte(uint8_t txData);
void HalSPIInitialize(void);
void HalSPIPoll(void);
#endif

