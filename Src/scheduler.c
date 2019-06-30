#include "stdbool.h"
#include "stdlib.h"

#include "glove_status_codes.h"
#include "scheduler.h"
#include "queue.h"
#include "tasks.h"

#define SCHEDULER_CHECK_INIT()                  \
do                                              \
{                                               \
    if (!gContext.fInit)                        \
    {                                           \
        return GLOVE_STATUS_MODULE_NOT_INIT;    \
    }                                           \
}while(0);

typedef struct
{
    queue_t tasks;
    bool fInit;
} scheduler_context_t;

static scheduler_context_t gContext = {0};


glove_status_t Scheduler_Init()
{
    glove_status_t status = GLOVE_STATUS_OK;

    status = Queue_Init(&gContext.tasks, SCHEDULER_MAX_NUM_TASKS);
    CHECK_STATUS_OK_RET(status);

    gContext.fInit = true;
    return GLOVE_STATUS_OK;
}

glove_status_t Scheduler_AddTask(task_t * task)
{
    SCHEDULER_CHECK_INIT();

    return Queue_Enqueue(&(gContext.tasks), task);
}

glove_status_t Scheduler_Tick()
{
    uint32_t i = 0;
    glove_status_t status = GLOVE_STATUS_OK;
    task_t * task_to_run = NULL;

    SCHEDULER_CHECK_INIT();

    for (i = 0; i < gContext.tasks.size; ++i)
    {
        task_to_run = (task_t *) Queue_Dequeue(&(gContext.tasks));
        CHECK_STATUS_OK_RET(status);
        if (task_to_run && task_to_run->pTaskFn)
        {
            status = task_to_run->pTaskFn();
            if (GLOVE_STATUS_OK != status)
            {
                printf("Task %s failed!\r\n", task_to_run->name);
            }
        }
    }
    
    return GLOVE_STATUS_OK;
}
