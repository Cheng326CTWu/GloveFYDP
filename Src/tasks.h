#ifndef TASKS_H
#define TASKS_H

typedef struct
{
    glove_status_t (*pTaskFn)(void);
    char * name;
} task_t;

glove_status_t Task_ReadAllMotionSensors();

extern task_t IMU_Task;

#endif
