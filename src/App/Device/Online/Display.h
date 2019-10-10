#ifndef DISPLAY_H
#define DISPLAY_H

#include "Sys.h"

typedef enum
{
    DISPLAY_ID_ADDR = 0, //���ߵ�ַ
    DISPLAY_ID_RFCHN,    //����ͨ��
    DISPLAY_ID_DEVTYPE,  //�豸����
    DISPLAY_ID_ERRCODE,  //�쳣����
    DISPLAY_ID_POWER,    //����
    DISPLAY_ID_TEMP,     //�¶�
    DISPLAY_ID_PRESS,    //Ӧ��
    DISPLAY_ID_THRESHOLD,//��ֵ
    DISPLAY_ID_COUNT,
}DisplayID_t;

typedef void (*DisplayEvent_cb)(bool done, DisplayID_t id);

void DisplayLoopShow(bool start, uint8_t lcount);
void DisplayContentUpdate(DisplayID_t id, uint8_t symbol, uint16_t value);
void DisplayInit(DisplayEvent_cb cb);
void DisplayPoll(void);

#endif
