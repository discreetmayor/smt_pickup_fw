#ifndef _ROUTER_H_
#define _ROUTER_H_

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <esp_err.h>
#include "system_events.h"

esp_err_t router_subscribe(event_type_t event_type, QueueHandle_t subscriber);
void router_publish(const system_event_t *event);

#endif /* _ROUTER_H_ */
