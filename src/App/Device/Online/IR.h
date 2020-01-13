#ifndef IR_H
#define IR_H

#define IR_SEND_PACKET_LEN 4

typedef enum
{
    IR_KEY_INVALID = 0,
    IR_KEY_MENU,
    IR_KEY_UP,
    IR_KEY_DOWN,
    IR_KEY_LEFT,
    IR_KEY_RIGHT,
    IR_KEY_ENTER,
    IR_KEY_CANCEL,
    IR_KEY_READDATA,
    IR_KEY_READDATA_ACK,
    IR_KEY_READDATA_DATA,
}IRKey_t;

typedef void (*IRKeyHandle_t)(unsigned char key, unsigned char *contents);

void IRSendData(IRKey_t key, unsigned char pid, unsigned char data[IR_SEND_PACKET_LEN]);

void IRInit(IRKeyHandle_t handle);
void IRPoll(void);

#endif

