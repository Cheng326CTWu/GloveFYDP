/*
 * LSM9DS1.c
 *
 *  Created on: May 10, 2019
 *      Author: tejasvi
 */

#include "stdint.h"
#include "stdbool.h"

#include "glove_status_codes.h"
#include "LSM9DS1.h"
#include "LSM9DS1_Reg.h"

#define IMU_AG_ADDR (0xD6)
#define IMU_M_ADDR (0x3C)
#define IMU_I2C_TIMEOUT 100

#define IMU_CHECK_INIT()                            \
do                                                  \
{                                                   \
    if (!gContext.fInit)                            \
    {                                               \
        return GLOVE_STATUS_MODULE_NOT_INIT;        \
    }                                               \
} while (0);


typedef struct {
    bool fInit;
    I2C_HandleTypeDef *hi2c;
} IMU_context_t;

IMU_context_t gContext = {0};

// forward declarations
static glove_status_t IMU_SetRegBits(uint8_t baseAddress, uint8_t regAddress, uint8_t mask, uint8_t values);
static glove_status_t IMU_ReadReg(uint8_t baseAddress, uint8_t regAddress, uint8_t *readvalue);
static glove_status_t IMU_Reset();

glove_status_t IMU_Init(I2C_HandleTypeDef * hi2c)
{
    glove_status_t status = GLOVE_STATUS_OK;

    if (!hi2c)
    {
        return GLOVE_STATUS_INVALID_ARGUMENT;
    }

    // global I2C handle
    gContext.hi2c = hi2c;

    // reset device
    status = IMU_Reset();
    CHECK_STATUS_OK_RET(status);

    // configure accelerometer
    status = IMU_SetRegBits(IMU_AG_ADDR, CTRL_REG6_XL, CTRL_REG6_XL_ODR_MASK, CTRL_REG5_XL_ODR_50);
    CHECK_STATUS_OK_RET(status);

    // configure gyro
    status = IMU_SetRegBits(IMU_AG_ADDR, CTRL_REG1_G, CTRL_REG1_G_ODR_MASK, CTRL_REG1_G_ODR_59p5);
    CHECK_STATUS_OK_RET(status);

    // configure magnetometer
    status = IMU_SetRegBits(IMU_M_ADDR, CTRL_REG1_M, CTRL_REG1_M_ODR_MASK, CTRL_REG1_M_ODR_40);
    CHECK_STATUS_OK_RET(status);

    gContext.fInit = true;
    return GLOVE_STATUS_OK;
}

glove_status_t IMU_ReadAll(motion_data_t * motionData)
{
    HAL_StatusTypeDef halStatus = HAL_OK;
    int16_t gyroAndAccelData[6] = {0};
    int16_t magData[3] = {0};
    float accelRange = 4.0f / (1 << 16);
    printf("%f\r\n", accelRange);

    if (!motionData)
    {
        return GLOVE_STATUS_INVALID_ARGUMENT;
    }

    // read gyro and accel data
    halStatus = HAL_I2C_Mem_Read(gContext.hi2c, IMU_AG_ADDR, OUT_X_G, 1, (uint8_t *)gyroAndAccelData, sizeof(gyroAndAccelData), IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    // read magnetometer data
    halStatus = HAL_I2C_Mem_Read(gContext.hi2c, IMU_M_ADDR, OUT_X_L_M, 1, (uint8_t *)magData, sizeof(magData), IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    // dump
    motionData->xAcc = gyroAndAccelData[0]*accelRange;
    motionData->yAcc = gyroAndAccelData[1]*accelRange;
    motionData->zAcc = gyroAndAccelData[2]*accelRange;

    printf("accel: %f %f %f\r\n", motionData->xAcc, motionData->yAcc, motionData->zAcc);
    // printf("gxl: %d %d %d %d %d %d\r\n", gyroAndAccelData[0], gyroAndAccelData[1], gyroAndAccelData[2], gyroAndAccelData[3], gyroAndAccelData[4], gyroAndAccelData[5]);
    // printf("m: %d %d %d\r\n", magData[0], magData[1], magData[2]);

    return GLOVE_STATUS_OK;
}

glove_status_t IMU_DumpConfigRegisters()
{
    uint8_t readValue = 0;
    glove_status_t status = GLOVE_STATUS_OK;

    IMU_CHECK_INIT();

    // accelerometer config reg
    status = IMU_ReadReg(IMU_AG_ADDR, CTRL_REG6_XL, &readValue);
    CHECK_STATUS_OK_RET(status);
    printf("CTRL_REG6_XL: 0x%X\r\n", readValue);

    // gyro config reg
    status = IMU_ReadReg(IMU_AG_ADDR, CTRL_REG1_G, &readValue);
    CHECK_STATUS_OK_RET(status);
    printf("CTRL_REG1_G: 0x%X\r\n", readValue);

    // magnetometer config reg
    status = IMU_ReadReg(IMU_M_ADDR, CTRL_REG1_M, &readValue);
    CHECK_STATUS_OK_RET(status);
    printf("CTRL_REG1_M: 0x%X\r\n", readValue);

    return GLOVE_STATUS_OK;
}

static glove_status_t IMU_SetRegBits(uint8_t baseAddress, uint8_t regAddress, uint8_t mask, uint8_t values)
{
    HAL_StatusTypeDef halStatus = HAL_OK;
    uint8_t readValue = 0;
    uint8_t writeValue = 0;

    if (!gContext.hi2c)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    // read current register value
    halStatus = HAL_I2C_Mem_Read(gContext.hi2c, baseAddress, regAddress, 1, &readValue, 1, IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    // set bits
    writeValue = readValue | (mask & values);

    // write back
    halStatus = HAL_I2C_Mem_Write(gContext.hi2c, baseAddress, regAddress, 1, &writeValue, 1, IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    return GLOVE_STATUS_OK;
}

static glove_status_t IMU_ReadReg(uint8_t baseAddress, uint8_t regAddress, uint8_t *readvalue)
{
    HAL_StatusTypeDef halStatus = HAL_OK;

    if (!gContext.hi2c)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    halStatus = HAL_I2C_Mem_Read(gContext.hi2c, baseAddress, regAddress, 1, readvalue, 1, IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    return GLOVE_STATUS_OK;
}

static glove_status_t IMU_Reset()
{
    HAL_StatusTypeDef halStatus = HAL_OK;
    uint8_t data = 0;

    if (!gContext.hi2c)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    // reset accel and gyro
    data = CTRL_REG8_RESET_VAL;
    halStatus = HAL_I2C_Mem_Write(gContext.hi2c, IMU_AG_ADDR, CTRL_REG8, 1, &data, 1, IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    // reset magnetometer
    data = CTRL_REG2_M_RESET_VAL;
    halStatus = HAL_I2C_Mem_Write(gContext.hi2c, IMU_M_ADDR, CTRL_REG2_M, 1, &data, 1, IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    HAL_Delay(1);

    return GLOVE_STATUS_OK;
}
