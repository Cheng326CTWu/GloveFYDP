/**
 * This file contains the implementation of state machine transition and event handling functions.
 */

#include "glove_status_codes.h"
#include "LSM9DS1.h"
#include "scheduler.h"
#include "sm.h"
#include "sm_states.h"


// DataTransfer state
glove_status_t DataTransferStateEntry();
glove_status_t DataTransferStateExit();
sm_state_t * DataTransferStateHandler(sm_event_t event);
static sm_state_t dataTransferState = 
{
    .name = DATA_TRANSFER_STATE,
    .pEnterFn = &DataTransferStateEntry,
    .pExitFn = &DataTransferStateExit,
    .pHandlerFn = &DataTransferStateHandler
};

// Log transfer state
glove_status_t LogTransferStateEntry();
glove_status_t LogTransferStateExit();
sm_state_t * LogTransferStateHandler(sm_event_t event);
static sm_state_t logTransferState = 
{
    .name = LOG_TRANSFER_STATE,
    .pEnterFn = &LogTransferStateEntry,
    .pExitFn = &LogTransferStateExit,
    .pHandlerFn = &LogTransferStateHandler
};


// idle state
glove_status_t IdleStateEntry()
{
    printf("%s\r\n", __FUNCTION__);
    return GLOVE_STATUS_OK;
}

glove_status_t IdleStateExit()
{
    printf("%s\r\n", __FUNCTION__);
    return GLOVE_STATUS_OK;
}

sm_state_t * IdleStateHandler(sm_event_t event)
{
    printf("%s\r\n", __FUNCTION__);
    if (EVENT_START_TRANSFERRING == event)
    {
        printf("Start transferring\r\n");
        return &dataTransferState;
    }
    else if (EVENT_GET_LOGS == event)
    {
        printf("Get logs\r\n");
        return &logTransferState;
    }
    else
    {
        return &idleState;
    }
}

// init state
glove_status_t InitStateEntry()
{
    printf("%s\r\n", __FUNCTION__);
    return GLOVE_STATUS_OK;
}

glove_status_t InitStateExit()
{
    printf("%s\r\n", __FUNCTION__);
    return GLOVE_STATUS_OK;
}

sm_state_t * InitStateHandler(sm_event_t event)
{
    printf("%s\r\n", __FUNCTION__);
    if (EVENT_START_TRANSFERRING == event)
    {
        printf("Start transferring\r\n");
        return &dataTransferState;
    }
    else if (EVENT_GET_LOGS == event)
    {
        printf("Get logs\r\n");
        return &logTransferState;
    }
    else
    {
        return &initState;
    }
}

// DataTransfer state
glove_status_t DataTransferStateEntry()
{
    printf("%s\r\n", __FUNCTION__);
    return IMU_StartContinuousRead();
}

glove_status_t DataTransferStateExit()
{
    glove_status_t status = GLOVE_STATUS_OK;

    printf("%s\r\n", __FUNCTION__);

    status = IMU_StopContinuousRead();
    CHECK_STATUS_OK_RET(status);

    Scheduler_AddTask(&Task_AckTransferStopped);
    return GLOVE_STATUS_OK;
}

sm_state_t * DataTransferStateHandler(sm_event_t event)
{
    printf("%s\r\n", __FUNCTION__);
    if (EVENT_STOP_TRANSFERRING == event)
    {
        printf("Stop tranferring!\r\n");
        return &idleState;
    }
    else if (EVENT_GET_LOGS == event)
    {
        return &logTransferState;
    }
    else
    {
        return &dataTransferState;
    }
}

// Log transfer state
glove_status_t LogTransferStateEntry()
{
    printf("%s\r\n", __FUNCTION__);
    return GLOVE_STATUS_OK;
}

glove_status_t LogTransferStateExit()
{
    printf("%s\r\n", __FUNCTION__);
    return GLOVE_STATUS_OK;
}

sm_state_t * LogTransferStateHandler(sm_event_t event)
{
    printf("%s\r\n", __FUNCTION__);
    if (EVENT_STOP_TRANSFERRING == event)
    {
        printf("Stop tranferring!\r\n");
        return &idleState;
    }
    else if (EVENT_START_TRANSFERRING == event)
    {
        return &dataTransferState;
    }
    else
    {
        return &dataTransferState;
    }
}
