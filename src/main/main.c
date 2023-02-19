#include "common.h"
#include "log.h"
#include <stdio.h>
#include "drivers/i2c.h"
#include "drivers/ser_lcd.h"
#include "drivers/pcf_8523.h"
#include "drivers/sd_card.h"
#include "drivers/analog.h"
#include "os.h"
#include "drivers/io.h"
#include "button.h"
#include "menu.h"
#include "screen.h"
#include "filelog.h"
#include "config.h"
#include <sys/time.h>
#include "libs.h"
#include "rainmeter.h"
#include "anemometer.h"
#include "winddir.h"
#include "ctrl.h"
#include "nvstorage.h"
#include "ble.h"
#include "zigbee.h"

void panic(void);

void driver_init(void)
{
  STATUS s;
  s = io_init();
  if (s != STATUS_OK)
  {
    log_error_print("io_init failed!");
    panic();
  }

  analog_display_efuse_config();

  s = i2c_init();
  if (s != STATUS_OK)
  {
    log_error_print("i2c_init failed!");
    panic();
  }

  s = pcf_8523_init();
  if (s != STATUS_OK)
  {
    log_error_print("pcf_8523_init failed!");
    panic();
  }

  s = ser_lcd_init();
  if (s != STATUS_OK)
  {
    log_error_print("ser_lcd_init failed!");
    panic();
  }
}

void middleware_init(void)
{
  STATUS s;

  s = nvstorage_init();
  if (s != STATUS_OK)
  {
    log_error_print("nvstorage failed!");
    panic();
  }

  s = button_init();
  if (s != STATUS_OK)
  {
    log_error_print("button_init failed!");
    panic();
  }

  s = screen_init();
  if (s != STATUS_OK)
  {
    log_error_print("screen_init failed!");
    panic();
  }

  s = menu_init();
  if (s != STATUS_OK)
  {
    log_error_print("menu_init failed!");
    panic();
  }

  s = sd_card_init(SD_CARD_MOUNT_POINT);
  if (s != STATUS_OK)
  {
    log_error_print("sd_card_init failed! --> desactivated");
    panic();
  }
}

void app_init(void)
{
  STATUS s;

  s = filelog_init();
  if (s != STATUS_OK)
  {
    log_error_print("filelog_init failed!");
    panic();
  }

  s = ctrl_init();
  if (s != STATUS_OK)
  {
    log_error_print("ctrl_init failed!");
    panic();
  }

  QueueHandle_t ctrDataQueue = ctrl_get_data_queue();

  s = rainmeter_init(ctrDataQueue);
  if (s != STATUS_OK)
  {
    log_error_print("rainmeter_init failed!");
    panic();
  }

  s = anemometer_init(ctrDataQueue);
  if (s != STATUS_OK)
  {
    log_error_print("anemometer_init failed!");
    panic();
  }

  s = winddir_init(ctrDataQueue);
  if (s != STATUS_OK)
  {
    log_error_print("winddir_init failed!");
    panic();
  }

  if (nvstorage_get_ble_state() == true)
  {
    s = ble_init();
    if (s != STATUS_OK)
    {
      log_error_print("ble failed!");
      panic();
    }
  }

  if (nvstorage_get_zb_state() == true)
  {
    s = zigbee_init();
    if (s != STATUS_OK)
    {
      log_error_print("zigbee failed!");
      panic();
    }
  }
}


void app_main(void)
{
  setenv("TZ", "CEST", 1);
  tzset();

  driver_init();
  middleware_init();

  struct tm dateTime;
  pcf8523_get_date(&dateTime);
  date_set_localtime(&dateTime);

  app_init();



  log_info_print("end of initialisation: everything is ok...");
}


void panic(void)
{
  while (1);
}
