#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "common.h"

struct screen_def_t
{
  char* line_1;
  char* line_2;
  void (*display)(struct screen_def_t* pScreen);
  void (*on_cmd)(struct screen_def_t* pScreen);
  void (*on_cmd_long_press)(struct screen_def_t* pScreen);
  void (*on_plus)(struct screen_def_t* pScreen);
  void (*on_minus)(struct screen_def_t* pScreen);
  void (*on_enter)(struct screen_def_t* pScreen);
  void (*on_exit)(struct screen_def_t* pScreen);
};

typedef struct screen_def_t screen_t;

extern STATUS screen_init(void);
extern void screen_generic_display(screen_t* screen);
extern void screen_change_to(screen_t* screen);
extern void screen_refresh(void);

#endif /* __SCREEN_H__ */
