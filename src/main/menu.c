#include "menu.h"
#include "drivers/ser_lcd.h"
#include "menu_date_time.h"

static void menu_display_splash_screen(screen_t* screen);

screen_t splash_screen =
{
  .line_1 = "METEO STATION   ",
  .line_2 = "          v0.0.0",
  .display = screen_generic_display,
  .on_plus = menu_display_config_screen,
  .on_minus = menu_display_config_screen,
  .on_cmd_long_press = NULL,
  .on_cmd = NULL,
  .on_enter = NULL,
  .on_exit = NULL,
};

screen_t config_screen =
{
  .line_1 = "METEO STATION   ",
  .line_2 = "CONFIGURATION...",
  .display = screen_generic_display,
  .on_cmd = menu_display_date_screen,
  .on_cmd_long_press = NULL,
  .on_plus = menu_display_splash_screen,
  .on_minus = menu_display_splash_screen,
  .on_enter = NULL,
  .on_exit = NULL,
};

STATUS menu_init(void)
{
  screen_change_to(&splash_screen);
  return STATUS_OK;
}

void menu_display_config_screen(screen_t* screen)
{
  screen_change_to(&config_screen);
}

void menu_display_splash_screen(screen_t* screen)
{
  screen_change_to(&splash_screen);
}
