#ifndef _BSP_UART_H
#define _BSP_UART_H
#include "stdint.h"
#include "usart.h"
#define MAX_SEND_DATA_LEN 256

typedef void (*uart_rx_callback)(uint8_t uart_index, uint8_t *data, uint32_t len);

void BSP_UART_Init();
void BSP_UART_RegisterRxCallback(uint8_t uart_index, uart_rx_callback func);

void BSP_UART_Send_blocking(uint8_t uart_index, uint8_t *data, uint16_t len);
void BSP_UART_Send_IT(uint8_t uart_index, uint8_t *data, uint16_t len);
void BSP_UART_Send_DMA(uint8_t uart_index, uint8_t *data, uint16_t len);
//请注意！使用Send_queue之后，其它所有的send模式都将失效！！
void BSP_UART_Send_queue(uint8_t uart_index, uint8_t *data, uint16_t len);
// DMA接收处理
void BSP_UART_IRQHandler(UART_HandleTypeDef *huart);
// DMA发送处理
void BSP_UART_DMAHandler(DMA_HandleTypeDef* dma);
#endif