#ifndef DISPLOOP_H
#define DISPLOOP_H

#include "Sys.h"

typedef enum
{
    DISPLOOP_ID_ADDR = 0, //���ߵ�ַ
    DISPLOOP_ID_RFCHN,    //����ͨ��
    DISPLOOP_ID_DEVTYPE,  //�豸����
    DISPLOOP_ID_ERRCODE,  //�쳣����
    DISPLOOP_ID_POWER,    //����
//    DISPLOOP_ID_TEMP,     //�¶�
    DISPLOOP_ID_PRESS,    //Ӧ��
    DISPLOOP_ID_THRESHOLD,//��ֵ
    DISPLOOP_ID_COUNT,
}DispLoopID_t;

typedef struct
{
    uint8_t flag;
    uint8_t contents[3];
}DispLoopValue_t;

typedef DispLoopValue_t *(*DispLoopGetValue_cb)(DispLoopID_t id);
typedef void (*DispLoopDone_cb)(void);

void DispLoopRegister(DispLoopID_t id, DispLoopGetValue_cb getvalue);
void DispLoopStop(void);
void DispLoopStart(uint8_t num);
void DispLoopInit(DispLoopDone_cb loopDone);
void DispLoopPoll(void);

#endif

