#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gptimer.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_check.h>
#include "../system_events.h"
#include "hardware_task.h"
#include "../router.h"
#include "board.h"
#include "pump.h"

#define PUMP_IO PUMP_PIN_IO
#define PUMP_TASK_PRIORITY 5
#define PUMP_TASK_STACK_SIZE 2048
#define PUMP_TIMEOUT_US ((uint32_t)(1000000 * 30)) // 30 seconds
#define PUMP_ENABLE 0
#define PUMP_DISABLE 1

static const char *tag = "PUMP";
static gptimer_handle_t timer = NULL;
static QueueHandle_t pump_queue_handle = NULL;
static bool enabled = false;

static void pump_start() {
    ESP_LOGI(tag, "Started");
    gpio_set_level(PUMP_IO, PUMP_ENABLE);
    enabled = true;
}

static void pump_stop() {
    ESP_LOGI(tag, "Stopped");
    gpio_set_level(PUMP_IO, PUMP_DISABLE);
    enabled = false;
}

static void reset_timer() {
    ESP_LOGI(tag, "Timer reset");
    gptimer_set_raw_count(timer, 0); 
}

static esp_err_t init_timer(uint32_t uSec) {
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1 us per tick
        .intr_priority = 3
    };
    ESP_RETURN_ON_ERROR(
        gptimer_new_timer(&timer_config, &timer), 
        tag, 
        "Timer failed to create timer"
    );
    
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = uSec,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = false
    };
    ESP_RETURN_ON_ERROR(
        gptimer_set_alarm_action(timer, &alarm_config), 
        tag, 
        "Timer failed to set alarm"
    );
    
    gptimer_event_callbacks_t cbs = {
        .on_alarm = timer_end
    };
    ESP_RETURN_ON_ERROR(
        gptimer_register_event_callbacks(timer, &cbs, NULL),
         tag, 
         "Timer failed to register callbacks"
    );
    
    ESP_RETURN_ON_ERROR(gptimer_enable(timer), tag, "Timer failed to enable");
    
    return ESP_OK;
}

static void pump_task(void *arg) {
    system_event_t event;
    while (1) {
        if(xQueueReceive(pump_queue_handle, &event, portMAX_DELAY)) {
            switch(event.payload.pump.state) {
                case PUMP_ON:
                    reset_timer();
                    if(!enabled) {
                        pump_start();
                        gptimer_start(timer);
                    }
                    break;
                case PUMP_OFF:
                    if(enabled) {
                        gptimer_stop(timer);
                        pump_stop();
                    }
                    break;
            }
        }
    }
}

bool timer_end(gptimer_handle_t timer, 
    const gptimer_alarm_event_data_t *data, 
    void *ctx) {
    const system_event_t event = {
        .type = EVENT_TYPE_PUMP,
        .payload.pump = { .state = PUMP_OFF }
    };
    router_publish(&event);
    return true;
}

esp_err_t pump_init() {
    hardware_config_t config = {
        .task_stack_size = PUMP_TASK_STACK_SIZE,
        .task_priority = PUMP_TASK_PRIORITY,
        .io_pin = PUMP_IO,
        .io_mode = GPIO_MODE_OUTPUT,
        .log_tag = tag
    };
    ESP_ERROR_CHECK(hardware_task_init(&config, pump_task, &pump_queue_handle));
    ESP_ERROR_CHECK(init_timer(PUMP_TIMEOUT_US));
    ESP_RETURN_ON_ERROR(
        router_subscribe(EVENT_TYPE_PUMP, pump_queue_handle), 
        tag, 
        "Failed to subscribe");
    ESP_LOGI(tag, "Initialized");

    return ESP_OK;
}