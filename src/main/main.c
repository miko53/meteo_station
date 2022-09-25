#include "common.h"
#include "log.h"
#include <stdio.h>
#include "drivers/i2c.h"
#include "drivers/ser_lcd.h"
#include "os/os.h"

void app_main(void)
{
  STATUS s;
  s = i2c_init();
  log_info_print("status s=%d\n", s);
  s = ser_lcd_init();
  log_info_print("status s=%d\n", s);
  fprintf(stdout, "Hello World\n");

  ser_lcd_write_screen("METEO STATION   "
                       "          v0.0.0");
  thread_msleep(5000);
  ser_lcd_write_line(1, " TOTO");

  thread_msleep(5000);
  ser_lcd_clear_screen();

  ser_lcd_write_line(1, " TATA");
  ser_lcd_write_line(0, "TUTU-->");

  thread_msleep(2000);

  ser_lcd_cursor_on();
  thread_msleep(2000);
  ser_lcd_blink_on();

}
