#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum
{
  STATUS_OK = 0,
  STATUS_ERROR = -1,
} STATUS;

#ifndef MIN
#define MIN(a,b)      ((a)<(b))?(a):(b)
#endif /* MIN */

#ifndef MAX
#define MAX(a,b)      ((a)>(b))?(a):(b)
#endif /* MIN */

#define UNUSED(x)     ((void)x)

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif


#endif /* __COMMON_H__ */

