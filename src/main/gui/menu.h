#ifndef __MENU_H__
#define __MENU_H__

#include "common.h"
#include "screen.h"

#ifdef __cplusplus
extern "C" {
#endif

extern STATUS menu_init(void);
extern void menu_display_splash_screen(screen_t* screen);
extern void menu_display_sdcard_screen(screen_t* pScreen);
extern void menu_display_date_time_screen(screen_t* pScreen);

#ifdef __cplusplus
}
#endif

#endif /* __MENU_H__ */

