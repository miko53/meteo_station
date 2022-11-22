#include "histogram.h"
#include <stdio.h>

STATUS histogram_init(histogram_t* h, uint32_t nbItems)
{
  STATUS s;
  s = STATUS_ERROR;
  h->current_index = 0;
  h->nbitems = nbItems;
  h->bFill = false;
  h->datas = calloc(nbItems, sizeof(variant_t));
  if (h->datas != NULL)
    s = STATUS_OK;
  return s;
}

void histogram_insert(histogram_t* h, variant_t* v)
{
  if (h->current_index >= h->nbitems)
  {
    h->current_index = 0;
    h->bFill = true;
  }
  h->datas[h->current_index++] = *v;
}

STATUS histogram_get(histogram_t* h, uint32_t index, variant_t* v)
{
  STATUS s;
  if (h == NULL)
    s = STATUS_ERROR;
  else
  {
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

int32_t histogram_nbItems(histogram_t* h)
{
  int32_t r;
  if (h == NULL)
    r = -1;
  else
  {
    if (h->bFill == true)
      r = h->nbitems;
    else
      r = h->current_index;
  }
  return r;
}

