#include "winddir.h"
#include "drivers/analog.h"
#include "os.h"
#include "log.h"
#include "data_defs.h"

#define WINDDIR_GPIO_INPUT     (37)

#define WINDDIR_WIDTH         ADC_WIDTH_BIT_12
#define WINDDIR_ATTEN         ADC_ATTEN_DB_11

#define WINDDIR_WAIT_TIME     (1)

static void* winddir_analog_handle;
static TimerHandle_t winddir_timer_handle;
static QueueHandle_t ctrl_data_queue;

static void winddir_do_calcul( TimerHandle_t xTimer );

STATUS winddir_init(QueueHandle_t queueData)
{
  STATUS s;

  ctrl_data_queue = queueData;

  s = analog_configure(WINDDIR_GPIO_INPUT, WINDDIR_WIDTH, WINDDIR_ATTEN, &winddir_analog_handle);

  if (s == STATUS_OK)
  {
    winddir_timer_handle = xTimerCreate("winddir_count", OS_SEC_TO_TICK(WINDDIR_WAIT_TIME), pdTRUE, NULL,
                                        winddir_do_calcul);
    if (winddir_timer_handle == NULL)
      s = STATUS_ERROR;
  }

  if (s == STATUS_OK)
  {
    if ( xTimerStart(winddir_timer_handle, 0) != pdPASS )
    {
      log_dbg_print("Timer start error");
      s = STATUS_ERROR;
    }
  }

  return s;
}


char* winddir_direction(winddir_direction_t dir)
{
  char* r;
  switch (dir)
  {
    case E:
      r = "Est";
      break;
    case N:
      r = "Nord";
      break;
    case NE:
      r = "Nord-Est";
      break;
    case NO:
      r = "Nord-Ouest";
      break;
    case O:
      r = "Ouest";
      break;
    case S:
      r = "Sud";
      break;
    case SE:
      r = "Sud-Est";
      break;
    case SO:
      r = "Sud-Ouest";
      break;
    default:
      r = "?";
      break;
  }
  return r;
}

void print_direction(winddir_direction_t dir)
{
  log_info_print("Dir: %s", winddir_direction(dir));
}

void winddir_do_calcul(TimerHandle_t xTimer)
{
  uint32_t v;
  winddir_direction_t direction;
  v = analog_do_conversion(winddir_analog_handle);
  v = v / 100;
  log_dbg_print("v %d", v);

  direction = INVALID;
  if (v <= 3)
    direction = E;
  else if ((v > 3) && (v <= 6))
    direction = SE;
  else if ((v > 6) && (v <= 9))
    direction = S;
  else if ((v > 11) && (v <= 15))
    direction = NE;
  else if ((v > 15) && (v <= 20))
    direction = SO;
  else if ((v > 20) && (v <= 25))
    direction = N;
  else if ((v > 25) && (v <= 28))
    direction = NO;
  else if ((v > 28))
    direction = O;

  print_direction(direction);
  if (ctrl_data_queue != NULL)
  {
    data_msg_t msg;
    msg.type = WIND_DIR;
    msg.container = INTEGER_32;
    msg.value.i = direction;
    xQueueSend(ctrl_data_queue, &msg, OS_WAIT_FOREVER);
  }
}


