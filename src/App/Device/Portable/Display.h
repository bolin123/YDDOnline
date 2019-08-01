#ifndef DISPLAY_H
#define DISPLAY_H

#include "Sys.h"

typedef enum
{
    DISPLAY_CHAR_SIZE_SMALL = 0,
    DISPLAY_CHAR_SIZE_NORMAL,
    DISPLAY_CHAR_SIZE_LARGE,
}DisplayCharSize_t;


//��ʾ��ɫRGB565
#define DISPLAY_COLOR_BLACK     0x0000  //��ɫ
#define DISPLAY_COLOR_NAVY      0x000F  //����ɫ
#define DISPLAY_COLOR_DGREEN    0x03E0  //����ɫ
#define DISPLAY_COLOR_DCYAN     0x03EF  //����ɫ
#define DISPLAY_COLOR_MAROON    0x7800  //���ɫ
#define DISPLAY_COLOR_PURPLE    0x780F  //��ɫ
#define DISPLAY_COLOR_OLIVE     0x7BE0  //�����
#define DISPLAY_COLOR_LGRAY     0xC618  //�Ұ�ɫ
#define DISPLAY_COLOR_DGRAY     0x7BEF  //���ɫ
#define DISPLAY_COLOR_BLUE      0x001F  //��ɫ
#define DISPLAY_COLOR_GREEN     0x07E0  //��ɫ
#define DISPLAY_COLOR_CYAN      0x07FF  //��ɫ
#define DISPLAY_COLOR_RED       0xF800  //��ɫ
#define DISPLAY_COLOR_MAGENTA   0xF81F  //Ʒ��
#define DISPLAY_COLOR_YELLOW    0xFFE0  //��ɫ
#define DISPLAY_COLOR_WHITE     0xFFFF  //��ɫ

#define DISPLAY_COLOR_BOTTOM    0x9EDD

void DisplayDrawRect(uint16_t color, SysDisplayPosition_t *top, SysDisplayPosition_t *bottom);
void DisplaySelectBox(bool selected, SysDisplayPosition_t *menuPos, uint8_t width, uint8_t height);
void DisplayBrightnessSet(uint8_t value);
//void DisplaySettingsSelect(uint8_t id, uint16_t color);
void DiplayStringPrint(char * data, uint8_t len, uint16_t color, SysDisplayPosition_t *pos, DisplayCharSize_t size);
void DisplayPictureShow(SysPictureID_t id);
void DisplayDateTimeUpdate(void);
void DisplayPowerPercent(void);
void DisplayInitialize(void);
void DisplayPoll(void);

#endif


