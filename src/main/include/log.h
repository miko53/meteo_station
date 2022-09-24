#ifndef __LOG_H__
#define __LOG_H__

#include "esp_log.h"

extern char* log_app_name;

#define log_info_print(...) ESP_LOGI(log_app_name, __VA_ARGS__)




#endif /* __LOG_H__ */
