#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_check.h>
#include "system_events.h"
#include "router.h"
#include "translator.h"
#include "hardware/pedal.h"
#include "hardware/pump.h"
#include "hardware/valve.h"
#include "hardware/regulator.h"
#include "hardware/buttons.h"
#include "handlers/pedal_handler.h"
#include "handlers/button_handler.h"
#include "handlers/debug_handler.h"

static esp_err_t init() {
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    //subscribers
    ESP_ERROR_CHECK(regulator_init());
    ESP_ERROR_CHECK(pump_init());
    ESP_ERROR_CHECK(valve_init());

    //publishers
    ESP_ERROR_CHECK(pedal_init());
    ESP_ERROR_CHECK(buttons_init());

    return ESP_OK;
}

void app_main(void) {
    // check voltage and start 12V and tasks if 15V
    // otherwise programming mode
    if(1) {
        init();

        //handlers
        translator_add_handler(EVENT_TYPE_PEDAL, handle_pedal_event);
        //translator_add_handler(EVENT_TYPE_BUTTON, handle_button_event);
        //translator_add_handler(EVENT_TYPE_ANY, handle_debug_event);
        
        // system_event_t forward_event = {
        //     .type = EVENT_TYPE_REGULATOR,
        //     .payload.regulator.state = REGULATOR_ON
        // };
        // router_publish(&forward_event);

        while(1) {
            vTaskDelay(10000 / portTICK_PERIOD_MS);
        }
    }
}
