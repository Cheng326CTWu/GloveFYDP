#include "stdlib.h"
#include "stdbool.h"

#include "stm32l4xx_hal.h"

#include "glove_status_codes.h"
#include "TCA9548A.h"

#define TCA_ADDR (0x70 << 1)
#define TCA_NUM_BUSES 8
#define I2C_TIMEOUT 1000

typedef struct
{
    bool fInit;
    I2C_HandleTypeDef * hi2c;
    uint8_t currentBus;
} i2c_mux_context_t;

static i2c_mux_context_t gContext = {0};

glove_status_t I2CMux_Init(I2C_HandleTypeDef * hi2c)
{
    if (!hi2c)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    gContext.hi2c = hi2c;
    gContext.currentBus = 0xFF;
    gContext.fInit = true;
    return GLOVE_STATUS_OK;
}

glove_status_t I2CMux_Select(uint8_t bus)
{
    uint8_t data = 1 << bus;
    HAL_StatusTypeDef hStatus = HAL_OK;

    if (bus > TCA_NUM_BUSES)
    {
        return GLOVE_STATUS_INVALID_ARGUMENT;
    }

    hStatus = HAL_I2C_Master_Transmit(gContext.hi2c, TCA_ADDR, &data, 1, I2C_TIMEOUT);
    CHECK_HAL_STATUS_OK(hStatus);

    return GLOVE_STATUS_OK;
}
