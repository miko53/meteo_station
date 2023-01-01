
if(UNIT_TEST)
add_compile_definitions(UNIT_TEST)
include_directories(include)
include_directories(.)
add_executable(tu_data_test 
                data_ope.c
                histogram.c
                tu/tu_data_ope.c
               )

target_link_libraries(tu_data_test ${CHECK_LIBRARIES} pthread)
add_test(tu_data_test tu_data_test)

add_executable(tu_data_fix_period_test
                data_ope.c
                histogram.c
                tu/tu_data_fix_period.c
               )
target_link_libraries(tu_data_fix_period_test ${CHECK_LIBRARIES} pthread)
add_test(tu_data_fix_period_test tu_data_fix_period_test)

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
               gui/screen.c
               gui/menu.c
               gui/menu_date_time.c
               gui/menu_config.c
               gui/menu_histogram.c
               filelog.c
               libs.c
               ctrl.c
               rainmeter.c
               anemometer.c
               winddir.c
               data_ope.c
               histogram.c
               data_ope_config.c
               nvstorage.c
               ble/ble.c
               ble/ble_gatt_svr.c
               ble/ble_env_srv.c
               ble/ble_date_srv.c
               zigbee.c
#               main_test.c
               )
               
SET(APP_INCLUDE_DIR "include" "gui" ".")

idf_component_register(SRCS ${APP_SOURCE}
                       INCLUDE_DIRS ${APP_INCLUDE_DIR})
endif()
