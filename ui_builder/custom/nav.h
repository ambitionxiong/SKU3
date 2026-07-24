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
    PAGE_COOK4_MENU,
    PAGE_COOKIE_MENU,
    PAGE_COOKIE_SET,
    PAGE_COOKIE_COOKING,
    PAGE_COOKIE_SETTING,
    PAGE_COOKIE_STOP,
    PAGE_COOKIE_STOP_BACK,
    PAGE_COOKIE_COMPLETE,
    PAGE_WEST_MENU,
    PAGE_WEST_SET,
    PAGE_WEST_COOKING,
    PAGE_WEST_SETTING,
    PAGE_WEST_STOP,
    PAGE_WEST_STOP_BACK,
    PAGE_WEST_COMPLETE,
    PAGE_PIZZA_MENU,
    PAGE_PIZZA_SET,
    PAGE_PIZZA_COOKING,
    PAGE_PIZZA_SETTING,
    PAGE_PIZZA_STOP,
    PAGE_PIZZA_STOP_BACK,
    PAGE_PIZZA_COMPLETE,
    PAGE_MENU_COOK_MENU,
    PAGE_MENU_COOK_SET,
    PAGE_MENU_COOK_COOKING,
    PAGE_MENU_COOK_SETTING,
    PAGE_MENU_COOK_STOP,
    PAGE_MENU_COOK_STOP_BACK,
    PAGE_MENU_COOK_COMPLETE,
    PAGE_AIR_MENU,
    PAGE_AIR_SET,
    PAGE_AIR_COOKING,
    PAGE_AIR_SETTING,
    PAGE_AIR_STOP,
    PAGE_AIR_STOP_BACK,
    PAGE_AIR_COMPLETE,
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
    PAGE_PIZZA_2_MENU,
    PAGE_PIZZA_2_SET,
    PAGE_PIZZA_2_COOKING,
    PAGE_PIZZA_2_SETTING,
    PAGE_PIZZA_2_STOP,
    PAGE_PIZZA_2_STOP_BACK,
    PAGE_PIZZA_2_COMPLETE,
    PAGE_SLOWCOOK_MENU,
    PAGE_SLOWCOOK_SET,
    PAGE_SLOWCOOK_COOKING,
    PAGE_SLOWCOOK_SETTING,
    PAGE_SLOWCOOK_STOP,
    PAGE_SLOWCOOK_STOP_BACK,
    PAGE_SLOWCOOK_COMPLETE,
    PAGE_UNFROZEN_MENU,
    PAGE_UNFROZEN_SET,
    PAGE_UNFROZEN_COOKING,
    PAGE_UNFROZEN_SETTING,
    PAGE_UNFROZEN_STOP,
    PAGE_UNFROZEN_STOP_BACK,
    PAGE_UNFROZEN_COMPLETE,
    PAGE_RISING_MENU,
    PAGE_RISING_SET,
    PAGE_RISING_COOKING,
    PAGE_RISING_SETTING,
    PAGE_RISING_STOP,
    PAGE_RISING_STOP_BACK,
    PAGE_RISING_COMPLETE,
    PAGE_CORN_MENU,
    PAGE_CORN_SET,
    PAGE_CORN_COOKING,
    PAGE_CORN_SETTING,
    PAGE_CORN_STOP,
    PAGE_CORN_STOP_BACK,
    PAGE_CORN_COMPLETE,
    PAGE_HEATCONTAIN_MENU,
    PAGE_HEATCONTAIN_SET,
    PAGE_HEATCONTAIN_COOKING,
    PAGE_HEATCONTAIN_SETTING,
    PAGE_HEATCONTAIN_STOP,
    PAGE_HEATCONTAIN_STOP_BACK,
    PAGE_HEATCONTAIN_COMPLETE,
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
extern lv_group_t *g_cook4_menu;

extern lv_group_t *g_cookie_menu;

extern lv_group_t *g_cookie_set;

extern lv_group_t *g_cookie_cooking;

extern lv_group_t *g_cookie_setting;

extern lv_group_t *g_cookie_stop;

extern lv_group_t *g_cookie_stop_back;

extern lv_group_t *g_cookie_complete;

extern lv_group_t *g_west_menu;

extern lv_group_t *g_west_set;

extern lv_group_t *g_west_cooking;

extern lv_group_t *g_west_setting;

extern lv_group_t *g_west_stop;

extern lv_group_t *g_west_stop_back;

extern lv_group_t *g_west_complete;

extern lv_group_t *g_pizza_menu;

extern lv_group_t *g_pizza_set;

extern lv_group_t *g_pizza_cooking;

extern lv_group_t *g_pizza_setting;

extern lv_group_t *g_pizza_stop;

extern lv_group_t *g_pizza_stop_back;

extern lv_group_t *g_pizza_complete;

extern lv_group_t *g_menu_cook_menu;

extern lv_group_t *g_menu_cook_set;

extern lv_group_t *g_menu_cook_cooking;

extern lv_group_t *g_menu_cook_setting;

extern lv_group_t *g_menu_cook_stop;

extern lv_group_t *g_menu_cook_stop_back;

extern lv_group_t *g_menu_cook_complete;
extern lv_group_t *g_air_menu;
extern lv_group_t *g_air_set;
extern lv_group_t *g_air_cooking;
extern lv_group_t *g_air_setting;
extern lv_group_t *g_air_stop;
extern lv_group_t *g_air_stop_back;
extern lv_group_t *g_air_complete;
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
void jump_to_cook4_menu(void);
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
void update_cookie_dir_icon(cookie_setting_t *set);
void update_west_dir_icon(west_setting_t *set);
void update_pizza_dir_icon(pizza_setting_t *set);
void update_menu_dir_icon(menu_setting_t *set);

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

void jump_to_cookie_menu(void);
void jump_to_cookie_set(void);
void jump_to_cookie_cooking(void);
void jump_to_cookie_setting(void);
void jump_to_cookie_stop(void);
void jump_to_cookie_stop_back(void);
void jump_to_cookie_complete(void);
void cookie_rebuild_menu(page_id_t child);
void cookie_rebuild_set(page_id_t child);
void cookie_rebuild_cooking(page_id_t child);
void cookie_rebuild_setting(void);
void cookie_rebuild_stop(void);
void cookie_rebuild_stop_back(void);
void cookie_rebuild_complete(void);
void cookie_resume_cooking(void);

void jump_to_west_menu(void);
void jump_to_west_set(void);
void jump_to_west_cooking(void);
void jump_to_west_setting(void);
void jump_to_west_stop(void);
void jump_to_west_stop_back(void);
void jump_to_west_complete(void);
void west_rebuild_menu(page_id_t child);
void west_rebuild_set(page_id_t child);
void west_rebuild_cooking(page_id_t child);
void west_rebuild_setting(void);
void west_rebuild_stop(void);
void west_rebuild_stop_back(void);
void west_rebuild_complete(void);
void west_resume_cooking(void);

void jump_to_pizza_menu(void);
void jump_to_pizza_set(void);
void jump_to_pizza_cooking(void);
void jump_to_pizza_setting(void);
void jump_to_pizza_stop(void);
void jump_to_pizza_stop_back(void);
void jump_to_pizza_complete(void);
void pizza_rebuild_menu(page_id_t child);
void pizza_rebuild_set(page_id_t child);
void pizza_rebuild_cooking(page_id_t child);
void pizza_rebuild_setting(void);
void pizza_rebuild_stop(void);
void pizza_rebuild_stop_back(void);
void pizza_rebuild_complete(void);
void pizza_resume_cooking(void);

void jump_to_menu_menu(void);
void jump_to_menu_set(void);
void jump_to_menu_cooking(void);
void jump_to_menu_setting(void);
void jump_to_menu_stop(void);
void jump_to_menu_stop_back(void);
void jump_to_menu_complete(void);
void menu_rebuild_menu(page_id_t child);
void menu_rebuild_set(page_id_t child);
void menu_rebuild_cooking(page_id_t child);
void menu_rebuild_setting(void);
void menu_rebuild_stop(void);
void menu_rebuild_stop_back(void);
void menu_rebuild_complete(void);
void menu_resume_cooking(void);

void jump_to_air_menu(void);
void jump_to_air_set(void);
void jump_to_air_cooking(void);
void jump_to_air_setting(void);
void jump_to_air_stop(void);
void jump_to_air_stop_back(void);
void jump_to_air_complete(void);
void air_rebuild_menu(page_id_t child);
void air_rebuild_set(page_id_t child);
void air_rebuild_cooking(page_id_t child);
void air_rebuild_setting(void);
void air_rebuild_stop(void);
void air_rebuild_stop_back(void);
void air_rebuild_complete(void);
void air_resume_cooking(void);
void air_back_to_stop(void);
void air_back_to_stop_back(void);

extern lv_group_t *g_pizza_2_menu;
extern lv_group_t *g_pizza_2_set;
extern lv_group_t *g_pizza_2_cooking;
extern lv_group_t *g_pizza_2_setting;
extern lv_group_t *g_pizza_2_stop;
extern lv_group_t *g_pizza_2_stop_back;
extern lv_group_t *g_pizza_2_complete;

void jump_to_pizza_2_menu(void);
void jump_to_pizza_2_set(void);
void jump_to_pizza_2_cooking(void);
void jump_to_pizza_2_setting(void);
void jump_to_pizza_2_stop(void);
void jump_to_pizza_2_stop_back(void);
void jump_to_pizza_2_complete(void);
void pizza_2_rebuild_menu(page_id_t child);
void pizza_2_rebuild_set(page_id_t child);
void pizza_2_rebuild_cooking(page_id_t child);
void pizza_2_rebuild_setting(void);
void pizza_2_rebuild_stop(void);
void pizza_2_rebuild_stop_back(void);
void pizza_2_rebuild_complete(void);
void pizza_2_resume_cooking(void);

void cook4menu_rebuild(page_id_t child);

extern lv_group_t *g_slowcook_menu;
extern lv_group_t *g_slowcook_set;
extern lv_group_t *g_slowcook_cooking;
extern lv_group_t *g_slowcook_setting;
extern lv_group_t *g_slowcook_stop;
extern lv_group_t *g_slowcook_stop_back;
extern lv_group_t *g_slowcook_complete;

extern lv_group_t *g_unfrozen_menu;
extern lv_group_t *g_unfrozen_set;
extern lv_group_t *g_unfrozen_cooking;
extern lv_group_t *g_unfrozen_setting;
extern lv_group_t *g_unfrozen_stop;
extern lv_group_t *g_unfrozen_stop_back;
extern lv_group_t *g_unfrozen_complete;

extern lv_group_t *g_rising_menu;
extern lv_group_t *g_rising_set;
extern lv_group_t *g_rising_cooking;
extern lv_group_t *g_rising_setting;
extern lv_group_t *g_rising_stop;
extern lv_group_t *g_rising_stop_back;
extern lv_group_t *g_rising_complete;

extern lv_group_t *g_corn_menu;
extern lv_group_t *g_corn_set;
extern lv_group_t *g_corn_cooking;
extern lv_group_t *g_corn_setting;
extern lv_group_t *g_corn_stop;
extern lv_group_t *g_corn_stop_back;
extern lv_group_t *g_corn_complete;

extern lv_group_t *g_heatcontain_menu;
extern lv_group_t *g_heatcontain_set;
extern lv_group_t *g_heatcontain_cooking;
extern lv_group_t *g_heatcontain_setting;
extern lv_group_t *g_heatcontain_stop;
extern lv_group_t *g_heatcontain_stop_back;
extern lv_group_t *g_heatcontain_complete;

void jump_to_slowcook_menu(void);
void jump_to_slowcook_set(void);
void jump_to_slowcook_cooking(void);
void jump_to_slowcook_setting(void);
void jump_to_slowcook_stop(void);
void jump_to_slowcook_stop_back(void);
void jump_to_slowcook_complete(void);
void slowcook_rebuild_menu(page_id_t child);
void slowcook_rebuild_set(page_id_t child);
void slowcook_rebuild_cooking(page_id_t child);
void slowcook_rebuild_setting(void);
void slowcook_rebuild_stop(void);
void slowcook_rebuild_stop_back(void);
void slowcook_rebuild_complete(void);
void slowcook_resume_cooking(void);

void jump_to_unfrozen_menu(void);
void jump_to_unfrozen_set(void);
void jump_to_unfrozen_cooking(void);
void jump_to_unfrozen_setting(void);
void jump_to_unfrozen_stop(void);
void jump_to_unfrozen_stop_back(void);
void jump_to_unfrozen_complete(void);
void unfrozen_rebuild_menu(page_id_t child);
void unfrozen_rebuild_set(page_id_t child);
void unfrozen_rebuild_cooking(page_id_t child);
void unfrozen_rebuild_setting(void);
void unfrozen_rebuild_stop(void);
void unfrozen_rebuild_stop_back(void);
void unfrozen_rebuild_complete(void);
void unfrozen_resume_cooking(void);

void jump_to_rising_menu(void);
void jump_to_rising_set(void);
void jump_to_rising_cooking(void);
void jump_to_rising_setting(void);
void jump_to_rising_stop(void);
void jump_to_rising_stop_back(void);
void jump_to_rising_complete(void);
void rising_rebuild_menu(page_id_t child);
void rising_rebuild_set(page_id_t child);
void rising_rebuild_cooking(page_id_t child);
void rising_rebuild_setting(void);
void rising_rebuild_stop(void);
void rising_rebuild_stop_back(void);
void rising_rebuild_complete(void);
void rising_resume_cooking(void);

void jump_to_corn_menu(void);
void jump_to_corn_set(void);
void jump_to_corn_cooking(void);
void jump_to_corn_setting(void);
void jump_to_corn_stop(void);
void jump_to_corn_stop_back(void);
void jump_to_corn_complete(void);
void corn_rebuild_menu(page_id_t child);
void corn_rebuild_set(page_id_t child);
void corn_rebuild_cooking(page_id_t child);
void corn_rebuild_setting(void);
void corn_rebuild_stop(void);
void corn_rebuild_stop_back(void);
void corn_rebuild_complete(void);
void corn_resume_cooking(void);

void jump_to_heatcontain_menu(void);
void jump_to_heatcontain_set(void);
void jump_to_heatcontain_cooking(void);
void jump_to_heatcontain_setting(void);
void jump_to_heatcontain_stop(void);
void jump_to_heatcontain_stop_back(void);
void jump_to_heatcontain_complete(void);
void heatcontain_rebuild_menu(page_id_t child);
void heatcontain_rebuild_set(page_id_t child);
void heatcontain_rebuild_cooking(page_id_t child);
void heatcontain_rebuild_setting(void);
void heatcontain_rebuild_stop(void);
void heatcontain_rebuild_stop_back(void);
void heatcontain_rebuild_complete(void);
void heatcontain_resume_cooking(void);

#endif
