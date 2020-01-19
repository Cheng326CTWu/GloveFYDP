/*
 * LSM9DS1.c
 *
 *  Created on: May 10, 2019
 *      Author: tejasvi
 * 
 * TODO
 * change anti aliasing filter to 105 Hz
 * copy calibration routine from IMU2
 * change sample period of filter to be 2*odr period
 * try using FIFO with a high ODR, and reading multiple FIFO values, then transmitting those
 */

#include "stdint.h"
#include "stdbool.h"

#include "stm32l4xx_hal.h"

#include "glove_status_codes.h"
#include "LSM9DS1.h"
#include "LSM9DS1_Reg.h"
#include "scheduler.h"
#include "serial.h"

#define IMU_I2C_TIMEOUT 100
#define NUM_IMUS 16

#define IMU_CHECK_INIT(imu)                         \
do                                                  \
{                                                   \
    if (!((imu)->fInit))                            \
    {                                               \
        return GLOVE_STATUS_MODULE_NOT_INIT;        \
    }                                               \
} while (0);

// forward declarations
static glove_status_t IMU_SelfTest(imu_t * imu);
static glove_status_t IMU_Calibrate(imu_t * imu, int16_t * caliAcc, int16_t * caliGyro, int16_t * caliMag);
static glove_status_t IMU_SetRegBits(imu_t * imu, uint8_t baseAddress, uint8_t regAddress, uint8_t mask, uint8_t values);
static glove_status_t IMU_ReadReg(imu_t * imu, uint8_t baseAddress, uint8_t regAddress, uint8_t *readvalue);
static glove_status_t IMU_WriteReg(imu_t * imu, uint8_t baseAddress, uint8_t regAddress, uint8_t value);
static glove_status_t IMU_Reset(imu_t * imu);
static bool are_addresses_valid(imu_t * imu);

glove_status_t IMU_Init(imu_t * imu)
{
    glove_status_t status = GLOVE_STATUS_OK;

    if (!imu)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    // check that address is valid
    if (!are_addresses_valid(imu))
    {
        // TODO: use a different status code here
        return GLOVE_STATUS_INVALID_ARGUMENT;
    }

    // // reset device
    status = IMU_Reset(imu);
    CHECK_STATUS_OK_RET(status);

    // enable the 3-axes of the gyroscope
    status = IMU_WriteReg(imu, imu->ag_addr, CTRL_REG4, 0x38);
    CHECK_STATUS_OK_RET(status);

    // configure the gyroscope
    status = IMU_WriteReg(imu, imu->ag_addr, CTRL_REG1_G, 2 << 5 | 0 << 3 | 0);
    CHECK_STATUS_OK_RET(status);
    HAL_Delay(200);

    // enable the three axes of the accelerometer 
    status = IMU_WriteReg(imu, imu->ag_addr, CTRL_REG5_XL, 0x38);
    CHECK_STATUS_OK_RET(status);

    // configure the accelerometer-specify bandwidth selection with Abw
    status = IMU_WriteReg(imu, imu->ag_addr, CTRL_REG6_XL, 2 << 5 | 0 << 3);
    CHECK_STATUS_OK_RET(status);

    HAL_Delay(200);

    // // set accel ODR to 50 Hz
    // status = IMU_SetRegBits(imu, imu->ag_addr, CTRL_REG6_XL, CTRL_REG6_XL_ODR_MASK, CTRL_REG6_XL_ODR_50);
    // CHECK_STATUS_OK_RET(status);

    // // set accel anti-aliasing filter bandwidth to 105 Hz
    // status = IMU_SetRegBits(imu, imu->ag_addr, CTRL_REG6_XL, CTRL_REG6_XL_AA_BW_MASK, CTRL_REG6_XL_AA_BW_105);
    // CHECK_STATUS_OK_RET(status);

    // // set gyro ODR to 59.5 Hz
    // status = IMU_SetRegBits(imu, imu->ag_addr, CTRL_REG1_G, CTRL_REG1_G_ODR_MASK, CTRL_REG1_G_ODR_59p5);
    // CHECK_STATUS_OK_RET(status);

    // configure the magnetometer, copied from:
    // https://github.com/kriswiner/LSM9DS1/blob/master/LSM9DS1_MS5611_BasicAHRS_t3.ino
    status = IMU_WriteReg(imu, imu->m_addr, CTRL_REG1_M, 0x80 | 2 << 5 | 4 << 2); // select x,y-axis mode
    CHECK_STATUS_OK_RET(status);

    status = IMU_WriteReg(imu, imu->m_addr, CTRL_REG2_M, 0 << 5 ); // select mag full scale
    CHECK_STATUS_OK_RET(status);

    status = IMU_WriteReg(imu, imu->m_addr, CTRL_REG3_M, 0x00 ); // continuous conversion mode
    CHECK_STATUS_OK_RET(status);
    
    status = IMU_WriteReg(imu, imu->m_addr, CTRL_REG4_M, 2 << 2 ); // select z-axis mode
    CHECK_STATUS_OK_RET(status);

    // // set magnetometer ODR to 40 Hz
    // status = IMU_SetRegBits(imu, imu->m_addr, CTRL_REG1_M, CTRL_REG1_M_ODR_MASK, CTRL_REG1_M_ODR_40);
    // CHECK_STATUS_OK_RET(status);
    
    // // self test
    // status = IMU_SelfTest();
    // CHECK_STATUS_OK_RET(status);

    // calibrate - for some reason this delay helps get steadier numbers
    // HAL_Delay(2000);
    // status = IMU_Calibrate(imu->accOffsets, imu->gyroOffsets, imu->magOffsets);
    // CHECK_STATUS_OK_RET(status);

    imu->fInit = true;

    return GLOVE_STATUS_OK;
}

static glove_status_t IMU_SelfTest(imu_t * imu)
{
    glove_status_t status = GLOVE_STATUS_OK;
    int16_t acc[3] = {0};
    int16_t gyro[3] = {0};
    int16_t mag[3] = {0};
    int16_t accNoTest[3] = {0};
    int16_t gyroNoTest[3] = {0};
    int16_t magNoTest[3] = {0};

    if (!imu)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    // disable self-test
    status = IMU_WriteReg(imu, imu->ag_addr, CTRL_REG10, CTRL_REG10_DISABLE_SELF_TEST);
    CHECK_STATUS_OK_RET(status);

    // get the calibration values without self-test
    status = IMU_Calibrate(imu, accNoTest, gyroNoTest, magNoTest);
    CHECK_STATUS_OK_RET(status);

    // enable self-test
    status = IMU_WriteReg(imu, imu->ag_addr, CTRL_REG10, CTRL_REG10_ENABLE_SELF_TEST);
    CHECK_STATUS_OK_RET(status);

    // get the calibration values, they are printed out in the calibration function
    status = IMU_Calibrate(imu, acc, gyro, mag);
    CHECK_STATUS_OK_RET(status);

    // disable self-test
    status = IMU_WriteReg(imu, imu->ag_addr, CTRL_REG10, CTRL_REG10_DISABLE_SELF_TEST);
    CHECK_STATUS_OK_RET(status);

    // print the difference in values, copying from 
    // https://github.com/kriswiner/LSM9DS1/blob/7dfa09fe141c9c206e742d666eb594d512615c40/LSM9DS1_MS5611_BasicAHRS_t3.ino#L729
    printf("Gyro results should be between 20 and 250 dps\r\n");
    printf("x: %.3f,    y: %.3f    z:%.3f\r\n", 
            SENS_GYROSCOPE_245 * (gyro[0] - gyroNoTest[0]),
            SENS_GYROSCOPE_245 * (gyro[1] - gyroNoTest[1]),
            SENS_GYROSCOPE_245 * (gyro[2] - gyroNoTest[2])
          );
    
    printf("Acc results should be between 0.6 and 1.7 g\r\n");
    printf("x: %.3f,    y: %.3f    z:%.3f\r\n", 
            SENS_ACCELEROMETER_2 * (acc[0] - accNoTest[0]),
            SENS_ACCELEROMETER_2 * (acc[1] - accNoTest[1]),
            SENS_ACCELEROMETER_2 * (acc[2] - accNoTest[2])
          );

    return GLOVE_STATUS_OK;
}

static glove_status_t IMU_Calibrate(imu_t * imu, int16_t * caliAcc, int16_t * caliGyro, int16_t * caliMag)
{
    // read each sensor for one second and average the data 
    uint32_t start = HAL_GetTick();
    glove_status_t status = GLOVE_STATUS_OK;
    motion_data_t data = {0};
    int32_t accSums[3] = {0};
    int32_t gyroSums[3] = {0};
    int32_t magSums[3] = {0};
    uint32_t count = 0;

    if (!imu)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    while (HAL_GetTick() - start < 1000)
    {
        status = IMU_ReadAll(imu, &data);
        CHECK_STATUS_OK_RET(status);

        accSums[0] += data.xAcc;
        accSums[1] += data.yAcc;
        accSums[2] += data.zAcc;

        gyroSums[0] += data.xGyro;
        gyroSums[1] += data.yGyro;
        gyroSums[2] += data.zGyro;

        magSums[0] += data.xMag;
        magSums[1] += data.yMag;
        magSums[2] += data.zMag;

        count += 1;
        HAL_Delay(20);
    }

    for (uint8_t i = 0; i < 3; ++i)
    {
        caliAcc[i] = accSums[i] / count;
        caliGyro[i] = gyroSums[i] / count;
        caliMag[i] = magSums[i] / count;
    }

    printf("Calibrations:\r\n");
    printf("acc: %.3f, %.3f, %.3f\r\n", caliAcc[0]*SENS_ACCELEROMETER_2, caliAcc[1]*SENS_ACCELEROMETER_2, caliAcc[2]*SENS_ACCELEROMETER_2);
    printf("gyro: %.3f, %.3f, %.3f\r\n", caliGyro[0]*SENS_GYROSCOPE_245, caliGyro[1]*SENS_GYROSCOPE_245, caliGyro[2]*SENS_GYROSCOPE_245);
    printf("mag: %d, %d, %d\r\n", caliMag[0], caliMag[1], caliMag[2]);

    return GLOVE_STATUS_OK;
}

glove_status_t IMU_ReadAll(imu_t * imu, motion_data_t * motionData)
{
    HAL_StatusTypeDef halStatus = HAL_OK;
    int16_t gyroAndAccelData[6] = {0};
    int16_t magData[3] = {0};

    // read gyro and accel data
    // printf("%s read from address 0x%X 0x%X\r\n", __FUNCTION__, imu->ag_addr, imu->m_addr);
    halStatus = HAL_I2C_Mem_Read(imu->hi2c, imu->ag_addr, OUT_X_G, 1, (uint8_t *)gyroAndAccelData, sizeof(gyroAndAccelData), IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    // read magnetometer data
    halStatus = HAL_I2C_Mem_Read(imu->hi2c, imu->m_addr, 0xFF & OUT_X_L_M, 1, (uint8_t *)magData, sizeof(magData), IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    if (motionData)
    {
        motionData->xGyro = gyroAndAccelData[0] - imu->gyroOffsets[0];
        motionData->yGyro = gyroAndAccelData[1] - imu->gyroOffsets[1];
        motionData->zGyro = gyroAndAccelData[2] - imu->gyroOffsets[2];

        motionData->xAcc = gyroAndAccelData[3] - imu->accOffsets[0];
        motionData->yAcc = gyroAndAccelData[4] - imu->accOffsets[1];
        motionData->zAcc = gyroAndAccelData[5] - imu->accOffsets[2];

        motionData->xMag = -(magData[0]);// - imu->magOffsets[0]);
        motionData->yMag = magData[1];// - imu->magOffsets[1];
        motionData->zMag = magData[2];// - imu->magOffsets[2];
    }

    return GLOVE_STATUS_OK;
}

glove_status_t IMU_DumpConfigRegisters(imu_t * imu)
{
    uint8_t readValue = 0;
    glove_status_t status = GLOVE_STATUS_OK;

    if (!imu)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    IMU_CHECK_INIT(imu);

    // accelerometer config reg
    status = IMU_ReadReg(imu, imu->ag_addr, CTRL_REG6_XL, &readValue);
    CHECK_STATUS_OK_RET(status);
    printf("CTRL_REG6_XL: 0x%X\r\n", readValue);

    // gyro config reg
    status = IMU_ReadReg(imu, imu->ag_addr, CTRL_REG1_G, &readValue);
    CHECK_STATUS_OK_RET(status);
    printf("CTRL_REG1_G: 0x%X\r\n", readValue);

    // magnetometer config reg
    status = IMU_ReadReg(imu, imu->m_addr, CTRL_REG1_M, &readValue);
    CHECK_STATUS_OK_RET(status);
    printf("CTRL_REG1_M: 0x%X\r\n", readValue);

    return GLOVE_STATUS_OK;
}


static glove_status_t IMU_SetRegBits(imu_t * imu, uint8_t baseAddress, uint8_t regAddress, uint8_t mask, uint8_t values)
{
    HAL_StatusTypeDef halStatus = HAL_OK;
    uint8_t readValue = 0;
    uint8_t writeValue = 0;

    if (!imu->hi2c)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    // read current register value
    halStatus = HAL_I2C_Mem_Read(imu->hi2c, baseAddress, regAddress, 1, &readValue, 1, IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    // set bits
    writeValue = readValue | (mask & values);

    // write back
    halStatus = HAL_I2C_Mem_Write(imu->hi2c, baseAddress, regAddress, 1, &writeValue, 1, IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    return GLOVE_STATUS_OK;
}

static glove_status_t IMU_WriteReg(imu_t * imu, uint8_t baseAddress, uint8_t regAddress, uint8_t value)
{
    HAL_StatusTypeDef halStatus = HAL_OK;

    if (!imu->hi2c)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    // write back
    halStatus = HAL_I2C_Mem_Write(imu->hi2c, baseAddress, regAddress, 1, &value, 1, IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    return GLOVE_STATUS_OK;
}

static glove_status_t IMU_ReadReg(imu_t * imu, uint8_t baseAddress, uint8_t regAddress, uint8_t *readvalue)
{
    HAL_StatusTypeDef halStatus = HAL_OK;

    if (!imu->hi2c)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    halStatus = HAL_I2C_Mem_Read(imu->hi2c, baseAddress, regAddress, 1, readvalue, 1, IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    return GLOVE_STATUS_OK;
}

static glove_status_t IMU_Reset(imu_t * imu)
{
    HAL_StatusTypeDef halStatus = HAL_OK;
    uint8_t data = 0;

    if (!imu->hi2c)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    // reset accel and gyro
    data = CTRL_REG8_RESET_VAL;
    halStatus = HAL_I2C_Mem_Write(imu->hi2c, imu->ag_addr, CTRL_REG8, 1, &data, 1, IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    // reset magnetometer
    data = CTRL_REG2_M_RESET_VAL;
    halStatus = HAL_I2C_Mem_Write(imu->hi2c, imu->m_addr, CTRL_REG2_M, 1, &data, 1, IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    HAL_Delay(1);

    return GLOVE_STATUS_OK;
}

static bool are_addresses_valid(imu_t * imu)
{
    if (imu)
    {
        return 
        (
            imu->ag_addr == IMU_GET_AG_ADDR(0) ||
            imu->ag_addr == IMU_GET_AG_ADDR(1)
        ) &&
        (
            imu->m_addr == IMU_GET_M_ADDR(0) ||
            imu->m_addr == IMU_GET_M_ADDR(1)
        );
    }
    return false;
}
