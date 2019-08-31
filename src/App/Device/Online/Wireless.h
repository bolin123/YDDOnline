#ifndef WIRELESS_H
#define WIRELESS_H

#include "Sys.h"

typedef enum
{
    WIRELESS_EVENT_QUERY,
}WirelessEvent_t;

typedef void (*WirelessDataEvent_cb)(WirelessEvent_t event, void *args);

void WirelessReportData(uint8_t err, uint8_t power, uint16_t *data, uint16_t dataNum);
void WirelessDataParse(char *data);
void WirelessSetChannel(uint8_t chn);
void WirelessInit(WirelessDataEvent_cb eventHandle);
void WirelessPoll(void);

#endif


