#ifndef IR_H
#define IR_H

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
}IRKey_t;

typedef void (*IRKeyHandle_t)(IRKey_t key);

void IRSendData(unsigned char *data, unsigned char len);
void IRInit(IRKeyHandle_t handle);
void IRPoll(void);

#endif

