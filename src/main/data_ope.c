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
  uint32_t nbitems;
  uint32_t current_index;
  bool bFill;
  variant* datas;
} histogram_t;

STATUS histogram_init(histogram_t* h, uint32_t nbItems)
{
  STATUS s;
  s = STATUS_ERROR;
  h->current_index = 0;
  h->nbitems = nbItems;
  h->bFill = false;
  h->datas = calloc(nbItems, sizeof(variant));
  if (h->datas != NULL)
    s = STATUS_OK;
  return s;
}

void histogram_insert(histogram_t* h, variant v)
{
  h->datas[h->current_index++] = v;
  if (h->current_index >= h->nbitems)
  {
    h->current_index = 0;
    h->bFill = true;
  }
}


typedef struct
{
  variant temp;
  uint32_t nbData;
  histogram_t histo;
} data_;

data_* data_temp;


STATUS histogram_get(uint32_t itemData, uint32_t index, variant* v)
{
  STATUS s;
  histogram_t* h = &data_temp[itemData].histo;
  if ((h->bFill == false) && (index >= h->current_index))
    s = STATUS_ERROR;
  else
  {
    s = STATUS_OK;
    uint32_t index_to_take = (h->current_index - 1 - index) % h->nbitems;
    *v = h->datas[index_to_take];
  }
  return s;
}

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
    s = histogram_init(&data_temp[i].histo, data_ope_list[i].history_depth);
  }

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
          data_temp[index].temp.f32 = pData->value.f;
        }
        else
        {
          if (pData->value.f > data_temp[index].temp.f32)
            data_temp[index].temp.f32 = pData->value.f;
        }
      }
      else if (pOperation->operation == OPE_MIN)
      {
        if (data_temp[index].nbData == 0)
        {
          data_temp[index].temp.f32 = pData->value.f;
        }
        else
        {
          if (pData->value.f < data_temp[index].temp.f32)
            data_temp[index].temp.f32 = pData->value.f;
        }
      }
      else
      {
        data_temp[index].temp.f32 += pData->value.f;
      }
      data_temp[index].nbData++;

      nbItems = pOperation->calcul_period.period / pOperation->refresh_period;
      fprintf(stdout, "nbItems = %d\n", nbItems);
      if (data_temp[index].nbData >= nbItems)
      {
        fprintf(stdout, "do operation\n");
        if (pOperation->operation == OPE_AVERAGE)
        {
          data_temp[index].temp.f32 /= nbItems;
        }
        data_temp[index].nbData = 0;
        histogram_insert(&data_temp[index].histo, data_temp[index].temp);
      }
      break;
  }
}
