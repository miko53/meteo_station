#include "common.h"
#include "driver/gpio.h"
#include "drivers/io.h"
#include <assert.h>

void io_init(void)
{
  esp_err_t err;
  err = gpio_install_isr_service(0);
  assert(err == ESP_OK);
}

STATUS io_configure_output(uint32_t gpio_id, bool initial_level)
{
  STATUS s;
  s = STATUS_ERROR;
  esp_err_t err;
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << gpio_id);
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;

  err =  gpio_config(&io_conf);
  if (err == ESP_OK)
    err = gpio_set_level(gpio_id, initial_level);

  if (err == ESP_OK)
    s = STATUS_OK;
  return s;
}

STATUS io_configure_inputs(gpio_int_type_t int_type, uint64_t gpio_mask)
{
  esp_err_t err;
  STATUS s;
  s = STATUS_ERROR;
  gpio_config_t io_conf = {};
  io_conf.intr_type = int_type;
  io_conf.pin_bit_mask = gpio_mask;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = 0;
  io_conf.pull_down_en = 0;

  err =  gpio_config(&io_conf);
  if (err == ESP_OK)
    s = STATUS_OK;

  return s;
}

STATUS io_configure_input_isr(uint32_t gpio_id, gpio_int_type_t int_type, gpio_isr_t isr_handler, void* args)
{
  esp_err_t err;
  STATUS s;
  s = STATUS_ERROR;

  err = gpio_set_intr_type(gpio_id, int_type);
  if (err == ESP_OK)
    err = gpio_isr_handler_add(gpio_id, isr_handler, args);

  if (err == ESP_OK)
    s = STATUS_OK;

  return s;
}

