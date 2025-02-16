#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>
#include "../system_events.h"
#include "../router.h"
#include "board.h"
#include "pedal.h"

#define PEDAL_IO PEDAL_PIN_IO
#define DEBOUNCE_TIME_MS 20

static const char *tag = "PEDAL";
static TaskHandle_t debounce_task_handle = NULL;
static bool isr_handled = true;

static void debounce_task() {
    system_event_t event = {
        .type = EVENT_TYPE_PEDAL
    };
    while(1) {
        if(xTaskNotifyWait(0, 0, NULL, portMAX_DELAY)) {
            vTaskDelay(DEBOUNCE_TIME_MS / portTICK_PERIOD_MS);
            event.payload.pedal.released = gpio_get_level(PEDAL_IO);
            router_publish(&event);
            isr_handled = true;
        }
    }
}

static esp_err_t create_debounce_task() {
    BaseType_t bt = xTaskCreate(
        debounce_task, 
        "PEDALdebounce", 
        2048, 
        NULL, 
        6, 
        &debounce_task_handle);
    if(bt == pdPASS) {
        ESP_LOGI(tag, "Debounce task created");
        return ESP_OK;
    } else {
        ESP_LOGE(tag, "Debounce task reation failed");
        return ESP_FAIL;
    }
}

static void IRAM_ATTR pedal_isr_handler(void *arg) {
    if(isr_handled) {        
        isr_handled = false;
        BaseType_t higher_priority_task_woken = pdFALSE;
        xTaskNotifyFromISR(
            debounce_task_handle, 
            0, 
            eNoAction, 
            &higher_priority_task_woken
        );
        if (higher_priority_task_woken) {
            portYIELD_FROM_ISR();
        }
    }
}

esp_err_t pedal_init(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1 << PEDAL_IO,
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    gpio_config(&io_conf);

    ESP_ERROR_CHECK(create_debounce_task());
    ESP_ERROR_CHECK(gpio_isr_handler_add(PEDAL_IO, pedal_isr_handler, NULL));
    ESP_LOGI(tag, "Initialized");
    return ESP_OK;
}
