#include "glove_status_codes.h"
#include "scheduler.h"

extern task_t Task_IMUSweep;
extern task_t Task_AckTransferStopped;


glove_status_t Hand_Init(I2C_HandleTypeDef * hi2c);
glove_status_t Hand_StartContinuousRead();
glove_status_t IMU_StopContinuousRead();
