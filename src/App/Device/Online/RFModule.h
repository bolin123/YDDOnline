#ifndef RFMODULE_H
#define RFMODULE_H

typedef enum
{
    RFMODULE_EVENT_GET_RFCHNL,
    RFMODULE_EVENT_GET_WAITTIME,
}RFModuleEvent_t;

typedef void (*RFModuleEvent_cb)(RFModuleEvent_t event, void *arg);

void RFModuleSendData(unsigned char *data, unsigned short len);
void RFModuleSleep(void);
void RFModuleWakeup(void);
void RFMoudleSetChannel(unsigned char chn);
void RFModuleGetChannel(void);
void RFModuleGetWaitTimes(void);
void RFModuleSetWaitTimes(unsigned int time);

unsigned char RFModuleDetected(void);
void RFModuleInit(RFModuleEvent_cb eventcb);
void RFModulePoll(void);
#endif

