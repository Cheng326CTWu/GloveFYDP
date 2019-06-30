/**
 * FIFO scheduler
 */

#ifndef SCHEDULER_H
#define SCHEDULE_H

#include "glove_status_codes.h"
#include "tasks.h"

#define SCHEDULER_MAX_NUM_TASKS (QUEUE_MAX_SIZE)

glove_status_t Scheduler_Init();
glove_status_t Scheduler_AddTask(task_t * task);
glove_status_t Scheduler_Tick();

#endif
