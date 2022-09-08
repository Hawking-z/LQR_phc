#include "bluetooth.h"
#include "string.h"

bt *instance;

void BlueTooth_RxCallBack(uint8_t uart_index, uint8_t *data, uint32_t len);
void BT_FeedbackData_Update(bt *obj, uint8_t *data, uint32_t len);

void BlueTooth_Init(bt *obj, bt_config *config)
{
    obj->bsp_uart_index = config->bsp_uart_index;
    obj->rxdata.rx_len = config->rx_len;
    instance = obj;
    BSP_UART_RegisterRxCallback(obj->bsp_uart_index, BlueTooth_RxCallBack);
}

void BlueTooth_RxCallBack(uint8_t uart_index, uint8_t *data, uint32_t len)
{
    BT_FeedbackData_Update(instance, data, len);
}

uint8_t lll;
void BT_FeedbackData_Update(bt *obj, uint8_t *data, uint32_t len)
{
    lll = len;
    obj->rxdata.data_update = 1;
    obj->rxdata.crc = 0;
    if (data[0] != 0xA5 || data[len - 1] != 0x5A || len != obj->rxdata.rx_len)
        return;
    obj->rxdata.cd.stop = data[1] & 0x01;
    obj->rxdata.cd.reset = (data[1] & 0x02)>>1;
    //obj->rxdata.cd.byte = data[2];
    obj->rxdata.cd.ch0 = (((int)data[2])) | (((int)data[3]) << 8) | (((int)data[4]) << 16) | (((int)data[5]) << 24);
    obj->rxdata.cd.ch1 = (((int)data[6])) | (((int)data[7]) << 8) | (((int)data[8]) << 16) | (((int)data[9]) << 24);
    obj->rxdata.cd.ch2 = (((int)data[10])) | (((int)data[11]) << 8) | (((int)data[12]) << 16) | (((int)data[13]) << 24);
    obj->rxdata.cd.ch3 = (((int)data[14])) | (((int)data[15]) << 8) | (((int)data[16]) << 16) | (((int)data[17]) << 24);
    for (uint8_t i = 1; i < len - 2; i++)
    {
        obj->rxdata.crc += data[i];
    }
    if (obj->rxdata.crc != data[len-2])
        obj->rxdata.correct = 0;
    else
        obj->rxdata.correct = 1;
}
