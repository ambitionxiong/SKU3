#ifndef NAV_H
#define NAV_H

#include <stdint.h>
#include "ui_objects.h"
#include "aic_ui.h"
#include "ui_util.h"
#include "custom_defs.h"

// === 实时时钟接口（实现位于 custom.c：模拟器=PC 时间，SDK=sd8568 RTC）===
typedef struct {
    int year, month, day;
    int hour, min, sec;
    int wday;                    // 星期 0=周日 ~ 6=周六
} rtc_time_t;
int rtc_get_time(rtc_time_t *t);              // 0=成功
int64_t rtc_now_ms(void);                     // 自 2000-01-01 00:00 起的绝对毫秒（统一计时戳）
int rtc_days_from_epoch(int year, int month, int day);  // 自 2000-01-01 起的天数
extern int64_t g_delay_target;                // 延时预约目标绝对毫秒

// === 页面 ID 枚举 ===
typedef enum {
    PAGE_WAITMENU_24,     // 等待界面（根页，开机首页）
    PAGE_PROBETIP,         // 探针提示页
    PAGE_MAJOR_MENU_TZ,    // 探针下主菜单
    PAGE_COOK_MENU_TZ,     // 探针下烹调菜单
    PAGE_SPECIAL_MENU_TZ,  // 探针下特色菜单
    PAGE_MAJOR_MENU,
    PAGE_COOKMENU,
    PAGE_SPECIAL_MENU,
    PAGE_UPDOWN_BBQ_MENU,
    PAGE_UPDOWN_BBQ_MENU_TOP,
    PAGE_UPDOWN_BBQ_MENU_LOW,
    PAGE_PREHEAT_MENU,
    PAGE_COLOR_MENU,       // 额外上色设置页（复用 preheatmenu 结构）
    PAGE_PREHEAT_COOKING,
    PAGE_PREHEAT_STOP,
    PAGE_PREHEAT_STOP_BACK,
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
    PAGE_UPDOWN_BBQ_MENU_PROBE,
    PAGE_UPDOWN_BBQ_SET_PROBE,
    PAGE_UPDOWN_BBQ_COOKING_PROBE,
    PAGE_UPDOWN_BBQ_STOP_PROBE,
    PAGE_UPDOWN_BBQ_STOP_BACK_PROBE,
    PAGE_UPDOWN_BBQ_COMPLETE_PROBE,
    PAGE_HOT_BBQ_MENU_PROBE,
    PAGE_HOT_BBQ_SET_PROBE,
    PAGE_HOT_BBQ_COOKING_PROBE,
    PAGE_HOT_BBQ_STOP_PROBE,
    PAGE_HOT_BBQ_STOP_BACK_PROBE,
    PAGE_HOT_BBQ_COMPLETE_PROBE,
    PAGE_BOTTOM_BBQ_MENU_PROBE,
    PAGE_BOTTOM_BBQ_SET_PROBE,
    PAGE_BOTTOM_BBQ_COOKING_PROBE,
    PAGE_BOTTOM_BBQ_STOP_PROBE,
    PAGE_BOTTOM_BBQ_STOP_BACK_PROBE,
    PAGE_BOTTOM_BBQ_COMPLETE_PROBE,
    PAGE_SLOWCOOK_MENU_PROBE,
    PAGE_SLOWCOOK_SET_PROBE,
    PAGE_SLOWCOOK_COOKING_PROBE,
    PAGE_SLOWCOOK_STOP_PROBE,
    PAGE_SLOWCOOK_STOP_BACK_PROBE,
    PAGE_SLOWCOOK_COMPLETE_PROBE,
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
    PAGE_FROZEN_COOK,
    PAGE_LASAGNA_MENU,
    PAGE_LASAGNA_SET,
    PAGE_LASAGNA_COOKING,
    PAGE_LASAGNA_SETTING,
    PAGE_LASAGNA_STOP,
    PAGE_LASAGNA_STOP_BACK,
    PAGE_LASAGNA_COMPLETE,
    PAGE_STRUDEL_MENU,
    PAGE_STRUDEL_SET,
    PAGE_STRUDEL_COOKING,
    PAGE_STRUDEL_SETTING,
    PAGE_STRUDEL_STOP,
    PAGE_STRUDEL_STOP_BACK,
    PAGE_STRUDEL_COMPLETE,
    PAGE_BREAD_MENU,
    PAGE_BREAD_SET,
    PAGE_BREAD_COOKING,
    PAGE_BREAD_SETTING,
    PAGE_BREAD_STOP,
    PAGE_BREAD_STOP_BACK,
    PAGE_BREAD_COMPLETE,
    PAGE_PIZZA3_MENU,
    PAGE_PIZZA3_SET,
    PAGE_PIZZA3_COOKING,
    PAGE_PIZZA3_SETTING,
    PAGE_PIZZA3_STOP,
    PAGE_PIZZA3_STOP_BACK,
    PAGE_PIZZA3_COMPLETE,
    PAGE_CHIP_MENU,
    PAGE_CHIP_SET,
    PAGE_CHIP_COOKING,
    PAGE_CHIP_SETTING,
    PAGE_CHIP_STOP,
    PAGE_CHIP_STOP_BACK,
    PAGE_CHIP_COMPLETE,
    PAGE_CUSTOM_MENU,
    PAGE_CUSTOM_SET,
    PAGE_CUSTOM_COOKING,
    PAGE_CUSTOM_SETTING,
    PAGE_CUSTOM_STOP,
    PAGE_CUSTOM_STOP_BACK,
    PAGE_CUSTOM_COMPLETE,
    PAGE_CLEAN_MENU,
    PAGE_WATER_CLEAN_SET,
    PAGE_WATER_CLEAN_COOKING,
    PAGE_WATER_CLEAN_STOP,
    PAGE_WATER_CLEAN_STOP_BACK,
    PAGE_WATER_CLEAN_COMPLETE,
    PAGE_HOTCLEAN_MENU,
    PAGE_HOTCLEANSAVE_SET,
    PAGE_HOTCLEANSAVE_COOKING,
    PAGE_HOTCLEANSAVE_STOP,
    PAGE_HOTCLEANSAVE_STOP_BACK,
    PAGE_HOTCLEANSAVE_COOLING,
    PAGE_HOTCLEANSAVE_COMPLETE,
    PAGE_HOTCLEANMIDDLE_SET,
    PAGE_HOTCLEANMIDDLE_COOKING,
    PAGE_HOTCLEANMIDDLE_STOP,
    PAGE_HOTCLEANMIDDLE_STOP_BACK,
    PAGE_HOTCLEANMIDDLE_COOLING,
    PAGE_HOTCLEANMIDDLE_COMPLETE,
    PAGE_HOTCLEANHIGH_SET,
    PAGE_HOTCLEANHIGH_COOKING,
    PAGE_HOTCLEANHIGH_STOP,
    PAGE_HOTCLEANHIGH_STOP_BACK,
    PAGE_HOTCLEANHIGH_COOLING,
    PAGE_HOTCLEANHIGH_COMPLETE,
    PAGE_DELAYSET,
    PAGE_DELAYCOOKING,
    PAGE_SOMECOOK,          /* 多段烹饪主页面 */
    PAGE_STEPSET,           /* 多段烹饪步骤设置页 */
    PAGE_SOMECOOK_COOKING,  /* 多段烹饪运行页（单页五态） */
    PAGE_SCREEN_SET,        /* 设置页（覆盖层，任何页面可进） */
    PAGE_SIXMENU,           /* 第六感菜单 */
    PAGE_BREAD6MENU,        /* 第六感-面包 */
    PAGE_RISINGPAGE,        /* 第六感-是否发酵 */
    PAGE_DESCRIPTIONMENU,   /* 第六感-烹饪描述 */
    PAGE_SIX_COOKING,       /* 第六感-运行页（复用 somecook_cooking UI） */
    PAGE_TOASTCOLOR,        /* 第六感-烤色选择 */
} page_id_t;

extern page_id_t page_stack[];
extern int depth;

extern lv_group_t *g_major_menu;
extern lv_group_t *g_cookmenu;
extern lv_group_t *g_probetip;
extern lv_group_t *g_major_menu_tz;
extern lv_group_t *g_cook_menu_tz;
extern lv_group_t *g_special_menu_tz;
extern lv_group_t *g_special_menu;
extern lv_group_t *g_updown_bbq_menu;
extern lv_group_t *g_updown_bbq_menu_top;
extern lv_group_t *g_updown_bbq_menu_low;
extern lv_group_t *g_updown_bbq_menu_probe;
extern lv_group_t *g_updown_bbq_set_probe;
extern lv_group_t *g_updown_bbq_cooking_probe;
extern lv_group_t *g_updown_bbq_stop_probe;
extern lv_group_t *g_updown_bbq_stop_back_probe;
extern lv_group_t *g_updown_bbq_complete_probe;
extern lv_group_t *g_hot_bbq_menu_probe;
extern lv_group_t *g_hot_bbq_set_probe;
extern lv_group_t *g_hot_bbq_cooking_probe;
extern lv_group_t *g_hot_bbq_stop_probe;
extern lv_group_t *g_hot_bbq_stop_back_probe;
extern lv_group_t *g_hot_bbq_complete_probe;
extern lv_group_t *g_bottom_bbq_menu_probe;
extern lv_group_t *g_bottom_bbq_set_probe;
extern lv_group_t *g_bottom_bbq_cooking_probe;
extern lv_group_t *g_bottom_bbq_stop_probe;
extern lv_group_t *g_bottom_bbq_stop_back_probe;
extern lv_group_t *g_bottom_bbq_complete_probe;
extern lv_group_t *g_slowcook_menu_probe;
extern lv_group_t *g_slowcook_set_probe;
extern lv_group_t *g_slowcook_cooking_probe;
extern lv_group_t *g_slowcook_stop_probe;
extern lv_group_t *g_slowcook_stop_back_probe;
extern lv_group_t *g_slowcook_complete_probe;

extern lv_group_t *g_preheat_menu;
extern lv_group_t *g_preheat_cooking;
extern lv_group_t *g_preheat_stop;
extern lv_group_t *g_preheat_stop_back;
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
extern lv_group_t *g_delayset;
extern lv_group_t *g_delaycooking;
extern uint8_t g_delay_cancel_to_stop_back;
extern uint8_t g_keepwarm_active;
extern int g_keepwarm_sec;
extern page_id_t g_delay_source_page;
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
extern int probe_target_temp;
extern int cook_start_probe;
extern int g_complete_to_stop_back;
extern int g_cooling_to_stop_back;
extern int g_extra_color_to_stop_back;
extern int set_temp_up;
extern int set_temp_down;
extern int set_hour;
extern int set_min;
extern uint8_t preheat_on;
extern uint8_t delay_on;
extern int delay_hour;
extern int delay_min;
extern uint8_t contain_on;
extern int cook_total_ms;
extern uint32_t cook_start_time;
extern uint8_t cook_is_color;
extern lv_timer_t *cook_timer;
extern uint8_t g_on_stop_back;
extern void (*g_stop_back_complete)(void);

static inline int stop_back_progress(uint32_t elapsed, int total_ms)
{
    if (!total_ms) total_ms = 1;
    return 3 + (int)((int64_t)elapsed * 97 / total_ms);
}
extern uint32_t cook_elapsed_saved;
extern int cook_bar_saved;

extern lv_group_t *g_updown_bbq_complete;
extern lv_group_t *g_extra_color;
extern lv_group_t *g_color_cookoing;
extern lv_group_t *g_color_stop;
extern lv_group_t *g_color_stop_back;

void jump_to_updown_bbq_menu(void);
void jump_to_delayset(void);
void jump_to_delaycooking(void);
const char *mode_display_name(void);          // 按 g_send.cook_mode 返回模式名
void mode_apply_icon(lv_obj_t *icon);         // 按 g_send.cook_mode 设置模式图标
void mode_set_apply_delay_label(lv_obj_t *ondelay_btn);  // 14 模式 set 页 ondelay 按钮文字
void jump_to_updown_bbq_menu_probe(void);
void jump_to_updown_bbq_set_probe(void);
void jump_to_updown_bbq_cooking_probe(void);
void jump_to_updown_bbq_cooking(void);
void jump_to_updown_bbq_stop_probe(void);
void jump_to_updown_bbq_stop_back_probe(void);
void jump_to_updown_bbq_complete_probe(void);
void updown_bbq_probe_resume_cooking(void);
void updown_bbq_probe_rebuild_menu(page_id_t child);
void updown_bbq_probe_rebuild_set(page_id_t child);
void updown_bbq_probe_rebuild_cooking(page_id_t child);
void updown_bbq_probe_rebuild_setting(void);
void updown_bbq_probe_rebuild_stop(void);
void updown_bbq_probe_rebuild_stop_back(void);
void updown_bbq_probe_rebuild_complete(void);

void jump_to_hot_bbq_menu_probe(void);
void jump_to_hot_bbq_set_probe(void);
void jump_to_hot_bbq_cooking_probe(void);
void jump_to_hot_bbq_stop_probe(void);
void jump_to_hot_bbq_stop_back_probe(void);
void jump_to_hot_bbq_complete_probe(void);
void hot_bbq_probe_resume_cooking(void);
void hot_bbq_probe_rebuild_menu(page_id_t child);
void hot_bbq_probe_rebuild_set(page_id_t child);
void hot_bbq_probe_rebuild_cooking(page_id_t child);
void hot_bbq_probe_rebuild_stop(void);
void hot_bbq_probe_rebuild_stop_back(void);
void hot_bbq_probe_rebuild_complete(void);
void hot_bbq_complete_rebind(lv_obj_t *btn);

void jump_to_bottom_bbq_menu_probe(void);
void jump_to_bottom_bbq_set_probe(void);
void jump_to_bottom_bbq_cooking_probe(void);
void jump_to_bottom_bbq_stop_probe(void);
void jump_to_bottom_bbq_stop_back_probe(void);
void jump_to_bottom_bbq_complete_probe(void);
void bottom_bbq_probe_resume_cooking(void);
void bottom_bbq_probe_rebuild_menu(page_id_t child);
void bottom_bbq_probe_rebuild_set(page_id_t child);
void bottom_bbq_probe_rebuild_cooking(page_id_t child);
void bottom_bbq_probe_rebuild_stop(void);
void bottom_bbq_probe_rebuild_stop_back(void);
void bottom_bbq_probe_rebuild_complete(void);

void jump_to_slowcook_menu_probe(void);
void jump_to_slowcook_set_probe(void);
void jump_to_slowcook_cooking_probe(void);
void jump_to_slowcook_stop_probe(void);
void jump_to_slowcook_stop_back_probe(void);
void jump_to_slowcook_complete_probe(void);
void slowcook_probe_resume_cooking(void);
void slowcook_probe_rebuild_menu(page_id_t child);
void slowcook_probe_rebuild_set(page_id_t child);
void slowcook_probe_rebuild_cooking(page_id_t child);
void slowcook_probe_rebuild_stop(void);
void slowcook_probe_rebuild_stop_back(void);
void slowcook_probe_rebuild_complete(void);

void jump_to_probetip(const char *text);
void probetip_rebuild(page_id_t child);
void probetip_cancel_auto_dismiss(void);
void jump_to_major_menu_tz(void);
void jump_to_cook_menu_tz(void);
void jump_to_special_menu_tz(void);
void major_menu_tz_rebuild(page_id_t child);
void cook_menu_tz_rebuild(page_id_t child);
void special_menu_tz_rebuild(page_id_t child);
uint16_t get_cavity_temp(void);
void jump_to_cookmenu(void);
void jump_to_preheat_menu(void);
void jump_to_top_bbq_cooking(void);
void jump_to_bottom_bbq_cooking(void);
void jump_to_hot_bbq_cooking(void);
void jump_to_preheat_cooking(void);
void jump_to_preheat_stop(void);
void jump_to_preheat_stop_back(void);
void preheat_rebuild_menu(page_id_t child);
void preheat_rebuild_cooking(void);
void preheat_rebuild_stop(void);
void preheat_rebuild_stop_back(void);
void preheat_rebuild_complete(void);
void preheat_resume_cooking(void);
void preheat_complete_exit(void);
extern int preheat_start_cavity;
extern int preheat_wait_door;
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
void stepset_bind_events(void);
void delay_cancel_exit_to_set(void);
extern uint8_t g_delay_cancel_btn;
void jump_to_somecook(void);
extern lv_group_t *g_somecook_btns;
extern lv_group_t *g_somecook_edit;
extern lv_group_t *g_stepset;
void jump_to_stepset(int i);
void somecook_back_to_btns(int focus_btn);
void somecook_rebuild(page_id_t child);
extern lv_group_t *g_somecook_cooking;
extern uint8_t g_somecook_running;
extern int g_somecook_run_idx;
void somecook_cooking_handle_back(void);
void somecook_cooking_auto_pause(void);
void somecook_cooking_next_step(void);
void somecook_cooking_update_timer(somecook_cooking_t *sc);
void on_somecook_sure_click(lv_event_t *e);
void somecook_cooking_refresh(void);
int somecook_cooking_is_stopback(void);

/* 设置页（覆盖层，nav_screen_set.c 实现） */
extern lv_group_t *g_screen_set;
void jump_to_screen_set(void);
void screen_set_back(void);
void screen_set_reset(void);

/* 功能键无效提示（nav_hint.c 实现） */
void nav_show_invalid_hint(void);
void nav_hint_cancel(void);
int nav_hint_active(void);

/* 第六感菜单（nav_sixmenu.c 实现） */
extern lv_group_t *g_sixmenu;
extern lv_group_t *g_bread6menu;
extern lv_group_t *g_risingpage;
extern lv_group_t *g_descriptionmenu;
extern int g_rising_choice;   /* -1 未选 0=否 1=是 */
void jump_to_sixmenu(void);
void sixmenu_rebuild(page_id_t child);
void jump_to_bread6menu(void);
void bread6menu_rebuild(page_id_t child);
void jump_to_risingpage(void);
void risingpage_rebuild(page_id_t child);
void jump_to_descriptionmenu(void);
void descriptionmenu_rebuild(page_id_t child);

/* 第六感运行（nav_six_cook.c 实现） */
extern lv_group_t *g_six_cooking;
extern uint8_t g_six_running;
extern int g_six_color_min;   /* 烤色分钟 2/4/6 */
void jump_to_six_cooking(void);
void six_cook_handle_back(void);
void six_cooking_rebuild(page_id_t child);
void six_cook_goto_setup(void);
void six_cook_reset(void);
void six_cook_refresh_display(void);
int six_cook_is_overlay(void);

/* 第六感烤色选择（nav_toastcolor.c 实现） */
extern lv_group_t *g_toastcolor;
void jump_to_toastcolor(void);
void toastcolor_rebuild(page_id_t child);
void toastcolor_cycle(int dir);

/* 多段烹饪步骤数据（nav_somecook.c 定义） */
typedef struct {
    uint8_t mode;      /* MODE_xxx */
    int temp;
    int hour, min;
    bool set;
} somecook_step_t;
extern somecook_step_t g_steps[3];
extern int g_cur_step;
void stepset_on_focus(lv_event_t *e);
void stepset_apply_sel_mode(bool restore);
void stepset_restore_mode(uint8_t mode);

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
void update_air_dir_icon(air_setting_t *set);
void update_pizza_2_dir_icon(pizza_2_setting_t *set);
void update_slowcook_dir_icon(slowcook_setting_t *set);
void update_unfrozen_dir_icon(unfrozen_setting_t *set);
void update_rising_dir_icon(rising_setting_t *set);
void update_corn_dir_icon(corn_setting_t *set);
void update_heatcontain_dir_icon(heatcontain_setting_t *set);

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
void hotwind_bbq_complete_rebind(lv_obj_t *btn);
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
void save_bbq_complete_rebind(lv_obj_t *btn);
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
void central_bbq_complete_rebind(lv_obj_t *btn);
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
void windchange_bbq_complete_rebind(lv_obj_t *btn);
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
void top_bbq_complete_rebind(lv_obj_t *btn);

void jump_to_bottom_bbq_menu(void);
void jump_to_bottom_bbq_set(void);
void jump_to_bottom_bbq_stop(void);
void jump_to_bottom_bbq_stop_back(void);
void jump_to_bottom_bbq_setting(void);
void bottom_bbq_complete_rebind(lv_obj_t *btn);
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
void cookie_complete_rebind(lv_obj_t *btn);
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
void west_complete_rebind(lv_obj_t *btn);
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
void pizza_complete_rebind(lv_obj_t *btn);
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
void menu_complete_rebind(lv_obj_t *btn);
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
void air_complete_rebind(lv_obj_t *btn);
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
void pizza_2_complete_rebind(lv_obj_t *btn);
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

extern lv_group_t *g_frozen_cook;

extern lv_group_t *g_lasagna_menu;
extern lv_group_t *g_lasagna_set;
extern lv_group_t *g_lasagna_cooking;
extern lv_group_t *g_lasagna_setting;
extern lv_group_t *g_lasagna_stop;
extern lv_group_t *g_lasagna_stop_back;
extern lv_group_t *g_lasagna_complete;

extern lv_group_t *g_strudel_menu;
extern lv_group_t *g_strudel_set;
extern lv_group_t *g_strudel_cooking;
extern lv_group_t *g_strudel_setting;
extern lv_group_t *g_strudel_stop;
extern lv_group_t *g_strudel_stop_back;
extern lv_group_t *g_strudel_complete;

extern lv_group_t *g_bread_menu;
extern lv_group_t *g_bread_set;
extern lv_group_t *g_bread_cooking;
extern lv_group_t *g_bread_setting;
extern lv_group_t *g_bread_stop;
extern lv_group_t *g_bread_stop_back;
extern lv_group_t *g_bread_complete;

extern lv_group_t *g_pizza3_menu;
extern lv_group_t *g_pizza3_set;
extern lv_group_t *g_pizza3_cooking;
extern lv_group_t *g_pizza3_setting;
extern lv_group_t *g_pizza3_stop;
extern lv_group_t *g_pizza3_stop_back;
extern lv_group_t *g_pizza3_complete;

extern lv_group_t *g_chip_menu;
extern lv_group_t *g_chip_set;
extern lv_group_t *g_chip_cooking;
extern lv_group_t *g_chip_setting;
extern lv_group_t *g_chip_stop;
extern lv_group_t *g_chip_stop_back;
extern lv_group_t *g_chip_complete;

extern lv_group_t *g_custom_menu;
extern lv_group_t *g_custom_set;
extern lv_group_t *g_custom_cooking;
extern lv_group_t *g_custom_setting;
extern lv_group_t *g_custom_stop;
extern lv_group_t *g_custom_stop_back;
extern lv_group_t *g_custom_complete;

extern lv_group_t *g_clean_menu;
extern lv_group_t *g_wc_set;
extern lv_group_t *g_wc_cooking;
extern lv_group_t *g_wc_stop;
extern lv_group_t *g_wc_stop_back;
extern lv_group_t *g_wc_complete;

extern lv_group_t *g_hotclean_menu;
extern lv_group_t *g_hcs_set;
extern lv_group_t *g_hcs_cooking;
extern lv_group_t *g_hcs_stop;
extern lv_group_t *g_hcs_stop_back;
extern lv_group_t *g_hcs_cooling;
extern lv_group_t *g_hcs_complete;

extern lv_group_t *g_hcm_set;
extern lv_group_t *g_hcm_cooking;
extern lv_group_t *g_hcm_stop;
extern lv_group_t *g_hcm_stop_back;
extern lv_group_t *g_hcm_cooling;
extern lv_group_t *g_hcm_complete;

extern lv_group_t *g_hch_set;
extern lv_group_t *g_hch_cooking;
extern lv_group_t *g_hch_stop;
extern lv_group_t *g_hch_stop_back;
extern lv_group_t *g_hch_cooling;
extern lv_group_t *g_hch_complete;

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
void slowcook_complete_rebind(lv_obj_t *btn);
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
void unfrozen_complete_rebind(lv_obj_t *btn);
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
void rising_complete_rebind(lv_obj_t *btn);
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
void corn_complete_rebind(lv_obj_t *btn);
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
void heatcontain_complete_rebind(lv_obj_t *btn);
void heatcontain_resume_cooking(void);

void lasagna_complete_rebind(lv_obj_t *btn);
void strudel_complete_rebind(lv_obj_t *btn);
void bread_complete_rebind(lv_obj_t *btn);
void pizza3_complete_rebind(lv_obj_t *btn);
void chip_complete_rebind(lv_obj_t *btn);
void custom_complete_rebind(lv_obj_t *btn);

void updown_bbq_probe_complete_rebind(lv_obj_t *btn);
void bottom_bbq_probe_complete_rebind(lv_obj_t *btn);
void hot_bbq_probe_complete_rebind(lv_obj_t *btn);
void slowcook_probe_complete_rebind(lv_obj_t *btn);

void jump_to_frozen_cook(void);
void frozen_rebuild(page_id_t child);

void jump_to_lasagna_menu(void);
void jump_to_lasagna_set(void);
void jump_to_lasagna_cooking(void);
void jump_to_lasagna_setting(void);
void jump_to_lasagna_stop(void);
void jump_to_lasagna_stop_back(void);
void jump_to_lasagna_complete(void);
void lasagna_rebuild_menu(page_id_t child);
void lasagna_rebuild_set(page_id_t child);
void lasagna_rebuild_cooking(page_id_t child);
void lasagna_rebuild_setting(void);
void lasagna_rebuild_stop(void);
void lasagna_rebuild_stop_back(void);
void lasagna_rebuild_complete(void);
void lasagna_resume_cooking(void);

void jump_to_strudel_menu(void);
void jump_to_strudel_set(void);
void jump_to_strudel_cooking(void);
void jump_to_strudel_setting(void);
void jump_to_strudel_stop(void);
void jump_to_strudel_stop_back(void);
void jump_to_strudel_complete(void);
void strudel_rebuild_menu(page_id_t child);
void strudel_rebuild_set(page_id_t child);
void strudel_rebuild_cooking(page_id_t child);
void strudel_rebuild_setting(void);
void strudel_rebuild_stop(void);
void strudel_rebuild_stop_back(void);
void strudel_rebuild_complete(void);
void strudel_resume_cooking(void);

void jump_to_bread_menu(void);
void jump_to_bread_set(void);
void jump_to_bread_cooking(void);
void jump_to_bread_setting(void);
void jump_to_bread_stop(void);
void jump_to_bread_stop_back(void);
void jump_to_bread_complete(void);
void bread_rebuild_menu(page_id_t child);
void bread_rebuild_set(page_id_t child);
void bread_rebuild_cooking(page_id_t child);
void bread_rebuild_setting(void);
void bread_rebuild_stop(void);
void bread_rebuild_stop_back(void);
void bread_rebuild_complete(void);
void bread_resume_cooking(void);

void jump_to_pizza3_menu(void);
void jump_to_pizza3_set(void);
void jump_to_pizza3_cooking(void);
void jump_to_pizza3_setting(void);
void jump_to_pizza3_stop(void);
void jump_to_pizza3_stop_back(void);
void jump_to_pizza3_complete(void);
void pizza3_rebuild_menu(page_id_t child);
void pizza3_rebuild_set(page_id_t child);
void pizza3_rebuild_cooking(page_id_t child);
void pizza3_rebuild_setting(void);
void pizza3_rebuild_stop(void);
void pizza3_rebuild_stop_back(void);
void pizza3_rebuild_complete(void);
void pizza3_resume_cooking(void);

void jump_to_chip_menu(void);
void jump_to_chip_set(void);
void jump_to_chip_cooking(void);
void jump_to_chip_setting(void);
void jump_to_chip_stop(void);
void jump_to_chip_stop_back(void);
void jump_to_chip_complete(void);
void chip_rebuild_menu(page_id_t child);
void chip_rebuild_set(page_id_t child);
void chip_rebuild_cooking(page_id_t child);
void chip_rebuild_setting(void);
void chip_rebuild_stop(void);
void chip_rebuild_stop_back(void);
void chip_rebuild_complete(void);
void chip_resume_cooking(void);

void jump_to_custom_menu(void);
void jump_to_custom_set(void);
void jump_to_custom_cooking(void);
void jump_to_custom_setting(void);
void jump_to_custom_stop(void);
void jump_to_custom_stop_back(void);
void jump_to_custom_complete(void);
void custom_rebuild_menu(page_id_t child);
void custom_rebuild_set(page_id_t child);
void custom_rebuild_cooking(page_id_t child);
void custom_rebuild_setting(void);
void custom_rebuild_stop(void);
void custom_rebuild_stop_back(void);
void custom_rebuild_complete(void);
void custom_resume_cooking(void);

void on_clean_water_click(lv_event_t *e);
void on_clean_hot_click(lv_event_t *e);
void jump_to_clean_menu(void);
void clean_rebuild(page_id_t child);

void jump_to_wc_set(void);
void jump_to_wc_cooking(void);
void jump_to_wc_stop(void);
void jump_to_wc_stop_back(void);
void jump_to_wc_complete(void);
void wc_rebuild_set(page_id_t child);
void wc_rebuild_cooking(page_id_t child);
void wc_rebuild_stop(void);
void wc_rebuild_stop_back(void);
void wc_rebuild_complete(void);
void wc_resume_cooking(void);

void on_hc_save_click(lv_event_t *e);
void jump_to_hotclean_menu(void);
void hotclean_rebuild(page_id_t child);

void jump_to_hcs_set(void);
void jump_to_hcs_cooking(void);
void jump_to_hcs_stop(void);
void jump_to_hcs_stop_back(void);
void jump_to_hcs_cooling(void);
void jump_to_hcs_complete(void);
void hcs_rebuild_set(page_id_t child);
void hcs_rebuild_cooking(page_id_t child);
void hcs_rebuild_stop(void);
void hcs_rebuild_stop_back(void);
void hcs_rebuild_cooling(void);
void hcs_rebuild_complete(void);
void hcs_resume_cooking(void);

void jump_to_hcm_set(void);
void jump_to_hcm_cooking(void);
void jump_to_hcm_stop(void);
void jump_to_hcm_stop_back(void);
void jump_to_hcm_cooling(void);
void jump_to_hcm_complete(void);
void hcm_rebuild_set(page_id_t child);
void hcm_rebuild_cooking(page_id_t child);
void hcm_rebuild_stop(void);
void hcm_rebuild_stop_back(void);
void hcm_rebuild_cooling(void);
void hcm_rebuild_complete(void);
void hcm_resume_cooking(void);

void jump_to_hch_set(void);
void jump_to_hch_cooking(void);
void jump_to_hch_stop(void);
void jump_to_hch_stop_back(void);
void jump_to_hch_cooling(void);
void jump_to_hch_complete(void);
void hch_rebuild_set(page_id_t child);
void hch_rebuild_cooking(page_id_t child);
void hch_rebuild_stop(void);
void hch_rebuild_stop_back(void);
void hch_rebuild_cooling(void);
void hch_rebuild_complete(void);
void hch_resume_cooking(void);

#endif
