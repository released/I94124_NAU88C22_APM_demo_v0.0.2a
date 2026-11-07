#ifndef __HWH_UART_H__
#define __HWH_UART_H__
#include "Platform.h"

#define UART_TT     6   //uart数据每帧之间有时间间隔，         /////modify 20170804
#define UART_TX_DA     50                                       /////modify 20170804
#define UART_RX_DA     1024 

typedef struct      //uart中断收发数据结构                     /////modify 20170804
{
    uint16_t uart_tx_len;
    uint8_t uart_tx_data[UART_TX_DA];
    uint16_t uart_rx_len;
    uint8_t uart_rx_data[UART_RX_DA];
    uint16_t uart_tx_per;
    uint16_t uart_rx_time;
    uint8_t record_ser;
    uint8_t voice_ch;
} uart_type;

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////2019年8月30日//////////////////////////////////

//#define AWE_FRAME_SIZE          40
//#define OUTPUT_CHANNEL_COUNT    2


extern uart_type ap_uart0;
void uart_time_task(void);
void UART_int_send(uint8_t *dara, uint16_t len);
void uart0_rev_task(void);



#endif
