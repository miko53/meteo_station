#include "ctrl.h"
#include "os/os.h"
#include "filelog.h"
#include "log.h"

static TaskHandle_t ctrl_taskHandle = NULL;

static void ctrl_task(void* arg);

STATUS ctrl_init(void)
{
  STATUS s;
  s = STATUS_OK;

  xTaskCreate(ctrl_task, "ctrl_task", 4196, NULL, configMAX_PRIORITIES - 10, &ctrl_taskHandle);
  if (ctrl_taskHandle == NULL)
    s = STATUS_ERROR;

  return s;
}

static void ctrl_task(void* arg)
{
  UNUSED(arg);
  STATUS s;

  while (1)
  {
    thread_sleep(10);
    filelog_msg* pMsg;
    pMsg = filelog_allocate_msg();
    if (pMsg != NULL)
    {
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
