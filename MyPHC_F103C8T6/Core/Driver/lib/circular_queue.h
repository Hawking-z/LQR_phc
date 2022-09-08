#ifndef _CIRCULAR_QUEUE_H
#define _CIRCULAR_QUEUE_H

#include "stdint.h"
#include "stdlib.h"
#include "string.h"
/*
cq_len:队列中元素个数
cq_max_len:队列最多能容纳的元素个数
cq_size:单个元素大小(字节)
*/
typedef struct circular_queue_t {
    void* cq_pdata;
    size_t cq_size, cq_len, cq_max_len;
    size_t head, tail;
} circular_queue;

circular_queue* create_circular_queue(size_t cq_size, size_t max_len);

void circular_queue_push(circular_queue* obj, void* memb);
void* circular_queue_pop(circular_queue* obj);
void* circular_queue_front(circular_queue* obj);
#endif