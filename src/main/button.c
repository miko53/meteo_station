#include "button.h"
#include "drivers/io.h"
#include "os/os.h"

#define BUTTON_GPIO_INPUT_CMD    (18)
#define BUTTON_GPIO_INPUT_MINUS  (10)
#define BUTTON_GPIO_INPUT_PLUS   (9)

typedef enum
{
  BUTTON_CMD,
  BUTTON_MINUS,
  BUTTON_PLUS,
  BUTTON_NB_MAX
} button_id;

typedef enum
{
  KEY_PRESSED,
  KEY_RELEASED,
  KEY_LONG_PRESS
} key_event;


static QueueHandle_t button_event = NULL;

typedef struct
{
  key_event state;
  button_id button_id;
} button_msg_evt;

static void IRAM_ATTR gpio_isr_handler(void* arg);
static void button_event_task(void* arg);

typedef struct
{
  key_event state;
  TickType_t last_pressed_time;
  //TODO handle
} button_t;

typedef struct
{
  uint32_t gpio_id;
  uint32_t button_id;
} button_gpio_t;

const button_gpio_t button_gpio[BUTTON_NB_MAX] =
{
  {.gpio_id = BUTTON_GPIO_INPUT_CMD, .button_id = BUTTON_CMD },
  {.gpio_id = BUTTON_GPIO_INPUT_MINUS, .button_id = BUTTON_MINUS },
  {.gpio_id = BUTTON_GPIO_INPUT_PLUS, .button_id = BUTTON_PLUS },
};

button_t button_state[BUTTON_NB_MAX];

STATUS button_init(void)
{
  STATUS s;
  uint64_t gpio_mask;
  gpio_mask = (1ULL << BUTTON_GPIO_INPUT_CMD) | (1ULL << BUTTON_GPIO_INPUT_MINUS) | (1ULL << BUTTON_GPIO_INPUT_PLUS);
  s = io_configure_inputs(GPIO_INTR_ANYEDGE, gpio_mask);

  io_configure_input_isr(BUTTON_GPIO_INPUT_CMD, GPIO_INTR_ANYEDGE, gpio_isr_handler, (void*) &button_gpio[BUTTON_CMD]);
  io_configure_input_isr(BUTTON_GPIO_INPUT_MINUS, GPIO_INTR_ANYEDGE, gpio_isr_handler,
                         (void*) &button_gpio[BUTTON_MINUS]);
  io_configure_input_isr(BUTTON_GPIO_INPUT_PLUS, GPIO_INTR_ANYEDGE, gpio_isr_handler, (void*) &button_gpio[BUTTON_PLUS]);
  button_event = xQueueCreate(5, sizeof(button_msg_evt));
  xTaskCreate(button_event_task, "button_event_task", 2048, NULL, 10, NULL);

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


static void button_event_task(void* arg)
{
  UNUSED(arg);
  static int counter;
  button_msg_evt evt;
  TickType_t delay;

  delay = portMAX_DELAY;
  for (;;)
  {
    if (xQueueReceive(button_event, &evt, delay))
    {
      printf("BUTTON[%d] nb = %d (%s)\n", evt.button_id, counter++,
             evt.state == KEY_PRESSED ? "KEY_PRESSED" : "KEY_RELEASED");
      if (evt.state == KEY_PRESSED)
        delay = OS_SEC_TO_TICK(2);
      else
        delay = portMAX_DELAY;
    }
    else
    {
      printf("LONG PRESS\n");
    }

  }
}

