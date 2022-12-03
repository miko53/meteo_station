#ifndef __LOG_H__
#define __LOG_H__

#ifndef UNIT_TEST

#include "esp_log.h"

extern char* log_app_name;

#define log_dbg_print(...)      ESP_LOGD(log_app_name, __VA_ARGS__)
#define log_info_print(...)     ESP_LOGI(log_app_name, __VA_ARGS__)
#define log_warning_print(...)  ESP_LOGW(log_app_name, __VA_ARGS__)
#define log_error_print(...)    ESP_LOGE(log_app_name, __VA_ARGS__)

#endif

#ifdef UNIT_TEST

#include <stdio.h>

#define log_dbg_print(...)      fprintf(stdout, __VA_ARGS__)
#define log_info_print(...)     fprintf(stdout, __VA_ARGS__)
#define log_warning_print(...)  fprintf(stdout, __VA_ARGS__)
#define log_error_print(...)    fprintf(stdout, __VA_ARGS__)

#endif


#endif /* __LOG_H__ */
