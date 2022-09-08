#ifndef __H_BLUETOOTH_H__
#define __H_BLUETOOTH_H__

#include "bsp_uart.h"


typedef struct command_data_t
{
    uint8_t stop;
    uint8_t reset;
    //-127-127
    int ch0;
    int ch1;
    int ch2;
    int ch3;
}command_data;


typedef struct RxData_a
{
    uint16_t rx_len;
    uint8_t correct;
    uint8_t data_update;
    uint8_t crc;
    command_data cd;
}RxData;

typedef struct bt_config_t
{
    uint8_t bsp_uart_index;
    uint8_t rx_len;
}bt_config;

typedef struct bluetooth
{
    uint8_t bsp_uart_index;
    RxData rxdata;
    uint8_t isConnect;

}bt;

void BlueTooth_Init(bt *obj, bt_config* config);

#endif