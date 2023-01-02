#include "zigbee.h"
#include "drivers/io.h"
#include "config.h"
#include "driver/uart.h"
#include "log.h"
#include "os.h"
#include "zb.h"
#include <string.h>
#include <assert.h>
#include "zb_data.h"

#define ZB_BAUD_RATE      (19200)
#define ZB_UART_PORT_NUM  (1)
#define ZB_BUFFER_SIZE    (128)

#define ZB_NB_MAX_MSG     (5)

typedef enum
{
  ZB_STATUS_NOT_JOINED,
  ZB_STATUS_JOINED,
} zb_status_t;

static TaskHandle_t zb_taskHandleRx = NULL;
static TaskHandle_t zb_taskHandleTx = NULL;
static QueueHandle_t zb_rxQueue;
static QueueHandle_t zb_txQueue;
static QueueHandle_t zb_msgAllocatedHandle;

static uint8_t zb_rx_buffer[ZB_BUFFER_SIZE];
static uint32_t zb_rx_size;
static zb_status_t zb_joined_status;

static STATUS zb_pre_allocate_msg(QueueHandle_t* msgReserveQueue);
static void zigbee_task_rx(void* arg);
static void zigbee_task_tx(void* arg);
static bool zigbee_on_receive_data(uint8_t* data, uint32_t size, zigbee_decodedFrame* decodedFrame);
static void zigbee_display_frame(uint8_t* data, uint32_t len);
static void zigbee_check_frame(zigbee_decodedFrame* decodedFrame);

STATUS zigbee_init(void)
{
  uart_config_t uart_zb_config =
  {
    .baud_rate = ZB_BAUD_RATE,
    .data_bits = UART_DATA_8_BITS,
    .parity    = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .source_clk = UART_SCLK_DEFAULT,
  };
  int intr_alloc_flags = 0;

  esp_err_t rc;
  rc = uart_driver_install(ZB_UART_PORT_NUM, ZB_BUFFER_SIZE * 2, ZB_BUFFER_SIZE * 2, 20, &zb_rxQueue, intr_alloc_flags);
  if (rc != ESP_OK)
  {
    log_error_print("driver install error %d", rc);
  }

  if (rc == ESP_OK)
  {
    rc = uart_param_config(ZB_UART_PORT_NUM, &uart_zb_config);
    if (rc != ESP_OK)
    {
      log_error_print("uart_param_config error %d", rc);
    }
  }

  if (rc == ESP_OK)
  {
    rc = uart_set_pin(ZB_UART_PORT_NUM, XBEE_TX, XBEE_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (rc != ESP_OK)
    {
      log_error_print("uart_set_pin error %d", rc);
    }
  }

  zb_msgAllocatedHandle = xQueueCreate(ZB_NB_MAX_MSG, sizeof(zb_payload_frame*));
  if (zb_msgAllocatedHandle == NULL)
    rc = ESP_ERR_INVALID_ARG;
  else
  {
    STATUS s;
    s = zb_pre_allocate_msg(&zb_msgAllocatedHandle);
    if (s != STATUS_OK)
      rc = ESP_ERR_INVALID_ARG;
  }


  if (rc == ESP_OK)
  {
    zb_txQueue = xQueueCreate(10, sizeof(zb_payload_frame*));
    if (zb_txQueue == NULL)
      rc = ESP_ERR_INVALID_ARG;
  }

  if (rc == ESP_OK)
  {
    xTaskCreate(zigbee_task_rx, "zb_task_rx", ZB_THREAD_STACK_SIZE, NULL, ZB_THREAD_PRIORITY, &zb_taskHandleRx);
    if (zb_taskHandleRx == NULL)
      rc = ESP_ERR_INVALID_ARG;
  }

  if (rc == ESP_OK)
  {
    xTaskCreate(zigbee_task_tx, "zb_task_tx", ZB_THREAD_STACK_SIZE, NULL, ZB_THREAD_PRIORITY, &zb_taskHandleTx);
    if (zb_taskHandleTx == NULL)
      rc = ESP_ERR_INVALID_ARG;
  }

  if (rc == ESP_OK)
  {
    io_configure_output(XBEE_RESET, false);
    io_configure_output(XBEE_SLEEP_RQ, false);
    io_configure_output(XBEE_RESET, true);
  }

  return STATUS_OK;
}

static STATUS zb_pre_allocate_msg(QueueHandle_t* msgReserveQueue)
{
  STATUS s;
  BaseType_t rc;
  zb_payload_frame* pMsg;
  s = STATUS_OK;

  for (uint32_t i = 0; ((i < ZB_NB_MAX_MSG) && (s == STATUS_OK)); i++)
  {
    pMsg = calloc(1, sizeof(zb_payload_frame));
    if (pMsg != NULL)
    {
      rc = xQueueSend(*msgReserveQueue, &pMsg, 0);
      assert(rc == pdTRUE);
    }
    else
    {
      log_info_print("Unable to allocate msg (%d)", i);
      s = STATUS_ERROR;
      break;
    }
  }

  return s;
}

zb_payload_frame* zb_allocate_msg()
{
  BaseType_t rc;
  zb_payload_frame* pMsg = NULL;
  rc = xQueueReceive(zb_msgAllocatedHandle, &pMsg, 0);
  if (rc != pdTRUE)
    pMsg = NULL;
  return pMsg;
}

void zb_free_msg(zb_payload_frame* msg)
{
  BaseType_t rc;
  rc = xQueueSend(zb_msgAllocatedHandle, &msg, 0);
  assert(rc == pdTRUE);
}

static void zigbee_task_rx ( void* arg )
{
  UNUSED(arg);
  zigbee_decodedFrame decodedFrame;
  uart_event_t event;
  uint8_t data[ZB_BUFFER_SIZE];
  uint32_t size;
  size = 0;

  while (1)
  {
    xQueueReceive(zb_rxQueue, (void*) &event, OS_WAIT_FOREVER);
    switch (event.type)
    {
      case UART_DATA:
        log_info_print("event %d, Recv (%d)", event.type, event.size);
        int len = uart_read_bytes(ZB_UART_PORT_NUM, data, event.size, OS_WAIT_FOREVER);
        zigbee_display_frame(data, len);
        bool hasFrame = zigbee_on_receive_data(data, len, &decodedFrame);
        if (hasFrame)
          zigbee_check_frame(&decodedFrame);
        break;

      default:
        log_info_print("event %d, Recv (%d)", event.type, event.size);
        break;
    }
  }
}

static void zigbee_display_frame(uint8_t* data, uint32_t len)
{
  for (uint32_t i = 0; i < len; i++)
  {
    fprintf(stdout, "0x%.2x-", data[i]);
  }
  fprintf(stdout, "\n");
}

static bool zigbee_on_receive_data(uint8_t* data, uint32_t size, zigbee_decodedFrame* decodedFrame)
{
  bool bSuccess;
  bSuccess = false;

  if ((zb_rx_size + size) < ZB_BUFFER_SIZE)
  {
    memcpy(&zb_rx_buffer[zb_rx_size], data, size);
    zb_rx_size += size;
  }
  else
  {
    zb_rx_size = 0;
  }

  if (zb_rx_size >= ZB_HEADER_SIZE)
  {
    //start decoding now...
    if (zb_rx_buffer[0] == ZB_START_DELIMITER)
    {
      uint32_t sizeOfNextData = (((uint16_t) zb_rx_buffer[1]) << 8) | (zb_rx_buffer[2]);
      if ((zb_rx_size - ZB_HEADER_SIZE) >= (sizeOfNextData + 1))
      {
        bSuccess = zb_decodage(zb_rx_buffer + ZB_HEADER_SIZE, sizeOfNextData + 1, decodedFrame);
        zb_rx_size = 0;
      }
      else
      {
        //not enough data wait ....
        ;
      }
    }
    else
    {
      //error
      zb_rx_size = 0;
    }
  }
  // log_info_print("bSuccess = %d", bSuccess);
  return bSuccess;
}

static void zigbee_check_frame(zigbee_decodedFrame* decodedFrame)
{
  switch (decodedFrame->type)
  {
    case ZIGBEE_API_AT_CMD:
      break;

    case ZIGBEE_API_TRANSMIT_REQUEST:
      break;

    case ZIGBEE_AT_COMMAND_RESPONSE:
      break;

    case ZIGBEE_MODEM_STATUS:
      if (decodedFrame->status == 0x02)
      {
        zb_joined_status = ZB_STATUS_JOINED;
        log_info_print("zigbee joined network");
      }
      else if (decodedFrame->status == 0x03)
      {
        zb_joined_status = ZB_STATUS_NOT_JOINED;
        log_info_print("zigbee leave network");
      }
      break;

    case ZIGBEE_TRANSMIT_STATUS:
#ifdef USE_FRAME_ID
      if (zb_currentFrameID == decodedFrame.frameID)
      {
        zb_currentAck = decodedFrame.status;
      }
      else
      {
        ;
      }
#endif /* USE_FRAME_ID */
      break;

    case ZIGBEE_RECEIVE_PACKET:
      break;

    default:
      break;
  }
}

static uint8_t zigbee_appendChecksum(uint8_t* buffer, uint8_t* sizeFrame);
static void zb_build_and_send_msg(zb_payload_frame* pMsg);

void zigbee_task_tx ( void* arg )
{
  UNUSED(arg);

  zb_payload_frame* pMsg;
  static uint8_t counter = 0;

  while (1)
  {
    thread_sleep(10);
    pMsg = zb_allocate_msg();

    pMsg->dataType = SENSOR_PROTOCOL_DBG_TYPE;
    pMsg->counter = counter++;
    pMsg->dbgFrame.v1 = 0;
    pMsg->dbgFrame.v2 = 1;
    pMsg->dbgFrame.v3 = counter;

    zb_build_and_send_msg(pMsg);
    /*
    xQueueReceive(zb_txQueue, (void*) &pMsg, OS_WAIT_FOREVER);
    if (zb_joined_status == ZB_STATUS_JOINED)
    {

    }*/
    log_info_print("zb_msg %d", pMsg->dataType);
    zb_free_msg(pMsg);
  }
}

void zb_build_and_send_msg(zb_payload_frame* pMsg)
{
  uint8_t buffer[ZB_BUFFER_SIZE];
  uint32_t size = 0;
  buffer[size++] = ZB_START_DELIMITER;
  buffer[size++] = 0;
  buffer[size++] = 0;
  buffer[size++] = ZIGBEE_API_TRANSMIT_REQUEST;
  buffer[size++] = 0; //frame ID
  buffer[size++] = 0; //coordinator @1
  buffer[size++] = 0; //coordinator @2
  buffer[size++] = 0; //coordinator @3
  buffer[size++] = 0; //coordinator @4
  buffer[size++] = 0; //coordinator @5
  buffer[size++] = 0; //coordinator @6
  buffer[size++] = 0; //coordinator @7
  buffer[size++] = 0; //coordinator @8
  buffer[size++] = 0xFF; //ZIGBEE_UNKNOWN_16B_ADDR
  buffer[size++] = 0xFE; //ZIGBEE_UNKNOWN_16B_ADDR
  buffer[size++] = 0; //broadcast radius
  buffer[size++] = 0; //options
  buffer[size++] = pMsg->dataType;
  buffer[size++] = pMsg->counter;

  switch (pMsg->dataType)
  {
    case SENSOR_PROTOCOL_DATA_TYPE:
      break;

    case SENSOR_PROTOCOL_DBG_TYPE:
      memcpy(&buffer[size], &pMsg->dbgFrame, sizeof(pMsg->dbgFrame));
      size += sizeof(pMsg->dbgFrame);
      break;

    default:
      break;
  }

  buffer[1] = ((size - ZB_HEADER_SIZE) & 0xFF00) >> 8;
  buffer[2] = ((size - ZB_HEADER_SIZE) & 0x00FF) >> 0;

  uint8_t frameSize = size;
  frameSize = zigbee_appendChecksum(buffer, &frameSize);

  int len = uart_write_bytes(ZB_UART_PORT_NUM, buffer, frameSize);

  zigbee_display_frame(buffer, frameSize);

  log_info_print("len %d", len);
}

static uint8_t zigbee_appendChecksum(uint8_t* buffer, uint8_t* sizeFrame)
{
  buffer[*sizeFrame] = zb_doChecksum(&buffer[ZB_HEADER_SIZE], &buffer[*sizeFrame] - &buffer[ZB_HEADER_SIZE]);
  return *sizeFrame + 1;
}

