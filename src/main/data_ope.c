#include "data_ope.h"
#include <stdio.h>
#include <stdlib.h>
#include "libs.h"
#include "histogram.h"

typedef struct
{
  variant_t temp;
  uint32_t nbData;
  histogram_t histo;
  struct tm beginPeriodDate;
  bool lastDateIsValid;
  bool activated;
} data_;

static data_* data_temp;
static data_operation_t* data_ope_list;
static uint32_t data_ope_nbItems;

static void data_ope_do_calcul(uint32_t index, data_operation_t* pOperation, variant_t* pSample);
static void data_ope_prepare_and_insert(uint32_t index, data_operation_t* pOperation, variant_t* pData);
static bool data_ope_is_hour_diff(struct tm* newDate, struct tm* previousDate, int32_t* diff);
static bool data_ope_is_day_diff(struct tm* newDate, struct tm* previousDate, int32_t* diff);
static bool data_ope_is_month_diff(struct tm* newDate, struct tm* previousDate, int32_t* diff);

STATUS data_ope_init(data_operation_t pDataOpeList[], uint32_t nbItemsInList)
{
  STATUS s;
  s = STATUS_OK;

  data_ope_list = pDataOpeList;
  data_ope_nbItems = nbItemsInList;

  fprintf(stdout, "array = %p, nbItems = %d\n", data_ope_list, data_ope_nbItems);

  data_temp = calloc(data_ope_nbItems, sizeof(data_));
  if (data_temp == NULL)
    s = STATUS_ERROR;

  for (uint32_t i = 0; ((i < data_ope_nbItems) && (s == STATUS_OK)); i++)
  {
    s = histogram_init(&data_temp[i].histo, data_ope_list[i].history_depth);
  }

  for (uint32_t i = 0; ((i < data_ope_nbItems) && (s == STATUS_OK)); i++)
  {
    if (data_ope_list[i].calcul_period.type == SLIDING_PERIOD)
    {
      uint32_t r = (data_ope_list[i].calcul_period.period_sec % data_ope_list[i].refresh_period_sec);
      if (r != 0)
      {
        fprintf(stdout, "calcul period not adequate with refresh_period for i = %d\n", i);
        s = STATUS_ERROR;
      }
    }
  }

  return s;
}

histogram_t* data_ope_get_histo(uint32_t indexOperation)
{
  histogram_t* r;
  if (indexOperation >= data_ope_nbItems)
    r = NULL;
  else
    r = &data_temp[indexOperation].histo;
  return r;
}

void data_ope_add_sample(data_type_t dataType, variant_t* pSample)
{
  data_operation_t* pCurrent;
  for (uint32_t i = 0; i < data_ope_nbItems; i++)
  {
    pCurrent = &data_ope_list[i];
    if (pCurrent->sensor == dataType)
    {
      data_* pData = &data_temp[i];
      if (pData->activated)
        data_ope_do_calcul(i, pCurrent, pSample);
    }
  }
}

void data_ope_activate_all(void)
{
  struct tm beginPeriodDate;
  date_get_localtime(&beginPeriodDate);

  for (uint32_t i = 0; i < data_ope_nbItems; i++)
  {
    data_* pData = &data_temp[i];
    pData->beginPeriodDate = beginPeriodDate;
    pData->lastDateIsValid = true;
    pData->activated = true;
  }
}

void data_ope_do_calcul(uint32_t index, data_operation_t* pOperation, variant_t* pSample)
{
  uint32_t nbItems;
  switch (pOperation->calcul_period.type)
  {
    case FIXED_PERIOD:
      {
        if (data_temp[index].lastDateIsValid == false)
        {
          date_get_localtime(&data_temp[index].beginPeriodDate);
          data_temp[index].lastDateIsValid = true;
        }
        else
        {
          struct tm currentDate;
          bool hasDiff = false;
          int32_t diff;

          data_ope_prepare_and_insert(index, pOperation, pSample);
          date_get_localtime(&currentDate);

          switch (pOperation->calcul_period.f_period.unit)
          {
            case BY_HOUR:
              hasDiff = data_ope_is_hour_diff(&currentDate, &data_temp[index].beginPeriodDate, &diff);
              break;

            case BY_DAY:
              hasDiff = data_ope_is_day_diff(&currentDate, &data_temp[index].beginPeriodDate, &diff);
              break;

            case BY_MONTH:
              hasDiff = data_ope_is_month_diff(&currentDate, &data_temp[index].beginPeriodDate, &diff);
              break;

            default:
              break;
          }

          if ((hasDiff) && ((uint32_t) diff >= pOperation->calcul_period.f_period.period))
          {
            fprintf(stdout, "diff detected (index = %d) do operation on (%d nb Data)\n", index, data_temp[index].nbData);
            if (pOperation->operation == OPE_AVERAGE)
            {
              data_temp[index].temp.f32 /= data_temp[index].nbData;
            }
            histogram_insert(&data_temp[index].histo, &data_temp[index].temp);
            data_temp[index].beginPeriodDate = currentDate;
            data_temp[index].nbData = 0;
          }
        }
      }
      break;

    case SLIDING_PERIOD:
      data_ope_prepare_and_insert(index, pOperation, pSample);

      nbItems = pOperation->calcul_period.period_sec / pOperation->refresh_period_sec;
      fprintf(stdout, "nbItems = %d, nbData = %d\n", nbItems, data_temp[index].nbData);
      if (data_temp[index].nbData >= nbItems)
      {
        fprintf(stdout, "do operation\n");
        if (pOperation->operation == OPE_AVERAGE)
        {
          data_temp[index].temp.f32 /= nbItems;
        }
        histogram_insert(&data_temp[index].histo, &data_temp[index].temp);
        data_temp[index].nbData = 0;
      }
      break;
  }
}

void data_ope_prepare_and_insert(uint32_t index, data_operation_t* pOperation, variant_t* pData)
{
  if (pOperation->operation == OPE_MAX)
  {
    if (data_temp[index].nbData == 0)
    {
      data_temp[index].temp = *pData;
    }
    else
    {
      bool b;
      b = variant_is_higher(pData, &data_temp[index].temp, data_temp[index].temp.type);
      if (b)
        data_temp[index].temp = *pData;
    }
  }
  else if (pOperation->operation == OPE_MIN)
  {
    if (data_temp[index].nbData == 0)
    {
      data_temp[index].temp = *pData;
    }
    else
    {
      bool b;
      b = variant_is_lower(pData, &data_temp[index].temp, data_temp[index].temp.type);
      if (b)
        data_temp[index].temp = *pData;
    }
  }
  else
  {
    if (data_temp[index].nbData == 0)
      data_temp[index].temp = *pData;
    else
    {
      variant_add(&data_temp[index].temp, &data_temp[index].temp, pData, data_temp[index].temp.type);
    }
  }
  data_temp[index].nbData++;
}


bool data_ope_is_hour_diff(struct tm* newDate, struct tm* previousDate, int32_t* diff)
{
  bool r = false;
  *diff = newDate->tm_hour - previousDate->tm_hour;
  if (*diff != 0)
  {
    r = true;
  }
  return r;
}

bool data_ope_is_day_diff(struct tm* newDate, struct tm* previousDate, int32_t* diff)
{
  bool r = false;
  *diff = newDate->tm_mday - previousDate->tm_mday;
  if (*diff != 0)
  {
    r = true;
  }
  return r;
}

bool data_ope_is_month_diff(struct tm* newDate, struct tm* previousDate, int32_t* diff)
{
  bool r = false;
  *diff = newDate->tm_mon - previousDate->tm_mon;
  if (*diff != 0)
  {
    r = true;
  }
  return r;
}
