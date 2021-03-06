#ifndef HAL_FLASH_H
#define HAL_FLASH_H

#include "HalCtype.h"

void HalFlashInitialize(void);
void HalFlashPoll(void);

void HalFlashOTAWrite(uint32_t offset, uint8_t *data, uint16_t length);
void HalFlashOTAErase(uint32_t length);
void HalFlashErase(uint32_t addr);
void HalFlashWrite(uint32_t addr, const uint8_t *data, uint32_t len);
void HalFlashRead(uint32_t addr, uint8_t *buf, uint32_t bufSize);

#endif // HAL_FLASH_H
