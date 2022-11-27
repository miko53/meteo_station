#ifndef __FILELOG_H__
#define __FILELOG_H__

#include "common.h"

#define FILELOG_STR_SIZE_MAX            (256)

typedef struct
{
  uint32_t msgType;
  char data[FILELOG_STR_SIZE_MAX];
} filelog_msg;

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS filelog_init(void);
extern filelog_msg* filelog_allocate_msg(void);
extern STATUS filelog_write(filelog_msg* pData);
extern void filelog_set_config(bool fileLogConfig);
extern bool filelog_get_config(void);

#ifdef __cplusplus
}
#endif


#endif /* __FILELOG_H__ */
