#ifndef NAV_H
#define NAV_H

#include "ui_objects.h"
#include "aic_ui.h"
#include "ui_util.h"
#include "custom_defs.h"

extern lv_group_t *g_major_menu;
extern lv_group_t *g_cookmenu;
extern lv_group_t *g_special_menu;
extern lv_group_t *g_cook_menu_tz;
extern lv_group_t *g_major_menu_tz;
extern lv_group_t *g_special_menu_tz;

extern lv_group_t *current_group;

void nav_handle_key(uint8_t key);
void nav_init(void);

#endif
