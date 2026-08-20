#ifndef NAV_INTERNAL_H
#define NAV_INTERNAL_H
/*
 * nav_internal.h - 导航控制器内部共享头
 *
 * 原 nav.c(9377 行)拆分后的跨文件共享符号声明。
 * 供 nav_core/nav_pop/nav_jump/nav_key/nav_keyio/nav_events/
 *    nav_cooktimer/nav_stop/nav_settings/nav_system 各文件引用。
 *
 * 内容：
 *   1. 按键状态机类型(key_state_t)与常量
 *   2. 跨文件共享变量 extern(焦点组、状态标志、setting 保存值等)
 *   3. 原 nav.c 内部函数声明(拆分后去 static 形成跨文件接口)
 *
 * 注意：此头仅 custom 内部使用，勿将对外接口(页面 ID、模式枚举、
 *   rtc 接口等)放于此——那些在 nav.h / protocol.h 中。
 */

#include "nav.h"
#include "protocol.h"
#include "i18n.h"

// key_state 状态机(原 nav.c 内部, nav_key.c 定义 / nav_keyio.c 使用)
typedef enum { KEY_IDLE, KEY_PRESSED } key_state_t;
#define ENC_REPEAT_MS 50
#define MAX_EDIT_FIELDS 8

// ===== 跨文件共享变量 extern =====
extern edit_field_t edit_fields[MAX_EDIT_FIELDS];
extern int edit_count;
extern int updown_setting_saved_temp_up, updown_setting_saved_temp_down;
extern int updown_setting_saved_hour, updown_setting_saved_min;
extern key_state_t key_state;
extern uint8_t active_key;
extern uint32_t active_key_time;
extern int updown_menu_top_saved, updown_menu_low_saved;
extern int g_color_from_probe;

// ===== 各模式 setting 页保存值(定义在各 nav_xxx.c) =====
extern int top_setting_saved_temp, top_setting_saved_hour, top_setting_saved_min;
extern int bottom_bbq_setting_saved_temp, bottom_bbq_setting_saved_hour, bottom_bbq_setting_saved_min;
extern int hot_bbq_setting_saved_temp, hot_bbq_setting_saved_hour, hot_bbq_setting_saved_min;
extern int hotwind_bbq_setting_saved_temp, hotwind_bbq_setting_saved_hour, hotwind_bbq_setting_saved_min;
extern int save_bbq_setting_saved_temp, save_bbq_setting_saved_hour, save_bbq_setting_saved_min;
extern int central_bbq_setting_saved_temp, central_bbq_setting_saved_hour, central_bbq_setting_saved_min;
extern int windchange_bbq_setting_saved_temp, windchange_bbq_setting_saved_hour, windchange_bbq_setting_saved_min;
extern int cookie_setting_saved_temp, cookie_setting_saved_hour, cookie_setting_saved_min;
extern int west_setting_saved_temp, west_setting_saved_hour, west_setting_saved_min;
extern int pizza_setting_saved_temp, pizza_setting_saved_hour, pizza_setting_saved_min;
extern int menu_setting_saved_temp, menu_setting_saved_hour, menu_setting_saved_min;
extern int air_setting_saved_temp, air_setting_saved_hour, air_setting_saved_min;
extern int pizza_2_setting_saved_temp, pizza_2_setting_saved_hour, pizza_2_setting_saved_min;
extern int slowcook_setting_saved_temp, slowcook_setting_saved_hour, slowcook_setting_saved_min;
extern int unfrozen_setting_saved_temp, unfrozen_setting_saved_hour, unfrozen_setting_saved_min;
extern int rising_setting_saved_temp, rising_setting_saved_hour, rising_setting_saved_min;
extern int corn_setting_saved_temp, corn_setting_saved_hour, corn_setting_saved_min;
extern int heatcontain_setting_saved_temp, heatcontain_setting_saved_hour, heatcontain_setting_saved_min;
extern int lasagna_setting_saved_hour, lasagna_setting_saved_min;
extern int strudel_setting_saved_hour, strudel_setting_saved_min;
extern int bread_setting_saved_hour, bread_setting_saved_min;
extern int pizza3_setting_saved_hour, pizza3_setting_saved_min;
extern int chip_setting_saved_hour, chip_setting_saved_min;
extern int custom_setting_saved_hour, custom_setting_saved_min;

// ===== 跨文件函数声明(去 static, 原 nav.c 内部函数) =====
void adjust_value(edit_field_t *f, int delta);
void anim_bar_set_value(void *obj, int32_t v);
void apply_toggle_state(lv_obj_t *btn_off, lv_obj_t *btn_on, int on);
void bind_events(void);
void clear_focus_states(lv_obj_t **btns, int count);
void color_exit_to_home(void);
void color_menu_rebuild(page_id_t child);
void cook4menu_rebuild(page_id_t child);
void cooking_timer_cb(lv_timer_t *timer);
void delay_cancel_to_stop_back(void);
void delay_start_cook(void);
void delayset_refresh_display(delayset_t *ds);
void edit_clear(void);
void edit_register(lv_obj_t *label, lv_obj_t *ind_s, lv_obj_t *ind_l, int *value, int min, int max, int step, const char *fmt);
edit_field_t *find_edit_field(lv_obj_t *obj);
uint16_t get_cavity_temp(void);
lv_group_t *group_create_for_page(lv_obj_t **btns, int count);
void groups_create(void);
void jump_to_color_complete(void);
void jump_to_color_cookoing(void);
void jump_to_color_menu(void);
void jump_to_color_stop(void);
void jump_to_color_stop_back(void);
void jump_to_cookmenu(void);
void jump_to_special_menu(void);
void jump_to_updown_bbq_complete(void);
void jump_to_updown_bbq_cooking(void);
void jump_to_updown_bbq_menu(void);
void jump_to_updown_bbq_set(void);
void jump_to_updown_bbq_stop(void);
void jump_to_updown_bbq_stop_back(void);
void nav_handle_key(uint8_t key);
void on_air_click(lv_event_t *e);
void on_bottom_bbq_click(lv_event_t *e);
void on_central_click(lv_event_t *e);
void on_color_start_click(lv_event_t *e);
void on_color_stop_back_sure_click(lv_event_t *e);
void on_color_stop_click(lv_event_t *e);
void on_color_stop_start_click(lv_event_t *e);
void on_contain_toggle(lv_event_t *e);
void on_cook_setting_click(lv_event_t *e);
void on_cook_stop_click(lv_event_t *e);
void on_cook_updown_click(lv_event_t *e);
void on_corn_click(lv_event_t *e);
void on_delay_toggle(lv_event_t *e);
void on_edit_focus(lv_event_t *e);
void on_frozen_click(lv_event_t *e);
void on_heatcontain_click(lv_event_t *e);
void on_hot_bbq_click(lv_event_t *e);
void on_hotwind_click(lv_event_t *e);
void on_pizza_click(lv_event_t *e);
void on_preheat_click(lv_event_t *e);
void on_preheat_toggle(lv_event_t *e);
void on_rising_click(lv_event_t *e);
void on_save_click(lv_event_t *e);
void on_setting_edit_focus(lv_event_t *e);
void on_setting_sure_click(lv_event_t *e);
void on_slowcook_click(lv_event_t *e);
void on_some_cook_click(lv_event_t *e);
void on_stop_back_sure_click(lv_event_t *e);
void on_stop_start_click(lv_event_t *e);
void on_sure_click(lv_event_t *e);
void on_top_bbq_click(lv_event_t *e);
void on_unfrozen_click(lv_event_t *e);
void on_updown_downtemp_click(lv_event_t *e);
void on_updown_next_click(lv_event_t *e);
void on_updown_uptemp_click(lv_event_t *e);
void on_windchange_click(lv_event_t *e);
void page_pop(void);
void page_push(page_id_t id);
void process_key(uint8_t key);
void rebuild_delaycooking(void);
void set_status_label_min(lv_obj_t *label, int temp_up, int temp_down, int hour, int min);
void setup_set_temp_display(updown_bbq_set_t *set);
void topflag_update_visibility(void);
void uart_print(void);
void update_setting_dir_icon(updown_bbq_setting_t *set);
void updown_set_apply_delay_label(updown_bbq_set_t *set);
void validate_constraints(void);
void waitmenu_apply_clock(void);
void waitmenu_clock_cache_reset(void);

#endif // NAV_INTERNAL_H
