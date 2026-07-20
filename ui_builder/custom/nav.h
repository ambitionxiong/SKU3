#ifndef NAV_H
#define NAV_H

#include "ui_objects.h"
#include "aic_ui.h"
#include "ui_util.h"
#include "custom_defs.h"

// === 页面 ID 枚举 ===
typedef enum {
    PAGE_WAITMENU_24,     // 等待界面（根页，开机首页）
    PAGE_MAJOR_MENU,
    PAGE_COOKMENU,
    PAGE_SPECIAL_MENU,
    PAGE_UPDOWN_BBQ_MENU,
    PAGE_UPDOWN_BBQ_MENU_TOP,
    PAGE_UPDOWN_BBQ_MENU_LOW,
    PAGE_PREHEAT_MENU,
    PAGE_PREHEAT_COOKING,
    PAGE_PREHEAT_STOP,
    PAGE_PREHEAT_COMPLETE,
    PAGE_UPDOWN_BBQ_SET,
    PAGE_UPDOWN_BBQ_COOKING,
    PAGE_UPDOWN_BBQ_COMPLETE,
    PAGE_EXTRA_COLOR,
    PAGE_COLOR_COOKING,
    PAGE_COLOR_COOKING_COMPLETE,
    PAGE_UPDOWN_BBQ_STOP,
    PAGE_UPDOWN_BBQ_STOP_BACK,
    PAGE_UPDOWN_BBQ_SETTING,
    PAGE_COLOR_STOP,
    PAGE_COLOR_STOP_BACK,
    PAGE_TOP_BBQ_MENU,
    PAGE_TOP_BBQ_SET,
    PAGE_TOP_BBQ_COOKING,
    PAGE_TOP_BBQ_SETTING,
    PAGE_TOP_BBQ_STOP,
    PAGE_TOP_BBQ_STOP_BACK,
    PAGE_TOP_BBQ_COMPLETE,
    PAGE_BOTTOM_BBQ_MENU,
    PAGE_BOTTOM_BBQ_SET,
    PAGE_BOTTOM_BBQ_COOKING,
    PAGE_BOTTOM_BBQ_SETTING,
    PAGE_BOTTOM_BBQ_STOP,
    PAGE_BOTTOM_BBQ_STOP_BACK,
    PAGE_BOTTOM_BBQ_COMPLETE,
    PAGE_HOT_BBQ_MENU,
    PAGE_HOT_BBQ_SET,
    PAGE_HOT_BBQ_COOKING,
    PAGE_HOT_BBQ_SETTING,
    PAGE_HOT_BBQ_STOP,
    PAGE_HOT_BBQ_STOP_BACK,
    PAGE_HOT_BBQ_COMPLETE,
    PAGE_HOTWIND_BBQ_MENU,
    PAGE_HOTWIND_BBQ_SET,
    PAGE_HOTWIND_BBQ_COOKING,
    PAGE_HOTWIND_BBQ_SETTING,
    PAGE_HOTWIND_BBQ_STOP,
    PAGE_HOTWIND_BBQ_STOP_BACK,
    PAGE_HOTWIND_BBQ_COMPLETE,
    PAGE_SAVE_BBQ_MENU,
    PAGE_SAVE_BBQ_SET,
    PAGE_SAVE_BBQ_COOKING,
    PAGE_SAVE_BBQ_SETTING,
    PAGE_SAVE_BBQ_STOP,
    PAGE_SAVE_BBQ_STOP_BACK,
    PAGE_SAVE_BBQ_COMPLETE,
    PAGE_CENTRAL_BBQ_MENU,
    PAGE_CENTRAL_BBQ_SET,
    PAGE_CENTRAL_BBQ_COOKING,
    PAGE_CENTRAL_BBQ_SETTING,
    PAGE_CENTRAL_BBQ_STOP,
    PAGE_CENTRAL_BBQ_STOP_BACK,
    PAGE_CENTRAL_BBQ_COMPLETE,
    PAGE_WINDCHANGE_BBQ_MENU,
    PAGE_WINDCHANGE_BBQ_SET,
    PAGE_WINDCHANGE_BBQ_COOKING,
    PAGE_WINDCHANGE_BBQ_SETTING,
    PAGE_WINDCHANGE_BBQ_STOP,
    PAGE_WINDCHANGE_BBQ_STOP_BACK,
    PAGE_WINDCHANGE_BBQ_COMPLETE,
} page_id_t;

extern page_id_t page_stack[];
extern int depth;

extern lv_group_t *g_major_menu;
extern lv_group_t *g_cookmenu;
extern lv_group_t *g_special_menu;
extern lv_group_t *g_updown_bbq_menu;
extern lv_group_t *g_updown_bbq_menu_top;
extern lv_group_t *g_updown_bbq_menu_low;

extern lv_group_t *g_preheat_menu;
extern lv_group_t *g_preheat_cooking;
extern lv_group_t *g_preheat_stop;
extern lv_group_t *g_preheat_complete;
extern lv_group_t *g_updown_bbq_set;
extern lv_group_t *g_updown_bbq_cooking;
extern lv_group_t *g_updown_bbq_complete;
extern lv_group_t *g_updown_bbq_stop;
extern lv_group_t *g_updown_bbq_stop_back;
extern lv_group_t *g_updown_bbq_setting;

extern lv_group_t *g_top_bbq_menu;
extern lv_group_t *g_top_bbq_set;
extern lv_group_t *g_top_bbq_cooking;
extern lv_group_t *g_top_bbq_setting;
extern lv_group_t *g_top_bbq_stop;
extern lv_group_t *g_top_bbq_stop_back;
extern lv_group_t *g_top_bbq_complete;

extern lv_group_t *g_bottom_bbq_menu;
extern lv_group_t *g_bottom_bbq_set;
extern lv_group_t *g_bottom_bbq_cooking;
extern lv_group_t *g_bottom_bbq_setting;
extern lv_group_t *g_bottom_bbq_stop;
extern lv_group_t *g_bottom_bbq_stop_back;
extern lv_group_t *g_bottom_bbq_complete;

extern lv_group_t *g_hot_bbq_menu;
extern lv_group_t *g_hot_bbq_set;
extern lv_group_t *g_hot_bbq_cooking;
extern lv_group_t *g_hot_bbq_setting;
extern lv_group_t *g_hot_bbq_stop;
extern lv_group_t *g_hot_bbq_stop_back;
extern lv_group_t *g_hot_bbq_complete;

extern lv_group_t *g_hotwind_bbq_menu;
extern lv_group_t *g_hotwind_bbq_set;
extern lv_group_t *g_hotwind_bbq_cooking;
extern lv_group_t *g_hotwind_bbq_setting;
extern lv_group_t *g_hotwind_bbq_stop;
extern lv_group_t *g_hotwind_bbq_stop_back;
extern lv_group_t *g_hotwind_bbq_complete;

extern lv_group_t *g_save_bbq_menu;
extern lv_group_t *g_save_bbq_set;
extern lv_group_t *g_save_bbq_cooking;
extern lv_group_t *g_save_bbq_setting;
extern lv_group_t *g_save_bbq_stop;
extern lv_group_t *g_save_bbq_stop_back;
extern lv_group_t *g_save_bbq_complete;

extern lv_group_t *g_central_bbq_menu;
extern lv_group_t *g_central_bbq_set;
extern lv_group_t *g_central_bbq_cooking;
extern lv_group_t *g_central_bbq_setting;
extern lv_group_t *g_central_bbq_stop;
extern lv_group_t *g_central_bbq_stop_back;
extern lv_group_t *g_central_bbq_complete;

extern lv_group_t *g_windchange_bbq_menu;

extern lv_group_t *g_windchange_bbq_menu;
extern lv_group_t *g_windchange_bbq_set;
extern lv_group_t *g_windchange_bbq_cooking;
extern lv_group_t *g_windchange_bbq_setting;
extern lv_group_t *g_windchange_bbq_stop;
extern lv_group_t *g_windchange_bbq_stop_back;
extern lv_group_t *g_windchange_bbq_complete;

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

uint16_t get_cavity_temp(void);
void jump_to_cookmenu(void);
void jump_to_preheat_menu(void);
void jump_to_preheat_cooking(void);
void jump_to_preheat_stop(void);
void jump_to_preheat_stop_back(void);
void preheat_resume_cooking(void);
void jump_to_preheat_complete(void);
#ifdef LV_USE_AIC_SIMULATOR
extern uint16_t g_sim_cavity_temp;
#endif
void edit_clear(void);
void edit_register(lv_obj_t *label, lv_obj_t *ind_s, lv_obj_t *ind_l,
                   int *value, int min, int max, int step, const char *fmt);

void nav_handle_key(uint8_t key);
void nav_key1_long_press(void);
uint8_t nav_key1_hold_check(void);
void nav_init(void);
void page_push(page_id_t id);
void page_pop(void);

lv_group_t *group_create_for_page(lv_obj_t **btns, int count);
void set_time_label(lv_obj_t *label, int remaining_ms);
void set_bar_progress(lv_obj_t *bar, int64_t elapsed_ms, int total_ms);
void apply_toggle_state(lv_obj_t *btn_off, lv_obj_t *btn_on, int on);
void clear_focus_states(lv_obj_t **btns, int count);
void update_top_bbq_dir_icon(top_bbq_setting_t *set);
void update_bottom_bbq_dir_icon(bottom_bbq_setting_t *set);
void update_hot_bbq_dir_icon(hot_bbq_setting_t *set);
void update_hotwind_bbq_dir_icon(hotwind_bbq_setting_t *set);
void update_save_bbq_dir_icon(save_bbq_setting_t *set);
void update_central_bbq_dir_icon(central_bbq_setting_t *set);
void update_windchange_bbq_dir_icon(windchange_bbq_setting_t *set);

void jump_to_hot_bbq_menu(void);
void jump_to_hot_bbq_set(void);
void jump_to_hot_bbq_stop(void);
void jump_to_hot_bbq_stop_back(void);
void jump_to_hot_bbq_setting(void);
void hot_bbq_resume_cooking(void);
void jump_to_hot_bbq_complete(void);

void hot_bbq_rebuild_menu(page_id_t child);
void hot_bbq_rebuild_set(page_id_t child);
void hot_bbq_rebuild_cooking(page_id_t child);
void hot_bbq_rebuild_setting(void);
void hot_bbq_rebuild_stop(void);
void hot_bbq_rebuild_stop_back(void);
void hot_bbq_rebuild_complete(void);

void jump_to_hotwind_bbq_menu(void);
void jump_to_hotwind_bbq_set(void);
void jump_to_hotwind_bbq_cooking(void);
void jump_to_hotwind_bbq_setting(void);
void jump_to_hotwind_bbq_stop(void);
void jump_to_hotwind_bbq_stop_back(void);
void jump_to_hotwind_bbq_complete(void);
void hotwind_bbq_rebuild_menu(page_id_t child);
void hotwind_bbq_rebuild_set(page_id_t child);
void hotwind_bbq_rebuild_cooking(page_id_t child);
void hotwind_bbq_rebuild_setting(void);
void hotwind_bbq_rebuild_stop(void);
void hotwind_bbq_rebuild_stop_back(void);
void hotwind_bbq_rebuild_complete(void);

void jump_to_save_bbq_menu(void);
void jump_to_save_bbq_set(void);
void jump_to_save_bbq_cooking(void);
void jump_to_save_bbq_setting(void);
void jump_to_save_bbq_stop(void);
void jump_to_save_bbq_stop_back(void);
void jump_to_save_bbq_complete(void);
void save_bbq_rebuild_menu(page_id_t child);
void save_bbq_rebuild_set(page_id_t child);
void save_bbq_rebuild_cooking(page_id_t child);
void save_bbq_rebuild_setting(void);
void save_bbq_rebuild_stop(void);
void save_bbq_rebuild_stop_back(void);
void save_bbq_rebuild_complete(void);

void jump_to_central_bbq_menu(void);
void jump_to_central_bbq_set(void);
void jump_to_central_bbq_cooking(void);
void jump_to_central_bbq_setting(void);
void jump_to_central_bbq_stop(void);
void jump_to_central_bbq_stop_back(void);
void jump_to_central_bbq_complete(void);
void central_bbq_rebuild_menu(page_id_t child);
void central_bbq_rebuild_set(page_id_t child);
void central_bbq_rebuild_cooking(page_id_t child);
void central_bbq_rebuild_setting(void);
void central_bbq_rebuild_stop(void);
void central_bbq_rebuild_stop_back(void);
void central_bbq_rebuild_complete(void);

void jump_to_windchange_bbq_menu(void);
void jump_to_windchange_bbq_set(void);
void jump_to_windchange_bbq_cooking(void);
void jump_to_windchange_bbq_setting(void);
void jump_to_windchange_bbq_stop(void);
void jump_to_windchange_bbq_stop_back(void);
void jump_to_windchange_bbq_complete(void);
void windchange_bbq_rebuild_menu(page_id_t child);
void windchange_bbq_rebuild_set(page_id_t child);
void windchange_bbq_rebuild_cooking(page_id_t child);
void windchange_bbq_rebuild_setting(void);
void windchange_bbq_rebuild_stop(void);
void windchange_bbq_rebuild_stop_back(void);
void windchange_bbq_rebuild_complete(void);

void on_edit_focus(lv_event_t *e);
void validate_constraints(void);
void groups_create(void);
void bind_events(void);
void anim_bar_set_value(void *obj, int32_t v);
void cooking_timer_cb(lv_timer_t *timer);

void jump_to_top_bbq_menu(void);
void jump_to_top_bbq_set(void);
void jump_to_top_bbq_stop(void);
void jump_to_top_bbq_stop_back(void);
void jump_to_top_bbq_setting(void);
void top_bbq_resume_cooking(void);
void jump_to_top_bbq_complete(void);

void top_bbq_rebuild_menu(page_id_t child);
void top_bbq_rebuild_set(page_id_t child);
void top_bbq_rebuild_cooking(page_id_t child);
void top_bbq_rebuild_setting(void);
void top_bbq_rebuild_stop(void);
void top_bbq_rebuild_stop_back(void);
void top_bbq_rebuild_complete(void);

void jump_to_bottom_bbq_menu(void);
void jump_to_bottom_bbq_set(void);
void jump_to_bottom_bbq_stop(void);
void jump_to_bottom_bbq_stop_back(void);
void jump_to_bottom_bbq_setting(void);
void bottom_bbq_resume_cooking(void);
void jump_to_bottom_bbq_complete(void);

void bottom_bbq_rebuild_menu(page_id_t child);
void bottom_bbq_rebuild_set(page_id_t child);
void bottom_bbq_rebuild_cooking(page_id_t child);
void bottom_bbq_rebuild_setting(void);
void bottom_bbq_rebuild_stop(void);
void bottom_bbq_rebuild_stop_back(void);
void bottom_bbq_rebuild_complete(void);

#endif
