
#include "common.h"
#include "drivers/i2c.h"
#include "driver/i2c.h"

#define I2C_LCD_SDA_IO              (25)
#define I2C_LCD_SCL_IO              (26)
#define I2C_LCD_FREQ_HZ             (100000)
#define I2C_LCD_TX_BUF_DISABLE      (0)
#define I2C_LCD_RX_BUF_DISABLE      (0)
#define I2C_LCD_TIMEOUT_MS          (1000)

#define I2C_SENSOR_SDA_IO              (33)
#define I2C_SENSOR_SCL_IO              (27)
#define I2C_SENSOR_FREQ_HZ             (100000)
#define I2C_SENSOR_TX_BUF_DISABLE      (0)
#define I2C_SENSOR_RX_BUF_DISABLE      (0)
#define I2C_SENSOR_TIMEOUT_MS          (1000)

STATUS i2c_init(void)
{
  STATUS status;
  esp_err_t s;
  status = STATUS_ERROR;


  i2c_config_t conf_lcd =
  {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_LCD_SDA_IO,
    .scl_io_num = I2C_LCD_SCL_IO,
    .sda_pullup_en = GPIO_PULLUP_DISABLE,
    .scl_pullup_en = GPIO_PULLUP_DISABLE,
    .master.clk_speed = I2C_LCD_FREQ_HZ,
  };

  i2c_config_t conf_sensor =
  {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_SENSOR_SDA_IO,
    .scl_io_num = I2C_SENSOR_SCL_IO,
    .sda_pullup_en = GPIO_PULLUP_DISABLE,
    .scl_pullup_en = GPIO_PULLUP_DISABLE,
    .master.clk_speed = I2C_LCD_FREQ_HZ,
  };

  s = i2c_param_config(I2C_LCD_ID, &conf_lcd);
  if (s == ESP_OK)
  {
    s = i2c_driver_install(I2C_LCD_ID, conf_lcd.mode, I2C_LCD_RX_BUF_DISABLE, I2C_LCD_TX_BUF_DISABLE, 0);
  }

  if (s == ESP_OK)
  {
    s = i2c_param_config(I2C_SENSOR_ID, &conf_sensor);
  }

  if (s == ESP_OK)
  {
    s = i2c_driver_install(I2C_SENSOR_ID, conf_sensor.mode, I2C_LCD_RX_BUF_DISABLE, I2C_LCD_TX_BUF_DISABLE, 0);
  }

  if (s == ESP_OK)
  {
    status = STATUS_OK;
  }

  return status;
}

