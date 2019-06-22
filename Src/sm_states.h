/**
 * This file contains static declarationso of all the states.
 * To add a new state, you must add the state name in sm.h, then declare the state 
 * entry/exit/handler functions in this file, implement them in sm_states.c, and 
 * also define the static state object in this file.
 */

#ifndef SM_STATES_H
#define SM_STATES_H

#include "glove_status_codes.h"
#include "sm.h"

// Below are the entry, exit, and event handler function declarations for public states

// idle state
glove_status_t IdleStateEntry();
glove_status_t IdleStateExit();
sm_state_t * IdleStateHandler(sm_event_t event);
static sm_state_t idleState = 
{
    .name = IDLE_STATE,
    .pEnterFn = &IdleStateEntry,
    .pExitFn = &IdleStateExit,
    .pHandlerFn = &IdleStateHandler
};

// init state
glove_status_t InitStateEntry();
glove_status_t InitStateExit();
sm_state_t * InitStateHandler(sm_event_t event);
static sm_state_t initState = 
{
    .name = INIT_STATE,
    .pEnterFn = &InitStateEntry,
    .pExitFn = &InitStateExit,
    .pHandlerFn = &InitStateHandler
};

#endif
