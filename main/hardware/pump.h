#ifndef _PUMP_H_
#define _PUMP_H_

#include <driver/gptimer.h>
#include <esp_err.h>

esp_err_t pump_init();
bool timer_end(
    gptimer_handle_t timer, 
    const gptimer_alarm_event_data_t *event_data, 
    void *user_ctx
);
#endif // _PUMP_H_