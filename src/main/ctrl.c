#include "ctrl.h"
#include "config.h"
#include "filelog.h"
#include "log.h"
#include "data_defs.h"

#define CTRL_NB_MSG       (20)

static TaskHandle_t ctrl_taskHandle = NULL;
static QueueHandle_t ctrl_dataReception;

static void ctrl_task(void* arg);

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
  STATUS s;
  data_msg_t dataMsg;
  BaseType_t rc;

  while (1)
  {
    rc = xQueueReceive(ctrl_dataReception, &dataMsg, OS_SEC_TO_TICK(10));
    if (rc == pdTRUE)
    {
      log_dbg_print("Data Reception");
      log_dbg_print("msg type = %d", dataMsg.type);
      if (dataMsg.container == INTEGER_32)
        log_dbg_print("msg value = %d", dataMsg.value.i);
      else
        log_dbg_print("msg value = %f", dataMsg.value.f);

    }
    else
    {
      log_dbg_print("Timeout");
    }

    filelog_msg* pMsg;
    pMsg = filelog_allocate_msg();
    if (pMsg != NULL)
    {
      snprintf(pMsg->data, FILELOG_STR_SIZE_MAX - 1, "test %d\n", tick_get());
      s = filelog_write(pMsg);
      if (s != STATUS_OK)
        log_info_print("error trying to send msg\n");
    }
    else
    {
      log_info_print("error trying to allocate msg\n");
    }
  }
}
