#include "rainmeter.h"
#include "drivers/io.h"
#include "os/os.h"
#include "os/atomic.h"
#include "log.h"

#define RAINMETER_GPIO_INPUT                          (34)
#define RAINMETER_WAIT_TIME                           (10)//  (15*60) //15 min
#define RAINMETER_CONVERTER                           (0.3)  // 1 pulse => 0.3 mm

static atomic_t rainmeter_count;
static TimerHandle_t rainmeter_timer_handle;
static void IRAM_ATTR rainmeter_isr_handler(void* arg);
static void rainmeter_do_calcul( TimerHandle_t xTimer );

STATUS rainmeter_init(void)
{
  STATUS s;
  uint64_t gpio_mask;

  s = STATUS_OK;
  rainmeter_count = 0;
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

static void rainmeter_do_calcul( TimerHandle_t xTimer )
{
  UNUSED(xTimer);
  int32_t count = atomic_get(&rainmeter_count);
  atomic_set(&rainmeter_count, 0);
  log_info_print("count = %d => %f mm\n", count, count * RAINMETER_CONVERTER);
}


