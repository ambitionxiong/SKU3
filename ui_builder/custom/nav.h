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
extern lv_group_t *g_updown_bbq_menu;
extern lv_group_t *g_updown_bbq_set;
extern lv_group_t *g_updown_bbq_cooking;
extern lv_group_t *g_updown_bbq_complete;

extern lv_group_t *current_group;

extern int set_temp;
extern int set_hour;
extern int set_min;
extern uint8_t preheat_on;
extern uint8_t delay_on;
extern uint8_t contain_on;
extern int cook_total_ms;
extern uint32_t cook_start_time;
extern lv_timer_t *cook_timer;

void edit_clear(void);
void edit_register(lv_obj_t *label, lv_obj_t *ind_s, lv_obj_t *ind_l,
                   int *value, int min, int max, int step, const char *fmt);

void nav_handle_key(uint8_t key);
void nav_init(void);

#endif
