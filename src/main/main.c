#include "common.h"
#include "log.h"
#include <stdio.h>
#include "drivers/i2c.h"
#include "drivers/ser_lcd.h"
#include "drivers/pcf_8523.h"
#include "drivers/sd_card.h"
#include "os/os.h"
#include "drivers/io.h"
#include "button.h"
#include "menu.h"
#include "screen.h"
#include "filelog.h"
#include "config.h"
#include <sys/time.h>
#include "libs.h"
#include "rainmeter.h"
#include "ctrl.h"

void app_main(void)
{
  STATUS s;

  io_init();
  setenv("TZ", "CEST", 1);
  tzset();

  s = i2c_init();
  log_info_print("i2c status s=%d\n", s);

  pcf_8523_init();

  s = ser_lcd_init();
  log_info_print("ser_lcd status s=%d\n", s);

  s = button_init();
  log_info_print("button status s=%d\n", s);

  s = screen_init();
  log_info_print("screen status s=%d\n", s);

  s = menu_init();
  log_info_print("menu status s=%d\n", s);

  s = sd_card_init(SD_CARD_MOUNT_POINT);
  log_info_print("sdcard init s=%d\n", s);

  struct tm dateTime;
  pcf8523_get_date(&dateTime);
  dateTime.tm_mday = 1;
  time_t t = mktime(&dateTime);
  struct timeval tv;
  tv.tv_sec = t;
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);

  s = filelog_init();
  log_info_print("filelog status s=%d\n", s);

  s = ctrl_init();
  log_info_print("ctrl status s=%d\n", s);

  s = rainmeter_init();
  log_info_print("rainmeter status s=%d\n", s);
}
