#include "circular_queue.h"

circular_queue* create_circular_queue(size_t cq_size, size_t max_len) {
    circular_queue* obj = malloc(sizeof(circular_queue));
    obj->cq_size = cq_size;
    obj->cq_len = 0;
    obj->cq_max_len = max_len;
    obj->cq_pdata = malloc(cq_size * max_len);
    obj->head = obj->tail = 0;
    return obj;
}

void circular_queue_push(circular_queue* obj, void* memb) {
    if (obj->cq_len + 1 > obj->cq_max_len) return;
    void* tail_pointer = obj->cq_pdata + obj->tail * obj->cq_size;
    memcpy(tail_pointer, memb, obj->cq_size);
    obj->tail = (obj->tail + 1) % (obj->cq_max_len);
    obj->cq_len++;
}

void* circular_queue_pop(circular_queue* obj) {
    if (obj->cq_len == 0) return NULL;
    obj->cq_len--;
    void* head_pointer = obj->cq_pdata + obj->head * obj->cq_size;
    obj->head = (obj->head + 1) % (obj->cq_max_len);
    return head_pointer;
}

void* circular_queue_front(circular_queue* obj){
    if (obj->cq_len == 0) return NULL;
    void* head_pointer = obj->cq_pdata + obj->head * obj->cq_size;
    return head_pointer;
}