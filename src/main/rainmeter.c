#include "rainmeter.h"
#include "drivers/io.h"
#include "os.h"
#include "atomic.h"
#include "log.h"
#include "data_defs.h"
#include "config.h"

#define RAINMETER_CONVERTER                           (0.2794)  // 1 pulse => 0.2794 mm

static atomic_t rainmeter_count;
static TimerHandle_t rainmeter_timer_handle;
static QueueHandle_t ctrl_data_queue;

static void IRAM_ATTR rainmeter_isr_handler(void* arg);
static void rainmeter_do_calcul( TimerHandle_t xTimer );

STATUS rainmeter_init(QueueHandle_t queueData)
{
  STATUS s;
  uint64_t gpio_mask;

  s = STATUS_OK;
  rainmeter_count = 0;
  ctrl_data_queue = queueData;

  gpio_mask = (1ULL << RAINMETER_GPIO_INPUT);
  s = io_configure_inputs(GPIO_INTR_NEGEDGE, gpio_mask);
  if (s == STATUS_OK)
  {
    io_configure_input_isr(RAINMETER_GPIO_INPUT, GPIO_INTR_NEGEDGE, rainmeter_isr_handler, NULL);

    rainmeter_timer_handle = xTimerCreate("rainmeter_count", OS_SEC_TO_TICK(RAINMETER_WAIT_TIME), pdTRUE, NULL,
                                          rainmeter_do_calcul);
    if (rainmeter_timer_handle == NULL)
      s = STATUS_ERROR;
  }

  if (s == STATUS_OK)
  {
    if ( xTimerStart(rainmeter_timer_handle, 0) != pdPASS )
    {
      log_dbg_print("Timer start error");
      s = STATUS_ERROR;
    }
  }
  return s;
}

static void rainmeter_isr_handler(void* arg)
{
  UNUSED(arg);
  atomic_add(&rainmeter_count, 1);
}

#ifdef SIMULATED_DATA
float rain_simulated;
#endif /* SIMULATED_DATA */

static void rainmeter_do_calcul( TimerHandle_t xTimer )
{
  UNUSED(xTimer);
  int32_t count = atomic_get(&rainmeter_count);
  atomic_set(&rainmeter_count, 0);
  //log_info_print("count = %d => %f mm\n", count, count * RAINMETER_CONVERTER);

  if (ctrl_data_queue != NULL)
  {
    data_msg_t msg;
    float v;
    msg.sensor = RAIN;

#ifdef SIMULATED_DATA
    rain_simulated += 1;
    v = rain_simulated;
#else
    v = count * RAINMETER_CONVERTER;
#endif /* SIMULATED_DATA */

    variant_f32(&msg.value, v);
    xQueueSend(ctrl_data_queue, &msg, OS_WAIT_FOREVER);
  }
}


