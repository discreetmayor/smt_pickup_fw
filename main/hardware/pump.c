#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
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
#define PUMP_PWM_FREQ 30000
#define PUMP_PWM_RESOLUTION LEDC_TIMER_3_BIT
#define PUMP_PWM_MAX_DUTY ((1 << PUMP_PWM_RESOLUTION) - 1)
#define PUMP_DISABLE_PWM PUMP_PWM_MAX_DUTY
#define PUMP_DEFAULT_PWM 4

static const char *tag = "PUMP";
static QueueHandle_t pump_queue_handle = NULL;
static bool enabled = false;
static uint8_t pump_level = PUMP_DEFAULT_PWM;
// input values 0 - 7
static void pump_set_pwm(uint8_t duty_cycle) {
    static bool initialized = false;
    static ledc_channel_config_t ledc_channel;
    
    if(!initialized) {
        ledc_timer_config_t ledc_timer = {
            .speed_mode       = LEDC_LOW_SPEED_MODE,
            .timer_num        = LEDC_TIMER_0,
            .duty_resolution  = PUMP_PWM_RESOLUTION,  // Use the constant
            .freq_hz          = PUMP_PWM_FREQ,  
            .clk_cfg          = LEDC_AUTO_CLK
        };
        ledc_timer_config(&ledc_timer);
        ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
        ledc_channel.channel    = LEDC_CHANNEL_0;
        ledc_channel.timer_sel  = LEDC_TIMER_0;
        ledc_channel.intr_type  = LEDC_INTR_DISABLE;
        ledc_channel.gpio_num   = PUMP_IO;
        ledc_channel.hpoint     = 0;
        initialized = true;
    }
    
    if (duty_cycle > PUMP_PWM_MAX_DUTY) {
        duty_cycle = PUMP_PWM_MAX_DUTY;
    }
    ledc_channel.duty = duty_cycle;
    ledc_channel_config(&ledc_channel);
}

static void pump_task(void *arg) {
    system_event_t event;
    while (1) {
        if(xQueueReceive(pump_queue_handle, &event, portMAX_DELAY) == pdTRUE) {
            switch(event.payload.pump.state) {
                case PUMP_ON:
                    pump_set_pwm(pump_level);               
                    break;

                case PUMP_OFF:
                    pump_set_pwm(PUMP_DISABLE_PWM);
                    break;
                
                case PUMP_SET_LEVEL:
                    pump_level = event.payload.pump.level;
                    pump_set_pwm(pump_level);
                    break;
            }
        } else {
            ESP_LOGE(tag, "Dropped Queue Message");
        }
    }
}


esp_err_t pump_init() {
    hardware_config_t config = {
        .log_tag = tag,
        .task_stack_size = PUMP_TASK_STACK_SIZE,
        .task_priority = PUMP_TASK_PRIORITY,
        .io_config.pin_bit_mask = 1 << PUMP_IO,
        .io_config.pull_up_en = 1
    };
    ESP_ERROR_CHECK(hardware_task_init(&config, pump_task, &pump_queue_handle));
    ESP_RETURN_ON_ERROR(
        router_subscribe(EVENT_TYPE_PUMP, pump_queue_handle), 
        tag, 
        "Failed to subscribe");
    ESP_LOGI(tag, "Initialized");

    return ESP_OK;
}