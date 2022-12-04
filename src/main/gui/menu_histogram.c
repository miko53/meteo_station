#include "menu_histogram.h"
#include "menu.h"
#include "data_ope.h"
#include "histogram.h"
#include <stdio.h>
#include "drivers/ser_lcd.h"
#include "log.h"
#include "winddir.h"

static void menu_histo_display_first(screen_t* screen);

static screen_t menu_histogram_screen =
{
  .line_1 = "METEO STATION   ",
  .line_2 = "HISTOGRAMME     ",
  .display = screen_generic_display,
  .on_plus = menu_display_date_time_screen,
  .on_minus = menu_display_splash_screen,
  .on_cmd_long_press = NULL,
  .on_cmd = menu_histo_display_first,
  .on_enter = NULL,
  .on_exit = NULL,
};

void menu_histo_prev ( screen_t* screen );
void menu_histo_next ( screen_t* screen );
void menu_histo_display(screen_t* screen);
void menu_histo_on_enter(screen_t* screen);

static screen_t menu_histogram__level_one_screen =
{
  .line_1 = "",
  .line_2 = "",
  .display = menu_histo_display,
  .on_plus = menu_histo_next,
  .on_minus = menu_histo_prev,
  .on_cmd_long_press = menu_histo_display_screen,
  .on_cmd = menu_histo_display_first,
  .on_enter = menu_histo_on_enter,
  .on_exit = NULL,
};



void menu_histo_display_screen ( screen_t* screen )
{
  screen_change_to(&menu_histogram_screen);
}

int32_t menu_histo_current_ope;
char line_1[17];
char line_2[17];
void menu_histo_fill(void);

void menu_histo_display(screen_t* screen)
{
  menu_histo_fill();

  ser_lcd_write_line(0, line_1);
  ser_lcd_write_line(1, line_2);
}

void menu_histo_display_first(screen_t* screen)
{
  screen_change_to(&menu_histogram__level_one_screen);
}

void menu_histo_on_enter ( screen_t* screen )
{
  uint32_t nbSensors;
  nbSensors = data_ope_nb_operation();
  menu_histo_current_ope = 0;
}


void menu_histo_next ( screen_t* screen )
{
  uint32_t nbSensors;
  nbSensors = data_ope_nb_operation();
  menu_histo_current_ope++;
  if (menu_histo_current_ope >= nbSensors)
    menu_histo_current_ope = 0;

  screen_refresh();
}


void menu_histo_prev ( screen_t* screen )
{
  uint32_t nbSensors;
  nbSensors = data_ope_nb_operation();
  menu_histo_current_ope--;
  if (menu_histo_current_ope < 0)
    menu_histo_current_ope = nbSensors - 1;

  screen_refresh();
}

#define STRING_SIZE   (17)


// typedef enum
// {
//   TEMPERATURE,
//   HUMIDITY,
//   RAIN,
//   WIND_SPEED,
//   WIND_DIR
// } data_type_t;

const char* sensor_name[5] =
{
  "Temp.",
  "Humd.",
  "Precip.",
  "Vit. vent",
  "Dir. vent",
};

const char* sensor_unit[5] =
{
  "degC",
  "%H",
  "mm",
  "km/h",
  "dir",
};

void menu_histo_fill(void)
{
  histogram_t* currentHisto;
  currentHisto = data_ope_get_histo(menu_histo_current_ope);

  data_operation_t* currentOpe;
  currentOpe = date_ope_get_operation(menu_histo_current_ope);

  char* pSensorName = sensor_name[currentOpe->sensor];
  char* pSensorUnit = sensor_unit[currentOpe->sensor];

  log_dbg_print("sensor_name : %s, sensor_unit : %s", pSensorName, pSensorUnit);
  log_dbg_print("menu_histo_current_ope = %d", menu_histo_current_ope);
  log_dbg_print("currentOpe->sensor = %d", currentOpe->sensor);

  snprintf(line_1, STRING_SIZE - 1, "%s", pSensorName);
  if (currentOpe->sensor == WIND_DIR)
    snprintf(line_2, STRING_SIZE - 1, "%s",  winddir_angle_to_direction(270), pSensorUnit);
  else
    snprintf(line_2, STRING_SIZE - 1, "%.1f %s",  0.0, pSensorUnit);
}

