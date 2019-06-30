#ifndef QUEUE_T
#define QUEUE_T

#include "stdlib.h"
#include "stdbool.h"

#include "glove_status_codes.h"

#define QUEUE_MAX_SIZE 32

typedef struct
{
    void * items[QUEUE_MAX_SIZE];
    int8_t head;
    int8_t tail;
    uint8_t maxSize;
    uint8_t size;
    bool fInit;
} queue_t;

glove_status_t Queue_Init(queue_t * queue, uint8_t size);
glove_status_t Queue_Enqueue(queue_t * queue, void * item);
void * Queue_Dequeue(queue_t * queue);
glove_status_t Queue_Print(queue_t * queue);

#endif
