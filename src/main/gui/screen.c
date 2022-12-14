#include "screen.h"
#include "drivers/ser_lcd.h"
#include "button.h"
#include "os.h"
#include "log.h"
#include "config.h"

#define SCREEN_WAKE_UP_BIT  (0x01)
#define SCREEN_SHUTDOWN_BIT (0x02)

static screen_t* current_screen;
static screen_t* default_screen = NULL;
static TimerHandle_t screen_shutdown_timer = NULL;
static EventGroupHandle_t screen_wakeup_event = NULL;

static void screen_on_cmd_event(key_event evt);
static void screen_on_minus_pressed(key_event evt);
static void screen_on_plus_pressed(key_event evt);
static void screen_shutdown_callback(TimerHandle_t timer);
static bool screen_check_display_and_wake_up(void);
static void screen_wakeup_task(void* arg);

STATUS screen_init(void)
{
  STATUS s;
  s = STATUS_OK;

  s = button_install_handler(BUTTON_CMD, KEY_CLICKED, screen_on_cmd_event);
  s |= button_install_handler(BUTTON_CMD, KEY_LONG_PRESS, screen_on_cmd_event);
  s |= button_install_handler(BUTTON_MINUS, KEY_PRESSED, screen_on_minus_pressed);
  s |= button_install_handler(BUTTON_PLUS, KEY_PRESSED, screen_on_plus_pressed);

  if (s == STATUS_OK)
  {
    screen_shutdown_timer = xTimerCreate("ShutDownTimer",
                                         OS_SEC_TO_TICK(15),
                                         pdFALSE,
                                         0,
                                         screen_shutdown_callback);
    if (screen_shutdown_timer == NULL)
      s = STATUS_ERROR;
  }

  if (s == STATUS_OK)
  {
    screen_wakeup_event = xEventGroupCreate();
    if (screen_wakeup_event == NULL)
      s = STATUS_ERROR;
  }


  if (s == STATUS_OK)
  {
    xTaskCreate(screen_wakeup_task, "screen_event_task", SCREEN_EVENT_THREAD_STACK_SIZE, NULL, SCREEN_EVENT_THREAD_PRIORITY,
                NULL);
    xTimerStart(screen_shutdown_timer, 0);
  }

  return s;
}

void screen_set_default_screen ( screen_t* pDefaultScreen )
{
  default_screen = pDefaultScreen;
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

void screen_refresh(void)
{
  if ((current_screen != NULL) && (ser_lcd_get_power_state() == true))
    current_screen->display(current_screen);
}

void screen_generic_display(screen_t* screen)
{
  ser_lcd_write_line(0, screen->line_1);
  ser_lcd_write_line(1, screen->line_2);
}

static void screen_on_cmd_event(key_event evt)
{
  bool p;
  p = screen_check_display_and_wake_up();
  if (p == false)
    return;

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
  bool p;
  p = screen_check_display_and_wake_up();
  if (p == false)
    return;

  if (current_screen->on_minus != NULL)
    current_screen->on_minus(current_screen);
}

static void screen_on_plus_pressed(key_event evt)
{
  bool p;
  p = screen_check_display_and_wake_up();
  if (p == false)
    return;

  if (current_screen->on_plus != NULL)
    current_screen->on_plus(current_screen);
}

static bool screen_check_display_and_wake_up(void)
{
  bool powerState;
  log_dbg_print("Restart screen");
  xTimerReset(screen_shutdown_timer, 10);
  powerState = ser_lcd_get_power_state();
  if (powerState == false)
  {
    xEventGroupSetBits(screen_wakeup_event, SCREEN_WAKE_UP_BIT);
  }
  return powerState;
}

static void screen_shutdown_callback ( TimerHandle_t timer )
{
  log_dbg_print("Stop screen");
  xEventGroupSetBits(screen_wakeup_event, SCREEN_SHUTDOWN_BIT);
}

static void screen_wakeup_task ( void* arg )
{
  UNUSED(arg);
  EventBits_t bits;

  while (1)
  {
    bits = xEventGroupWaitBits(screen_wakeup_event, (SCREEN_WAKE_UP_BIT | SCREEN_SHUTDOWN_BIT), pdTRUE, pdFALSE,
                               OS_WAIT_FOREVER);
    if (bits & SCREEN_WAKE_UP_BIT)
    {
      log_dbg_print("wakeup screen");
      ser_lcd_power_on();
      screen_refresh();
    }

    if (bits & SCREEN_SHUTDOWN_BIT)
    {
      if (default_screen != NULL)
        screen_change_to(default_screen);

      thread_msleep(OS_MSEC_TO_TICK(100)); //NOTE ?? to be checked when clock screen is displayed

      log_dbg_print("shutdown screen");
      ser_lcd_power_off();
    }
  }
}

