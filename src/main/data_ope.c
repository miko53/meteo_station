#include "data_ope.h"
#include <stdio.h>
#include <stdlib.h>
#include "libs.h"
#include "histogram.h"
#include "log.h"

typedef struct
{
  variant_t temp;
  uint32_t nbData;
  histogram_t histo;
  struct tm beginPeriodDate;
  bool lastDateIsValid;
  bool activated;
} data_ope_context;

static data_ope_context* data_samples;
static data_ope_cnf data_ope_config;

static void data_ope_do_calcul(uint32_t index, data_operation_t* pOperation, variant_t* pSample);
static void data_ope_prepare_and_insert(uint32_t index, data_operation_t* pOperation, variant_t* pData);
static bool data_ope_is_hour_diff(struct tm* newDate, struct tm* previousDate, int32_t* diff);
static bool data_ope_is_day_diff(struct tm* newDate, struct tm* previousDate, int32_t* diff);
static bool data_ope_is_month_diff(struct tm* newDate, struct tm* previousDate, int32_t* diff);

const char* period_unit[3] =
{
  "hrs",
  "jrs",
  "mois",
};

STATUS data_ope_init(data_ope_cnf* pConfig)
{
  STATUS s;
  s = STATUS_OK;

  data_ope_config = *pConfig;

  fprintf(stdout, "array = %p, nbItems = %d\n", data_ope_config.pDataOpeList, data_ope_config.nbItemsInList);

  data_samples = calloc(data_ope_config.nbItemsInList, sizeof(data_ope_context));
  if (data_samples == NULL)
    s = STATUS_ERROR;

  for (uint32_t i = 0; ((i < data_ope_config.nbItemsInList) && (s == STATUS_OK)); i++)
  {
    s = histogram_init(&data_samples[i].histo, data_ope_config.pDataOpeList[i].history_depth);
  }

  for (uint32_t i = 0; ((i < data_ope_config.nbItemsInList) && (s == STATUS_OK)); i++)
  {
    if (data_ope_config.pDataOpeList[i].calcul_period.type == SLIDING_PERIOD)
    {
      uint32_t r = (data_ope_config.pDataOpeList[i].calcul_period.period_sec %
                    data_ope_config.pDataOpeList[i].refresh_period_sec);
      if (r != 0)
      {
        fprintf(stdout, "calcul period not adequate with refresh_period for i = %d\n", i);
        s = STATUS_ERROR;
      }
    }
  }

  return s;
}

uint32_t data_ope_nb_operation(void)
{
  return data_ope_config.nbItemsInList;
}

data_operation_t* date_ope_get_operation(uint32_t indexOperation)
{
  return &data_ope_config.pDataOpeList[indexOperation];
}

histogram_t* data_ope_get_histo(uint32_t indexOperation)
{
  histogram_t* r;
  if (indexOperation >= data_ope_config.nbItemsInList)
    r = NULL;
  else
    r = &data_samples[indexOperation].histo;
  return r;
}

void data_ope_add_sample(data_type_t dataType, variant_t* pSample)
{
  data_operation_t* pCurrent;
  for (uint32_t i = 0; i < data_ope_config.nbItemsInList; i++)
  {
    pCurrent = &data_ope_config.pDataOpeList[i];
    if (pCurrent->sensor == dataType)
    {
      data_ope_context* pData = &data_samples[i];
      if (pData->activated)
        data_ope_do_calcul(i, pCurrent, pSample);
    }
  }
}

void data_ope_activate_all(void)
{
  struct tm beginPeriodDate;
  date_get_localtime(&beginPeriodDate);

  for (uint32_t i = 0; i < data_ope_config.nbItemsInList; i++)
  {
    data_ope_context* pData = &data_samples[i];
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
        if (data_samples[index].lastDateIsValid == false)
        {
          date_get_localtime(&data_samples[index].beginPeriodDate);
          data_samples[index].lastDateIsValid = true;
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
              hasDiff = data_ope_is_hour_diff(&currentDate, &data_samples[index].beginPeriodDate, &diff);
              break;

            case BY_DAY:
              hasDiff = data_ope_is_day_diff(&currentDate, &data_samples[index].beginPeriodDate, &diff);
              break;

            case BY_MONTH:
              hasDiff = data_ope_is_month_diff(&currentDate, &data_samples[index].beginPeriodDate, &diff);
              break;

            default:
              break;
          }

          if ((hasDiff) && ((uint32_t) diff >= pOperation->calcul_period.f_period.period))
          {
            //fprintf(stdout, "diff detected (index = %d) do operation on (%d nb Data)\n", index, data_samples[index].nbData);
            if (pOperation->operation == OPE_AVERAGE)
            {
              data_samples[index].temp.f32 /= data_samples[index].nbData;
            }
            histogram_insert(&data_samples[index].histo, &data_samples[index].temp);
            if (data_ope_config.on_new_calculated_data != NULL)
              data_ope_config.on_new_calculated_data(index, &data_samples[index].temp);

            data_samples[index].beginPeriodDate = currentDate;
            data_samples[index].nbData = 0;
          }
        }
      }
      break;

    case SLIDING_PERIOD:
      data_ope_prepare_and_insert(index, pOperation, pSample);

      nbItems = pOperation->calcul_period.period_sec / pOperation->refresh_period_sec;
      //log_info_print("index = %d, nbItems = %d, nbData = %d", index, nbItems, data_samples[index].nbData);
      if (data_samples[index].nbData >= nbItems)
      {
        //fprintf(stdout, "do operation\n");
        if (pOperation->operation == OPE_AVERAGE)
        {
          data_samples[index].temp.f32 /= nbItems;
        }
        histogram_insert(&data_samples[index].histo, &data_samples[index].temp);
        if (data_ope_config.on_new_calculated_data != NULL)
          data_ope_config.on_new_calculated_data(index, &data_samples[index].temp);

        data_samples[index].nbData = 0;
      }
      break;
  }
}

void data_ope_prepare_and_insert(uint32_t index, data_operation_t* pOperation, variant_t* pData)
{
  if (pOperation->operation == OPE_MAX)
  {
    if (data_samples[index].nbData == 0)
    {
      data_samples[index].temp = *pData;
    }
    else
    {
      bool b;
      b = variant_is_higher(pData, &data_samples[index].temp, data_samples[index].temp.type);
      if (b)
        data_samples[index].temp = *pData;
    }
  }
  else if (pOperation->operation == OPE_MIN)
  {
    if (data_samples[index].nbData == 0)
    {
      data_samples[index].temp = *pData;
    }
    else
    {
      bool b;
      b = variant_is_lower(pData, &data_samples[index].temp, data_samples[index].temp.type);
      if (b)
        data_samples[index].temp = *pData;
    }
  }
  else
  {
    if (data_samples[index].nbData == 0)
      data_samples[index].temp = *pData;
    else
    {
      variant_add(&data_samples[index].temp, &data_samples[index].temp, pData, data_samples[index].temp.type);
    }
  }
  data_samples[index].nbData++;
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
