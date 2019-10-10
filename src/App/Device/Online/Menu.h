#ifndef MENU_H
#define MENU_H

#include "IR.h"

typedef enum
{
    MENU_ID_ADDR = 0, //通信地址
    MENU_ID_RFCHN,    //无线通道
    MENU_ID_DEVTYPE,  //设备类型
    MENU_ID_COMMTYPE, //通信方式
    //MENU_ID_THRESHOLD,
    MENU_ID_COUNT,
}MenuID_t;

typedef enum
{
    MENU_EVENT_ACTIVE,
    MENU_EVENT_DEACTIVE,
    MENU_EVENT_TIMEOUT,
}MenuEvent_t;

typedef unsigned short (*MenuGetValue_cb)(MenuID_t id);
typedef void (*MenuSetValue_cb)(MenuID_t id, unsigned short value);
typedef void (*MenuEventHandle_cb)(MenuEvent_t event);

typedef struct
{
    char enable;
    char flag;     //显示标志
    char digitNum; //位数
    char step;     //步进数
    unsigned short max;     //最大值
    unsigned short min;     //最小值
    MenuGetValue_cb getValue;
    MenuSetValue_cb setValue;
}MenuItem_t;

void MenuDeactive(void);
void MenuKeyHandle(IRKey_t key);
void MenuRegister(MenuID_t id, MenuItem_t *item);
void MenuInit(MenuEventHandle_cb eventHandle);
void MenuPoll(void);
#endif
