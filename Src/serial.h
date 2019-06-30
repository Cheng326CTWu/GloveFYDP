/**
 * Serial interface for the Gauntlet. 
 */

#ifndef SERIAL_H
#define SERIAL_H

#include "stdlib.h"

#include "stm32l4xx_hal.h"

#include "glove_status_codes.h"

glove_status_t Serial_Init(UART_HandleTypeDef * huart);
glove_status_t Serial_Write(uint8_t * data, uint32_t len);

#endif
