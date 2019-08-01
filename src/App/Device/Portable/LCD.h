#ifndef LCD_H
#define LCD_H

#include "Sys.h"

#define LCD_PIXEL_WIDTH  480
#define LCD_PIXEL_HEIGHT 272

typedef enum
{
    LCD_CHAR_SIZE_6x12 = 0,
    LCD_CHAR_SIZE_8x16,
    LCD_CHAR_SIZE_10x20,
    LCD_CHAR_SIZE_12x24,
    LCD_CHAR_SIZE_14x28,
    LCD_CHAR_SIZE_16x32,
    LCD_CHAR_SIZE_20x40,
    LCD_CHAR_SIZE_24x48,
    LCD_CHAR_SIZE_28x56,
    LCD_CHAR_SIZE_32x64,
}LCDCharSize_t;


void LCDStringsPrint(char *data, uint8_t len, SysDisplayPosition_t *pos, bool hasBgColor, 
                                LCDCharSize_t size, uint16_t color, uint16_t bgcolor);
void LCDBrightnessSet(uint8_t value);
void LCDPictureShow(SysPictureID_t id);
void LCDRectangle(uint8_t mode, uint16_t color, SysDisplayPosition_t *top, SysDisplayPosition_t *bottom);
void LCDRectMove(uint8_t dir, uint8_t distance, uint16_t color, SysDisplayPosition_t *top, SysDisplayPosition_t *bottom);
void LCDClear(void);
void LCDInitialize(void);
void LCDPoll(void);

#endif

