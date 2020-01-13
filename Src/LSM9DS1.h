/*
 * LSM9DS1.h
 *
 *  Created on: May 10, 2019
 *      Author: tejasvi
 */

#ifndef LSM9DS1_H_
#define LSM9DS1_H_

#include "stdbool.h"

#include "glove_status_codes.h"
#include "scheduler.h"

#define IMU_AG_ADDR_BASE 0xD4
#define IMU_M_ADDR_BASE 0x38
#define IMU_GET_AG_ADDR(sad0_pin_value) (IMU_AG_ADDR_BASE | ((sad0_pin_value) << 1))
#define IMU_GET_M_ADDR(sad1_pin_value) (IMU_M_ADDR_BASE | ((sad1_pin_value) << 2))

typedef struct {
    bool fInit;
    I2C_HandleTypeDef *hi2c;
    uint32_t ag_addr;
    uint32_t m_addr;
    int16_t accOffsets[3];
    int16_t gyroOffsets[3];
    int16_t magOffsets[3];
} imu_t;

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


glove_status_t IMU_Init(imu_t * imu);
glove_status_t IMU_ReadAll(imu_t * imu, motion_data_t * motionData);
glove_status_t IMU_DumpConfigRegisters(imu_t * imu);

#endif /* LSM9DS1_H_ */
