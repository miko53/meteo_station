# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

if(UNIT_TEST)
    project(meteo-station-tu)
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    message("UNIT_TEST activated")
    find_program(MEMORYCHECK_COMMAND valgrind )
    set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full" )
    find_package(Check REQUIRED)
    include_directories(${CHECK_INCLUDE_DIRS})
    link_directories(${CHECK_LIBRARY_DIRS})
    include (CTest)
    enable_testing()
    add_subdirectory(main)
else()
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(meteo-station)
endif()


