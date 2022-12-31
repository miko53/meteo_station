#ifndef __BLE_DATE_SRV_H__
#define __BLE_DATE_SRV_H__

#include "common.h"
#include <time.h>

#define BLE_DATE_FRAME_SIZE       (10)

#ifdef __cplusplus
extern "C" {
#endif

extern void ble_date_build_frame(uint8_t frame[BLE_DATE_FRAME_SIZE]);
extern STATUS ble_date_decode(struct tm* pDate, uint8_t frame[BLE_DATE_FRAME_SIZE]);

#ifdef __cplusplus
}
#endif
#endif /* __BLE_DATE_SRV_H__ */
