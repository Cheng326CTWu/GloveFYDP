/**
 * Serial interface for the Gauntlet. 
 */

#ifndef SERIAL_H
#define SERIAL_H

#include "stdint-gcc.h"
#include "stdlib.h"

#include "stm32l4xx_hal.h"

#include "glove_status_codes.h"

typedef void (*serial_rx_callback_t)(void);
typedef void (*serial_tx_callback_t)(void);

glove_status_t Serial_Init(UART_HandleTypeDef * huart);

glove_status_t Serial_WriteBlocking(uint8_t * data, uint32_t len);
glove_status_t Serial_WriteAsync(uint8_t * data, uint32_t len, serial_tx_callback_t callback);
glove_status_t Serial_RegisterCommandListener(serial_rx_callback_t callback);

#endif
