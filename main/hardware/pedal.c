#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>
#include "../system_events.h"
#include "../router.h"
#include "board.h"
#include "pedal.h"

#define PEDAL_IO PEDAL_PIN_IO

static const char *tag = "PEDAL";

static void IRAM_ATTR pedal_isr_handler(void *arg) {
    const system_event_t event = {
        .type = EVENT_TYPE_PEDAL,
        .payload.pedal = { .released = gpio_get_level(PEDAL_IO) }
    };
    router_publish(&event);
}

esp_err_t pedal_init(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1 << PEDAL_IO,
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);

    ESP_ERROR_CHECK(gpio_isr_handler_add(PEDAL_IO, pedal_isr_handler, NULL));
    ESP_LOGI(tag, "Initialized");
    return ESP_OK;
}
