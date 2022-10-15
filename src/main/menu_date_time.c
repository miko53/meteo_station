#include <stdio.h>
#include "menu_date_time.h"
#include "screen.h"
#include "menu.h"
#include "drivers/ser_lcd.h"

typedef enum
{
  IDLE,
  CHANGE_DAY,
  CHANGE_MONTH,
  CHANGE_YEAR
} date_modif_state_t;

typedef struct
{
  uint8_t day;
  uint8_t month;
  uint16_t year;
  char date_display[16];
  uint8_t cursor_pos_c;
  date_modif_state_t state;
} date_context_t;

date_context_t date_context =
{
  .day = 01,
  .month = 01,
  .year = 2022,
  .cursor_pos_c = 4
};

static void menu_date_display_date_screen(screen_t* screen);
static void menu_date_display_time_screen(screen_t* screen);
static void menu_date_goto_modif_date_screen(screen_t* screen);

static void date_screen_display(screen_t* screen);
static void date_screen_on_cmd(screen_t* screen);
static void date_screen_on_minus(screen_t* screen);
static void date_screen_on_plus(screen_t* screen);
static void date_screen_on_enter(screen_t* screen);
static void date_screen_on_exit(screen_t* screen);

screen_t config_date_screen =
{
  .line_1 = "CONFIGURATION   ",
  .line_2 = "SET DATE...",
  .display = screen_generic_display,
  .on_cmd = menu_date_goto_modif_date_screen,
  .on_cmd_long_press = menu_display_config_screen,
  .on_plus = menu_date_display_time_screen,
  .on_minus = menu_date_display_time_screen,
  .on_enter = NULL,
  .on_exit = NULL,
};

screen_t config_time_screen =
{
  .line_1 = "CONFIGURATION   ",
  .line_2 = "SET TIME...",
  .display = screen_generic_display,
  .on_cmd = NULL,
  .on_cmd_long_press = menu_display_config_screen,
  .on_plus = menu_date_display_date_screen,
  .on_minus = menu_date_display_date_screen,
};

screen_t config_date_modif_screen =
{
  .line_1 = "SET DATE",
  .line_2 = "   xx/xx/xxxx",
  .display = date_screen_display,
  .on_cmd = date_screen_on_cmd,
  .on_cmd_long_press = menu_date_display_date_screen,
  .on_plus = date_screen_on_plus,
  .on_minus = date_screen_on_minus,
  .on_enter = date_screen_on_enter,
  .on_exit = date_screen_on_exit,
};


static void menu_date_display_date_screen(screen_t* screen)
{
  screen_change_to(&config_date_screen);
}

static void menu_date_display_time_screen(screen_t* screen)
{
  screen_change_to(&config_time_screen);
}

void menu_date_goto_modif_date_screen(screen_t* screen)
{
  screen_change_to(&config_date_modif_screen);
}

void date_screen_on_enter(screen_t* screen)
{
  date_context.state = IDLE;
  fprintf(stdout, "read the DATE\n");
}

void date_screen_on_exit(screen_t* screen)
{
  ser_lcd_cursor_off();
  ser_lcd_blink_off();
}

void date_screen_on_cmd(screen_t* screen)
{
  date_context.state++;
  if (date_context.state > CHANGE_YEAR)
    date_context.state = IDLE;

  if (date_context.state > IDLE)
  {
    ser_lcd_cursor_on();
    ser_lcd_blink_on();
  }

  switch (date_context.state)
  {
    case IDLE:
      ser_lcd_cursor_off();
      ser_lcd_blink_off();
      fprintf(stdout, "set the DATE\n");
      break;

    case CHANGE_DAY:
      date_context.cursor_pos_c = 4;
      break;
    case CHANGE_MONTH:
      date_context.cursor_pos_c = 7;
      break;
    case CHANGE_YEAR:
      date_context.cursor_pos_c = 12;
      break;

  }
  date_screen_display(&config_date_modif_screen);
}

void date_screen_display(screen_t* screen)
{
  ser_lcd_write_line(0, screen->line_1);
  snprintf(date_context.date_display, 16, "   %.2d/%.2d/%.4d", date_context.day, date_context.month, date_context.year);
  ser_lcd_write_line(1, date_context.date_display);
  ser_lcd_set_cursor(1, date_context.cursor_pos_c);
}

void date_screen_on_minus(screen_t* screen)
{
  switch (date_context.state)
  {
    case IDLE:
      break;

    case CHANGE_DAY:
      date_context.day--;
      if (date_context.day == 0)
        date_context.day = 31;
      break;

    case CHANGE_MONTH:
      date_context.month--;
      if (date_context.month == 0)
        date_context.month = 12;
      break;

    case CHANGE_YEAR:
      date_context.year--;
      if (date_context.year == 1999)
        date_context.year = 2020;
      break;
  }
  date_screen_display(&config_date_modif_screen);
}

void date_screen_on_plus(screen_t* screen)
{
  switch (date_context.state)
  {
    case IDLE:
      break;
    case CHANGE_DAY:
      date_context.day++;
      if (date_context.day >= 31)
        date_context.day = 1;
      break;

    case CHANGE_MONTH:
      date_context.month++;
      if (date_context.month >= 12)
        date_context.month = 1;
      break;

    case CHANGE_YEAR:
      date_context.year++;
      break;
  }
  date_screen_display(&config_date_modif_screen);
}

void menu_display_date_screen(screen_t* screen)
{
  screen_change_to(&config_date_screen);
}
