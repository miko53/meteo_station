#include "ctrl.h"
#include "config.h"
#include "filelog.h"
#include "log.h"
#include "data_defs.h"
#include "data_ope.h"
#include "data_ope_config.h"
#include "libs.h"
#include "ble.h"

#define CTRL_NB_MSG       (20)

static TaskHandle_t ctrl_taskHandle = NULL;
static QueueHandle_t ctrl_dataReception;

static void ctrl_task(void* arg);
static void ctrl_log_data(data_msg_t* pDataMsg);
static void ctrl_display_data_reception(data_msg_t* pDataMsg);

void insert_calculated_data(uint32_t indexSensor, variant_t* pData);

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

  if (s == STATUS_OK)
  {
    data_ope_cnf dataConfig;
    dataConfig.pDataOpeList = date_ope_config_get();
    dataConfig.nbItemsInList = date_ope_config_nbItems();
    dataConfig.on_new_calculated_data = insert_calculated_data;
    s = data_ope_init(&dataConfig);
    if (s == STATUS_OK)
      data_ope_activate_all(); //TODO
  }

  return s;
}

QueueHandle_t ctrl_get_data_queue(void)
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
      data_ope_add_sample(dataMsg.sensor, &dataMsg.value);
      ctrl_log_data(&dataMsg);
    }
    else
    {
      log_dbg_print("Timeout");
    }
  }
}


void ctrl_build_datalog_msg(char* string, uint32_t size, bool isComputed, uint32_t indexSensor, variant_t* pData)
{
  char* typeDataStr;
  switch (indexSensor)
  {
    case RAIN:
      typeDataStr = "rain";
      break;

    case WIND_DIR:
      typeDataStr = "wind_dir";
      break;

    case WIND_SPEED:
      typeDataStr = "wind_speed";
      break;

    default:
      typeDataStr = "???";
      break;
  }

  char computed;
  if (isComputed)
    computed = 'c';
  else
    computed = 'd';

  struct tm dateTime;
  date_get_localtime(&dateTime);

  if (pData->type == INTEGER_32)
  {
    snprintf(string, size - 1, "%c;%s;%.2d:%.2d:%.2d;%d\n", computed, typeDataStr, dateTime.tm_hour, dateTime.tm_min,
             dateTime.tm_sec, pData->i32);
  }
  else
  {
    snprintf(string, size - 1, "%c;%s;%.2d:%.2d:%.2d;%f\n", computed, typeDataStr, dateTime.tm_hour, dateTime.tm_min,
             dateTime.tm_sec, pData->f32);
  }
}

void insert_calculated_data(uint32_t indexSensor, variant_t* pData)
{
  log_info_print("insert calculated data (%d)", indexSensor);
  STATUS s;
  filelog_msg* pMsg;
  pMsg = filelog_allocate_msg();
  if (pMsg != NULL)
  {
    ctrl_build_datalog_msg(pMsg->data, FILELOG_STR_SIZE_MAX, true, indexSensor, pData);
    s = filelog_write(pMsg);
    if (s != STATUS_OK)
      log_info_print("error trying to send msg\n");
  }
  else
  {
    log_info_print("error trying to allocate msg\n");
  }

  ble_notify_new_data(date_ope_config_get_data_type(indexSensor), pData);
}

static void ctrl_log_data(data_msg_t* pDataMsg)
{
  STATUS s;
  filelog_msg* pMsg;
  pMsg = filelog_allocate_msg();
  if (pMsg != NULL)
  {
    ctrl_build_datalog_msg(pMsg->data, FILELOG_STR_SIZE_MAX, false, pDataMsg->sensor, &pDataMsg->value);
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
  switch (pDataMsg->sensor)
  {
    case HUMIDITY:
      break;

    case TEMPERATURE:
      break;

    case RAIN:
      log_info_print("RAIN %f mm", pDataMsg->value.f32);
      break;

    case WIND_DIR:
      log_info_print("WIND_DIR %d ", pDataMsg->value.i32);
      break;

    case WIND_SPEED:
      log_info_print("WIND_SPEED %f m/s", pDataMsg->value.f32);
      break;

    default:
      break;
  }
}

