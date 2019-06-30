#include "string.h"

#include "stm32l4xx_hal.h"

#include "glove_status_codes.h"
#include "LSM9DS1.h"
#include "scheduler.h"
#include "serial.h"
#include "tasks.h"
#include "TCA9548A.h"

#define NUM_IMUS 16

// task definitions
task_t IMU_Task = 
{
    .pTaskFn = &Task_ReadAllMotionSensors,
    .name = "IMU Task"
};

glove_status_t Task_ReadAllMotionSensors()
{
    uint8_t i = 0;
    glove_status_t status = GLOVE_STATUS_OK;
    motion_data_t motionData = {0};
    motion_data_t allMotionData[16] = {0};

    status = IMU_ReadAll(&motionData, NULL);
    CHECK_STATUS_OK_RET(status);

    // pretend we have 16 sensors by copying the data
    for (i = 0; i < NUM_IMUS; i++)
    {
        memcpy(&(allMotionData[i]), &motionData, sizeof(motion_data_t));
    }

    // transmit the data
    status = Serial_Write((uint8_t *)allMotionData, sizeof(allMotionData));
    CHECK_STATUS_OK_RET(status);
    
    // TODO: remove delay
    HAL_Delay(1500);

    // schedule the next sensor scan + transmission
    return Scheduler_AddTask(&IMU_Task);
}
