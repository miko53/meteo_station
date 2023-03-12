#include "anemometer.h"
#include "drivers/io.h"
#include "os.h"
#include "atomic.h"
#include "log.h"
#include "data_defs.h"
#include "config.h"

#define ANEMOMETER_CONVERTER                           ((0.666666)/ANEMOMETER_WAIT_TIME)  // 1 pulse/s => 2.4km/h ==> 0.66 m/s

static atomic_t anemometer_count;
static TimerHandle_t anemometer_timer_handle;
static QueueHandle_t ctrl_data_queue;

static void IRAM_ATTR anemometer_isr_handler(void* arg);
static void anemometer_do_calcul( TimerHandle_t xTimer );

STATUS anemometer_init(QueueHandle_t queueData)
{
  STATUS s;
  uint64_t gpio_mask;

  s = STATUS_OK;
  anemometer_count = 0;
  ctrl_data_queue = queueData;

  gpio_mask = (1ULL << ANEMOMETER_GPIO_INPUT);
  s = io_configure_inputs(GPIO_INTR_NEGEDGE, gpio_mask);
  if (s == STATUS_OK)
  {
    io_configure_input_isr(ANEMOMETER_GPIO_INPUT, GPIO_INTR_NEGEDGE, anemometer_isr_handler, NULL);

    anemometer_timer_handle = xTimerCreate("anemometer_count", OS_SEC_TO_TICK(ANEMOMETER_WAIT_TIME), pdTRUE, NULL,
                                           anemometer_do_calcul);
    if (anemometer_timer_handle == NULL)
      s = STATUS_ERROR;
  }

  if (s == STATUS_OK)
  {
    if ( xTimerStart(anemometer_timer_handle, 0) != pdPASS )
    {
      log_dbg_print("Timer start error");
      s = STATUS_ERROR;
    }
  }
  return s;
}

#ifdef SIMULATED_DATA
float windspeed_simulated;
#endif /* SIMULATED_DATA */

static void anemometer_isr_handler(void* arg)
{
  UNUSED(arg);
  atomic_add(&anemometer_count, 1);
}

static void anemometer_do_calcul( TimerHandle_t xTimer )
{
  UNUSED(xTimer);
  int32_t count = atomic_get(&anemometer_count);
  atomic_set(&anemometer_count, 0);
  //log_info_print("count = %d => %f m/s (%f km/h)\n", count, count * ANEMOMETER_CONVERTER,
  //               count * ANEMOMETER_CONVERTER * 3.6);

  if (ctrl_data_queue != NULL)
  {
    data_msg_t msg;
    msg.sensor = WIND_SPEED;
    float v;
#ifdef SIMULATED_DATA
    windspeed_simulated += 0.3;
    v = windspeed_simulated;
#else
    v = count * ANEMOMETER_CONVERTER;
#endif /* SIMULATED_DATA */
    variant_f32(&msg.value, v);
    xQueueSend(ctrl_data_queue, &msg, OS_WAIT_FOREVER);
  }
}
