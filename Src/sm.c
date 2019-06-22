/**
 * This file implements the state machine.
 */

#include "glove_status_codes.h"
#include "sm.h"
#include "sm_states.h"

static sm_context_t gContext = {0};

glove_status_t SM_Init()
{
    gContext.fInit = true;
    gContext.currentState = &initState;
    gContext.currentState->pEnterFn();
    gContext.lastEvent = EVENT_NONE;
    return GLOVE_STATUS_OK;
}

glove_status_t SM_Tick()
{
    sm_state_t * nextState = NULL;
    glove_status_t status = GLOVE_STATUS_OK;

    if (!gContext.fInit)
    {
        return GLOVE_STATUS_MODULE_NOT_INIT;
    }

    // check for new events
    if (EVENT_NONE != gContext.lastEvent)
    {
        // there is a new event waiting, call the current state's event handler
        nextState = gContext.currentState->pHandlerFn(gContext.lastEvent);

        // clear the event (queue)
        gContext.lastEvent = EVENT_NONE;

        if (!nextState)
        {
            // TODO: log error here

            // transition to idle state due to unsupported event
            nextState = &idleState;
        }

        printf("Transition from state %d to %d\r\n", gContext.currentState->name, nextState->name);

        // transition the state, if it is a different state
        if (nextState->name != gContext.currentState->name)
        {
            status = gContext.currentState->pExitFn();
            if (GLOVE_STATUS_OK != status)
            {
                nextState = &idleState;
            }

            gContext.currentState = nextState;

            status = gContext.currentState->pEnterFn();

            // upon state error, go into idle state, but don't call entry function in case we already failed once
            // at entering the idle state
            if (GLOVE_STATUS_OK != status)
            {
                printf("State entry error. State = %d\r\n", gContext.currentState->name);
                nextState = &idleState;
            }
        }
    }
    return GLOVE_STATUS_OK;
}

glove_status_t SM_PostEvent(sm_event_t event)
{
    printf("%s event=%d\r\n", __FUNCTION__, event);
    if (!gContext.fInit)
    {
        return GLOVE_STATUS_MODULE_NOT_INIT;
    }
    gContext.lastEvent = event;
    return GLOVE_STATUS_OK;
}
