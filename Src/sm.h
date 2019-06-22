/**
 * This file contains the definitions for the state machine, but not the states themselves.
 * For the actual states, see sm_states.h.
 */

#ifndef SM_H
#define SM_H

#include "stdbool.h"

#include "glove_status_codes.h"

// List of state names
typedef enum
{
    IDLE_STATE = 0,
    INIT_STATE = 1,
    DATA_TRANSFER_STATE = 2,
    LOG_TRANSFER_STATE = 3
} sm_state_name_t;

// List of possible events
typedef enum
{
    EVENT_NONE = 0,
    EVENT_START_TRANSFERRING = 1,       // start transferring sensor data
    EVENT_STOP_TRANSFERRING = 2,        // stop transferring data (can be sensor or log data)
    EVENT_GET_LOGS = 3,                 // start transferring logs
} sm_event_t;

// State type
typedef struct sm_state_t sm_state_t;
struct sm_state_t
{
    sm_state_name_t name;                               // name from the enum above
    glove_status_t (*pEnterFn)();                       // function called when the SM enters this state
    glove_status_t (*pExitFn)();                        // function called when SM exits this state
    sm_state_t* (*pHandlerFn)(sm_event_t event);        // Event handler function
};

// context struct for holding state machine globals
// TODO: implement event queue
typedef struct
{
    sm_state_t * currentState;
    sm_event_t lastEvent;           // last event posted - we don't have a queue for now
    bool fInit;
} sm_context_t;

// Public API
glove_status_t SM_Init();
glove_status_t SM_Tick();
glove_status_t SM_PostEvent(sm_event_t event);

#endif
