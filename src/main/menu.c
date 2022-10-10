#include "menu.h"
#include "drivers/ser_lcd.h"
#include "button.h"

// typedef enum
// {
//   SCREEN_STATIC
// } screen_type;

struct screen_def_t
{
  //   screen_type type;
  char* line_1;
  char* line_2;
  void (*display)(struct screen_def_t* pScreen);
  void (*on_cmd)(struct screen_def_t* pScreen);
  void (*on_cmd_long_press)(struct screen_def_t* pScreen);
  void (*on_plus)(struct screen_def_t* pScreen);
  void (*on_minus)(struct screen_def_t* pScreen);
} ;

typedef struct screen_def_t screen_t;
static screen_t* current_screen;

static void menu_generic_display(screen_t* screen);
static void splash_screen_next_screen(screen_t* screen);
static void splash_screen_previous_screen(screen_t* screen);
static void config_screen_next_screen(screen_t* screen);
static void config_screen_previous_screen(screen_t* screen);
static void config_screen_display_menu_list(screen_t* screen);
static void config_sub_return_main_menu(screen_t* screen);
static void config_date_screen_next_screen(screen_t* screen);
static void config_date_screen_previous_screen(screen_t* screen);
static void config_time_screen_next_screen(screen_t* screen);
static void config_time_screen_previous_screen(screen_t* screen);
static void config_date_goto_modif_date(screen_t* screen);

screen_t splash_screen =
{
  .line_1 = "METEO STATION   ",
  .line_2 = "          v0.0.0",
  .display = menu_generic_display,
  .on_plus = splash_screen_next_screen,
  .on_minus = splash_screen_previous_screen,
  .on_cmd_long_press = NULL,
  .on_cmd = NULL
};

screen_t config_screen =
{
  .line_1 = "METEO STATION   ",
  .line_2 = "CONFIGURATION...",
  .display = menu_generic_display,
  .on_cmd = config_screen_display_menu_list,
  .on_cmd_long_press = NULL,
  .on_plus = config_screen_next_screen,
  .on_minus = config_screen_previous_screen,
};

screen_t config_date_screen =
{
  .line_1 = "CONFIGURATION   ",
  .line_2 = "SET DATE...",
  .display = menu_generic_display,
  .on_cmd = config_date_goto_modif_date,
  .on_cmd_long_press = config_sub_return_main_menu,
  .on_plus = config_date_screen_next_screen,
  .on_minus = config_date_screen_previous_screen,
};

static void config_date_modif_display(screen_t* screen);
static void config_date_on_cmd(screen_t* screen);
static void config_date_modif_on_plus(screen_t* screen);
static void config_date_modif_on_minus(screen_t* screen);

#include <stdio.h>

uint8_t day = 01;
uint8_t month = 01;
uint16_t year = 2022;

char date_display[16];
uint8_t cursor_pos_c = 4;

typedef enum
{
  CHANGE_DAY,
  CHANGE_MONTH,
  CHANGE_YEAR
} date_modif_state_t;

date_modif_state_t date_state;

screen_t config_date_modif_screen =
{
  .line_1 = "SET DATE",
  .line_2 = "   xx/xx/xxxx",
  .display = config_date_modif_display,
  .on_cmd = config_date_on_cmd,
  .on_cmd_long_press = config_screen_display_menu_list,
  .on_plus = config_date_modif_on_plus,
  .on_minus = config_date_modif_on_minus,
};

void config_date_modif_display(screen_t* screen)
{
  ser_lcd_write_line(0, screen->line_1);
  snprintf(date_display, 16, "   %.2d/%.2d/%.4d", day, month, year);
  ser_lcd_write_line(1, date_display);
  ser_lcd_set_cursor(1, cursor_pos_c);
  ser_lcd_cursor_on();
  ser_lcd_blink_on();
}

void config_date_modif_on_minus(screen_t* screen)
{
  switch (date_state)
  {
    case CHANGE_DAY:
      day--;
      if (day == 0)
        day = 31;
      break;
    case CHANGE_MONTH:
      month--;
      if (month == 0)
        month = 12;
      break;
    case CHANGE_YEAR:
      year--;
      if (year == 1999)
        year = 2020;
      break;
  }
  config_date_modif_display(current_screen);
}

void config_date_modif_on_plus(screen_t* screen)
{
  day++;
  if (day == 31)
    day = 1;
  config_date_modif_display(current_screen);
}

void config_date_on_cmd(screen_t* screen)
{
  date_state++;
  if (date_state > CHANGE_YEAR)
    date_state = CHANGE_DAY;

  switch (date_state)
  {
    case CHANGE_DAY:
      cursor_pos_c = 4;
      break;
    case CHANGE_MONTH:
      cursor_pos_c = 7;
      break;
    case CHANGE_YEAR:
      cursor_pos_c = 12;
      break;

  }
  config_date_modif_display(current_screen);
}



screen_t config_time_screen =
{
  .line_1 = "CONFIGURATION   ",
  .line_2 = "SET TIME...",
  .display = menu_generic_display,
  .on_cmd = NULL,
  .on_cmd_long_press = config_sub_return_main_menu,
  .on_plus = config_time_screen_next_screen,
  .on_minus = config_time_screen_previous_screen,
};


static void menu_set_display(screen_t* screen);
static void menu_cmd_handler(key_event evt);
static void menu_plus_handler(key_event evt);
static void menu_minus_handler(key_event evt);

STATUS menu_init(void)
{
  STATUS s;
  s = STATUS_OK;

  s = button_install_handler(BUTTON_CMD, KEY_CLICKED, menu_cmd_handler);
  s = button_install_handler(BUTTON_CMD, KEY_LONG_PRESS, menu_cmd_handler);
  s = button_install_handler(BUTTON_MINUS, KEY_PRESSED, menu_minus_handler);
  s = button_install_handler(BUTTON_PLUS, KEY_PRESSED, menu_plus_handler);
  menu_set_display(&splash_screen);
  return s;
}

void menu_cmd_handler(key_event evt)
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

void menu_set_display(screen_t* screen)
{
  current_screen = screen;
  screen->display(current_screen);
}

void menu_minus_handler(key_event evt)
{
  if (current_screen->on_minus != NULL)
    current_screen->on_minus(current_screen);
}

void menu_plus_handler(key_event evt)
{
  if (current_screen->on_plus != NULL)
    current_screen->on_plus(current_screen);
}

static void menu_generic_display(screen_t* screen)
{
  ser_lcd_write_line(0, screen->line_1);
  ser_lcd_write_line(1, screen->line_2);
}

void splash_screen_previous_screen(screen_t* screen)
{
  menu_set_display(&config_screen);
}

void splash_screen_next_screen(screen_t* screen)
{
  menu_set_display(&config_screen);
}

void config_screen_previous_screen(screen_t* screen)
{
  menu_set_display(&splash_screen);
}

void config_screen_next_screen(screen_t* screen)
{
  menu_set_display(&splash_screen);
}

void config_screen_display_menu_list(screen_t* screen)
{
  ser_lcd_cursor_off();
  ser_lcd_blink_off();
  menu_set_display(&config_date_screen);
}

void config_date_screen_next_screen(screen_t* screen)
{
  menu_set_display(&config_time_screen);
}

void config_date_screen_previous_screen(screen_t* screen)
{
  menu_set_display(&config_time_screen);
}

void config_sub_return_main_menu(screen_t* screen)
{
  menu_set_display(&config_screen);
}

void config_time_screen_next_screen(screen_t* screen)
{
  menu_set_display(&config_date_screen);
}

void config_time_screen_previous_screen(screen_t* screen)
{
  menu_set_display(&config_date_screen);

}

void config_date_goto_modif_date(screen_t* screen)
{
  menu_set_display(&config_date_modif_screen);
}


