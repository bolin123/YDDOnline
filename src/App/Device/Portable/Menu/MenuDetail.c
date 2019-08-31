#include "MenuDetail.h"
#include "Display.h"
#include "DisplayCoordinate.h"

void MenuDetailHide(void)
{
    //do nothing
}

void MenuDetailKeyHanlde(HalKeyValue_t key)
{
    //do nothing
}

void MenuDetailShow(void)
{
    char buff[10];
    uint8_t i = 0;
    
    DisplayPictureShow(SYS_PICTURE_DETAILS_ID);

    /*�����*/
    SysDataRecord_t record;
    SysArgsGetRecord(&record);
    buff[0] = '\0';
    sprintf(buff, "%d", record.num);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_detailsPos[i++], DISPLAY_CHAR_SIZE_NORMAL);
    
    /*�ź���ֵ*/
    SysDeviceArgs_t args;
    SysDeviceArgsGet(&args);
    buff[0] = '\0';
    sprintf(buff, "%d", args.signalThreshold);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_detailsPos[i++], DISPLAY_CHAR_SIZE_NORMAL);

    /*����ʱ��*/
    buff[0] = '\0';
    sprintf(buff, "%d", args.runTime);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_detailsPos[i++], DISPLAY_CHAR_SIZE_NORMAL);

    /*ǿ�ȱ���ֵ*/
    buff[0] = '\0';
    sprintf(buff, "%d", args.intensityAlarm);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_detailsPos[i++], DISPLAY_CHAR_SIZE_NORMAL);

    /*������*/
    buff[0] = '\0';
    sprintf(buff, "%s", args.beep ? "ON":"OFF");
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_detailsPos[i++], DISPLAY_CHAR_SIZE_NORMAL);
    
    /*���屨��ֵ*/
    buff[0] = '\0';
    sprintf(buff, "%d", args.ringAlarm);
    DiplayStringPrint(buff, strlen(buff), DISPLAY_COLOR_BLACK, &g_detailsPos[i++], DISPLAY_CHAR_SIZE_NORMAL);

    DisplayDateTimeUpdate(); //ʱ����ʾ
    DisplayPowerPercent();
}


