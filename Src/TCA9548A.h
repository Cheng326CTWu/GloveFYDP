/**
 * Driver for the TCA9548A I2C mux.
 */

#ifndef TCA9548A_H
#define TCA9548A_H

#include "stdlib.h"

#include "stm32l4xx_hal.h"

#include "glove_status_codes.h"

glove_status_t I2CMux_Init(I2C_HandleTypeDef * hi2c);
glove_status_t I2CMux_Select(uint8_t bus);

#endif
