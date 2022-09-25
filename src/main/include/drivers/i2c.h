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
extern STATUS i2c_read(i2c_id id, uint8_t i2c_address, uint8_t* buffer, uint32_t size);
extern STATUS i2c_write(i2c_id id, uint8_t i2c_address, uint8_t* buffer, uint32_t size);


#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */
