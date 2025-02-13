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
#include "regulator.h"

#define REGULATOR_IO LDO_12V_ENABLE_PIN_IO
#define REGULATOR_TASK_PRIORITY 5
#define REGULATOR_TASK_STACK_SIZE 2048
#define REGULATOR_ENABLE 1
#define REGULATOR_DISABLE 0

static const char *tag = "REGULATOR";
static QueueHandle_t regulator_queue_handle = NULL;
static bool enabled = false;

static void regulator_enable() {
    ESP_LOGI(tag, "Enabled");
    gpio_set_level(REGULATOR_IO, REGULATOR_ENABLE);
    enabled = true;
}

static void regulator_disable() {
    ESP_LOGI(tag, "Disabled");
    gpio_set_level(REGULATOR_IO, REGULATOR_DISABLE);
    enabled = false;
}

static void regulator_task(void *arg) {
    system_event_t event;
    while(1) {
        if(xQueueReceive(regulator_queue_handle, &event, portMAX_DELAY)) {
            switch(event.payload.regulator.state) {
                case REGULATOR_TOGGLE:
                    if(enabled) {
                        regulator_disable();
                    } else {
                        regulator_enable();
                    }
                    break;
                case REGULATOR_ON:
                    if(!enabled) {
                        regulator_enable();
                    }
                    break;
                case REGULATOR_OFF:
                    if(enabled) {
                        regulator_disable();
                    }
                    break;
            }
        }
    }
}

esp_err_t regulator_init() {
    hardware_config_t config = {
        .task_stack_size = REGULATOR_TASK_STACK_SIZE,
        .task_priority = REGULATOR_TASK_PRIORITY,
        .io_pin = REGULATOR_IO,
        .io_mode = GPIO_MODE_OUTPUT,
        .log_tag = tag
    };
    ESP_ERROR_CHECK(
        hardware_task_init(&config, regulator_task, &regulator_queue_handle)
    );
    ESP_RETURN_ON_ERROR(
        router_subscribe(EVENT_TYPE_REGULATOR, regulator_queue_handle), 
        tag, 
        "Failed to subscribe"
    );
    ESP_LOGI(tag, "Initialized");
    return ESP_OK;
}