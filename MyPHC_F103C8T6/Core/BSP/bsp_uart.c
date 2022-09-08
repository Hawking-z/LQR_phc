#include "bsp_uart.h"

#include "bsp_log.h"
#include "circular_queue.h"
#include "cvector.h"
#define MAX_SEND_PACK 10
#define BSP_UART_DMA_BUFF_SIZE 255
#define DEVICE_UART_CNT 1

typedef struct BSP_UART_Send_Pack_t {
    uint8_t len;
    uint8_t buffer[MAX_SEND_DATA_LEN];
} BSP_UART_Send_Pack;

typedef struct BSP_UART_Typedef_t {
    UART_HandleTypeDef *port;  //自定义uart编号
    cvector *call_backs;
    circular_queue *send_packs;               //缓冲模式发送
    uint8_t queue_send_enable;                //使能队列缓冲DMA发送
    uint8_t rx_buff[BSP_UART_DMA_BUFF_SIZE];  // DMA接收数组，为了存放DMA搬运的串口数据
} BSP_UART_Typedef;

BSP_UART_Typedef uart_ports[DEVICE_UART_CNT];

void BSP_UART_Init() {
    uart_ports[0].port = &huart1;
    uart_ports[0].queue_send_enable = 0;
    
    for (size_t i = 0; i < DEVICE_UART_CNT; ++i) {
        uart_ports[i].call_backs = cvector_create(sizeof(uart_rx_callback));
        uart_ports[i].send_packs = create_circular_queue(sizeof(BSP_UART_Send_Pack), MAX_SEND_PACK);
        // HAL库的BUG处理，对于DMA需要先DeInit再Init，不然GG
        HAL_DMA_DeInit(uart_ports[i].port->hdmatx);
        HAL_DMA_DeInit(uart_ports[i].port->hdmarx);
        HAL_DMA_Init(uart_ports[i].port->hdmatx);
        HAL_DMA_Init(uart_ports[i].port->hdmarx);
        HAL_UART_DMAStop(uart_ports[i].port);
        //使能串口空闲中断
        __HAL_UART_ENABLE_IT(uart_ports[i].port, UART_IT_IDLE);  //使能串口空闲中断
        //开启DMA接收
        HAL_UART_Receive_DMA(uart_ports[i].port, uart_ports[i].rx_buff, BSP_UART_DMA_BUFF_SIZE);
    }
}

// 注册回调函数
void BSP_UART_RegisterRxCallback(uint8_t uart_index, uart_rx_callback func) { cvector_pushback(uart_ports[uart_index].call_backs, &func); }

//三种模式的发送函数(仅简单封装)
void BSP_UART_Send_blocking(uint8_t uart_index, uint8_t *data, uint16_t len) {
    if (uart_ports[uart_index].queue_send_enable) return;
    HAL_UART_Transmit(uart_ports[uart_index].port, data, len, 20);
}
void BSP_UART_Send_IT(uint8_t uart_index, uint8_t *data, uint16_t len) {
    if (uart_ports[uart_index].queue_send_enable) return;
    HAL_UART_Transmit_IT(uart_ports[uart_index].port, data, len);
}
void BSP_UART_Send_DMA(uint8_t uart_index, uint8_t *data, uint16_t len) {
    if (uart_ports[uart_index].queue_send_enable) return;
    HAL_UART_Transmit_DMA(uart_ports[uart_index].port, data, len);
}
void BSP_UART_Send_queue(uint8_t uart_index, uint8_t *data, uint16_t len) {
    if (len > MAX_SEND_DATA_LEN) return;
    uart_ports[uart_index].queue_send_enable = 1;
    if (uart_ports[uart_index].send_packs->cq_len == MAX_SEND_PACK) return;
    static BSP_UART_Send_Pack temp_pack;
    temp_pack.len = len;
    memcpy(temp_pack.buffer, data, len);
    circular_queue_push(uart_ports[uart_index].send_packs, &temp_pack);
    //第一次发送
    if (uart_ports[uart_index].send_packs->cq_len == 1) {
        BSP_UART_Send_Pack *now_pack = circular_queue_front(uart_ports[uart_index].send_packs);
        // printf_log("send len:%d\n",now_pack->len);
        HAL_UART_Transmit_IT(uart_ports[uart_index].port, now_pack->buffer, now_pack->len);
    }
}
// 空闲中断
void BSP_UART_IDLECallback(uint8_t uart_index, UART_HandleTypeDef *huart) {
    //判断是否是空闲中断
    if (RESET != __HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE)) {
        __HAL_UART_CLEAR_IDLEFLAG(huart);  //清除空闲中断标志（否则会一直不断进入中断）
        // 下面进行空闲中断相关处理
        HAL_UART_DMAStop(huart);                                                              //暂时停止本次DMA传输，进行数据处理
        uint8_t data_length = BSP_UART_DMA_BUFF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);  //计算接收到的数据长度
        //调用回调函数对数据进行处理
        for (size_t i = 0; i < uart_ports[uart_index].call_backs->cv_len; i++) {
            uart_rx_callback funcnow = *(uart_rx_callback *)cvector_val_at(uart_ports[uart_index].call_backs, i);
            funcnow(uart_index, uart_ports[uart_index].rx_buff, data_length);
        }
        HAL_UART_Receive_DMA(huart, uart_ports[uart_index].rx_buff, BSP_UART_DMA_BUFF_SIZE);  //重启开始DMA传输 每次255字节数据
    }
}

//发送完成回调函数
void BSP_UART_TxCpltCallback(uint8_t uart_index, UART_HandleTypeDef *huart) {
    // printf_log("tx cplt:%d\n", uart_index);
    if (!uart_ports[uart_index].queue_send_enable) return;
    // POP掉之前的数据
    if (uart_ports[uart_index].send_packs->cq_len > 0) {
        circular_queue_pop(uart_ports[uart_index].send_packs);
    }
    //还有没发送完的数据
    if (uart_ports[uart_index].send_packs->cq_len > 0) {
        BSP_UART_Send_Pack *now_pack = circular_queue_front(uart_ports[uart_index].send_packs);
        //再次发送
        //printf_log("queue send len:%d\n",now_pack->len);
        HAL_UART_Transmit_IT(uart_ports[uart_index].port, now_pack->buffer, now_pack->len);
    }
}

/**
 * @brief 串口空闲中断（中断回调）函数
 * @param 串口号
 * @retval None
 * @note  放在"stm32f4xx_it.c"里形如"void USART2_IRQHandler(void)"类的函数中，只要用了DMA接收的串口都放
 * 具体位置：系统调用的HAL_UART_IRQHandler函数下面，"USER CODE BEGIN USART1_IRQn 1"和"USER CODE END USART1_IRQn 1"两行注释之间
 */
void BSP_UART_IRQHandler(UART_HandleTypeDef *huart) {
    if (huart == uart_ports[0].port) {
        BSP_UART_IDLECallback(0, huart);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == uart_ports[0].port) {
        BSP_UART_TxCpltCallback(0, huart);
    }
}