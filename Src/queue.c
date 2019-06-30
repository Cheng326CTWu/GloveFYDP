/**
 * Circular queue implementation.
 */

#include "stdio.h"
#include "stdlib.h"

#include "glove_status_codes.h"
#include "queue.h"

#define QUEUE_CHECK_INIT(q)                         \
do                                                  \
{                                                   \
    if (!( (q)->fInit ) )                           \
    {                                               \
        return GLOVE_STATUS_MODULE_NOT_INIT;        \
    }                                               \
}while(0);

glove_status_t Queue_Init(queue_t * queue, uint8_t size)
{
    if (!queue)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    if (size < 1 || size > QUEUE_MAX_SIZE)
    {
        return GLOVE_STATUS_INVALID_ARGUMENT;
    }

    queue->head = 0;
    queue->tail = -1;
    queue->size = 0;
    queue->maxSize = size;
    queue->fInit = true;

    return GLOVE_STATUS_OK;
}

glove_status_t Queue_Enqueue(queue_t * queue, void * item)
{
    if (!queue)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    QUEUE_CHECK_INIT(queue);

    queue->tail = (queue->tail + 1) % queue->maxSize;
    queue->items[queue->tail] = item;

    // if tail has run into head circularly, increment head
    if (queue->size > 0 && queue->head == queue->tail)
    {
        queue->head = (queue->head + 1) % queue->maxSize;
    }

    // increment up to maxSize
    if (queue->size < queue->maxSize)
    {
        ++(queue->size);
    }
    
    // printf("%s head=%d, tail=%d, size=%d *item=%d\r\n", 
    //         __FUNCTION__, 
    //         queue->head,
    //         queue->tail,
    //         queue->size,
    //         item ? *(uint32_t * )(item) : 999);

    return GLOVE_STATUS_OK;
}

void * Queue_Dequeue(queue_t * queue)
{
    void * item = NULL;
    
    if (!queue)
    {
        printf("%s:%d: null queue!\r\n", __FUNCTION__, __LINE__);
        return item;
    }

    QUEUE_CHECK_INIT(queue);

    // dequeue item if there is one available, otherwise leave it as null
    if (queue->size > 0)
    {
        item = queue->items[queue->head];
        queue->head = (queue->head + 1) % queue->maxSize;
        --(queue->size);
    }

    // if head has run into tail, increment tail
    if (queue->size > 0 && queue->head == queue->tail)
    {
        queue->tail = (queue->tail + 1) % queue->maxSize;
    }

    // printf("%s head=%d, tail=%d, size=%d, *head=%d\r\n", 
    //     __FUNCTION__,
    //     queue->head,
    //     queue->tail,
    //     queue->size,
    //     queue->items[queue->head] ? *(uint32_t *)(queue->items[queue->head]) : 999);

    return item;
}

// glove_status_t Queue_Print(queue_t * queue)
// {
//     if (!queue)
//     {
//         return GLOVE_STATUS_NULL_PTR;
//     }

//     for (uint8_t i = queue->head; i < queue->size; i = (i + 1) % queue->maxSize)
//     {

//     }
//     printf("\r\n");

//     return GLOVE_STATUS_OK;
// }
