#include "common.h"
#include "driver/gpio.h"
#include "drivers/io.h"

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

