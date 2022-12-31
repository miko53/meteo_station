#ifndef __NVSTORAGE_H__
#define __NVSTORAGE_H__

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS nvstorage_init(void);
extern bool nvstorage_get_sdcard_log_state(void);
extern STATUS nvstorage_set_sdcard_log_state(bool bState);
extern bool nvstorage_get_ble_state(void);
extern STATUS nvstorage_set_ble_state(bool bState);


#ifdef __cplusplus
}
#endif
#endif /* __NVSTORAGE_H__ */
