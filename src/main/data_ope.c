#include "data_ope.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

data_operation_t data_ope_list[] =
{
  {
    .sensor = RAIN, .refresh_period = RAINMETER_WAIT_TIME,
    .calcul_period = { .type = SLIDING_PERIOD, .period = 10 },
    .operation = OPE_CUMUL, .history_depth = 24, .bStoreInSD = false
  },
#if 0
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
#endif
};

typedef struct
{
  union
  {
    float ftemp;
    uint32_t itemp;
  };
  uint32_t nbData;
} data_;

data_* data_temp;

void data_ope_do_calcul(uint32_t index, data_operation_t* pOperation, data_msg_t* pData);

STATUS data_ope_init(void)
{
  STATUS s;
  s = STATUS_OK;

  uint32_t nbItems = (sizeof(data_ope_list) / sizeof(data_operation_t));
  fprintf(stdout, "nbItems = %d\n", nbItems);

  data_temp = calloc(nbItems, sizeof(data_));
  if (data_temp == NULL)
    s = STATUS_ERROR;

  for (uint32_t i = 0; ((i < nbItems) && (s == STATUS_OK)); i++)
  {
    if (data_ope_list[i].calcul_period.type == SLIDING_PERIOD)
    {
      uint32_t r = (data_ope_list[i].calcul_period.period % data_ope_list[i].refresh_period);
      if (r != 0)
      {
        fprintf(stdout, "calcul period not adequate with refresh_period for i = %d\n", i);
        s = STATUS_ERROR;
      }
    }
  }

  return s;
}

void data_ope_add(data_msg_t* pData)
{
  data_operation_t* pCurrent;
  for (uint32_t i = 0; i < (sizeof(data_ope_list) / sizeof(data_operation_t)); i++)
  {
    pCurrent = &data_ope_list[i];
    if (pCurrent->sensor == pData->type)
    {
      data_ope_do_calcul(i, pCurrent, pData);
    }
  }
}

void data_ope_do_calcul(uint32_t index, data_operation_t* pOperation, data_msg_t* pData)
{
  uint32_t nbItems;
  switch (pOperation->calcul_period.type)
  {
    case FIXED_PERIOD:
      break;

    case SLIDING_PERIOD:
      if (pOperation->operation == OPE_MAX)
      {
        if (data_temp[index].nbData == 0)
        {
          data_temp[index].ftemp = pData->value.f;
        }
        else
        {
          if (pData->value.f > data_temp[index].ftemp)
            data_temp[index].ftemp = pData->value.f;
        }
      }
      else if (pOperation->operation == OPE_MIN)
      {
        if (data_temp[index].nbData == 0)
        {
          data_temp[index].ftemp = pData->value.f;
        }
        else
        {
          if (pData->value.f < data_temp[index].ftemp)
            data_temp[index].ftemp = pData->value.f;
        }
      }
      else
      {
        data_temp[index].ftemp += pData->value.f;
        data_temp[index].nbData++;
      }

      nbItems = pOperation->calcul_period.period / pOperation->refresh_period;
      fprintf(stdout, "nbItems = %d\n", nbItems);
      if (data_temp[index].nbData >= nbItems)
      {
        fprintf(stdout, "do operation\n");
        if (pOperation->operation == OPE_AVERAGE)
        {
          data_temp[index].ftemp /= nbItems;
        }
        data_temp[index].nbData = 0;
      }
      break;
  }
}
