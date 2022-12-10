#ifndef __CONFIG_H__
#define __CONFIG_H__

#define SD_CARD_MOUNT_POINT        "/sdcard"

#define CTRL_THREAD_PRIORITY       (configMAX_PRIORITIES - 10)
#define BUTTON_THREAD_PRIORTY      (configMAX_PRIORITIES - 10)
#define SCREEN_EVENT_THREAD_PRIORITY (configMAX_PRIORITIES - 12)
#define FILELOG_THREAD_PRIORITY    (configMAX_PRIORITIES - 15)

#define CTRL_THREAD_STACK_SIZE     (4196)
#define BUTTON_THREAD_STACK_SIZE   (4196)
#define FILELOG_THREAD_STACK_SIZE  (4196)
#define SCREEN_EVENT_THREAD_STACK_SIZE (2148)


//gpio
#define ANEMOMETER_GPIO_INPUT      (38)
#define WINDDIR_GPIO_INPUT         (37)
#define RAINMETER_GPIO_INPUT       (34)

//in second
#define ANEMOMETER_WAIT_TIME       (10)
#define WINDDIR_WAIT_TIME          (10)
#define RAINMETER_WAIT_TIME        (10) //  (15*60) //15 min

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


#endif /* __CONFIG_H__ */
