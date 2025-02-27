#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_check.h>
#include <stdio.h>
#include "hardware_task.h"
#include "../system_events.h"

static const char *tag = "HARDWARE_TASK";

static esp_err_t hardware_io_init(gpio_config_t *config) {
    config->intr_type = GPIO_INTR_DISABLE;
    config->mode = GPIO_MODE_OUTPUT;
    return gpio_config(config);
}

esp_err_t hardware_task_init(
    hardware_config_t *config, 
    TaskFunction_t task_function, 
    QueueHandle_t *queue_handle
) {
    ESP_RETURN_ON_ERROR(
        hardware_io_init(&config->io_config),
         tag, 
         "Failed to initialize IO"
    );

    char task_name[32];
    snprintf(task_name, sizeof(task_name), "%sTask", config->log_tag);
    *queue_handle = xQueueCreate(5, sizeof(system_event_t));

    if(*queue_handle == NULL) {
        ESP_LOGE(config->log_tag, "Failed to create queue");
        return ESP_FAIL;
    }

    BaseType_t bt = xTaskCreate(
            task_function, 
            task_name, 
            config->task_stack_size, 
            NULL, 
            config->task_priority, 
            NULL);
    if(bt == pdPASS) {
        ESP_LOGI(config->log_tag, "Created");
        return ESP_OK;
    } else {
        ESP_LOGE(config->log_tag, "Creation failed");
        vQueueDelete(*queue_handle);
        *queue_handle = NULL;
        return ESP_FAIL;
    }
}