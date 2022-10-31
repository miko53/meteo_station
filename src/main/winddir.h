#ifndef __WINDDIR_H__
#define __WINDDIR_H__

#include "common.h"

typedef enum
{
  N,
  NE,
  E,
  SE,
  S,
  SO,
  O,
  NO,
  INVALID
} winddir_direction_t;

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS winddir_init(void);
extern char* winddir_direction(winddir_direction_t dir);
extern void print_direction(winddir_direction_t dir);


#ifdef __cplusplus
}
#endif


#endif /* __WINDDIR_H__ */


