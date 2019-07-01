/**
 * FIFO scheduler
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "glove_status_codes.h"

#define SCHEDULER_MAX_NUM_TASKS (QUEUE_MAX_SIZE)

typedef struct
{
    glove_status_t (*pTaskFn)(void);
    char * name;
} task_t;

glove_status_t Scheduler_Init();
glove_status_t Scheduler_AddTask(task_t * task);
glove_status_t Scheduler_RemoveTask(task_t * task);
glove_status_t Scheduler_Tick();

#endif
