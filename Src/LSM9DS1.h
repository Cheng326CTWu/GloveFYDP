/*
 * LSM9DS1.h
 *
 *  Created on: May 10, 2019
 *      Author: tejasvi
 */

#ifndef LSM9DS1_H_
#define LSM9DS1_H_

#include "glove_status_codes.h"
#include "scheduler.h"

typedef struct
{
    int16_t xAcc;
    int16_t yAcc;
    int16_t zAcc;

    int16_t xGyro;
    int16_t yGyro;
    int16_t zGyro;

    int16_t xMag;
    int16_t yMag;
    int16_t zMag;
} motion_data_t;


glove_status_t IMU_Init(I2C_HandleTypeDef * hi2c);
glove_status_t IMU_ReadAll(motion_data_t * motionData);
glove_status_t IMU_DumpConfigRegisters();
glove_status_t IMU_StartContinuousRead();
glove_status_t IMU_StopContinuousRead();

extern task_t Task_IMUSweep;
extern task_t Task_AckTransferStopped;

#endif /* LSM9DS1_H_ */
