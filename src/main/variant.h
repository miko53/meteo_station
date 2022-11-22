#ifndef __VARIANT_H__
#define __VARIANT_H__

#include "common.h"

typedef enum
{
  INTEGER_32,
  FLOAT,
} variant_type;

typedef struct
{
  variant_type type;
  union
  {
    float f32;
    uint32_t i32;
  };
} variant_t;

#ifdef __cplusplus
extern "C" {
#endif

static inline void variant_u32(variant_t* v, uint32_t value)
{
  v->type = INTEGER_32;
  v->i32 = value;
}

static inline void variant_f32(variant_t* v, float value)
{
  v->type = FLOAT;
  v->f32 = value;
}

static inline float variant_to_f32(variant_t* v)
{
  if (v->type == FLOAT)
    return v->f32;

  return (float) v->i32;
}

static inline uint32_t variant_to_u32(variant_t* v)
{
  if (v->type == INTEGER_32)
    return v->i32;

  return (uint32_t) v->f32;
}

#ifdef __cplusplus
}
#endif

#endif /* __VARIANT_H__ */
