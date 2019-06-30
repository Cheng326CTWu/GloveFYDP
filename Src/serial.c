#include "stdlib.h"
#include "stdbool.h"

#include "stm32l4xx_hal.h"

#include "glove_status_codes.h"

#define SERIAL_UART_TIMEOUT 100

typedef struct
{
    bool fInit;
    UART_HandleTypeDef * huart;
} serial_context_t;

static serial_context_t gContext = {0};

glove_status_t Serial_Init(UART_HandleTypeDef * huart)
{
    if (!huart)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    gContext.huart = huart;
    gContext.fInit = true;

    return GLOVE_STATUS_OK;
}

glove_status_t Serial_Write(uint8_t * data, uint32_t len)
{
    return HALstatusToGlove(HAL_UART_Transmit(gContext.huart, (uint8_t *)data, len, SERIAL_UART_TIMEOUT));
}

// TODO: add ISR for data received over uart
