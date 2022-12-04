#include "menu_histogram.h"
#include "menu.h"


static screen_t menu_histogram_screen =
{
  .line_1 = "METEO STATION   ",
  .line_2 = "HISTOGRAMME     ",
  .display = screen_generic_display,
  .on_plus = menu_display_date_time_screen,
  .on_minus = menu_display_splash_screen,
  .on_cmd_long_press = NULL,
  .on_cmd = NULL,
  .on_enter = NULL,
  .on_exit = NULL,
};


void menu_histo_display_screen ( screen_t* screen )
{
  screen_change_to(&menu_histogram_screen);
}
