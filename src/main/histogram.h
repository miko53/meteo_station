#ifndef __HISTOGRAM_H__
#define __HISTOGRAM_H__

#include "common.h"

#define LAST_VALUE  (0)

typedef union
{
  float f32;
  uint32_t i32;
} variant;

typedef struct
{
  uint32_t nbitems;
  uint32_t current_index;
  bool bFill;
  variant* datas;
} histogram_t;

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS histogram_init(histogram_t* h, uint32_t nbItems);
extern void histogram_insert(histogram_t* h, variant v);
extern STATUS histogram_get(histogram_t* h, uint32_t index, variant* v);
extern int32_t histogram_nbItems(histogram_t* h);

#ifdef __cplusplus
}
#endif

#endif /* __HISTOGRAM_H__ */
