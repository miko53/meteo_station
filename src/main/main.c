#include "common.h"
#include "log.h"
#include <stdio.h>
#include "drivers/i2c.h"
#include "drivers/ser_lcd.h"
#include "os/os.h"
#include "drivers/io.h"
#include "button.h"
#include "menu.h"
#include "screen.h"

static void test_lcd(void);
static void test_button(void);

void app_main(void)
{
  STATUS s;

  io_init();

  s = i2c_init();
  log_info_print("i2c status s=%d\n", s);
  s = ser_lcd_init();
  log_info_print("ser_lcd status s=%d\n", s);
  s = button_init();
  log_info_print("button status s=%d\n", s);

  s = screen_init();
  log_info_print("screen status s=%d\n", s);

  s = menu_init();
  log_info_print("menu status s=%d\n", s);

  //test_button();
  //test_lcd();
}

void button_cmd_handler(key_event evt)
{
  printf("button cmd evt %d\n", evt);
}

void button_plus_handler(key_event evt)
{
  printf("button plus evt %d\n", evt);
}
void button_minus_handler(key_event evt)
{
  printf("button minus evt %d\n", evt);
}


void test_button()
{
  STATUS s;
  s = button_install_handler(BUTTON_CMD, KEY_PRESSED, button_cmd_handler);
  assert(s == STATUS_OK);
  s = button_install_handler(BUTTON_CMD, KEY_RELEASED, button_cmd_handler);
  assert(s == STATUS_OK);
  s = button_install_handler(BUTTON_CMD, KEY_LONG_PRESS, button_cmd_handler);
  assert(s == STATUS_OK);
  s = button_install_handler(BUTTON_MINUS, KEY_PRESSED, button_minus_handler);
  assert(s == STATUS_OK);
  s = button_install_handler(BUTTON_MINUS, KEY_RELEASED, button_minus_handler);
  assert(s == STATUS_OK);
  s = button_install_handler(BUTTON_MINUS, KEY_LONG_PRESS, button_minus_handler);
  assert(s == STATUS_OK);
  s = button_install_handler(BUTTON_PLUS, KEY_PRESSED, button_plus_handler);
  assert(s == STATUS_OK);
  s = button_install_handler(BUTTON_PLUS, KEY_RELEASED, button_plus_handler);
  assert(s == STATUS_OK);
  s = button_install_handler(BUTTON_PLUS, KEY_LONG_PRESS, button_plus_handler);
  assert(s == STATUS_OK);
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
