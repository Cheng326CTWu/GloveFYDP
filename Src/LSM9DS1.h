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
    uint16_t xAcc;
    uint16_t yAcc;
    uint16_t zAcc;

    uint16_t xGyro;
    uint16_t yGyro;
    uint16_t zGyro;

    uint16_t xMag;
    uint16_t yMag;
    uint16_t zMag;
} motion_data_t;


glove_status_t IMU_Init(I2C_HandleTypeDef * hi2c);
glove_status_t IMU_ReadAll(motion_data_t * motionData);
glove_status_t IMU_DumpConfigRegisters();
glove_status_t IMU_StartContinuousRead();
glove_status_t IMU_StopContinuousRead();

extern task_t Task_IMUSweep;
extern task_t Task_AckTransferStopped;

#endif /* LSM9DS1_H_ */
