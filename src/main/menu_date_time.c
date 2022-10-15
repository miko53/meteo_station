#include <stdio.h>
#include "menu_date_time.h"
#include "screen.h"
#include "menu.h"
#include "drivers/ser_lcd.h"
#include "drivers/pcf_8523.h"

#define DATE_STRING_SIZE        (16)
#define TIME_STRING_SIZE        (16)

typedef enum
{
  DATE_IDLE,
  CHANGE_DAY,
  CHANGE_MONTH,
  CHANGE_YEAR
} date_modif_state_t;

typedef struct
{
  uint8_t day;
  uint8_t month;
  uint16_t year;
  char date_display[DATE_STRING_SIZE];
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

typedef enum
{
  TIME_IDLE,
  CHANGE_HOUR,
  CHANGE_MINUTE,
  CHANGE_SECOND
} time_modif_state_t;


typedef struct
{
  int8_t hour;
  int8_t minute;
  int8_t second;
  time_modif_state_t state;
  char time_display[TIME_STRING_SIZE];
  uint8_t cursor_pos_c;
} time_context_t;

time_context_t time_context =
{
  .hour = 20,
  .minute = 00,
  .second = 00,
  .cursor_pos_c = 4
};

static void menu_date_display_date_screen(screen_t* screen);
static void menu_date_display_time_screen(screen_t* screen);
static void menu_date_goto_modif_date_screen(screen_t* screen);
static void menu_date_goto_modif_time_screen(screen_t* screen);

static void date_screen_display(screen_t* screen);
static void date_screen_on_cmd(screen_t* screen);
static void date_screen_on_minus(screen_t* screen);
static void date_screen_on_plus(screen_t* screen);
static void date_screen_on_enter(screen_t* screen);
static void date_screen_on_exit(screen_t* screen);

static void time_screen_display(screen_t* screen);
static void time_screen_on_cmd(screen_t* screen);
static void time_screen_on_minus(screen_t* screen);
static void time_screen_on_plus(screen_t* screen);
static void time_screen_on_enter(screen_t* screen);
static void time_screen_on_exit(screen_t* screen);

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
  .on_cmd = menu_date_goto_modif_time_screen,
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

screen_t config_time_modif_screen =
{
  .line_1 = "SET TIME",
  .line_2 = "    xx:xx:xx",
  .display = time_screen_display,
  .on_cmd = time_screen_on_cmd,
  .on_cmd_long_press = menu_date_display_time_screen,
  .on_plus = time_screen_on_plus,
  .on_minus = time_screen_on_minus,
  .on_enter = time_screen_on_enter,
  .on_exit = time_screen_on_exit,
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

void menu_date_goto_modif_time_screen(screen_t* screen)
{
  screen_change_to(&config_time_modif_screen);
}


void date_screen_on_enter(screen_t* screen)
{
  date_context.state = DATE_IDLE;
  struct tm currentDate;
  pcf8523_get_date(&currentDate);
  date_context.day = currentDate.tm_mday;
  date_context.month = currentDate.tm_mon;
  date_context.year = currentDate.tm_year + 1900;
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
    date_context.state = DATE_IDLE;

  if (date_context.state > DATE_IDLE)
  {
    ser_lcd_cursor_on();
    ser_lcd_blink_on();
  }

  switch (date_context.state)
  {
    case DATE_IDLE:
      ser_lcd_cursor_off();
      ser_lcd_blink_off();
      fprintf(stdout, "set the DATE\n");
      {
        struct tm currentDate;
        pcf8523_get_date(&currentDate);
        currentDate.tm_mday = date_context.day;
        currentDate.tm_mon = date_context.month;
        currentDate.tm_year = date_context.year - 1900;
        pcf8523_set_date(&currentDate);
      }
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
  snprintf(date_context.date_display, DATE_STRING_SIZE, "   %.2d/%.2d/%.4d", date_context.day, date_context.month,
           date_context.year);
  ser_lcd_write_line(1, date_context.date_display);
  ser_lcd_set_cursor(1, date_context.cursor_pos_c);
}

void date_screen_on_minus(screen_t* screen)
{
  switch (date_context.state)
  {
    case DATE_IDLE:
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
    case DATE_IDLE:
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


void time_screen_on_cmd(screen_t* screen)
{
  time_context.state++;
  if (time_context.state > CHANGE_SECOND)
    time_context.state = TIME_IDLE;

  if (time_context.state > TIME_IDLE)
  {
    ser_lcd_cursor_on();
    ser_lcd_blink_on();
  }

  switch (time_context.state)
  {
    case DATE_IDLE:
      ser_lcd_cursor_off();
      ser_lcd_blink_off();
      fprintf(stdout, "set the TIME\n");
      {
        struct tm currentDate;
        pcf8523_get_date(&currentDate);
        currentDate.tm_sec = time_context.second;
        currentDate.tm_min = time_context.minute;
        currentDate.tm_hour = time_context.hour;
        pcf8523_set_date(&currentDate);
      }
      break;

    case CHANGE_HOUR:
      time_context.cursor_pos_c = 5;
      break;
    case CHANGE_MINUTE:
      time_context.cursor_pos_c = 8;
      break;
    case CHANGE_SECOND:
      time_context.cursor_pos_c = 11;
      break;

  }
  time_screen_display(&config_time_modif_screen);
}


void time_screen_display(screen_t* screen)
{
  ser_lcd_write_line(0, screen->line_1);
  snprintf(time_context.time_display, TIME_STRING_SIZE, "    %.2d:%.2d:%.2d", time_context.hour, time_context.minute,
           time_context.second);
  ser_lcd_write_line(1, time_context.time_display);
  ser_lcd_set_cursor(1, time_context.cursor_pos_c);
}

void time_screen_on_enter(screen_t* screen)
{
  time_context.state = DATE_IDLE;
  struct tm currentDate;

  pcf8523_get_date(&currentDate);
  time_context.hour = currentDate.tm_hour;
  time_context.minute = currentDate.tm_min;
  time_context.second = currentDate.tm_sec;
  fprintf(stdout, "read the TIME\n");
}

void time_screen_on_exit(screen_t* screen)
{
  ser_lcd_cursor_off();
  ser_lcd_blink_off();
}

void time_screen_on_minus(screen_t* screen)
{
  switch (time_context.state)
  {
    case TIME_IDLE:
      break;

    case CHANGE_HOUR:
      time_context.hour--;
      if (time_context.hour < 0)
        time_context.hour = 23;
      break;

    case CHANGE_MINUTE:
      time_context.minute--;
      if (time_context.minute < 0)
        time_context.minute = 59;
      break;

    case CHANGE_SECOND:
      time_context.second--;
      if (time_context.second < 0)
        time_context.second = 59;
      break;
  }
  time_screen_display(&config_time_modif_screen);
}

void time_screen_on_plus(screen_t* screen)
{
  switch (time_context.state)
  {
    case TIME_IDLE:
      break;
    case CHANGE_HOUR:
      time_context.hour++;
      if (time_context.hour >= 24)
        time_context.hour = 0;
      break;

    case CHANGE_MINUTE:
      time_context.minute++;
      if (time_context.minute >= 60)
        time_context.minute = 0;
      break;

    case CHANGE_SECOND:
      time_context.second++;
      if (time_context.second >= 60)
        time_context.second = 0;
      break;
  }
  time_screen_display(&config_time_modif_screen);
}

