#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  I2C_LCD_ID,
  I2C_SENSOR_ID
} i2c_id;

extern STATUS i2c_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */
