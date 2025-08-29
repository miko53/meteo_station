#ifndef __CONFIG_H__
#define __CONFIG_H__

#define SD_CARD_MOUNT_POINT        "/sdcard"
#define METEO_STATION_VERSION      "v0.0.0"

#define CTRL_THREAD_PRIORITY       (configMAX_PRIORITIES - 10)
#define BUTTON_THREAD_PRIORTY      (configMAX_PRIORITIES - 10)
#define SCREEN_EVENT_THREAD_PRIORITY (configMAX_PRIORITIES - 12)
#define FILELOG_THREAD_PRIORITY    (configMAX_PRIORITIES - 15)
#define ZB_THREAD_PRIORITY         (configMAX_PRIORITIES - 11)

#define CTRL_THREAD_STACK_SIZE     (4196)
#define BUTTON_THREAD_STACK_SIZE   (4196)
#define FILELOG_THREAD_STACK_SIZE  (4196)
#define SCREEN_EVENT_THREAD_STACK_SIZE (2148)
#define ZB_THREAD_STACK_SIZE           (2148)



//gpio
#define ANEMOMETER_GPIO_INPUT      (38)
#define WINDDIR_GPIO_INPUT         (37)
#define RAINMETER_GPIO_INPUT       (34)

#define XBEE_RESET                 (22)
#define XBEE_SLEEP_RQ              (23)
#define XBEE_TX                    (21)
#define XBEE_RX                    (19)

//in second
#define ANEMOMETER_WAIT_TIME       (90.0)
#define WINDDIR_WAIT_TIME          (90.0)
#define RAINMETER_WAIT_TIME        (90.0)

#define SENSOR_INDEX_SLIDE_RAIN_FALL     (0)
#define SENSOR_INDEX_SLIDE_WIND_SPEED    (2)
#define SENSOR_INDEX_SLIDE_WIND_DIR      (4)

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


#endif /* __CONFIG_H__ */
