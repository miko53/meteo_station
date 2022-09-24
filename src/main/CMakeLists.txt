
SET(APP_SOURCE main.c 
               log.c
               drivers/i2c.c 
               drivers/pcf_8523.c 
               drivers/ser_lcd.c)
SET(APP_INCLUDE_DIR include)

idf_component_register(SRCS ${APP_SOURCE}
                       INCLUDE_DIRS ${APP_INCLUDE_DIR})
