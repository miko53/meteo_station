#include "anemometer.h"
#include "drivers/io.h"
#include "os/os.h"
#include "os/atomic.h"
#include "log.h"

#define ANEMOMETER_GPIO_INPUT                          (38)
#define ANEMOMETER_WAIT_TIME                           (10)
#define ANEMOMETER_CONVERTER                           ((0.33)/ANEMOMETER_WAIT_TIME)  // 1 pulse => 0.33 m/s

static atomic_t anemometer_count;
static TimerHandle_t anemometer_timer_handle;
static void IRAM_ATTR anemometer_isr_handler(void* arg);
static void anemometer_do_calcul( TimerHandle_t xTimer );

STATUS anemometer_init(void)
{
  STATUS s;
  uint64_t gpio_mask;

  s = STATUS_OK;
  anemometer_count = 0;
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
  log_info_print("count = %d => %f m/s (%f km/h)\n", count, count * ANEMOMETER_CONVERTER,
                 count * ANEMOMETER_CONVERTER * 3.6);
}


