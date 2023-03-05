#include "menu_histogram.h"
#include "menu.h"
#include "data_ope.h"
#include "histogram.h"
#include <stdio.h>
#include "drivers/ser_lcd.h"
#include "log.h"
#include "winddir.h"

static void menu_histo_display_top(screen_t* screen);

static void menu_histo_prev_data ( screen_t* screen );
static void menu_histo_get_next_data ( screen_t* screen );
static void menu_histo_display(screen_t* screen);
static void menu_histo_display_stacked_data(screen_t* screen);
static void menu_histo_splash_on_enter(screen_t* screen);

static screen_t menu_histo_splash_screen =
{
  .line_1 = "  METEO STATION ",
  .line_2 = "    DONNEES     ",
  .display = screen_generic_display,
  .on_plus = menu_display_date_time_screen,
  .on_minus = menu_display_splash_screen,
  .on_cmd_long_press = NULL,
  .on_cmd = menu_histo_display_top,
  .on_enter = menu_histo_splash_on_enter,
  .on_exit = NULL,
};


static screen_t menu_histogram_top_screen =
{
  .line_1 = "",
  .line_2 = "",
  .display = menu_histo_display,
  .on_plus = menu_histo_get_next_data,
  .on_minus = menu_histo_prev_data,
  .on_cmd_long_press = menu_histo_display_screen,
  .on_cmd = menu_histo_display_stacked_data,
  .on_enter = NULL,
  .on_exit = NULL,
};

static int32_t menu_histo_current_ope;
static int32_t menu_histo_current_item;

#define STRING_SIZE   (17)

static char line_1[STRING_SIZE];
static char line_2[STRING_SIZE];

static void menu_histo_no_data(data_operation_t* currentOpe)
{
  const char* pSensorName = sensor_name[currentOpe->sensor];
  const char* pSensorUnit = sensor_unit[currentOpe->sensor];
  snprintf(line_1, STRING_SIZE, "%s (-)", pSensorName);
  snprintf(line_2, STRING_SIZE, "- %s",  0.0, pSensorUnit);
}

static void menu_histo_top_fill_contents(void);

static void menu_histo_display(screen_t* screen)
{
  menu_histo_top_fill_contents();

  ser_lcd_write_line(0, line_1);
  ser_lcd_write_line(1, line_2);
}

static void menu_histo_get_next_data ( screen_t* screen )
{
  uint32_t nbSensors;
  nbSensors = data_ope_nb_operation();

  menu_histo_current_ope++;
  if (menu_histo_current_ope >= nbSensors)
    menu_histo_current_ope = 0;

  screen_refresh();
}


static void menu_histo_prev_data ( screen_t* screen )
{
  uint32_t nbSensors;
  nbSensors = data_ope_nb_operation();

  menu_histo_current_ope--;
  if (menu_histo_current_ope < 0)
    menu_histo_current_ope = nbSensors - 1;

  screen_refresh();
}

void menu_histo_splash_on_enter ( screen_t* screen )
{
  menu_histo_current_ope = 0;
}

void menu_histo_display_screen ( screen_t* screen )
{
  screen_change_to(&menu_histo_splash_screen);
}

enum
{
  NO_SELECTED,
  SECOND,
  MINUTE,
  HOUR,
  DAY
};

static void menu_choose_period_value(char* period_value, uint32_t v)
{
  int32_t unitSelected = NO_SELECTED;

  if ((v / 60) == 0)
    unitSelected = SECOND;

  if ((unitSelected == NO_SELECTED) && ((v / 3600) == 0))
    unitSelected = MINUTE;

  if ((unitSelected == NO_SELECTED) && ((v / (3600 * 24)) == 0))
    unitSelected = HOUR;

  if (unitSelected == NO_SELECTED)
    unitSelected = DAY;

  switch (unitSelected)
  {
    case SECOND:
      snprintf(period_value, 10, "%ds", v);
      break;

    case MINUTE:
      snprintf(period_value, 10, "%dmin", v / 60);
      break;

    case HOUR:
      snprintf(period_value, 10, "%dhrs", v / 3600);
      break;

    case DAY:
      snprintf(period_value, 10, "%djrs", v / (3600 * 24));
      break;

    default:
      break;
  }
}

static void menu_histo_top_fill_contents(void)
{
  histogram_t* currentHisto;
  currentHisto = data_ope_get_histo(menu_histo_current_ope);

  data_operation_t* currentOpe;
  currentOpe = date_ope_get_operation(menu_histo_current_ope);

  variant_t v;
  STATUS s;

  s = histogram_get(currentHisto, 0, &v);
  const char* pSensorName = sensor_name[currentOpe->sensor];
  const char* pSensorUnit = sensor_unit[currentOpe->sensor];
  const char* period;
  char period_value[10];

  log_dbg_print("sensor_name : %s, sensor_unit : %s", pSensorName, pSensorUnit);
  log_dbg_print("menu_histo_current_ope = %d", menu_histo_current_ope);
  log_dbg_print("currentOpe->sensor = %d", currentOpe->sensor);

  if (currentOpe->calcul_period.type == SLIDING_PERIOD)
  {
    period = "glis.";
    menu_choose_period_value(period_value, currentOpe->calcul_period.period_sec);
  }
  else
  {
    period  = "fixe";
    snprintf(period_value, 10, "%d%s", currentOpe->calcul_period.f_period.period,
             period_unit[currentOpe->calcul_period.f_period.unit]);
  }

  snprintf(line_1, STRING_SIZE, "%s %s", pSensorName, period);

  if (s == STATUS_OK)
  {
    if (currentOpe->sensor == WIND_DIR)
    {
      snprintf(line_2, STRING_SIZE, "%s %s %s",  winddir_angle_to_direction(v.i32), pSensorUnit, period_value);
    }
    else
    {
      snprintf(line_2, STRING_SIZE, "%.1f %s %s",  v.f32, pSensorUnit, period_value);
    }
  }
  else
  {
    snprintf(line_2, STRING_SIZE, "--- %s %s",  pSensorUnit, period_value);
  }
}

static void menu_histo_items_prev ( screen_t* screen );
static void menu_histo_items_next ( screen_t* screen );
static void menu_histo_items_display(screen_t* screen);
static void menu_histo_items_on_enter(screen_t* screen);
static void menu_histo_items_fill(void);

static screen_t menu_histogram_items_screen =
{
  .line_1 = "",
  .line_2 = "",
  .display = menu_histo_items_display,
  .on_plus = menu_histo_items_next,
  .on_minus = menu_histo_items_prev,
  .on_cmd_long_press = menu_histo_display_top,
  .on_cmd = NULL,
  .on_enter = menu_histo_items_on_enter,
  .on_exit = NULL,
};

void menu_histo_items_on_enter ( screen_t* screen )
{
  menu_histo_current_item = 1;
}

void menu_histo_items_display(screen_t* screen)
{
  menu_histo_items_fill();

  ser_lcd_write_line(0, line_1);
  ser_lcd_write_line(1, line_2);
}

static void menu_histo_display_top(screen_t* screen)
{
  screen_change_to(&menu_histogram_top_screen);
}

static void menu_histo_display_stacked_data ( screen_t* screen )
{
  screen_change_to(&menu_histogram_items_screen);
}

static void menu_histo_items_next ( screen_t* screen )
{
  int32_t nextIndex;
  histogram_t* currentHisto;
  currentHisto = data_ope_get_histo(menu_histo_current_ope);
  uint32_t nbItems;
  variant_t v;
  STATUS s;

  nbItems = histogram_nbItems(currentHisto);
  nextIndex = menu_histo_current_item + 1;
  if (nextIndex < nbItems)
  {
    s =  histogram_get(currentHisto, menu_histo_current_item, &v);
    if (s == STATUS_OK)
      menu_histo_current_item = nextIndex;
  }

  screen_refresh();
}

static void menu_histo_items_prev ( screen_t* screen )
{
  int32_t nextIndex;
  histogram_t* currentHisto;
  currentHisto = data_ope_get_histo(menu_histo_current_ope);
  variant_t v;
  STATUS s;

  nextIndex = menu_histo_current_item - 1;
  if (nextIndex >= 0)
  {
    s =  histogram_get(currentHisto, menu_histo_current_item, &v);
    if (s == STATUS_OK)
      menu_histo_current_item = nextIndex;
  }

  screen_refresh();
}

static void menu_histo_items_fill(void)
{
  histogram_t* currentHisto;
  currentHisto = data_ope_get_histo(menu_histo_current_ope);

  data_operation_t* currentOpe;
  currentOpe = date_ope_get_operation(menu_histo_current_ope);

  variant_t v;
  STATUS s;

  s = histogram_get(currentHisto, menu_histo_current_item, &v);
  if (s == STATUS_OK)
  {
    const char* pSensorName = sensor_name[currentOpe->sensor];
    const char* pSensorUnit = sensor_unit[currentOpe->sensor];
    const char* period;

    log_dbg_print("sensor_name : %s, sensor_unit : %s", pSensorName, pSensorUnit);
    log_dbg_print("menu_histo_current_ope = %d menu_histo_current_item = %d", menu_histo_current_ope,
                  menu_histo_current_item);
    log_dbg_print("currentOpe->sensor = %d", currentOpe->sensor);

    if (currentOpe->calcul_period.type == SLIDING_PERIOD)
      period = "glis.";
    else
      period  = "fixe";

    snprintf(line_1, STRING_SIZE, "%s %s", pSensorName, period);

    if (currentOpe->sensor == WIND_DIR)
      snprintf(line_2, STRING_SIZE, "%s (-%d)",  winddir_angle_to_direction(v.i32), menu_histo_current_item);
    else
      snprintf(line_2, STRING_SIZE, "%.1f %s (-%d)",  v.f32, pSensorUnit, menu_histo_current_item);
  }
  else
  {
    menu_histo_no_data(currentOpe);
  }
}
