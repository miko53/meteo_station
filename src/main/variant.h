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

static inline bool variant_is_higher(variant_t* v1, variant_t* v2, variant_type type)
{
  bool b = false;
  switch (type)
  {
    case INTEGER_32:
      b = v1->i32 > v2->i32;
      break;

    case FLOAT:
      b = v1->f32 > v2->f32;
      break;

    default:
      break;
  }
  return b;
}

static inline bool variant_is_lower(variant_t* v1, variant_t* v2, variant_type type)
{
  bool b = false;
  switch (type)
  {
    case INTEGER_32:
      b = v1->i32 < v2->i32;
      break;

    case FLOAT:
      b = v1->f32 < v2->f32;
      break;

    default:
      break;
  }
  return b;
}

static inline void variant_add(variant_t* r, variant_t* v1, variant_t* v2, variant_type type)
{
  switch (type)
  {
    case INTEGER_32:
      r->i32 = v1->i32 + v2->i32;
      break;

    case FLOAT:
      r->f32 = v1->f32 + v2->f32;
      break;

    default:
      break;
  }
}
#ifdef __cplusplus
}
#endif

#endif /* __VARIANT_H__ */
