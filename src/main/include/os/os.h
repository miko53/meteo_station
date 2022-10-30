#ifndef __OS_H__
#define __OS_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#define OS_WAIT_FOREVER                   portMAX_DELAY

#define OS_MSEC_TO_TICK(msec)             ((msec)/(portTICK_PERIOD_MS))
#define OS_SEC_TO_TICK(sec)               ((sec*1000)/(portTICK_PERIOD_MS))

#define thread_msleep(msec)               vTaskDelay(OS_MSEC_TO_TICK(msec))
#define thread_sleep(sec)                 vTaskDelay(OS_SEC_TO_TICK(sec))

#define tick_get()                        xTaskGetTickCount()

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif /* __OS_H__ */
