#include "stdbool.h"
#include "stdint-gcc.h"
#include "stdlib.h"
#include "string.h"

#include "stm32l4xx_hal.h"

#include "glove_status_codes.h"
#include "scheduler.h"
#include "serial.h"
#include "sm.h"

#define SERIAL_UART_TIMEOUT 100

#define SERIAL_CHECK_INIT()                         \
do                                                  \
{                                                   \
    if (!gContext.fInit)                            \
    {                                               \
        return GLOVE_STATUS_MODULE_NOT_INIT;        \
    }                                               \
} while(0);

#define COMMAND_STRING_LENGTH 4 // not null terminated
#define COMMAND_DATA "data"
#define COMMAND_STOP "stop"
#define COMMAND_LOG "log"

typedef struct
{
    bool fInit;
    UART_HandleTypeDef * huart;
    serial_tx_callback_t txCallback;
    serial_rx_callback_t rxCallback;
} serial_context_t;

static serial_context_t gContext = {0};
static uint8_t gEventData[COMMAND_STRING_LENGTH] = {0};

static glove_status_t Serial_ReceiveAsync(uint8_t * buffer, uint32_t len);

glove_status_t Serial_ReceiveTask()
{
    return Serial_ReceiveAsync(gEventData, sizeof(gEventData));
}

task_t Task_Receive = 
{
    .pTaskFn = &Serial_ReceiveTask,
    .name = "Receive Task"
};


glove_status_t Serial_Init(UART_HandleTypeDef * huart)
{
    if (!huart)
    {
        return GLOVE_STATUS_NULL_PTR;
    }

    gContext.huart = huart;
    gContext.fInit = true;

    // start listening for serial commands
    return Serial_ReceiveAsync(gEventData, sizeof(gEventData));
}

glove_status_t Serial_WriteBlocking(uint8_t * data, uint32_t len)
{
    SERIAL_CHECK_INIT();
    if (!data)
    {
        return GLOVE_STATUS_NULL_PTR;
    }
    return HALstatusToGlove(HAL_UART_Transmit(gContext.huart, data, len, SERIAL_UART_TIMEOUT));
}

glove_status_t Serial_WriteAsync(uint8_t * data, uint32_t len, serial_tx_callback_t callback)
{
    SERIAL_CHECK_INIT();
    if (!callback || !data)
    {
        return GLOVE_STATUS_NULL_PTR;
    }
    return HALstatusToGlove(HAL_UART_Transmit_DMA(gContext.huart, data, len));
}

glove_status_t OtherTask()
{
    printf("SERIAL RECEIVE ASYNC\r\n");
    return GLOVE_STATUS_OK;
}

task_t Task_Other = 
{
    .pTaskFn = &OtherTask,
    .name = "Other Task"
};

glove_status_t Serial_ReceiveAsync(uint8_t * buffer, uint32_t len)
{
    SERIAL_CHECK_INIT();

    if  (!buffer)
    {
        return GLOVE_STATUS_NULL_PTR;
    }
    Scheduler_AddTask(&Task_Other);
    return HALstatusToGlove(HAL_UART_Receive_DMA(gContext.huart, buffer, len));
}

// ISR
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == gContext.huart)
    {
        if (gContext.txCallback)
        {
            gContext.txCallback();
        }
    }
}

// ISR
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    sm_event_t event = EVENT_NONE;
    char commandStr[COMMAND_STRING_LENGTH] = {0};
    printf("asdf\r\n");
    if (huart == gContext.huart)
    {
        if (!strncmp((char *)gEventData, COMMAND_DATA, COMMAND_STRING_LENGTH))
        {
            event = EVENT_START_TRANSFERRING;
        }
        else if (!strncmp((char *)gEventData, COMMAND_STOP, COMMAND_STRING_LENGTH))
        {
            event = EVENT_STOP_TRANSFERRING;
        }
        else if (!strncmp((char *)gEventData, COMMAND_LOG, COMMAND_STRING_LENGTH))
        {
            event = EVENT_GET_LOGS;
        }
        memcpy(commandStr, gEventData, sizeof(gEventData));
        SM_PostEventDebug(event, commandStr);
        Scheduler_AddTask(&Task_Receive);
    }
}
