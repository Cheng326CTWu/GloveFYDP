/**
 * FIFO scheduler
 */

#ifndef SCHEDULER_H
#define SCHEDULE_H

#include "glove_status_codes.h"

#define SCHEDULER_MAX_NUM_TASKS (QUEUE_MAX_SIZE)

typedef void (*task_t)(void);

glove_status_t Scheduler_Init();
glove_status_t Scheduler_AddTask(task_t task);
glove_status_t Scheduler_Tick();

#endif
