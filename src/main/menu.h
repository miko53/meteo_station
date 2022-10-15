#ifndef __MENU_H__
#define __MENU_H__

#include "common.h"
#include "screen.h"

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS menu_init(void);
extern void menu_display_config_screen(screen_t* screen);


#ifdef __cplusplus
}
#endif

#endif /* __MENU_H__ */

