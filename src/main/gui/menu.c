#include "menu.h"
#include "config.h"
#include "drivers/ser_lcd.h"
#include "menu_date_time.h"
#include "menu_config.h"
#include "drivers/pcf_8523.h"
#include "os.h"
#include "drivers/sd_card.h"
#include "log.h"
#include "menu_histogram.h"
#include <string.h>

static void menu_enter_date_screen(screen_t* pScreen);
static void menu_exit_date_screen(screen_t* pScreen);
static void menu_enter_sdcard_screen(screen_t* pScreen);

screen_t splash_screen =
{
  .line_1 = "METEO STATION   ",
  .line_2 = "          " METEO_STATION_VERSION,
  .display = screen_generic_display,
  .on_plus = menu_histo_display_screen,
  .on_minus = menu_display_config_screen,
  .on_cmd_long_press = NULL,
  .on_cmd = NULL,
  .on_enter = NULL,
  .on_exit = NULL,
};

#define DATE_STRING_SIZE      (16)
#define TIME_STRING_SIZE      (16)

char date_line[DATE_STRING_SIZE];
char time_line[TIME_STRING_SIZE];

screen_t date_time_display_screen =
{
  .line_1 = date_line,
  .line_2 = time_line,
  .display = screen_generic_display,
  .on_plus = menu_display_sdcard_screen,
  .on_minus = menu_histo_display_screen,
  .on_enter = menu_enter_date_screen,
  .on_exit = menu_exit_date_screen,
};

#define SDCARD_STRING_SIZE  (16)

char sdcard_info_line1[SDCARD_STRING_SIZE];
char sdcard_info_line2[SDCARD_STRING_SIZE];

screen_t sd_card_infos =
{
  .line_1 = sdcard_info_line1,
  .line_2 = sdcard_info_line2,
  .display = screen_generic_display,
  .on_plus = menu_display_config_screen,
  .on_minus = menu_display_date_time_screen,
  .on_enter = menu_enter_sdcard_screen,
  .on_exit = NULL,
};

static TimerHandle_t date_time_handle;
static void date_time_refresh_fun( TimerHandle_t xTimer );

STATUS menu_init(void)
{
  screen_set_default_screen(&splash_screen);
  screen_change_to(&splash_screen);
  return STATUS_OK;
}

void menu_display_splash_screen(screen_t* screen)
{
  screen_change_to(&splash_screen);
}

void menu_display_date_time_screen(screen_t* pScreen)
{
  screen_change_to(&date_time_display_screen);
}

void menu_enter_date_screen(screen_t* pScreen)
{
  date_time_handle = xTimerCreate("date_time_refresh", OS_SEC_TO_TICK(1), pdTRUE, NULL, date_time_refresh_fun);
  if (date_time_handle == NULL)
  {
    log_info_print("error when trying date_time handle\n");
  }
  else
  {
    if ( xTimerStart(date_time_handle, 0) != pdPASS )
    {
      log_info_print("Timer start error");
    }
  }
  date_time_refresh_fun(date_time_handle);
}

void date_time_refresh_fun( TimerHandle_t xTimer )
{
  UNUSED(xTimer);

  struct tm currentDate;
  pcf8523_get_date(&currentDate);
  snprintf(date_line, DATE_STRING_SIZE, "   %.2d/%.2d/%.4d", currentDate.tm_mday, currentDate.tm_mon + 1,
           currentDate.tm_year + 1900);
  snprintf(time_line, TIME_STRING_SIZE, "    %.2d:%.2d:%.2d", currentDate.tm_hour, currentDate.tm_min,
           currentDate.tm_sec);
  screen_refresh();
}

void menu_exit_date_screen(screen_t* pScreen)
{
  xTimerStop(date_time_handle, 0);
  xTimerDelete(date_time_handle, 0);
}

void menu_display_sdcard_screen(screen_t* pScreen)
{
  screen_change_to(&sd_card_infos);
}

void menu_enter_sdcard_screen(screen_t* pScreen)
{
  sd_cart_infos_t sdInfos;
  STATUS s;
  s = sd_card_get_infos(&sdInfos);
  if (s == STATUS_OK)
  {
    strncpy(sdcard_info_line1, "SD-CARD: OK", SDCARD_STRING_SIZE - 1);
    snprintf(sdcard_info_line2, SDCARD_STRING_SIZE, "CAPA: %llu MB", sdInfos.capacity);
  }
  else
  {
    strncpy(sdcard_info_line1, "SD-CARD: FAILED", SDCARD_STRING_SIZE);
  }
}
