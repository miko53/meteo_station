
if(UNIT_TEST)
include_directories(include)
include_directories(.)
add_executable(tu_data_test 
                data_ope.c
                tu/tu_data_ope.c
               )

target_link_libraries(tu_data_test ${CHECK_LIBRARIES} pthread)
add_test(tu_data_test tu_data_test)


else()

SET(APP_SOURCE main.c 
               log.c
               drivers/i2c.c 
               drivers/pcf_8523.c 
               drivers/ser_lcd.c
               drivers/sd_card.c
               drivers/analog.c
               drivers/io.c
               button.c
               screen.c
               menu.c
               menu_date_time.c
               filelog.c
               libs.c
               ctrl.c
               rainmeter.c
               anemometer.c
               winddir.c
               data_ope.c
               data_ope_config.c
#               main_test.c
               )
SET(APP_INCLUDE_DIR include)

idf_component_register(SRCS ${APP_SOURCE}
                       INCLUDE_DIRS ${APP_INCLUDE_DIR})
endif()
