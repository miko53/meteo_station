#ifndef __OS_H__
#define __OS_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define OS_MSEC_TO_TICK(msec)             ((msec)/(portTICK_PERIOD_MS))
#define OS_SEC_TO_TICK(sec)               ((sec*1000)/(portTICK_PERIOD_MS))

#define thread_msleep(msec)               vTaskDelay(OS_MSEC_TO_TICK(msec));

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif /* __OS_H__ */
