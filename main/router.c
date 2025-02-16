#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_check.h>
#include "system_events.h"
#include "router.h"

#define MAX_SUBSCRIBERS 16

typedef struct {
    event_type_t type;
    QueueHandle_t queue;
} router_subscription_t;
static const char *tag = "ROUTER";
static router_subscription_t subscribers[MAX_SUBSCRIBERS];
static size_t subscriber_count = 0;

esp_err_t router_subscribe(event_type_t event_type, QueueHandle_t subscriber) {
    if (subscriber_count >= MAX_SUBSCRIBERS) {
        ESP_RETURN_ON_ERROR(
            ESP_FAIL, 
            tag, 
            "Failed to register subscriber: maximum reached"
        );
    }
    subscribers[subscriber_count].queue = subscriber;
    subscribers[subscriber_count++].type = event_type;
    ESP_LOGI(
        tag, 
        "Subscriber registered for event_type %s; total count: %d", 
        get_event_type_name(event_type), 
        subscriber_count
    );
    return ESP_OK;
}

static void router_publish_from_isr(const system_event_t *event) {
    BaseType_t higher_priority_task_woken = pdFALSE;
    for (size_t i = 0; i < subscriber_count; i++) {
        if (subscribers[i].type == event->type ) {
            xQueueSendFromISR(
                subscribers[i].queue, 
                event, 
                &higher_priority_task_woken
            );
        }
        if(subscribers[i].type == EVENT_TYPE_ANY) {
            const system_event_t evt = {
                .type = EVENT_TYPE_ANY,
            };
            xQueueSendFromISR(
                subscribers[i].queue, 
                &evt, 
                &higher_priority_task_woken
            );
        }            
    }
    if (higher_priority_task_woken) {
        portYIELD_FROM_ISR();
    }
}

void router_publish(const system_event_t *event) {
    if(xPortInIsrContext() == pdTRUE) {
        router_publish_from_isr(event);
        return;
    }
    for (size_t i = 0; i < subscriber_count; i++) {
        if (subscribers[i].type == event->type) {
            xQueueSend(subscribers[i].queue, event, (TickType_t)0);
        }
        if(subscribers[i].type == EVENT_TYPE_ANY) {
            const system_event_t evt = {
                .type = EVENT_TYPE_ANY,
                .original_type = event->type
            };
            xQueueSend(subscribers[i].queue, &evt, (TickType_t)0);
        } 
    }
}
 