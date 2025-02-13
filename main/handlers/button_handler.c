#include "../router.h"
#include "../system_events.h"
#include "button_handler.h"

static void handle_power_button(const system_event_t *event) {
    system_event_t forward_event = {
        .type = EVENT_TYPE_REGULATOR,
        .payload.regulator.state = REGULATOR_TOGGLE
    };
    router_publish(&forward_event);
}

void handle_button_event(const system_event_t *event) {
    switch(event->payload.button.id) {
        case BUTTON_POWER:
            handle_power_button(event);
            break;
        default:
            //more button handlers
            break;
    }
}