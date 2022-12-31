#include "menu.h"
#include "menu_config.h"
#include "menu_date_time.h"
#include "screen.h"
#include "drivers/ser_lcd.h"
#include "filelog.h"
#include <string.h>
#include <stdio.h>
#include "nvstorage.h"
//#include "ble.h"

#define FILELOG_STATUS_SIZE     (17)

typedef enum
{
  CONFIG_FILELOG_DISPLAY,
  CONFIG_FILELOG_MODIFY
} config_filelog_state_t;

static char config_filelog_status[FILELOG_STATUS_SIZE];
static config_filelog_state_t config_filelog_state;

static void menu_display_config_activate_set_filelog_screen(screen_t* screen);
static void config_filelog_display(screen_t* screen);
static void config_filelog_on_clk_btn(screen_t* screen);
static void config_filelog_on_cmd(screen_t* screen);
static void config_filelog_on_enter(screen_t* screen);
static void menu_display_config_activate_set_ble_screen(screen_t* screen);
static void config_ble_display(screen_t* screen);
static void config_ble_on_cmd(screen_t* screen);
static void config_ble_on_clk_btn(screen_t* screen);
static void config_ble_on_enter(screen_t* screen);

screen_t config_screen =
{
  .line_1 = "METEO STATION   ",
  .line_2 = "CONFIGURATION...",
  .display = screen_generic_display,
  .on_cmd = menu_display_date_screen,
  .on_cmd_long_press = NULL,
  .on_plus = menu_display_splash_screen,
  .on_minus = menu_display_sdcard_screen,
  .on_enter = NULL,
  .on_exit = NULL,
};

void menu_display_config_screen(screen_t* screen)
{
  screen_change_to(&config_screen);
}

screen_t config_filelog_screen =
{
  .line_1 = "CONFIGURATION   ",
  .line_2 = "SET SDCARD LOG..",
  .display = screen_generic_display,
  .on_cmd = menu_display_config_activate_set_filelog_screen,
  .on_cmd_long_press = menu_display_config_screen,
  .on_plus =  menu_display_config_activate_ble_screen, //menu_date_display_date_screen,
  .on_minus = menu_date_display_time_screen,
  .on_enter = NULL,
  .on_exit = NULL,
};

void menu_display_config_activate_filelog_screen(screen_t* screen)
{
  screen_change_to(&config_filelog_screen);
}

screen_t config_filelog_set_screen =
{
  .line_1 = "SET SDCARD LOG..",
  .line_2 = NULL,
  .display = config_filelog_display,
  .on_cmd = config_filelog_on_cmd,
  .on_cmd_long_press = menu_display_config_activate_filelog_screen,
  .on_plus = config_filelog_on_clk_btn, //menu_date_display_date_screen,
  .on_minus = config_filelog_on_clk_btn, //menu_date_display_time_screen,
  .on_enter = config_filelog_on_enter,
  .on_exit = NULL,
};

void menu_display_config_activate_set_filelog_screen(screen_t* screen)
{
  screen_change_to(&config_filelog_set_screen);
}

void config_filelog_on_enter(screen_t* screen)
{
  config_filelog_state  = CONFIG_FILELOG_DISPLAY;
}

void config_filelog_display(screen_t* screen)
{
  ser_lcd_write_line(0, screen->line_1);

  bool isActivated;
  isActivated = filelog_get_config();

  snprintf(config_filelog_status, FILELOG_STATUS_SIZE, "             %s", isActivated ? "ON " : "OFF");
  ser_lcd_write_line(1, config_filelog_status);
  if (config_filelog_state == CONFIG_FILELOG_MODIFY)
  {
    ser_lcd_set_cursor(1, 13);
    ser_lcd_cursor_on();
    ser_lcd_blink_on();
  }
  else
  {
    ser_lcd_cursor_off();
    ser_lcd_blink_off();
  }
}

void config_filelog_on_clk_btn(screen_t* screen)
{
  if (config_filelog_state == CONFIG_FILELOG_MODIFY)
  {
    bool isActivated;
    isActivated = filelog_get_config();
    filelog_set_config(!isActivated);
    config_filelog_display(screen);
  }
}

void config_filelog_on_cmd(screen_t* screen)
{
  if (config_filelog_state == CONFIG_FILELOG_MODIFY)
    config_filelog_state = CONFIG_FILELOG_DISPLAY;
  else
    config_filelog_state = CONFIG_FILELOG_MODIFY;

  config_filelog_display(screen);
}

screen_t config_ble_screen =
{
  .line_1 = "CONFIGURATION   ",
  .line_2 = "SET BLUETOOTH...",
  .display = screen_generic_display,
  .on_cmd = menu_display_config_activate_set_ble_screen,
  .on_cmd_long_press = menu_display_config_screen,
  .on_plus = menu_date_display_date_screen,
  .on_minus = menu_display_config_activate_filelog_screen,
  .on_enter = NULL,
  .on_exit = NULL,
};

void menu_display_config_activate_ble_screen(screen_t* screen)
{
  screen_change_to(&config_ble_screen);
}

screen_t config_ble_set_screen =
{
  .line_1 = "SET BLUETOOTH...",
  .line_2 = NULL,
  .display = config_ble_display,
  .on_cmd = config_ble_on_cmd,
  .on_cmd_long_press = menu_display_config_activate_ble_screen,
  .on_plus = config_ble_on_clk_btn, //menu_date_display_date_screen,
  .on_minus = config_ble_on_clk_btn, //menu_date_display_time_screen,
  .on_enter = config_ble_on_enter,
  .on_exit = NULL,
};

void menu_display_config_activate_set_ble_screen(screen_t* screen)
{
  screen_change_to(&config_ble_set_screen);
}

void config_ble_display(screen_t* screen)
{
  ser_lcd_write_line(0, screen->line_1);

  bool isActivated;
  isActivated = nvstorage_get_ble_state();

  snprintf(config_filelog_status, FILELOG_STATUS_SIZE, "             %s", isActivated ? "ON " : "OFF");
  ser_lcd_write_line(1, config_filelog_status);
  if (config_filelog_state == CONFIG_FILELOG_MODIFY)
  {
    ser_lcd_set_cursor(1, 13);
    ser_lcd_cursor_on();
    ser_lcd_blink_on();
  }
  else
  {
    ser_lcd_cursor_off();
    ser_lcd_blink_off();
  }
}

void config_ble_on_enter(screen_t* screen)
{
  config_filelog_on_enter(screen);
}

void config_ble_on_cmd(screen_t* screen)
{
  if (config_filelog_state == CONFIG_FILELOG_MODIFY)
    config_filelog_state = CONFIG_FILELOG_DISPLAY;
  else
    config_filelog_state = CONFIG_FILELOG_MODIFY;

  config_ble_display(screen);
}

void config_ble_on_clk_btn(screen_t* screen)
{
  if (config_filelog_state == CONFIG_FILELOG_MODIFY)
  {
    bool isActivated;
    isActivated = nvstorage_get_ble_state();
    nvstorage_set_ble_state(!isActivated);
    config_ble_display(screen);
  }
}

