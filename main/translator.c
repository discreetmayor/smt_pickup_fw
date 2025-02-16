#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_check.h>
#include "system_events.h"
#include "router.h"
#include "translator.h"

static const char *tag = "TRANSLATOR";
static QueueHandle_t translator_queue_handle = NULL;
static translator_handler_t handlers[NUM_EVENT_TYPES] = { NULL };

static void translator_task(void *arg) {
    system_event_t event;
    while (1) {
        if (xQueueReceive(
            translator_queue_handle, 
            &event, 
            portMAX_DELAY
        ) == pdTRUE) {
            if(handlers[event.type]) {
                handlers[event.type](&event);
            }
        } else {
            ESP_LOGE(tag, "Dropped Queue Message");
        }
    }
}

static esp_err_t ensure_initialized() {
    static bool initialized = false;
    if(initialized) {
        return ESP_OK;
    }

    translator_queue_handle = xQueueCreate(5, sizeof(system_event_t));
    if (!translator_queue_handle) {
        ESP_LOGE(tag, "Failed to create translator queue");
        return ESP_FAIL;
    }

    BaseType_t ret = xTaskCreate(
        translator_task, 
        "TranslatorTask", 
        2048, 
        NULL, 
        5, 
        NULL
    );
    if(ret != pdPASS) {
        vQueueDelete(translator_queue_handle);
        translator_queue_handle = NULL;
        ESP_RETURN_ON_ERROR(ESP_FAIL, tag, "Failed to create translator task");
    }
    initialized = true;
    ESP_LOGI(tag, "Initialized");
    return ESP_OK;
}

esp_err_t translator_add_handler(
    event_type_t type, 
    translator_handler_t handler) {
    ESP_ERROR_CHECK(ensure_initialized());

    if (type < NUM_EVENT_TYPES) {
        handlers[type] = handler;
        ESP_RETURN_ON_ERROR(
            router_subscribe(type, translator_queue_handle), 
            tag, 
            "Failed to subscribe"
        );
        return ESP_OK;
    }
    ESP_LOGE(tag, "Invalid event type %d", type);
    return ESP_ERR_INVALID_ARG;
}