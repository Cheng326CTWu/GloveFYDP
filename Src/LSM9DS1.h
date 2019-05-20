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
} motion_data_float_t;

typedef struct
{
    uint16_t xAcc;
    uint16_t yAcc;
    uint16_t zAcc;

    uint16_t xGryo;
    uint16_t yGryo;
    uint16_t zGryo;

    uint16_t xMag;
    uint16_t yMag;
    uint16_t zMag;
} motion_data_t;


glove_status_t IMU_Init(I2C_HandleTypeDef * hi2c);
glove_status_t IMU_ReadAll(motion_data_t * motionData, motion_data_float_t * motionDataFloat);
glove_status_t IMU_DumpConfigRegisters();

#endif /* LSM9DS1_H_ */
