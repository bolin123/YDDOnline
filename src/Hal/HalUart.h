#ifndef HAL_UART_H
#define HAL_UART_H

#include "HalCtype.h"

typedef enum
{
    HAL_UART_PORT_1 = 0,
    HAL_UART_PORT_2 = 1,
    HAL_UART_PORT_3 = 2,
    HAL_UART_COUNT,
}HalUartPort_t;

typedef void (* HalUartDataRecv_cb)(uint8_t *data, uint16_t len);

typedef struct
{
    bool enble;
/*
    USART_Parity_No  
    USART_Parity_Even
    USART_Parity_Odd 
*/
    uint16_t parity;
/*
    USART_WordLength_8b
    USART_WordLength_9b
*/
    uint16_t wordLength;
/*
    USART_HardwareFlowControl_None   
    USART_HardwareFlowControl_RTS    
    USART_HardwareFlowControl_CTS    
    USART_HardwareFlowControl_RTS_CTS
*/
    uint16_t flowControl;

    uint32_t baudrate;
    HalUartDataRecv_cb recvCb;
    
}HalUartConfig_t;

void HalUartInitialize(void);
void HalUartPoll(void);
void HalUartConfig(uint8_t uart, HalUartConfig_t *config);
void HalUartWrite(uint8_t uart, const uint8_t *data, uint16_t len);

#endif

