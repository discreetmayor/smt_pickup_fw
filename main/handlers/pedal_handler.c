#include "../router.h"
#include "../system_events.h"
#include "pedal_handler.h"

void handle_pedal_event(const system_event_t *event) {
    system_event_t forward_event = {
        .type = EVENT_TYPE_REGULATOR,
        .payload.regulator.state = REGULATOR_ON
    };
    router_publish(&forward_event);
    
    forward_event.type = EVENT_TYPE_VALVE;
    forward_event.payload.valve.closed = event->payload.pedal.released;
    router_publish(&forward_event);

    forward_event.type = EVENT_TYPE_PUMP;
    if(!event->payload.pedal.released) {
        forward_event.payload.pump.state = PUMP_ON;
        router_publish(&forward_event);
    }
}