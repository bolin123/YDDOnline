#include "MenuContact.h"
#include "Display.h"

void MenuContactHide(void)
{
}

void MenuContactKeyHanlde(HalKeyValue_t key)
{
    //do nothing
}

void MenuContactShow(void)
{
    DisplayPictureShow(SYS_PICTURE_CONTACT_ID);
    DisplayDateTimeUpdate(); // ±º‰œ‘ æ
    DisplayPowerPercent();
}

