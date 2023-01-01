#include "zigbee.h"
#include "drivers/io.h"
#include "config.h"
#include "driver/uart.h"
#include "log.h"

#define ZB_BAUD_RATE      (19200)
#define ZB_UART_PORT_NUM  (1)
#define ZB_BUFFER_SIZE    (256)


static TaskHandle_t zb_taskHandle = NULL;
static void zigbee_task(void* arg);

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
  rc = uart_driver_install(ZB_UART_PORT_NUM, ZB_BUFFER_SIZE * 2, 0, 0, NULL, intr_alloc_flags);
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

  if (rc == ESP_OK)
  {
    xTaskCreate(zigbee_task, "zb_task", ZB_THREAD_STACK_SIZE, NULL, ZB_THREAD_PRIORITY, &zb_taskHandle);
    if (zb_taskHandle == NULL)
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

void zigbee_task ( void* arg )
{
  UNUSED(arg);

  uint8_t* data = (uint8_t*) malloc(ZB_BUFFER_SIZE);
  while (1)
  {
    int len = uart_read_bytes(ZB_UART_PORT_NUM, data, (ZB_BUFFER_SIZE - 1), 20 / portTICK_PERIOD_MS);
    // Write data back to the UART
    uart_write_bytes(ZB_UART_PORT_NUM, (const char*) data, len);
    if (len)
    {
      data[len] = '\0';
      log_info_print("Recv (%d) '%s'", len, (char*) data);
      for (uint32_t i = 0; i < len; i++)
      {
        fprintf(stdout, "0x%.2x-", data[i]);
      }
      fprintf(stdout, "\n");
    }
  }
}


