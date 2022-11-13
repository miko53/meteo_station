#include "data_ope.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
  uint32_t nbitems;
  uint32_t current_index;
  bool bFill;
  variant* datas;
} histogram_t;



typedef struct
{
  variant temp;
  uint32_t nbData;
  histogram_t histo;
} data_;

static data_* data_temp;
static data_operation_t* data_ope_list;
static uint32_t data_ope_nbItems;

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
  if (h->current_index >= h->nbitems)
  {
    h->current_index = 0;
    h->bFill = true;
  }
  h->datas[h->current_index++] = v;
}

STATUS histogram_get(uint32_t histoIndex, uint32_t index, variant* v)
{
  STATUS s;
  if (histoIndex >= data_ope_nbItems)
    s = STATUS_ERROR;
  else
  {
    histogram_t* h = &data_temp[histoIndex].histo;
    if ((h->bFill == false) && (index >= h->current_index))
      s = STATUS_ERROR;
    else
    {
      s = STATUS_OK;
      uint32_t index_to_take = (h->current_index - 1 - index) % h->nbitems;
      fprintf(stdout, "index_to_take = %d\n", index_to_take);
      *v = h->datas[index_to_take];
    }
  }
  return s;
}

int32_t histogram_nbItems(uint32_t histoIndex)
{
  int32_t r;
  if (histoIndex >= data_ope_nbItems)
    r = -1;
  else
  {
    histogram_t* h = &data_temp[histoIndex].histo;
    if (h->bFill == true)
      r = h->nbitems;
    else
      r = h->current_index;
  }
  return r;
}


static void data_ope_do_calcul(uint32_t index, data_operation_t* pOperation, data_msg_t* pData);

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

void data_ope_add(data_msg_t* pData)
{
  data_operation_t* pCurrent;
  for (uint32_t i = 0; i < data_ope_nbItems; i++)
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

      nbItems = pOperation->calcul_period.period_sec / pOperation->refresh_period_sec;
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
