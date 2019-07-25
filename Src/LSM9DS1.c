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
#include "string.h"

#include "stm32l4xx_hal.h"

#include "glove_status_codes.h"
#include "LSM9DS1.h"
#include "LSM9DS1_Reg.h"
#include "scheduler.h"
#include "serial.h"

#define IMU_AG_ADDR (0xD6)
#define IMU_M_ADDR (0x3C)
#define IMU_I2C_TIMEOUT 100
#define NUM_IMUS 16

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
    bool continuousRead;
    I2C_HandleTypeDef *hi2c;
    int16_t accOffsets[3];
    int16_t gyroOffsets[3];
    int16_t magOffsets[3];
} IMU_context_t;

static IMU_context_t gContext = {0};

// for profiling!
static uint32_t gCount = 0;
static uint32_t gTotal = 0;

// forward declarations
static glove_status_t IMU_SelfTest();
static glove_status_t IMU_Calibrate(int16_t * caliAcc, int16_t * caliGyro, int16_t * caliMag);
static glove_status_t AcknowledgeTransferStopped();
static glove_status_t ReadAllMotionSensors();
static glove_status_t IMU_SetRegBits(uint8_t baseAddress, uint8_t regAddress, uint8_t mask, uint8_t values);
static glove_status_t IMU_ReadReg(uint8_t baseAddress, uint8_t regAddress, uint8_t *readvalue);
static glove_status_t IMU_WriteReg(uint8_t baseAddress, uint8_t regAddress, uint8_t value);
static glove_status_t IMU_Reset();

// task definitions
task_t Task_IMUSweep = 
{
    .pTaskFn = &ReadAllMotionSensors,
    .name = "IMU Task"
};

task_t Task_AckTransferStopped = {
    .pTaskFn = &AcknowledgeTransferStopped,
    .name = "AckStop"
};


glove_status_t IMU_Init(I2C_HandleTypeDef * hi2c)
{
    glove_status_t status = GLOVE_STATUS_OK;

    if (!hi2c)
    {
        return GLOVE_STATUS_INVALID_ARGUMENT;
    }

    // global I2C handle
    gContext.hi2c = hi2c;

    // // reset device
    // status = IMU_Reset();
    // CHECK_STATUS_OK_RET(status);

    // enable the 3-axes of the gyroscope
    status = IMU_WriteReg(IMU_AG_ADDR, CTRL_REG4, 0x38);
    CHECK_STATUS_OK_RET(status);

    // configure the gyroscope
    status = IMU_WriteReg(IMU_AG_ADDR, CTRL_REG1_G, 2 << 5 | 0 << 3 | 0);
    CHECK_STATUS_OK_RET(status);
    HAL_Delay(200);

    // enable the three axes of the accelerometer 
    status = IMU_WriteReg(IMU_AG_ADDR, CTRL_REG5_XL, 0x38);
    CHECK_STATUS_OK_RET(status);

    // configure the accelerometer-specify bandwidth selection with Abw
    status = IMU_WriteReg(IMU_AG_ADDR, CTRL_REG6_XL, 2 << 5 | 0 << 3);
    CHECK_STATUS_OK_RET(status);

    HAL_Delay(200);

    // // set accel ODR to 50 Hz
    // status = IMU_SetRegBits(IMU_AG_ADDR, CTRL_REG6_XL, CTRL_REG6_XL_ODR_MASK, CTRL_REG6_XL_ODR_50);
    // CHECK_STATUS_OK_RET(status);

    // // set accel anti-aliasing filter bandwidth to 105 Hz
    // status = IMU_SetRegBits(IMU_AG_ADDR, CTRL_REG6_XL, CTRL_REG6_XL_AA_BW_MASK, CTRL_REG6_XL_AA_BW_105);
    // CHECK_STATUS_OK_RET(status);

    // // set gyro ODR to 59.5 Hz
    // status = IMU_SetRegBits(IMU_AG_ADDR, CTRL_REG1_G, CTRL_REG1_G_ODR_MASK, CTRL_REG1_G_ODR_59p5);
    // CHECK_STATUS_OK_RET(status);

    // configure the magnetometer, copied from:
    // https://github.com/kriswiner/LSM9DS1/blob/master/LSM9DS1_MS5611_BasicAHRS_t3.ino
    status = IMU_WriteReg(IMU_M_ADDR, CTRL_REG1_M, 0x80 | 2 << 5 | 4 << 2); // select x,y-axis mode
    CHECK_STATUS_OK_RET(status);

    status = IMU_WriteReg(IMU_M_ADDR, CTRL_REG2_M, 0 << 5 ); // select mag full scale
    CHECK_STATUS_OK_RET(status);

    status = IMU_WriteReg(IMU_M_ADDR, CTRL_REG3_M, 0x00 ); // continuous conversion mode
    CHECK_STATUS_OK_RET(status);
    
    status = IMU_WriteReg(IMU_M_ADDR, CTRL_REG4_M, 2 << 2 ); // select z-axis mode
    CHECK_STATUS_OK_RET(status);

    // // set magnetometer ODR to 40 Hz
    // status = IMU_SetRegBits(IMU_M_ADDR, CTRL_REG1_M, CTRL_REG1_M_ODR_MASK, CTRL_REG1_M_ODR_40);
    // CHECK_STATUS_OK_RET(status);
    
    // // self test
    // status = IMU_SelfTest();
    // CHECK_STATUS_OK_RET(status);

    // calibrate - for some reason this delay helps get steadier numbers
    // HAL_Delay(2000);
    // status = IMU_Calibrate(gContext.accOffsets, gContext.gyroOffsets, gContext.magOffsets);
    // CHECK_STATUS_OK_RET(status);

    gContext.fInit = true;
    return GLOVE_STATUS_OK;
}

static glove_status_t IMU_SelfTest()
{
    glove_status_t status = GLOVE_STATUS_OK;
    int16_t acc[3] = {0};
    int16_t gyro[3] = {0};
    int16_t mag[3] = {0};
    int16_t accNoTest[3] = {0};
    int16_t gyroNoTest[3] = {0};
    int16_t magNoTest[3] = {0};

    // disable self-test
    status = IMU_WriteReg(IMU_AG_ADDR, CTRL_REG10, CTRL_REG10_DISABLE_SELF_TEST);
    CHECK_STATUS_OK_RET(status);

    // get the calibration values without self-test
    status = IMU_Calibrate(accNoTest, gyroNoTest, magNoTest);
    CHECK_STATUS_OK_RET(status);

    // enable self-test
    status = IMU_WriteReg(IMU_AG_ADDR, CTRL_REG10, CTRL_REG10_ENABLE_SELF_TEST);
    CHECK_STATUS_OK_RET(status);

    // get the calibration values, they are printed out in the calibration function
    status = IMU_Calibrate(acc, gyro, mag);
    CHECK_STATUS_OK_RET(status);

    // disable self-test
    status = IMU_WriteReg(IMU_AG_ADDR, CTRL_REG10, CTRL_REG10_DISABLE_SELF_TEST);
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

static glove_status_t IMU_Calibrate(int16_t * caliAcc, int16_t * caliGyro, int16_t * caliMag)
{
    // read each sensor for one second and average the data 
    uint32_t start = HAL_GetTick();
    glove_status_t status = GLOVE_STATUS_OK;
    motion_data_t data = {0};
    int32_t accSums[3] = {0};
    int32_t gyroSums[3] = {0};
    int32_t magSums[3] = {0};
    uint32_t count = 0;

    while (HAL_GetTick() - start < 1000)
    {
        status = IMU_ReadAll(&data);
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

glove_status_t IMU_ReadAll(motion_data_t * motionData)
{
    HAL_StatusTypeDef halStatus = HAL_OK;
    int16_t gyroAndAccelData[6] = {0};
    int16_t magData[3] = {0};

    // read gyro and accel data
    halStatus = HAL_I2C_Mem_Read(gContext.hi2c, IMU_AG_ADDR, OUT_X_G, 1, (uint8_t *)gyroAndAccelData, sizeof(gyroAndAccelData), IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    // read magnetometer data
    halStatus = HAL_I2C_Mem_Read(gContext.hi2c, IMU_M_ADDR, 0xFF & OUT_X_L_M, 1, (uint8_t *)magData, sizeof(magData), IMU_I2C_TIMEOUT);
    CHECK_STATUS_OK_RET(HALstatusToGlove(halStatus));

    if (motionData)
    {
        motionData->xGyro = gyroAndAccelData[0] - gContext.gyroOffsets[0];
        motionData->yGyro = gyroAndAccelData[1] - gContext.gyroOffsets[1];
        motionData->zGyro = gyroAndAccelData[2] - gContext.gyroOffsets[2];

        motionData->xAcc = gyroAndAccelData[3] - gContext.accOffsets[0];
        motionData->yAcc = gyroAndAccelData[4] - gContext.accOffsets[1];
        motionData->zAcc = gyroAndAccelData[5] - gContext.accOffsets[2];

        motionData->xMag = -(magData[0]);// - gContext.magOffsets[0]);
        motionData->yMag = magData[1];// - gContext.magOffsets[1];
        motionData->zMag = magData[2];// - gContext.magOffsets[2];
    }

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

glove_status_t IMU_StartContinuousRead()
{
    glove_status_t status = GLOVE_STATUS_OK;

    IMU_CHECK_INIT();

    gContext.continuousRead = true;
    status = Scheduler_AddTask(&Task_IMUSweep);
    CHECK_STATUS_OK_RET(status);

    Scheduler_DisableDebug();

    return GLOVE_STATUS_OK;
}

glove_status_t IMU_StopContinuousRead()
{
    glove_status_t status = GLOVE_STATUS_OK;
    gContext.continuousRead = false;

    status = Scheduler_RemoveTask(&Task_IMUSweep);
    CHECK_STATUS_OK_RET(status);
    return GLOVE_STATUS_OK;
}

glove_status_t ReadAllMotionSensors()
{
    uint8_t i = 0;
    glove_status_t status = GLOVE_STATUS_OK;
    motion_data_t allMotionData[16] = {0};
    uint32_t startTime = HAL_GetTick();

    // TODO: need to switch I2C bus every 3 reads
    for (i = 0; i < NUM_IMUS; ++i)
    {
        status = IMU_ReadAll(allMotionData + i);
        CHECK_STATUS_OK_RET(status);
    }

    // transmit the data
    // printf("acc: %.3f, %.3f, %.3f\r\n", 
    //     allMotionData[0].xAcc*SENS_ACCELEROMETER_2, 
    //     allMotionData[0].yAcc*SENS_ACCELEROMETER_2, 
    //     allMotionData[0].zAcc*SENS_ACCELEROMETER_2);
    // printf("gyro: %.3f, %.3f, %.3f\r\n", 
    //     allMotionData[0].xGyro*SENS_GYROSCOPE_245, 
    //     allMotionData[0].yGyro*SENS_GYROSCOPE_245, 
    //     allMotionData[0].zGyro*SENS_GYROSCOPE_245);
    // printf("mag: %d, %d, %d\r\n",
    //     allMotionData[0].xMag,
    //     allMotionData[0].yMag,
    //     allMotionData[0].zMag);
    status = Serial_WriteBlocking((uint8_t *)allMotionData, sizeof(allMotionData));
    CHECK_STATUS_OK_RET(status);
    gTotal += HAL_GetTick() - startTime;
    ++gCount;

    // schedule the next sensor scan if we are still in continuous mode
    if (gContext.continuousRead)
    {
        status = Scheduler_AddTask(&Task_IMUSweep);
    }
    return status;
}

glove_status_t AcknowledgeTransferStopped()
{
    // for profiling
    char msg [50] = {0};
    sprintf(msg, "average: %ld\r\n", gTotal/gCount);
    Serial_WriteBlocking((uint8_t *) msg, strlen(msg));
    Scheduler_EnableDebug();
    char * message = "ack\r\n";
    return Serial_WriteBlocking((uint8_t *)message, sizeof(message));
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

static glove_status_t IMU_WriteReg(uint8_t baseAddress, uint8_t regAddress, uint8_t value)
{
    HAL_StatusTypeDef halStatus = HAL_OK;

    // write back
    halStatus = HAL_I2C_Mem_Write(gContext.hi2c, baseAddress, regAddress, 1, &value, 1, IMU_I2C_TIMEOUT);
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
