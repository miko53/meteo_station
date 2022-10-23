#include "button.h"
#include "drivers/io.h"
#include "os/os.h"

#define BUTTON_LONG_PRESS_DURATION        OS_MSEC_TO_TICK(1000)
#define BUTTON_GPIO_INPUT_CMD                          (18)
#define BUTTON_GPIO_INPUT_MINUS                        (10)
#define BUTTON_GPIO_INPUT_PLUS                          (9)

typedef struct
{
  key_event state;
  button_id button_id;
} button_msg_evt;

typedef struct
{
  key_event state;
  TickType_t last_pressed_time;
  button_cb_handler key_pressed_handler;
  button_cb_handler key_released_handler;
  button_cb_handler key_long_press_handler;
  button_cb_handler click_handler;
} button_t;

typedef struct
{
  uint32_t gpio_id;
  uint32_t button_id;
} button_gpio_t;

const button_gpio_t button_gpio[BUTTON_NB_MAX] =
{
  {.gpio_id = BUTTON_GPIO_INPUT_CMD,   .button_id = BUTTON_CMD },
  {.gpio_id = BUTTON_GPIO_INPUT_MINUS, .button_id = BUTTON_MINUS },
  {.gpio_id = BUTTON_GPIO_INPUT_PLUS,  .button_id = BUTTON_PLUS },
};

static QueueHandle_t button_event = NULL;
static button_t button_state[BUTTON_NB_MAX];

static void IRAM_ATTR gpio_isr_handler(void* arg);
static void button_event_task(void* arg);

STATUS button_init(void)
{
  STATUS s;
  uint64_t gpio_mask;

  for (uint32_t i = 0; i < BUTTON_NB_MAX; i++)
    button_state[i].state = KEY_RELEASED;

  gpio_mask = (1ULL << BUTTON_GPIO_INPUT_CMD) | (1ULL << BUTTON_GPIO_INPUT_MINUS) | (1ULL << BUTTON_GPIO_INPUT_PLUS);
  s = io_configure_inputs(GPIO_INTR_ANYEDGE, gpio_mask);

  io_configure_input_isr(BUTTON_GPIO_INPUT_CMD, GPIO_INTR_ANYEDGE, gpio_isr_handler, (void*) &button_gpio[BUTTON_CMD]);
  io_configure_input_isr(BUTTON_GPIO_INPUT_MINUS, GPIO_INTR_ANYEDGE, gpio_isr_handler,
                         (void*) &button_gpio[BUTTON_MINUS]);
  io_configure_input_isr(BUTTON_GPIO_INPUT_PLUS, GPIO_INTR_ANYEDGE, gpio_isr_handler, (void*) &button_gpio[BUTTON_PLUS]);
  button_event = xQueueCreate(5, sizeof(button_msg_evt));
  xTaskCreate(button_event_task, "button_event_task", 2048, NULL, configMAX_PRIORITIES - 10, NULL);

  return s;
}

static void gpio_isr_handler(void* arg)
{
  button_msg_evt evt;
  button_gpio_t* pButtonDef = (button_gpio_t*) arg;

  evt.button_id =  pButtonDef->button_id;
  if (gpio_get_level(pButtonDef->gpio_id) == 1)
    evt.state = KEY_PRESSED;
  else
    evt.state = KEY_RELEASED;

  xQueueSendFromISR(button_event, &evt, NULL);
}

STATUS button_install_handler(button_id buttonId, key_event event, button_cb_handler button_callback)
{
  STATUS s;
  s = STATUS_OK;
  if ((buttonId < BUTTON_NB_MAX) && (button_callback != NULL))
  {
    switch (event)
    {
      case KEY_PRESSED:
        button_state[buttonId].key_pressed_handler = button_callback;
        break;

      case KEY_LONG_PRESS:
        button_state[buttonId].key_long_press_handler = button_callback;
        break;

      case KEY_RELEASED:
        button_state[buttonId].key_released_handler = button_callback;
        break;

      case KEY_CLICKED:
        button_state[buttonId].click_handler = button_callback;
        break;

      default:
        s = STATUS_ERROR;
        break;
    }
  }
  else
    s = STATUS_ERROR;
  return s;
}


static void button_event_task(void* arg)
{
  UNUSED(arg);
  //   static int counter;
  button_msg_evt evt;
  TickType_t delay;
  TickType_t currentTick;

  delay = OS_WAIT_FOREVER;
  for (;;)
  {
    if (xQueueReceive(button_event, &evt, delay))
    {
      currentTick = tick_get();

      button_state[evt.button_id].state = evt.state;
      //       printf("BUTTON[%d] nb = %d tick = %d, (%s)\n", evt.button_id, counter++, button_state[evt.button_id].last_pressed_time,
      //              evt.state == KEY_PRESSED ? "KEY_PRESSED" : "KEY_RELEASED");

      if ((button_state[evt.button_id].state == KEY_PRESSED) && (button_state[evt.button_id].key_pressed_handler != NULL))
        button_state[evt.button_id].key_pressed_handler(KEY_PRESSED);
      else
      {
        if (button_state[evt.button_id].state == KEY_RELEASED)
        {
          if (button_state[evt.button_id].key_released_handler != NULL)
            button_state[evt.button_id].key_released_handler(KEY_RELEASED);

          if ((button_state[evt.button_id].click_handler != NULL)
              && ((currentTick - button_state[evt.button_id].last_pressed_time) < BUTTON_LONG_PRESS_DURATION))
            button_state[evt.button_id].click_handler(KEY_CLICKED);
        }
      }

      button_state[evt.button_id].last_pressed_time = tick_get();
      delay = OS_WAIT_FOREVER;
      for (uint32_t i = 0; i < BUTTON_NB_MAX; i++)
      {
        if (evt.state == KEY_PRESSED)
        {
          delay = BUTTON_LONG_PRESS_DURATION;
          break;
        }
      }
    }
    else
    {
      currentTick = tick_get();
      for (uint32_t i = 0; i < BUTTON_NB_MAX; i++)
      {
        if (button_state[i].state == KEY_PRESSED)
        {
          TickType_t delta = currentTick - button_state[i].last_pressed_time;
          if (delta >= BUTTON_LONG_PRESS_DURATION)
          {
            //             printf("BUTTON[%d] LONG_PRESS\n", i);
            if (button_state[i].key_long_press_handler != NULL)
              button_state[i].key_long_press_handler(KEY_LONG_PRESS);
          }
        }
      }
    }
  }
}

