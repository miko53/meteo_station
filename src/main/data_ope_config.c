#include "data_ope_config.h"


static data_operation_t data_ope_config_list[] =
{
  {
    .sensor = RAIN, .refresh_period_sec = RAINMETER_WAIT_TIME,
    .calcul_period = { .type = SLIDING_PERIOD, .period_sec = 10 },
    .operation = OPE_CUMUL, .history_depth = 6, .bStoreInSD = true
  },
  {
    .sensor = RAIN, .refresh_period_sec = RAINMETER_WAIT_TIME,
    .calcul_period = { .type = FIXED_PERIOD, .f_period = { .period = 1, .unit = BY_DAY }},
    .operation = OPE_CUMUL, .history_depth = 5, .bStoreInSD = true
  },
  {
    .sensor = WIND_SPEED, .refresh_period_sec = ANEMOMETER_WAIT_TIME,
    .calcul_period = { .type = SLIDING_PERIOD, .period_sec = 10 },
    .operation = OPE_AVERAGE, .history_depth = 24, .bStoreInSD = false
  },
  {
    .sensor = WIND_SPEED, .refresh_period_sec = ANEMOMETER_WAIT_TIME,
    .calcul_period = { .type = FIXED_PERIOD, .f_period = { .period = 1, .unit = BY_DAY }},
    .operation = OPE_AVERAGE, .history_depth = 5, .bStoreInSD = true
  },
  {
    .sensor = WIND_DIR, .refresh_period_sec = WINDDIR_WAIT_TIME,
    .calcul_period = { .type = SLIDING_PERIOD, .period_sec = 10 },
    .operation = OPE_AVERAGE, .history_depth = 24, .bStoreInSD = false
  },
  {
    .sensor = WIND_DIR, .refresh_period_sec = WINDDIR_WAIT_TIME,
    .calcul_period = { .type = FIXED_PERIOD, .f_period = { .period = 1, .unit = BY_DAY }},
    .operation = OPE_AVERAGE, .history_depth = 5, .bStoreInSD = true
  },
};

data_operation_t* date_ope_config_get(void)
{
  return data_ope_config_list;
}

uint32_t date_ope_config_nbItems(void)
{
  return sizeof(data_ope_config_list) / sizeof(data_operation_t);
}
