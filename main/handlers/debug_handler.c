#include <esp_log.h>
#include "../router.h"
#include "../system_events.h"
#include "debug_handler.h"

void handle_debug_event(const system_event_t *event) {
    ESP_LOGI("DEBUG", "Received debug event with type: %d", event->type);
}