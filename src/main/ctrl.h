#ifndef __CTR_H__
#define __CTR_H__

#include "common.h"
#include "os.h"

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS ctrl_init(void);
extern QueueHandle_t ctrl_get_data_queue(void);


#ifdef __cplusplus
}
#endif


#endif /* __CTR_H__ */

