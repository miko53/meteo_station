#include "common.h"
#include "log.h"
#include <stdio.h>
#include "drivers/i2c.h"
#include "drivers/ser_lcd.h"
#include "os/os.h"
#include "drivers/io.h"
#include "button.h"

static void test_lcd(void);

void app_main(void)
{
  STATUS s;

  io_init();

  s = i2c_init();
  log_info_print("status s=%d\n", s);
  s = ser_lcd_init();
  log_info_print("status s=%d\n", s);
  s = button_init();
  log_info_print("status s=%d\n", s);


  test_lcd();
}

void test_lcd()
{
  fprintf(stdout, "Hello World\n");

  ser_lcd_write_screen("METEO STATION   "
                       "          v0.0.0");
  thread_msleep(5000);
  ser_lcd_write_line(1, " TOTO");

  thread_msleep(5000);
  ser_lcd_clear_screen();

  //ser_lcd_autoscroll_on();

  ser_lcd_write_line(1, " TATA");
  ser_lcd_write_line(0, "TUTU-->");

  //ser_lcd_autoscroll_off();

  thread_msleep(2000);

  ser_lcd_cursor_on();
  thread_msleep(2000);
  ser_lcd_blink_on();

  for (uint32_t i = 0; i < 10; i++)
  {
    ser_lcd_scroll_left(1);
    thread_msleep(500);
  }

  for (uint32_t i = 0; i < 10; i++)
  {
    ser_lcd_scroll_right(1);
    thread_msleep(500);
  }

  ser_lcd_system_msg_off();

  for (uint32_t i = 0; i < 5; i++)
  {
    uint8_t constrast = (i) * 63;
    char s[16];
    snprintf(s, 16, "C--> %d", constrast);
    ser_lcd_write_line(0, s);
    ser_lcd_set_contrast((i - 1) * 64);
    thread_msleep(5000);
  }
  ser_lcd_set_contrast(0);
  //ser_lcd_set_fast_backlight(128, 0, 128);


}
