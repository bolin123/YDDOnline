#ifndef IR_DATA_TRANSFER_H
#define IR_DATA_TRANSFER_H

typedef enum
{
    IR_DTRANS_EVENT_PROCESS = 0,
    IR_DTRANS_EVENT_RESULT,
}IRDataTransferEvent_t;

typedef void (* IRDataTransferEventHandle_t)(IRDataTransferEvent_t event, unsigned int value);

unsigned char IRDataTransferIsStart(void);
unsigned char IRDataTransferStart(void);
void IRDataTransferStop(void);
void IRDataTransferResume(void);

void IRDataTransferInit(IRDataTransferEventHandle_t eventHandle);
void IRDataTransferPoll(void);

#endif


