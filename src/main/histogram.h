#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include "common.h"
#include "variant.h"

#define LAST_VALUE  (0)

typedef struct
{
  uint32_t nbitems;
  uint32_t current_index;
  bool bFill;
  variant_t* datas;
} histogram_t;

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS histogram_init(histogram_t* h, uint32_t nbItems);
extern void histogram_insert(histogram_t* h, variant_t v);
extern STATUS histogram_get(histogram_t* h, uint32_t index, variant_t* v);
extern int32_t histogram_nbItems(histogram_t* h);

#ifdef __cplusplus
}
#endif

#endif /* __HISTOGRAM_H__ */
