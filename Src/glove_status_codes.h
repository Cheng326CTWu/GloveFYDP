/*
 * glove_status_codes.h
 *
 *  Created on: May 10, 2019
 *      Author: tejasvi
 */

#ifndef GLOVE_STATUS_CODES_H_
#define GLOVE_STATUS_CODES_H_

#include "stdio.h"
#include "stm32l4xx_hal.h"

// status base defines
#define GLOVE_STATUS_BASE 0x0000
#define IMU_STATUS_BASE 0x1000
#define HAL_STATUS_BASE 0x2000

// All status codes
typedef enum
{
	// general statuses
	GLOVE_STATUS_OK =                   		   GLOVE_STATUS_BASE + 0x0000,
	GLOVE_STATUS_INVALID_ARGUMENT =                GLOVE_STATUS_BASE + 0x0001,
	GLOVE_STATUS_MODULE_NOT_INIT = 				   GLOVE_STATUS_BASE + 0x0002,
	GLOVE_STATUS_NULL_PTR = 					   GLOVE_STATUS_BASE + 0x0003,
	GLOVE_STATUS_FAIL = 						   GLOVE_STATUS_BASE + 0x0004

	// IMU driver statuses

	// HAL statuses
	// calculated in HALstatusToGlove()
} glove_status_t;

glove_status_t HALstatusToGlove(HAL_StatusTypeDef hStatus);

#define CHECK_STATUS_OK_RET(status)															\
do																							\
{																							\
	if (GLOVE_STATUS_OK != (status))														\
	{																						\
		printf("%s:%d error, status=%X\r\n", __FUNCTION__, __LINE__, (status));				\
		return (status);																	\
	}																						\
} while (0);

#define CHECK_NULL_RET(item) 												\
do																			\
{																			\
	if (!(item))															\
	{																		\
		printf("%s:%d error, null pointer\r\n", __FUNCTION__, __LINE__);	\
		return GLOVE_STATUS_FAIL;											\
	}																		\
}while(0);


#endif /* GLOVE_STATUS_CODES_H_ */
