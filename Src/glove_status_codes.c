/*
 * glove_status_codes.c
 *
 *  Created on: May 11, 2019
 *      Author: tejasvi
 */

#include "glove_status_codes.h"
#include "stm32l4xx_hal.h"

glove_status_t HALstatusToGlove(HAL_StatusTypeDef hStatus)
{
    if (HAL_OK == hStatus)
    {
        return GLOVE_STATUS_OK;
    }
    return HAL_STATUS_BASE + hStatus;
}

