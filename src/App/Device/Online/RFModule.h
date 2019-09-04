#ifndef RFMODULE_H
#define RFMODULE_H

typedef enum
{
    RFMODULE_EVENT_GET_RFCHNL,
}RFModuleEvent_t;

typedef void (*RFModuleEvent_cb)(RFModuleEvent_t event, void *arg);

void RFModuleSendData(unsigned char *data, unsigned short len);
void RFModuleSleep(void);
void RFModuleWakeup(void);
void RFMoudleSetChannel(unsigned char chn);
void RFModuleGetChannel(void);
unsigned char RFModuleDetected(void);
void RFModuleInit(RFModuleEvent_cb eventcb);
void RFModulePoll(void);
#endif

