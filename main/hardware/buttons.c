#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_check.h>
#include "../system_events.h"
#include "../router.h"
#include "board.h"
#include "buttons.h"

#define BUTTON_UP_IO BUTTON_UP_PIN_IO
#define BUTTON_DOWN_IO BUTTON_DOWN_PIN_IO
#define BUTTON_LEFT_IO BUTTON_LEFT_PIN_IO
#define BUTTON_RIGHT_IO BUTTON_RIGHT_PIN_IO
#define BUTTON_SELECT_IO BUTTON_SELECT_PIN_IO
#define BUTTON_POWER_IO BUTTON_POWER_PIN_IO

static const int button_ios[] = {
    BUTTON_UP_IO,
    BUTTON_DOWN_IO,
    BUTTON_LEFT_IO,
    BUTTON_RIGHT_IO,
    BUTTON_SELECT_IO,
    BUTTON_POWER_IO
};
#define BUTTON_COUNT (sizeof(button_ios) / sizeof(button_ios[0]))

static const char *tag = "BUTTON";

static void IRAM_ATTR buttons_isr_handler(void *arg) {
    const system_event_t event = {
        .type = EVENT_TYPE_BUTTON,
        .payload.button = { 
            .released = gpio_get_level((gpio_num_t)arg), .id = (gpio_num_t)arg 
        }
    };
    router_publish(&event);    
}

esp_err_t buttons_init(void) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 
            1 << BUTTON_DOWN_IO 
            | 1 << BUTTON_UP_IO 
            | 1 << BUTTON_LEFT_IO 
            | 1 << BUTTON_RIGHT_IO 
            | 1 << BUTTON_SELECT_IO 
            | 1 << BUTTON_POWER_IO,
        .pull_down_en = 0,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);

    for(int i = 0; i < BUTTON_COUNT; i++) {
        ESP_ERROR_CHECK(
            gpio_isr_handler_add(
                button_ios[i], 
                buttons_isr_handler, 
                (void *)button_ios[i]
            )
        );
    }
    ESP_LOGI(tag, "Initialized");
    return ESP_OK;
}
