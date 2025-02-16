#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_check.h>
#include "../system_events.h"
#include "hardware_task.h"
#include "../router.h"
#include "board.h"
#include "valve.h"

#define VALVE_IO VALVE_PIN_IO
#define VALVE_TASK_PRIORITY 5
#define VALVE_TASK_STACK_SIZE 2048

static const char *tag = "VALVE";
static QueueHandle_t valve_queue_handle = NULL;

static void valve_task(void *arg) {
    system_event_t event;
    while(1) {
        if(xQueueReceive(valve_queue_handle, &event, portMAX_DELAY) == pdTRUE) {
            gpio_set_level(VALVE_IO, event.payload.valve.closed);
            ESP_LOGI(tag, "%s", event.payload.valve.closed ? "CLOSED" : "OPEN");
        } else {
            ESP_LOGE(tag, "Dropped Queue Message");
        }
    }
}

esp_err_t valve_init() {
    hardware_config_t config = {
        .log_tag = tag,
        .task_stack_size = VALVE_TASK_STACK_SIZE,
        .task_priority = VALVE_TASK_PRIORITY,
        .io_config.pin_bit_mask = 1 << VALVE_IO,
        .io_config.pull_up_en = 1
    };
    ESP_ERROR_CHECK(
        hardware_task_init(&config, valve_task, &valve_queue_handle)
    );
    ESP_RETURN_ON_ERROR(
        router_subscribe(EVENT_TYPE_VALVE, valve_queue_handle), 
        tag, 
        "Failed to subscribe"
    );
    ESP_LOGI(tag, "Initialized");
    return ESP_OK;
}