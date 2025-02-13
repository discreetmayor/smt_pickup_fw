#ifndef _HARDWARE_TASK_H_
#define _HARDWARE_TASK_H_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <esp_err.h>

typedef struct {
    uint32_t task_stack_size;
    UBaseType_t task_priority;
    gpio_num_t io_pin;
    gpio_mode_t io_mode;
    const char *log_tag;
} hardware_config_t;

esp_err_t hardware_task_init(
    hardware_config_t *config, 
    TaskFunction_t task_function, 
    QueueHandle_t *queue
);
esp_err_t hardware_io_init(hardware_config_t *config);

#endif