#ifndef _SYSTEM_EVENTS_H_
#define _SYSTEM_EVENTS_H_

#define NUM_EVENT_TYPES 6

/*******************************************************************************
 * Event Types
 ******************************************************************************/
typedef enum {
    EVENT_TYPE_BUTTON,
    EVENT_TYPE_PEDAL,
    EVENT_TYPE_PUMP,
    EVENT_TYPE_VALVE,
    EVENT_TYPE_REGULATOR,
    EVENT_TYPE_ANY
} event_type_t;

/*******************************************************************************
 * Buttons
 ******************************************************************************/
typedef enum {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_SELECT,
    BUTTON_POWER
} button_id_t;

/*******************************************************************************
 * Pump
 ******************************************************************************/

typedef enum {
    PUMP_OFF,
    PUMP_ON
} pump_state_t;

/*******************************************************************************
 * Regulator
 ******************************************************************************/

typedef enum {
    REGULATOR_OFF,
    REGULATOR_ON,
    REGULATOR_TOGGLE
} regulator_state_t;

/*******************************************************************************
 * Payloads
 ******************************************************************************/

typedef struct {
    button_id_t id;
    bool released;
} button_event_payload_t;

typedef struct {
    bool released;
} pedal_event_payload_t;
 
typedef struct {
    bool closed;
} valve_command_payload_t;

typedef struct {
    pump_state_t state;
} pump_command_payload_t;

typedef struct {
    regulator_state_t state;
} regulator_command_payload_t;

/*******************************************************************************
 * Generic Event
 ******************************************************************************/
typedef struct {
    event_type_t type;
    union {
        button_event_payload_t button;
        pedal_event_payload_t pedal;
        pump_command_payload_t pump;
        valve_command_payload_t valve;
        regulator_command_payload_t regulator;
    } payload;
} system_event_t;

#endif // _SYSTEM_EVENTS_H_
