#ifndef DISPLAY_COORDINATE_H
#define DISPLAY_COORDINATE_H

#include "Sys.h"

//选择框的长度
#define YDD_SELECT_SETTINGS_WIDTH 80
#define YDD_SELECT_SETTINGS_HEIGTH 28
#define YDD_SELECT_CLEAR_WIDTH 60
#define YDD_SELECT_TIMING_WIDTH 60
#define YDD_SELECT_TIMING_HEIGTH 30

typedef enum
{
    YDD_SETTINGS_ITEMID_CSSJ = 0,
    YDD_SETTINGS_ITEMID_QDBJ,
    YDD_SETTINGS_ITEMID_XHFZ,
    YDD_SETTINGS_ITEMID_ZLBJ,
    YDD_SETTINGS_ITEMID_BEEP,
    YDD_SETTINGS_ITEMID_BRIGHTNESS,
    YDD_SETTINGS_ITEMID_CLEAR,
    YDD_SETTINGS_ITEMID_TIMING,
    YDD_SETTINGS_ITEMID_COUNT,
}YDDSettingsItemID_t;

typedef enum
{
    YDD_TIMING_ITEMID_YEAR = 0,
    YDD_TIMING_ITEMID_MONTH,
    YDD_TIMING_ITEMID_DAY,
    YDD_TIMING_ITEMID_HOUR,
    YDD_TIMING_ITEMID_MINUTE,
    YDD_TIMING_ITEMID_SECOND,
    YDD_TIMING_ITEMID_SURE,
    YDD_TIMING_ITEMID_CANCLE,
    YDD_TIMING_ITEMID_COUNT,
}YDDTimingItemID_t;

typedef enum
{
    YDD_CLEAR_ITEMID_SURE,
    YDD_CLEAR_ITEMID_CANCLE,
    YDD_CLEAR_ITEMID_COUNT,
}YDDClearItemID_t;


// TODO: 完善坐标，修改设置界面
static const SysDisplayPosition_t g_settingsPos[YDD_SETTINGS_ITEMID_COUNT] = {
    {138, 72}, //测试时间
    {327, 72}, //强度报警
    {138, 112},//信号阀值
    {327, 112},
    {138, 152},
    {327, 152},
    {56, 192},
    {239, 192},
};

static const SysDisplayPosition_t g_detailsPos[6] = {
    {138, 72},
    {327, 72},
    {138, 112},
    {327, 112},
    {138, 152},
    {327, 152},
};

static const SysDisplayPosition_t g_timingPos[YDD_TIMING_ITEMID_COUNT] = {
    {30, 130}, //year
    {115, 130},//month
    {185, 130},//day
    {255, 130},//hour
    {325, 130},//minute
    {395, 130},//second
    {173, 178},//sure
    {256, 178},//cancle
};

static const SysDisplayPosition_t g_clearPos[2] = {
    {173, 178},
    {255, 178},
};

typedef struct
{
    SysDisplayPosition_t vltg;
    SysDisplayPosition_t freq;
}YDDCapturePosition_t;

static const YDDCapturePosition_t g_capturePos[4] = {
    {
        {120, 115},
        {120, 166},

    },
    {
        {210, 115},
        {210, 166},
    },
    {
        {300, 115},
        {300, 166},
    },
    {
        {390, 115},
        {390, 166},
    },
};



#endif

