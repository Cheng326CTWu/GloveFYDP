#include "stdint.h"
#include "string.h"

#include "glove_status_codes.h"
#include "LSM9DS1.h"
#include "LSM9DS1_Reg.h"
#include "serial.h"
#include "TCA9548A.h"

typedef enum {
	PINKY,
	RING,
	MIDDLE,
	INDEX,
	THUMB,
	HAND
} finger_t;

typedef enum {
	TIP,
	SECOND,
	BASE
} knuckle_t;

typedef struct {
   	finger_t finger;
    knuckle_t knuckle;
    uint32_t sad0;
    uint32_t sad1;
    uint32_t bus;
    imu_t imu;
} imu_board_info_t;

static glove_status_t AcknowledgeTransferStopped();
static glove_status_t ReadAllMotionSensors();

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

// for profiling!
static uint32_t gCount = 0;
static uint32_t gTotal = 0;

// // the actual glove
// static imu_board_info_t FINGER_INFOS[] = 
// {
// 	// pinky
// 	{.finger = PINKY, .knuckle = TIP,     .sad0 = 1, .sad1 = 1, .bus = 0, .imu = {0}},  // tip
// 	{.finger = PINKY, .knuckle = SECOND,  .sad0 = 0, .sad1 = 0, .bus = 0, .imu = {0}},  // middle
// 	{.finger = PINKY, .knuckle = BASE,    .sad0 = 0, .sad1 = 0, .bus = 1, .imu = {0}},  // base

//     // ring finger
// 	{.finger = RING, .knuckle = TIP,      .sad0 = 1, .sad1 = 1, .bus = 2, .imu = {0}},	// tip
// 	{.finger = RING, .knuckle = SECOND,   .sad0 = 0, .sad1 = 0, .bus = 2, .imu = {0}},	// middle
// 	{.finger = RING, .knuckle = BASE,     .sad0 = 1, .sad1 = 1, .bus = 1, .imu = {0}},	// base

//     // middle finger
// 	{.finger = MIDDLE, .knuckle = TIP,    .sad0 = 1, .sad1 = 1, .bus = 3, .imu = {0}},	// tip
// 	{.finger = MIDDLE, .knuckle = SECOND, .sad0 = 0, .sad1 = 0, .bus = 3, .imu = {0}},	// middle
// 	{.finger = MIDDLE, .knuckle = BASE,   .sad0 = 0, .sad1 = 0, .bus = 4, .imu = {0}},	// base

//     // index finger
// 	{.finger = INDEX, .knuckle = TIP,     .sad0 = 1, .sad1 = 1, .bus = 5, .imu = {0}},	// tip
// 	{.finger = INDEX, .knuckle = SECOND,  .sad0 = 0, .sad1 = 0, .bus = 5, .imu = {0}},	// middle
// 	{.finger = INDEX, .knuckle = BASE,    .sad0 = 1, .sad1 = 1, .bus = 4, .imu = {0}},	// base

//     // thumb
// 	{.finger = THUMB, .knuckle = TIP,     .sad0 = 1, .sad1 = 1, .bus = 6, .imu = {0}},	// tip
// 	{.finger = THUMB, .knuckle = SECOND,  .sad0 = 0, .sad1 = 0, .bus = 6, .imu = {0}},	// middle
// 	{.finger = THUMB, .knuckle = BASE,    .sad0 = 1, .sad1 = 1, .bus = 7, .imu = {0}},	// base

//     // middle of the hand
// 	{.finger = HAND, .knuckle = SECOND,   .sad0 = 0, .sad1 = 0, .bus = 7, .imu = {0}},
// };

// for breadboard testing
static imu_board_info_t FINGER_INFOS[] = {
    {.finger = PINKY, .knuckle = TIP,     .sad0 = 1, .sad1 = 1, .bus = 0, .imu = {0}},  // tip
    {.finger = PINKY, .knuckle = SECOND,  .sad0 = 0, .sad1 = 0, .bus = 0, .imu = {0}},  // middle

    {.finger = PINKY, .knuckle = TIP,     .sad0 = 1, .sad1 = 1, .bus = 1, .imu = {0}},  // tip
    {.finger = PINKY, .knuckle = SECOND,  .sad0 = 0, .sad1 = 0, .bus = 1, .imu = {0}},  // middle

    {.finger = PINKY, .knuckle = BASE,    .sad0 = 0, .sad1 = 0, .bus = 2, .imu = {0}},  // base

};

// // a single middle board
// static imu_board_info_t FINGER_INFOS[] = {
//     {{.finger = PINKY, .knuckle = SECOND,  .sad0 = 0, .sad1 = 0, .bus = 0, .imu = {0}},  // middle
// };

// // a single base board
// static imu_board_info_t FINGER_INFOS[] = {
//     {.finger = PINKY, .knuckle = BASE,    .sad0 = 0, .sad1 = 0, .bus = 1, .imu = {0}},  // base
// };


#define NUM_IMUS (sizeof(FINGER_INFOS) / sizeof(imu_board_info_t))

// globals
static struct
{
	bool fContinuousRead;
} gContext = {0};


glove_status_t Hand_Init(I2C_HandleTypeDef * hi2c)
{
	imu_board_info_t info = {0};
	glove_status_t status = GLOVE_STATUS_OK;
	imu_t * imu = NULL;

	if (!hi2c)
	{
		return GLOVE_STATUS_NULL_PTR;
	}

	for (int i = 0; i < NUM_IMUS; ++i)
	{
		info = FINGER_INFOS[i];
		imu = &(info.imu);
		info.imu.ag_addr = IMU_GET_AG_ADDR(info.sad0);
		info.imu.m_addr = IMU_GET_M_ADDR(info.sad1);
        printf("IMU addr: 0x%X and 0x%X\r\n", info.imu.ag_addr, info.imu.m_addr);
		info.imu.hi2c = hi2c;

        // select the bus for the current IMU
        status = I2CMux_Select(info.bus);
        CHECK_STATUS_OK_RET(status);

        // init the imu
        // TODO: change status check to return bad status code?
		status = IMU_Init(imu);
		CHECK_STATUS_OK_NO_RET(status);

		// dump config registers
		if ((status = IMU_DumpConfigRegisters(imu)))
		{
			printf("IMU reg dump failed\r\n");
		}
	}

    return GLOVE_STATUS_OK;
}

glove_status_t Hand_StartContinuousRead()
{
    glove_status_t status = GLOVE_STATUS_OK;

    gContext.fContinuousRead = true;
    status = Scheduler_AddTask(&Task_IMUSweep);
    CHECK_STATUS_OK_RET(status);

    Scheduler_DisableDebug();

    return GLOVE_STATUS_OK;
}

glove_status_t Hand_StopContinuousRead()
{
    glove_status_t status = GLOVE_STATUS_OK;
    gContext.fContinuousRead = false;

    status = Scheduler_RemoveTask(&Task_IMUSweep);
    CHECK_STATUS_OK_RET(status);
    return GLOVE_STATUS_OK;
}
#define PRETTY_PRINT_DATA 1
static glove_status_t ReadAllMotionSensors()
{
    uint8_t i = 0;
    glove_status_t status = GLOVE_STATUS_OK;
    motion_data_t allMotionData[16] = {0};
    uint32_t startTime = HAL_GetTick();

    for (i = 0; i < NUM_IMUS; ++i)
    {
        // select the bus for the current IMU
        status = I2CMux_Select(FINGER_INFOS[i].bus);
        CHECK_STATUS_OK_RET(status);

        // read the data
        status = IMU_ReadAll(&(FINGER_INFOS[i].imu), allMotionData + i);
        CHECK_STATUS_OK_RET(status);
    }

    if (PRETTY_PRINT_DATA)
    {
        // transmit the data
        printf("acc: %.3f, %.3f, %.3f\r\n", 
            allMotionData[0].xAcc*SENS_ACCELEROMETER_2, 
            allMotionData[0].yAcc*SENS_ACCELEROMETER_2, 
            allMotionData[0].zAcc*SENS_ACCELEROMETER_2);
        printf("gyro: %.3f, %.3f, %.3f\r\n", 
            allMotionData[0].xGyro*SENS_GYROSCOPE_245, 
            allMotionData[0].yGyro*SENS_GYROSCOPE_245, 
            allMotionData[0].zGyro*SENS_GYROSCOPE_245);
        printf("mag: %d, %d, %d\r\n",
            allMotionData[0].xMag,
            allMotionData[0].yMag,
            allMotionData[0].zMag);
    }
    else
    {
        status = Serial_WriteBlocking((uint8_t *)allMotionData, sizeof(allMotionData));
        CHECK_STATUS_OK_RET(status);
    }
    
    gTotal += HAL_GetTick() - startTime;
    ++gCount;

    // schedule the next sensor scan if we are still in continuous mode
    if (gContext.fContinuousRead)
    {
        status = Scheduler_AddTask(&Task_IMUSweep);
    }
    return status;
}

static glove_status_t AcknowledgeTransferStopped()
{
    // for profiling
    char msg [50] = {0};
    sprintf(msg, "average: %ld\r\n", gTotal/gCount);
    Serial_WriteBlocking((uint8_t *) msg, strlen(msg));
    Scheduler_EnableDebug();
    char * message = "ack\r\n";
    return Serial_WriteBlocking((uint8_t *)message, sizeof(message));
}












