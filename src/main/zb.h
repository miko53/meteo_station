#ifndef ZB_H
#define	ZB_H

#include "common.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define ZB_START_DELIMITER    (0x7E)
#define ZB_HEADER_SIZE          (3)

typedef enum
{
  ZIGBEE_API_AT_CMD           = 0x08,
  ZIGBEE_API_TRANSMIT_REQUEST = 0x10,
  ZIGBEE_AT_COMMAND_RESPONSE  = 0x88,
  ZIGBEE_MODEM_STATUS         = 0x8A,
  ZIGBEE_TRANSMIT_STATUS      = 0x8B,
  ZIGBEE_RECEIVE_PACKET       = 0x90,
} zigbee_frameType;

typedef struct
{
  uint8_t type;
  uint8_t status;
  uint8_t AT[2];
  uint8_t* data;
  uint8_t size;
  uint8_t frameID;
} zigbee_decodedFrame;


extern bool zb_decodage(uint8_t* frame, uint8_t frameSize, zigbee_decodedFrame* decodedFrame);
extern uint8_t zb_doChecksum(uint8_t* frame, uint8_t size);

#ifdef	__cplusplus
}
#endif

#endif	/* ZB_H */

