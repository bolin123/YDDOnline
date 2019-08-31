#include "MenuSettings.h"
#include "Display.h"
#include "DisplayCoordinate.h"

#define MENU_SETTINGS_CSSJ_NUM 8
#define MENU_SETTINGS_ZLBJ_NUM 9

typedef enum
{
    MSDIPLAY_MENU_MAIN = 0,
    MSDIPLAY_MENU_CLEAR,
    MSDIPLAY_MENU_TIMING,
}MSDisplayMenu_t;

static YDDSettingsItemID_t g_settingItemId = YDD_SETTINGS_ITEMID_CSSJ;
static SysDeviceArgs_t g_collectArgs;
static const uint16_t CSSJ[MENU_SETTINGS_CSSJ_NUM] = {20, 60, 120, 240, 480, 600, 800, 999};
static const uint8_t ZLBJ[MENU_SETTINGS_ZLBJ_NUM] = {5, 20, 50, 80, 100, 120, 150, 180, 200};
//static bool g_argsModified = false;
static MSDisplayMenu_t g_displayMenu;
static SysDateTime_t g_date;
static YDDTimingItemID_t g_timmingItemId;
static YDDClearItemID_t g_clearItemId;

static void clearNextItem(void)
{
    //unselect front
    DisplaySelectBox(false, &g_clearPos[g_clearItemId++], YDD_SELECT_CLEAR_WIDTH, YDD_SELECT_TIMING_HEIGTH);
    if(g_clearItemId == YDD_CLEAR_ITEMID_COUNT)
    {
        g_clearItemId = YDD_CLEAR_ITEMID_SURE;
    }
    DisplaySelectBox(true, &g_clearPos[g_clearItemId], YDD_SELECT_CLEAR_WIDTH, YDD_SELECT_TIMING_HEIGTH); //select

}

static void clearItemSelectHandle(HalKeyValue_t key)
{
    char *notice = "数据已清空";
    SysDisplayPosition_t top;

    if(key != HAL_KEY_VALUE_CONFIG)
    {
        return;
    }
    
    switch (g_clearItemId)
    {
        case YDD_CLEAR_ITEMID_SURE:
            SysArgsClear();
            top.x = 190;
            top.y = 140;
            DiplayStringPrint(notice, strlen(notice), DISPLAY_COLOR_BLACK, &top, DISPLAY_CHAR_SIZE_NORMAL);
            HalWaitMs(500);
            MenuSettingsShow();
        break;
        case YDD_CLEAR_ITEMID_CANCLE:
            MenuSettingsShow();
        break;
        default:
        break;
    }
}

static void clearShow(void)
{
    DisplayPictureShow(SYS_PICTURE_CLEAR_ID);    
    g_displayMenu = MSDIPLAY_MENU_CLEAR;
    g_clearItemId = YDD_CLEAR_ITEMID_SURE;    
    DisplaySelectBox(true, &g_clearPos[g_clearItemId], YDD_SELECT_CLEAR_WIDTH, YDD_SELECT_TIMING_HEIGTH); //select
    DisplayDateTimeUpdate(); //时间显示
    DisplayPowerPercent();
}

static bool isLeapYear(uint16_t year)
{
    if(year && year % 4 == 0)
    {
        if(year % 100 == 0)
        {
            if(year % 400 != 0)
            {
                return false;
            }
        }
        return true;
    }

    return false;
}

static uint8_t dayInMonth(uint16_t year, uint8_t month)
{
    uint8_t monthDays;   
    bool isLeap = isLeapYear(year);
    
    switch(month)
    {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        monthDays = 31;
        break;

    case 2:
        monthDays = isLeap ? 29 : 28;
        break;
    default:
        monthDays = 30;
    }
    return monthDays;
}

static void timingItemValueUpdate(YDDTimingItemID_t id, uint16_t value)
{
    SysDisplayPosition_t top, bottom;
    char buff[8] = "";
    
    top.x = g_timingPos[id].x;
    top.y = g_timingPos[id].y;

    Syslog("ID=%d, value=%d", id, value);
    if(id == YDD_TIMING_ITEMID_YEAR)
    {
        bottom.x = g_timingPos[id].x + 12 * 4;
        sprintf(buff, "%d", value);
    }
    else
    {
        bottom.x = g_timingPos[id].x + 12 * 2;
        sprintf(buff, "%02d", value);
    }
    bottom.y = g_timingPos[id].y + 24;
    DisplayDrawRect(DISPLAY_COLOR_BOTTOM, &top, &bottom);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &top, DISPLAY_CHAR_SIZE_NORMAL);    
}

static uint32_t calcValue(uint32_t curValue, uint32_t max, uint32_t min, bool add, uint16_t step)
{
    uint32_t value = curValue;
    if(add)
    {
        value += step;
        if(value > max)
        {
            value = min;
        }
    }
    else
    {
        if(value <= min)
        {
            value = max;
        }
        else
        {
            value -= step;
        }
    }
    return value;
}

static void timmingItemSelectHandle(HalKeyValue_t key)
{
    
    if(g_timmingItemId == YDD_TIMING_ITEMID_SURE || g_timmingItemId == YDD_TIMING_ITEMID_CANCLE)
    {
        if(key != HAL_KEY_VALUE_CONFIG)
        {
            return ;
        }
    }
    else
    {
        if(key == HAL_KEY_VALUE_CONFIG)
        {
            return ;
        }
    }
    
    switch (g_timmingItemId)
    {
    case YDD_TIMING_ITEMID_YEAR:
        g_date.year = (uint16_t)calcValue(g_date.year, 2099, 2019, key == HAL_KEY_VALUE_UP, 1);
        timingItemValueUpdate(YDD_TIMING_ITEMID_YEAR, g_date.year);
    break;
    case YDD_TIMING_ITEMID_MONTH:
        g_date.month = (uint8_t)calcValue(g_date.month, 12, 1, key == HAL_KEY_VALUE_UP, 1);
        timingItemValueUpdate(YDD_TIMING_ITEMID_MONTH, g_date.month);
    break;
    case YDD_TIMING_ITEMID_DAY:
        g_date.day = (uint8_t)calcValue(g_date.day, dayInMonth(g_date.year, g_date.month), 1, key == HAL_KEY_VALUE_UP, 1);
        timingItemValueUpdate(YDD_TIMING_ITEMID_DAY, g_date.day);
    break;
    case YDD_TIMING_ITEMID_HOUR:
        g_date.hour = (uint8_t)calcValue(g_date.hour, 23, 0, key == HAL_KEY_VALUE_UP, 1);
        timingItemValueUpdate(YDD_TIMING_ITEMID_HOUR, g_date.hour);
    break;
    case YDD_TIMING_ITEMID_MINUTE:
        g_date.minute = (uint8_t)calcValue(g_date.minute, 59, 0, key == HAL_KEY_VALUE_UP, 1);
        timingItemValueUpdate(YDD_TIMING_ITEMID_MINUTE, g_date.minute);
    break;
    case YDD_TIMING_ITEMID_SECOND:
        g_date.second = (uint8_t)calcValue(g_date.second, 59, 0, key == HAL_KEY_VALUE_UP, 1);
         timingItemValueUpdate(YDD_TIMING_ITEMID_SECOND, g_date.second);
    break;
    case YDD_TIMING_ITEMID_SURE:
        SysDateTimeSet(&g_date);
        MenuSettingsShow();
    break;
    case YDD_TIMING_ITEMID_CANCLE:
        //return to setting menu
        MenuSettingsShow();
    break;
    default:
    break;
    }
}

static void timmingNextItem(void)
{
    uint8_t width = YDD_SELECT_TIMING_WIDTH / 2;
    
    if(g_timmingItemId == YDD_TIMING_ITEMID_YEAR || 
        g_timmingItemId == YDD_TIMING_ITEMID_SURE ||
        g_timmingItemId == YDD_TIMING_ITEMID_CANCLE)
    {
        width = YDD_SELECT_TIMING_WIDTH;
    }
    //unselect front
    DisplaySelectBox(false, &g_timingPos[g_timmingItemId++], width, YDD_SELECT_TIMING_HEIGTH);
    if(g_timmingItemId == YDD_TIMING_ITEMID_COUNT)
    {
        g_timmingItemId = YDD_TIMING_ITEMID_YEAR;
    }
    
    width = YDD_SELECT_TIMING_WIDTH / 2;
    if(g_timmingItemId == YDD_TIMING_ITEMID_YEAR || 
        g_timmingItemId == YDD_TIMING_ITEMID_SURE ||
        g_timmingItemId == YDD_TIMING_ITEMID_CANCLE)
    {
        width = YDD_SELECT_TIMING_WIDTH;
    }
    DisplaySelectBox(true, &g_timingPos[g_timmingItemId], width, YDD_SELECT_TIMING_HEIGTH); //select

}

static void timmingShow(void)
{
    char buff[5];
    uint8_t i = 0;
    g_date = *SysDateTime();
    
    DisplayPictureShow(SYS_PICTURE_DATE_ID);

    if(g_date.year < 2019)
    {
        g_date.year = 2019;
    }
    buff[0] = '\0';
    sprintf(buff, "%d", g_date.year);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_timingPos[i++], DISPLAY_CHAR_SIZE_NORMAL);
    buff[0] = '\0';
    sprintf(buff, "%02d", g_date.month);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_timingPos[i++], DISPLAY_CHAR_SIZE_NORMAL);
    buff[0] = '\0';
    sprintf(buff, "%02d", g_date.day);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_timingPos[i++], DISPLAY_CHAR_SIZE_NORMAL);
    buff[0] = '\0';
    sprintf(buff, "%02d", g_date.hour);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_timingPos[i++], DISPLAY_CHAR_SIZE_NORMAL);
    buff[0] = '\0';
    sprintf(buff, "%02d", g_date.minute);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_timingPos[i++], DISPLAY_CHAR_SIZE_NORMAL);
    buff[0] = '\0';
    sprintf(buff, "%02d", g_date.second);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_timingPos[i++], DISPLAY_CHAR_SIZE_NORMAL);

    DisplaySelectBox(true, &g_timingPos[YDD_TIMING_ITEMID_YEAR], YDD_SELECT_TIMING_WIDTH, YDD_SELECT_TIMING_HEIGTH);
    g_displayMenu = MSDIPLAY_MENU_TIMING;
    g_timmingItemId = YDD_TIMING_ITEMID_YEAR;
    DisplayDateTimeUpdate(); //时间显示
    DisplayPowerPercent();
}

static void selectNextMenuItem(void)
{
    //unselect front
    DisplaySelectBox(false, &g_settingsPos[g_settingItemId++], YDD_SELECT_SETTINGS_WIDTH, YDD_SELECT_SETTINGS_HEIGTH);
    if(g_settingItemId == YDD_SETTINGS_ITEMID_COUNT)
    {
        g_settingItemId = YDD_SETTINGS_ITEMID_CSSJ;
    }
    DisplaySelectBox(true, &g_settingsPos[g_settingItemId], YDD_SELECT_SETTINGS_WIDTH, YDD_SELECT_SETTINGS_HEIGTH); //select
}

static void updateItemValue(YDDSettingsItemID_t id, uint16_t value, char *str)
{
    SysDisplayPosition_t top, bottom;
    char buff[8] = "";
    
    top.x = g_settingsPos[id].x;
    top.y = g_settingsPos[id].y;
    if(str == NULL)
    {
        sprintf(buff, "%d", value);
    }
    else
    {
        sprintf(buff, "%s", str);
    }
    Syslog("ID=%d, value=%d", id, value);
    bottom.x = g_settingsPos[id].x + 12 * 4;
    bottom.y = g_settingsPos[id].y + 24;
    DisplayDrawRect(DISPLAY_COLOR_BOTTOM, &top, &bottom);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &top, DISPLAY_CHAR_SIZE_NORMAL);
}

static void itemSelectHandle(HalKeyValue_t key)
{
    uint8_t i, j;

    if(g_settingItemId == YDD_SETTINGS_ITEMID_BEEP || \
        g_settingItemId == YDD_SETTINGS_ITEMID_CLEAR || \
        g_settingItemId == YDD_SETTINGS_ITEMID_TIMING)
    {
        if(key != HAL_KEY_VALUE_CONFIG)
        {
            return ;
        }
    }
    else
    {
        if(key == HAL_KEY_VALUE_CONFIG)
        {
            return ;
        }
    }
    
    switch (g_settingItemId)
    {
    case YDD_SETTINGS_ITEMID_CSSJ:
        for(i = 0; i < MENU_SETTINGS_CSSJ_NUM; i++)
        {
            if(g_collectArgs.runTime == CSSJ[i])
            {
            /*
                if(key == HAL_KEY_VALUE_UP)
                {
                    j = i + 1;
                    if(j >= MENU_SETTINGS_CSSJ_NUM)
                    {
                        j = 0;
                    }
                }
                else
                {
                    if(i == 0)
                    {
                        j = MENU_SETTINGS_CSSJ_NUM - 1;
                    }
                    else
                    {
                        j = i - 1;
                    }
                }
                */
                j = (uint8_t)calcValue(i, MENU_SETTINGS_CSSJ_NUM - 1, 0, key == HAL_KEY_VALUE_UP, 1);
                g_collectArgs.runTime = CSSJ[j];
                updateItemValue(YDD_SETTINGS_ITEMID_CSSJ, g_collectArgs.runTime, NULL);
                break;
            }
        }
        break;
    case YDD_SETTINGS_ITEMID_QDBJ: //500~1500,step = 500
        g_collectArgs.intensityAlarm = (uint16_t)calcValue(g_collectArgs.intensityAlarm, 1500, 500, key == HAL_KEY_VALUE_UP, 500);
        updateItemValue(YDD_SETTINGS_ITEMID_QDBJ, g_collectArgs.intensityAlarm, NULL);
        break;
    case YDD_SETTINGS_ITEMID_XHFZ: //50~300,step=50
        g_collectArgs.signalThreshold = (uint16_t)calcValue(g_collectArgs.signalThreshold, 300, 50, key == HAL_KEY_VALUE_UP, 50);
        SysSignalThresholdSet(g_collectArgs.signalThreshold);
        updateItemValue(YDD_SETTINGS_ITEMID_XHFZ, g_collectArgs.signalThreshold, NULL);
        break;
    case YDD_SETTINGS_ITEMID_ZLBJ: //5, 20, 50, 80, 100, 120, 150, 180, 200K
        for(i = 0; i < MENU_SETTINGS_ZLBJ_NUM; i++)
        {
            if(g_collectArgs.ringAlarm == ZLBJ[i])
            {
                j = (uint8_t)calcValue(i, MENU_SETTINGS_ZLBJ_NUM - 1, 0, key == HAL_KEY_VALUE_UP, 1);
                g_collectArgs.ringAlarm = ZLBJ[j];
                updateItemValue(YDD_SETTINGS_ITEMID_ZLBJ, g_collectArgs.ringAlarm, NULL);
                break;
            }
        }
        break;
    case YDD_SETTINGS_ITEMID_BEEP:
        g_collectArgs.beep = !g_collectArgs.beep;
        SysBeepEnable(g_collectArgs.beep);
        updateItemValue(YDD_SETTINGS_ITEMID_BEEP, 0, g_collectArgs.beep ? "ON" : "OFF");
        break;
    case YDD_SETTINGS_ITEMID_BRIGHTNESS:
        g_collectArgs.brightness = (uint8_t)calcValue(g_collectArgs.brightness, 100, 5, key == HAL_KEY_VALUE_UP, 5);
        DisplayBrightnessSet(g_collectArgs.brightness);
        updateItemValue(YDD_SETTINGS_ITEMID_BRIGHTNESS, g_collectArgs.brightness, NULL);
        break;
    case YDD_SETTINGS_ITEMID_CLEAR:
        clearShow();
        break;
    case YDD_SETTINGS_ITEMID_TIMING:
        timmingShow();
        break;
    default:
        break;
    }
    //修改flash保存标志位
    if(g_settingItemId < YDD_SETTINGS_ITEMID_CLEAR)
    {
        SysDeviceArgsSet(&g_collectArgs);
    }
}

void MenuSettingsKeyHanlde(HalKeyValue_t key)
{
    if(key == HAL_KEY_VALUE_TOGGLE)
    {
        if(g_displayMenu == MSDIPLAY_MENU_MAIN)
        {
            selectNextMenuItem();
        }else if(g_displayMenu == MSDIPLAY_MENU_TIMING)
        {
            timmingNextItem();
        }
        else
        {
            clearNextItem();
        }
    }
    else if(key == HAL_KEY_VALUE_CONFIG || \
            key == HAL_KEY_VALUE_DOWN || \
            key == HAL_KEY_VALUE_UP)
    {
        if(g_displayMenu == MSDIPLAY_MENU_MAIN)
        {
            itemSelectHandle(key);
        }else if(g_displayMenu == MSDIPLAY_MENU_TIMING)
        {
            timmingItemSelectHandle(key);
        }
        else
        {
            clearItemSelectHandle(key);
        }
    }
    else
    {
    }
}

void MenuSettingsHide(void)
{
/*
    if(g_argsModified)
    {
        SysDeviceArgsSet(&g_collectArgs);
        g_argsModified = false;
    }
    */
}

void MenuSettingsShow(void)
{   
    char buff[10];
    uint8_t i = 0;

    /*背景图片*/
    DisplayPictureShow(SYS_PICTURE_SETTING_ID);

    /*测试时间*/
    SysDeviceArgsGet(&g_collectArgs);
    buff[0] = '\0';
    sprintf(buff, "%d", g_collectArgs.runTime);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_settingsPos[i++], DISPLAY_CHAR_SIZE_NORMAL);
    
    /*强度报警值*/
    buff[0] = '\0';
    sprintf(buff, "%d", g_collectArgs.intensityAlarm);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_settingsPos[i++], DISPLAY_CHAR_SIZE_NORMAL);
    
    /*信号阈值*/
    buff[0] = '\0';
    sprintf(buff, "%d", g_collectArgs.signalThreshold);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_settingsPos[i++], DISPLAY_CHAR_SIZE_NORMAL);
    
    /*振铃报警值*/
    buff[0] = '\0';
    sprintf(buff, "%d", g_collectArgs.ringAlarm);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_settingsPos[i++], DISPLAY_CHAR_SIZE_NORMAL);

    /*蜂鸣器*/
    buff[0] = '\0';
    sprintf(buff, "%s", g_collectArgs.beep ? "ON":"OFF");
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_settingsPos[i++], DISPLAY_CHAR_SIZE_NORMAL);

    /*亮度*/
    buff[0] = '\0';
    sprintf(buff, "%d", g_collectArgs.brightness);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_settingsPos[i++], DISPLAY_CHAR_SIZE_NORMAL);

    DisplaySelectBox(true, &g_settingsPos[YDD_SETTINGS_ITEMID_CSSJ], YDD_SELECT_SETTINGS_WIDTH, YDD_SELECT_SETTINGS_HEIGTH);
    g_settingItemId  = YDD_SETTINGS_ITEMID_CSSJ;
    g_displayMenu = MSDIPLAY_MENU_MAIN;

    DisplayDateTimeUpdate(); //时间显示
    DisplayPowerPercent();
}



