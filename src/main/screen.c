#include "screen.h"
#include "drivers/ser_lcd.h"
#include "button.h"

static screen_t* current_screen;

static void screen_on_cmd_event(key_event evt);
static void screen_on_minus_pressed(key_event evt);
static void screen_on_plus_pressed(key_event evt);

STATUS screen_init(void)
{
  STATUS s;
  s = STATUS_OK;

  s = button_install_handler(BUTTON_CMD, KEY_CLICKED, screen_on_cmd_event);
  s |= button_install_handler(BUTTON_CMD, KEY_LONG_PRESS, screen_on_cmd_event);
  s |= button_install_handler(BUTTON_MINUS, KEY_PRESSED, screen_on_minus_pressed);
  s |= button_install_handler(BUTTON_PLUS, KEY_PRESSED, screen_on_plus_pressed);
  return s;
}

void screen_change_to(screen_t* screen)
{
  if ((current_screen != NULL) && (current_screen->on_exit != NULL))
    current_screen->on_exit(current_screen);

  current_screen = screen;

  if (current_screen->on_enter != NULL)
    current_screen->on_enter(current_screen);

  screen->display(current_screen);
}

void screen_generic_display(screen_t* screen)
{
  ser_lcd_write_line(0, screen->line_1);
  ser_lcd_write_line(1, screen->line_2);
}

static void screen_on_cmd_event(key_event evt)
{
  switch (evt)
  {
    case KEY_CLICKED:
      if (current_screen->on_cmd != NULL)
        current_screen->on_cmd(current_screen);
      break;

    case KEY_LONG_PRESS:
      if (current_screen->on_cmd_long_press != NULL)
        current_screen->on_cmd_long_press(current_screen);
      break;

    default:
      break;
  }
}

static void screen_on_minus_pressed(key_event evt)
{
  if (current_screen->on_minus != NULL)
    current_screen->on_minus(current_screen);
}

static void screen_on_plus_pressed(key_event evt)
{
  if (current_screen->on_plus != NULL)
    current_screen->on_plus(current_screen);
}
