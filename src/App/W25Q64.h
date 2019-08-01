#ifndef W25Q64_H
#define W25Q64_H

#include "HalCtype.h"

void W25Q64Write(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void W25Q64Read(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
void W25Q64PageWrite(uint8_t *pBuffer, uint32_t page);

void W25Q64EraseChip(void);
void W25Q64PowerDown(void);
void W25Q64Wakeup(void);

uint16_t W25Q64ReadID(void);
uint16_t W25Q64Initialize(void);
void W25Q64Poll(void);

#endif

