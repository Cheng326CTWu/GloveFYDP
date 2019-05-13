/*
 * LSM9DS1.h
 *
 *  Created on: May 10, 2019
 *      Author: tejasvi
 */

#ifndef LSM9DS1_H_
#define LSM9DS1_H_

#include "glove_status_codes.h"

typedef struct
{
    float xAcc;
    float yAcc;
    float zAcc;

    float xGryo;
    float yGryo;
    float zGryo;

    float xMag;
    float yMag;
    float zMag;
} motion_data_t;

glove_status_t IMU_Init(I2C_HandleTypeDef * hi2c);
glove_status_t IMU_ReadAll(motion_data_t * motionData);
glove_status_t IMU_DumpConfigRegisters();

#endif /* LSM9DS1_H_ */
