#ifndef NAV_H
#define NAV_H

#include "ui_objects.h"
#include "aic_ui.h"
#include "ui_util.h"
#include "custom_defs.h"

extern lv_group_t *g_major_menu;
extern lv_group_t *g_cookmenu;
extern lv_group_t *g_special_menu;
extern lv_group_t *g_updown_bbq_menu;
extern lv_group_t *g_updown_bbq_set;
extern lv_group_t *g_updown_bbq_cooking;
extern lv_group_t *g_updown_bbq_complete;
extern lv_group_t *g_updown_bbq_stop;
extern lv_group_t *g_updown_bbq_stop_back;
extern lv_group_t *g_updown_bbq_setting;

extern lv_group_t *current_group;

extern int set_temp;
extern int set_temp_up;
extern int set_temp_down;
extern int set_hour;
extern int set_min;
extern uint8_t preheat_on;
extern uint8_t delay_on;
extern uint8_t contain_on;
extern int cook_total_ms;
extern uint32_t cook_start_time;
extern uint8_t cook_is_color;
extern lv_timer_t *cook_timer;
extern uint32_t cook_elapsed_saved;
extern int cook_bar_saved;

extern lv_group_t *g_updown_bbq_complete;
extern lv_group_t *g_extra_color;
extern lv_group_t *g_color_cookoing;
extern lv_group_t *g_color_stop;
extern lv_group_t *g_color_stop_back;

void edit_clear(void);
void edit_register(lv_obj_t *label, lv_obj_t *ind_s, lv_obj_t *ind_l,
                   int *value, int min, int max, int step, const char *fmt);

void nav_handle_key(uint8_t key);
void nav_init(void);

#endif
