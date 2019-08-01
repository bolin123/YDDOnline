#include "Display.h"
#include "LCD.h"

#define DISP_POWER_TOP_X 415
#define DISP_POWER_TOP_Y 9
#define DISP_POWER_BOTTOM_X 465
#define DISP_POWER_BOTTOM_Y 30

#define DISP_DATE_X 15
#define DISP_DATE_Y 8

void DisplayPictureShow(SysPictureID_t id)
{
    //LCDClear();
    LCDPictureShow(id);
}

void DisplaySelectBox(bool selected, SysDisplayPosition_t *menuPos, uint8_t width, uint8_t height)
{
    SysDisplayPosition_t top, bottom;
    uint16_t color = selected ?  DISPLAY_COLOR_BLUE : DISPLAY_COLOR_BOTTOM;

    top.x = menuPos->x - 2;
    top.y = menuPos->y - 2;
    bottom.x = top.x + width;
    bottom.y = top.y + height;
    
    LCDRectangle(0, color, &top, &bottom);
}

void DisplayDateTimeUpdate(void)
{
    SysDateTime_t *time = SysDateTime();
    char buff[32] = {0};
    SysDisplayPosition_t pos;
    SysDisplayPosition_t top, bottom;

    top.x = 12;
    top.y = 9;
    bottom.x = 215;
    bottom.y = 30;
    LCDRectangle(1, DISPLAY_COLOR_BOTTOM, &top, &bottom);
    pos.x = DISP_DATE_X;
    pos.y = DISP_DATE_Y;
    sprintf(buff, "%d-%02d-%02d %02d:%02d", time->year, time->month, time->day, time->hour, time->minute);
    LCDStringsPrint(buff, strlen(buff), &pos, false, LCD_CHAR_SIZE_12x24, DISPLAY_COLOR_BLACK, DISPLAY_COLOR_BLUE);
}

void DisplayPowerPercent(void)
{
    SysDisplayPosition_t top, bottom;
    uint8_t percent = SysPowerPercent();
    
    top.x = DISP_POWER_TOP_X;
    top.y = DISP_POWER_TOP_Y;
    bottom.x = DISP_POWER_BOTTOM_X;
    bottom.y = DISP_POWER_BOTTOM_Y;
    if(percent < 15)
    {
        LCDRectangle(1, DISPLAY_COLOR_RED, &top, &bottom);
    }
    else
    {
        LCDRectangle(1, DISPLAY_COLOR_GREEN, &top, &bottom);
    }
    LCDRectMove(1, (100 - percent) / 2, DISPLAY_COLOR_CYAN, &top, &bottom);

    char buff[5];
    SysDisplayPosition_t pos;
    sprintf(buff, "%d%%", percent);
    if(percent == 100)
    {
        pos.x = DISP_POWER_TOP_X;
    }
    else if(percent > 10)
    {
        pos.x = DISP_POWER_TOP_X + 6;
    }
    else
    {
        pos.x = DISP_POWER_TOP_X + 12;
    }
    pos.y = 8;
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &pos, DISPLAY_CHAR_SIZE_NORMAL);
}

void DisplayDrawRect(uint16_t color, SysDisplayPosition_t *top, SysDisplayPosition_t *bottom)
{
    LCDRectangle(1, color, top, bottom);
}

void DiplayStringPrint(char * data, uint8_t len, uint16_t color, SysDisplayPosition_t *pos, DisplayCharSize_t size)
{
    LCDCharSize_t lsize;

    switch (size)
    {
        case DISPLAY_CHAR_SIZE_SMALL:
            lsize = LCD_CHAR_SIZE_6x12;
            break;
        case DISPLAY_CHAR_SIZE_NORMAL:
            lsize = LCD_CHAR_SIZE_12x24;
            break;
        case DISPLAY_CHAR_SIZE_LARGE:
            lsize = LCD_CHAR_SIZE_24x48;
            break;
        default:
            break;
    }
    LCDStringsPrint(data, len, pos, false, lsize, color, DISPLAY_COLOR_BLUE);
}

void DisplayBrightnessSet(uint8_t value)
{
    LCDBrightnessSet(value);
}

void DisplayInitialize(void)
{
    LCDInitialize();
    //LCDBrightnessSet(5);
    //LCDClear();
    //LCDPictureShow(SYS_PICTURE_SETTING_ID);
    //DisplayPowerPercent(50);
    //DisplayDateTimeUpdate();
    
    SysCollectArgs_t args;
    SysCollectArgsGet(&args);
    DisplayBrightnessSet(args.brightness);
}

void DisplayPoll(void)
{
    LCDPoll();
}

