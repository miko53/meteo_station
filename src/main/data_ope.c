#include "data_ope.h"
#include "config.h"

data_operation_t data_ope_list[] =
{
  {
    .sensor = RAIN, .refresh_period = RAINMETER_WAIT_TIME,
    .calcul_period = { .type = SLIDING_PERIOD, .period = 3600 },
    .operation = OPE_CUMUL, .history_depth = 24, .bStoreInSD = false
  },
  {
    .sensor = RAIN, .refresh_period = RAINMETER_WAIT_TIME,
    .calcul_period = { .type = FIXED_PERIOD, .f_period = { .period = 1, .unit = BY_DAY }},
    .operation = OPE_CUMUL, .history_depth = 5, .bStoreInSD = true
  },
  {
    .sensor = WIND_SPEED, .refresh_period = ANEMOMETER_WAIT_TIME,
    .calcul_period = { .type = SLIDING_PERIOD, .period = 3600 },
    .operation = OPE_AVERAGE, .history_depth = 24, .bStoreInSD = false
  },
  {
    .sensor = WIND_SPEED, .refresh_period = ANEMOMETER_WAIT_TIME,
    .calcul_period = { .type = FIXED_PERIOD, .f_period = { .period = 1, .unit = BY_DAY }},
    .operation = OPE_AVERAGE, .history_depth = 5, .bStoreInSD = true
  },
  {
    .sensor = WIND_SPEED, .refresh_period = ANEMOMETER_WAIT_TIME,
    .calcul_period = { .type = FIXED_PERIOD, .f_period = { .period = 1, .unit = BY_DAY }},
    .operation = OPE_MAX, .history_depth = 5, .bStoreInSD = true
  },
  {
    .sensor = WIND_DIR, .refresh_period = WINDDIR_WAIT_TIME,
    .calcul_period = { .type = SLIDING_PERIOD, .period = 3600 },
    .operation = OPE_AVERAGE, .history_depth = 24, .bStoreInSD = false
  },
  {
    .sensor = WIND_DIR, .refresh_period = WINDDIR_WAIT_TIME,
    .calcul_period = { .type = FIXED_PERIOD, .f_period = { .period = 1, .unit = BY_DAY }},
    .operation = OPE_AVERAGE, .history_depth = 5, .bStoreInSD = true
  },
};
