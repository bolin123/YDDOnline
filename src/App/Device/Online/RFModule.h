#ifndef RFMODULE_H
#define RFMODULE_H

void RFModuleSendData(unsigned char *data, unsigned short len);
void RFModuleSleep(void);
void RFModuleWakeup(void);
void RFMoudleSetChannel(unsigned char chn);
unsigned char RFModuleDetected(void);
void RFModuleInit(void);
void RFModulePoll(void);
#endif

