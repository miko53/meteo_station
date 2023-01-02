#ifndef __ZIGBEE_H__
#define __ZIGBEE_H__

#include "common.h"
#include "zb_data.h"
#include "data_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS zigbee_init(void);
extern STATUS zigbee_send_sensor_data(data_type_t indexSensor, variant_t* pData);
extern STATUS zigbee_send_data(zb_payload_frame* pMsg);
extern zb_payload_frame* zb_allocate_msg(void);
extern void zb_free_msg(zb_payload_frame* msg);

#ifdef __cplusplus
}
#endif
#endif /* __ZIGBEE_H__ */
