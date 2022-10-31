#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#include <stdint.h>

typedef int32_t atomic_t;
typedef atomic_t atomic_data_t;

static inline atomic_data_t atomic_add(atomic_t* pAtomic, atomic_data_t value)
{
  return __atomic_fetch_add(pAtomic, value, __ATOMIC_SEQ_CST);
}

static inline atomic_data_t atomic_get(const atomic_t* pAtomic)
{
  return __atomic_load_n(pAtomic, __ATOMIC_SEQ_CST);
}

static inline void atomic_set(atomic_t* pAtomic, atomic_data_t value)
{
  __atomic_store_n(pAtomic, value, __ATOMIC_SEQ_CST);
}

#endif /* __ATOMIC_H__ */
