#include "ctrl.h"
#include "config.h"
#include "filelog.h"
#include "log.h"
#include "data_defs.h"
#include <time.h>
#include <sys/time.h>

#define CTRL_NB_MSG       (20)

static TaskHandle_t ctrl_taskHandle = NULL;
static QueueHandle_t ctrl_dataReception;

static void ctrl_task(void* arg);
static void ctrl_log_data(data_msg_t* pDataMsg);
static void ctrl_display_data_reception(data_msg_t* pDataMsg);

STATUS ctrl_init(void)
{
  STATUS s;
  s = STATUS_OK;

  ctrl_dataReception = xQueueCreate(CTRL_NB_MSG, sizeof(data_msg_t));
  if (ctrl_dataReception == NULL)
    s = STATUS_ERROR;

  if (s == STATUS_OK)
  {
    xTaskCreate(ctrl_task, "ctrl_task", CTRL_THREAD_STACK_SIZE, NULL, CTRL_THREAD_PRIORITY, &ctrl_taskHandle);
    if (ctrl_taskHandle == NULL)
      s = STATUS_ERROR;
  }

  return s;
}

QueueHandle_t ctrl_get_data_queue()
{
  return ctrl_dataReception;
}

static void ctrl_task(void* arg)
{
  UNUSED(arg);
  data_msg_t dataMsg;
  BaseType_t rc;

  while (1)
  {
    rc = xQueueReceive(ctrl_dataReception, &dataMsg, OS_SEC_TO_TICK(10));
    if (rc == pdTRUE)
    {
      log_dbg_print("Data Reception");
      ctrl_display_data_reception(&dataMsg);
      ctrl_log_data(&dataMsg);
    }
    else
    {
      log_dbg_print("Timeout");
    }
  }
}

void ctrl_get_time(struct tm* pDate)
{
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  localtime_r(&currentTime.tv_sec, pDate);
}

static void ctrl_log_data(data_msg_t* pDataMsg)
{
  STATUS s;
  struct tm dateTime;
  filelog_msg* pMsg;
  pMsg = filelog_allocate_msg();
  if (pMsg != NULL)
  {
    ctrl_get_time(&dateTime);
    switch (pDataMsg->type)
    {
      case RAIN:
        snprintf(pMsg->data, FILELOG_STR_SIZE_MAX - 1, "d;rain;%.2d:%.2d:%.2d;%f\n", dateTime.tm_hour, dateTime.tm_min,
                 dateTime.tm_sec, pDataMsg->value.f);
        break;

      case WIND_DIR:
        snprintf(pMsg->data, FILELOG_STR_SIZE_MAX - 1, "d;wind_dir;%.2d:%.2d:%.2d;%d\n", dateTime.tm_hour, dateTime.tm_min,
                 dateTime.tm_sec, pDataMsg->value.i);
        break;

      case WIND_SPEED:
        snprintf(pMsg->data, FILELOG_STR_SIZE_MAX - 1, "d;wind_speed;%.2d:%.2d:%.2d;%f\n", dateTime.tm_hour, dateTime.tm_min,
                 dateTime.tm_sec, pDataMsg->value.f);
        break;

      default:
        pMsg->data[0] = '\0';
        break;
    }

    s = filelog_write(pMsg);
    if (s != STATUS_OK)
      log_info_print("error trying to send msg\n");
  }
  else
  {
    log_info_print("error trying to allocate msg\n");
  }

}

static void ctrl_display_data_reception(data_msg_t* pDataMsg)
{
  switch (pDataMsg->type)
  {
    case HUMIDITY:
      break;
    case TEMPERATURE:
      break;
    case RAIN:
      log_info_print("RAIN %f mm", pDataMsg->value.f);
      break;

    case WIND_DIR:
      log_info_print("WIND_DIR %d ", pDataMsg->value.i);
      break;

    case WIND_SPEED:
      log_info_print("WIND_SPEED %f m/s", pDataMsg->value.f);
      break;

    default:
      break;
  }
}

