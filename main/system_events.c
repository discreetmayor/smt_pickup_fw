#include "system_events.h"

const event_type_name_t event_type_names[] = {
    { EVENT_TYPE_BUTTON, "EVENT_TYPE_BUTTON" },
    { EVENT_TYPE_PEDAL, "EVENT_TYPE_PEDAL" },
    { EVENT_TYPE_PUMP, "EVENT_TYPE_PUMP" },
    { EVENT_TYPE_VALVE, "EVENT_TYPE_VALVE" },
    { EVENT_TYPE_REGULATOR, "EVENT_TYPE_REGULATOR" },
    { EVENT_TYPE_ANY, "EVENT_TYPE_ANY" }
};

const char* get_event_type_name(event_type_t type) {
    int event_count = sizeof(event_type_names) / sizeof(event_type_names[0]);
    for (int i = 0; i < event_count; i++) {
        if (event_type_names[i].type == type) {
            return event_type_names[i].name;
        }
    }
    return "UNKNOWN_EVENT_TYPE";
}
