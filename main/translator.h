#ifndef _TRANSLATOR_H_
#define _TRANSLATOR_H_

#include <esp_err.h>

typedef void (*translator_handler_t)(const system_event_t *event);

esp_err_t translator_add_handler(
    event_type_t type, 
    translator_handler_t handler
);

#endif // _TRANSLATOR_H_