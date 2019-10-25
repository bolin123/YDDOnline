#include "Menu.h"
#include "Sys.h"
#include "DigitalLED.h"

#define MENU_TIMEOUT_COUNT 30000

typedef struct
{
    bool enable;
    uint8_t lightLevel;
    uint8_t pos;
    uint8_t value[DIGITAL_LED_ID_COUNT];
    uint16_t realValue;
    MenuID_t curMenuID;
    uint32_t lastTime;
}MenuFlashData_t;

static MenuItem_t g_menuItems[MENU_ID_COUNT];
static MenuFlashData_t g_flashData;
static bool g_menuActived = false;
static uint32_t g_activeTime;
static MenuEventHandle_cb g_eventHandle;

static void menuShowPoll(void)
{
    MenuFlashData_t *flash = &g_flashData;
    
    if(g_menuActived && flash->enable && SysTimeHasPast(flash->lastTime, 500))
    {
        flash->lightLevel = !flash->lightLevel;
        if(flash->lightLevel)
        {
            DigitalLEDSetChars((DigitalLEDId_t)flash->pos, flash->value[flash->pos], (flash->value[flash->pos] & 0x80) != 0);
        }
        else
        {
            DigitalLEDSetChars((DigitalLEDId_t)flash->pos, DIGITAL_FLAG_NONE, false);
        }
        flash->lastTime = SysTime();
    }
}

static void menuNext(void)
{
    //static MenuID_t curMenuID = MENU_ID_ADDR;;
    uint16_t value;
    uint8_t flag, i;
    MenuFlashData_t *flash = &g_flashData;
    MenuItem_t *item;
    
    if(g_menuActived)
    {
        do
        {
            flash->curMenuID++;
            if(flash->curMenuID >= MENU_ID_COUNT)
            {
                flash->curMenuID = MENU_ID_ADDR;
            }
        }
        while (!g_menuItems[flash->curMenuID].enable);
    }
    else
    {
        flash->curMenuID = MENU_ID_ADDR;
        g_menuActived = true;
        g_eventHandle(MENU_EVENT_ACTIVE);
    }
    item = &g_menuItems[flash->curMenuID];
    value = item->getValue(flash->curMenuID);
    flag = item->flag;

    flash->pos = DIGITAL_LED_ID_3; //初始时，个位数闪烁
    flash->value[DIGITAL_LED_ID_CMD] = flag;
    flash->value[DIGITAL_LED_ID_1] = value / 100;
    flash->value[DIGITAL_LED_ID_2] = value % 100 / 10;
    flash->value[DIGITAL_LED_ID_3] = value % 10;
    for(i = 0; i < DIGITAL_LED_ID_COUNT; i++)
    {
        DigitalLEDSetChars((DigitalLEDId_t)i, flash->value[i], (flash->value[i] & 0x80) != 0);
    }
    flash->realValue = value;
    flash->lastTime = SysTime();
    flash->lightLevel = 1;
    flash->enable = true;
}

static void menuValueSet(bool add)
{
    MenuFlashData_t *flash = &g_flashData;
    MenuItem_t *item = &g_menuItems[flash->curMenuID];
    uint8_t step = 1;

    switch (flash->pos)
    {
    case DIGITAL_LED_ID_1:
        step = 100;
    break;
    case DIGITAL_LED_ID_2:
        step = 10;
    break;
    case DIGITAL_LED_ID_3:
        step = 1;
    break;
    }

    if(add)
    {
        if(flash->realValue + step > item->max || flash->value[flash->pos] == 9)
        {
            return ;
        }
        else
        {
            flash->value[flash->pos]++;
        }
    }
    else
    {
        if(flash->value[flash->pos] == 0 || flash->realValue - step < item->min)
        {
            return;
        }
        else
        {
            flash->value[flash->pos]--;
        }
    }
    flash->realValue = flash->value[DIGITAL_LED_ID_1] * 100 + flash->value[DIGITAL_LED_ID_2] * 10 + flash->value[DIGITAL_LED_ID_3];
}

static void menuSelect(bool left)
{
    MenuFlashData_t *flash = &g_flashData;
    MenuItem_t *item = &g_menuItems[flash->curMenuID];
    
    DigitalLEDSetChars((DigitalLEDId_t)flash->pos, flash->value[flash->pos], (flash->value[flash->pos] & 0x80) != 0);
    if(left)
    {
        flash->pos--;
        if(flash->pos < item->digitNum)
        {
            flash->pos = DIGITAL_LED_ID_3;
        }
    }
    else
    {
        flash->pos++;
        if(flash->pos > DIGITAL_LED_ID_3)
        {
            flash->pos = item->digitNum;
        }
    }
    /*
    switch (flash->curMenuID)
    {
    case MENU_ID_ADDR:
        min = DIGITAL_LED_ID_1;
        break;
    case MENU_ID_RFCHN:
        min = DIGITAL_LED_ID_2;
        break;
    case MENU_ID_DEVTYPE:
        min = DIGITAL_LED_ID_3;
        break;
    default:
        break;
    }
    */
    
}

void MenuDeactive(void)
{
    g_menuActived = false;
}

void MenuKeyHandle(IRKey_t key)
{
    MenuFlashData_t *flash;
    MenuItem_t *item;

    if(!g_menuActived && key != IR_KEY_MENU)
    {
        return ;
    }
    
    switch (key)
    {
    case IR_KEY_MENU:
        menuNext();
        break;
    case IR_KEY_UP:
        menuValueSet(true);
        break;
    case IR_KEY_DOWN:
        menuValueSet(false);
        break;
    case IR_KEY_LEFT:
        menuSelect(true);
        break;
    case IR_KEY_RIGHT:
        menuSelect(false);
        break;
    case IR_KEY_ENTER:
        flash = &g_flashData;
        item = &g_menuItems[flash->curMenuID];
        item->setValue(flash->curMenuID, flash->realValue);
        DigitalLEDSetChars((DigitalLEDId_t)flash->pos, flash->value[flash->pos], (flash->value[flash->pos] & 0x80) != 0);
        flash->enable = false;
        break;
    case IR_KEY_CANCEL:
        flash = &g_flashData;
        DigitalLEDSetChars((DigitalLEDId_t)flash->pos, flash->value[flash->pos], (flash->value[flash->pos] & 0x80) != 0);
        //g_menuActived = false;
        flash->enable = false;
        g_eventHandle(MENU_EVENT_DEACTIVE);
        break;
    default:
        break;
    }
    g_activeTime = SysTime();
}

void MenuRegister(MenuID_t id, MenuItem_t *item)
{
    if(id < MENU_ID_COUNT && item)
    {
        g_menuItems[id] = *item;
        g_menuItems[id].enable = true;
    }
}

static void menuActiveTimePoll(void)
{
    if(g_menuActived && SysTimeHasPast(g_activeTime, MENU_TIMEOUT_COUNT))
    {
        g_eventHandle(MENU_EVENT_TIMEOUT);
        //g_menuActived = false;
    }
}

void MenuInit(MenuEventHandle_cb eventHandle)
{
    memset(g_menuItems, 0, sizeof(MenuItem_t) * MENU_ID_COUNT);
    g_eventHandle = eventHandle;
}

void MenuPoll(void)
{
    menuShowPoll();
    menuActiveTimePoll();
}

