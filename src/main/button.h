#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "common.h"

typedef enum
{
  BUTTON_CMD,
  BUTTON_MINUS,
  BUTTON_PLUS,
  BUTTON_NB_MAX
} button_id;

typedef enum
{
  KEY_PRESSED,
  KEY_RELEASED,
  KEY_LONG_PRESS
} key_event;

typedef void (*button_cb_handler)(key_event evt);

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS button_init(void);
extern STATUS button_install_handler(button_id buttonId, key_event event, button_cb_handler button_callback);

#ifdef __cplusplus
}
#endif

#endif /* __BUTTON_H__ */
