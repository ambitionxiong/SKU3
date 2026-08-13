#include "nav.h"
#include "protocol.h"

// === 页面栈 ===
#define MAX_STACK 16
page_id_t page_stack[MAX_STACK];  // 栈数组，stack[0]=根页，stack[depth-1]=当前页
int depth = 0;                     // 栈高度，depth=1 时只有根页

int set_temp_up = 180;
int set_temp_down = 180;
int cook_bar_saved = 0;
int probe_target_temp = 80;
int preheat_start_cavity = 0;
int preheat_wait_door = 0;
int g_complete_to_stop_back = 0;
int g_cooling_to_stop_back = 0;
int g_extra_color_to_stop_back = 0;
int g_color_from_probe = 0;   // 进入额外上色时是否为探针模式（color 返回时跳探针主菜单）

#ifdef LV_USE_AIC_SIMULATOR

static void uart_print(void)
{
    uart_send_fill();
    printf("[UART]");
    for (int i = 0; i < 24; i++) printf(" %02X", uart_data_send[i]);
    static const char *st[] = {"stdby","set","cook","pause","done","sleep"};
    static const char *md[] = {"none","","hotwind","updown","top","bottom","pizza2","","hot","","","heatcontain","rising","air","","","","","","","waterclean","","","","","windchange","central","save","preheat","","","","","cook4","frozen","slowcook","corn","","color","","unfrozen","hotclean"};
     printf("\n[UART] decoded: st=%s(%d) mode=%s(%d) flag=%d t=%d t_lo=%d time=%02d:%02d:%02d buz=%d\n",
            st[g_send.iface_status<=5?g_send.iface_status:0], g_send.iface_status,
            md[g_send.cook_mode<=41?g_send.cook_mode:0], g_send.cook_mode,
            g_send.cook_flag,
            g_send.set_temp, g_send.set_temp_lower,
           uart_data_send[SEND_TIME_HOUR],
           uart_data_send[SEND_TIME_MIN],
           uart_data_send[SEND_TIME_SEC],
           uart_data_send[SEND_BUZZER]);
}
#else
#define uart_print() ((void)0)
#endif

// === 各页面焦点组（NULL=未创建）===
lv_group_t *g_major_menu;
lv_group_t *g_cookmenu;
lv_group_t *g_probetip;
lv_group_t *g_major_menu_tz;
lv_group_t *g_cook_menu_tz;
lv_group_t *g_special_menu_tz;
lv_group_t *g_special_menu;
lv_group_t *g_updown_bbq_menu;
lv_group_t *g_updown_bbq_set;
lv_group_t *g_delayset;
lv_group_t *g_delaycooking;
uint8_t g_delay_cancel_to_stop_back;
uint8_t g_delay_cancel_btn;         /* 仅 delaycooking 点"取消"按钮进入的 stop_back（区别于 BACK 键） */
uint8_t g_keepwarm_active;
int g_keepwarm_sec;
page_id_t g_delay_source_page = PAGE_WAITMENU_24;
int64_t g_delay_target = -1;   /* -1=无有效预约目标（哨兵，到点检测不触发） */
lv_group_t *g_updown_bbq_cooking;
lv_group_t *g_updown_bbq_complete;

lv_group_t *g_updown_bbq_menu_top;
lv_group_t *g_updown_bbq_menu_low;
lv_group_t *g_updown_bbq_menu_probe;
lv_group_t *g_updown_bbq_set_probe;
lv_group_t *g_updown_bbq_cooking_probe;
lv_group_t *g_updown_bbq_stop_probe;
lv_group_t *g_updown_bbq_stop_back_probe;
lv_group_t *g_updown_bbq_complete_probe;
lv_group_t *g_hot_bbq_menu_probe;
lv_group_t *g_hot_bbq_set_probe;
lv_group_t *g_hot_bbq_cooking_probe;
lv_group_t *g_hot_bbq_stop_probe;
lv_group_t *g_hot_bbq_stop_back_probe;
lv_group_t *g_hot_bbq_complete_probe;
lv_group_t *g_bottom_bbq_menu_probe;
lv_group_t *g_bottom_bbq_set_probe;
lv_group_t *g_bottom_bbq_cooking_probe;
lv_group_t *g_bottom_bbq_stop_probe;
lv_group_t *g_bottom_bbq_stop_back_probe;
lv_group_t *g_bottom_bbq_complete_probe;
lv_group_t *g_slowcook_menu_probe;
lv_group_t *g_slowcook_set_probe;
lv_group_t *g_slowcook_cooking_probe;
lv_group_t *g_slowcook_stop_probe;
lv_group_t *g_slowcook_stop_back_probe;
lv_group_t *g_slowcook_complete_probe;
lv_group_t *g_preheat_menu;

lv_group_t *g_preheat_cooking;

lv_group_t *g_preheat_stop;

lv_group_t *g_preheat_stop_back;

lv_group_t *g_preheat_complete;
lv_group_t *g_cook4_menu;
lv_group_t *g_cookie_menu;
lv_group_t *g_cookie_set;
lv_group_t *g_cookie_cooking;
lv_group_t *g_cookie_setting;
lv_group_t *g_cookie_stop;
lv_group_t *g_cookie_stop_back;
lv_group_t *g_cookie_complete;
lv_group_t *g_west_menu;
lv_group_t *g_west_set;
lv_group_t *g_west_cooking;
lv_group_t *g_west_setting;
lv_group_t *g_west_stop;
lv_group_t *g_west_stop_back;
lv_group_t *g_west_complete;
lv_group_t *g_pizza_menu;
lv_group_t *g_pizza_set;
lv_group_t *g_pizza_cooking;
lv_group_t *g_pizza_setting;
lv_group_t *g_pizza_stop;
lv_group_t *g_pizza_stop_back;
lv_group_t *g_pizza_complete;
lv_group_t *g_menu_cook_menu;
lv_group_t *g_menu_cook_set;
lv_group_t *g_menu_cook_cooking;
lv_group_t *g_menu_cook_setting;
lv_group_t *g_menu_cook_stop;
lv_group_t *g_menu_cook_stop_back;
lv_group_t *g_menu_cook_complete;
lv_group_t *g_air_menu;
lv_group_t *g_air_set;
lv_group_t *g_air_cooking;
lv_group_t *g_air_setting;
lv_group_t *g_air_stop;
lv_group_t *g_air_stop_back;
lv_group_t *g_air_complete;
lv_group_t *g_pizza_2_menu;
lv_group_t *g_pizza_2_set;
lv_group_t *g_pizza_2_cooking;
lv_group_t *g_pizza_2_setting;
lv_group_t *g_pizza_2_stop;
lv_group_t *g_pizza_2_stop_back;
lv_group_t *g_pizza_2_complete;
lv_group_t *g_slowcook_menu;
lv_group_t *g_slowcook_set;
lv_group_t *g_slowcook_cooking;
lv_group_t *g_slowcook_setting;
lv_group_t *g_slowcook_stop;
lv_group_t *g_slowcook_stop_back;
lv_group_t *g_slowcook_complete;
lv_group_t *g_unfrozen_menu;
lv_group_t *g_unfrozen_set;
lv_group_t *g_unfrozen_cooking;
lv_group_t *g_unfrozen_setting;
lv_group_t *g_unfrozen_stop;
lv_group_t *g_unfrozen_stop_back;
lv_group_t *g_unfrozen_complete;
lv_group_t *g_rising_menu;
lv_group_t *g_rising_set;
lv_group_t *g_rising_cooking;
lv_group_t *g_rising_setting;
lv_group_t *g_rising_stop;
lv_group_t *g_rising_stop_back;
lv_group_t *g_rising_complete;
lv_group_t *g_corn_menu;
lv_group_t *g_corn_set;
lv_group_t *g_corn_cooking;
lv_group_t *g_corn_setting;
lv_group_t *g_corn_stop;
lv_group_t *g_corn_stop_back;
lv_group_t *g_corn_complete;
lv_group_t *g_heatcontain_menu;
lv_group_t *g_heatcontain_set;
lv_group_t *g_heatcontain_cooking;
lv_group_t *g_heatcontain_setting;
lv_group_t *g_heatcontain_stop;
lv_group_t *g_heatcontain_stop_back;
lv_group_t *g_heatcontain_complete;
lv_group_t *g_frozen_cook;
lv_group_t *g_lasagna_menu;
lv_group_t *g_lasagna_set;
lv_group_t *g_lasagna_cooking;
lv_group_t *g_lasagna_setting;
lv_group_t *g_lasagna_stop;
lv_group_t *g_lasagna_stop_back;
lv_group_t *g_lasagna_complete;
lv_group_t *g_strudel_menu;
lv_group_t *g_strudel_set;
lv_group_t *g_strudel_cooking;
lv_group_t *g_strudel_setting;
lv_group_t *g_strudel_stop;
lv_group_t *g_strudel_stop_back;
lv_group_t *g_strudel_complete;
lv_group_t *g_bread_menu;
lv_group_t *g_bread_set;
lv_group_t *g_bread_cooking;
lv_group_t *g_bread_setting;
lv_group_t *g_bread_stop;
lv_group_t *g_bread_stop_back;
lv_group_t *g_bread_complete;
lv_group_t *g_pizza3_menu;
lv_group_t *g_pizza3_set;
lv_group_t *g_pizza3_cooking;
lv_group_t *g_pizza3_setting;
lv_group_t *g_pizza3_stop;
lv_group_t *g_pizza3_stop_back;
lv_group_t *g_pizza3_complete;
lv_group_t *g_chip_menu;
lv_group_t *g_chip_set;
lv_group_t *g_chip_cooking;
lv_group_t *g_chip_setting;
lv_group_t *g_chip_stop;
lv_group_t *g_chip_stop_back;
lv_group_t *g_chip_complete;
lv_group_t *g_custom_menu;
lv_group_t *g_custom_set;
lv_group_t *g_custom_cooking;
lv_group_t *g_custom_setting;
lv_group_t *g_custom_stop;
lv_group_t *g_custom_stop_back;
lv_group_t *g_custom_complete;
lv_group_t *g_clean_menu;
lv_group_t *g_wc_set;
lv_group_t *g_wc_cooking;
lv_group_t *g_wc_stop;
lv_group_t *g_wc_stop_back;
lv_group_t *g_wc_complete;
lv_group_t *g_hotclean_menu;
lv_group_t *g_hcs_set;
lv_group_t *g_hcs_cooking;
lv_group_t *g_hcs_stop;
lv_group_t *g_hcs_stop_back;
lv_group_t *g_hcs_cooling;
lv_group_t *g_hcs_complete;
lv_group_t *g_hcm_set;
lv_group_t *g_hcm_cooking;
lv_group_t *g_hcm_stop;
lv_group_t *g_hcm_stop_back;
lv_group_t *g_hcm_cooling;
lv_group_t *g_hcm_complete;
lv_group_t *g_hch_set;
lv_group_t *g_hch_cooking;
lv_group_t *g_hch_stop;
lv_group_t *g_hch_stop_back;
lv_group_t *g_hch_cooling;
lv_group_t *g_hch_complete;
lv_group_t *g_cook4_menu;

static int updown_menu_top_saved, updown_menu_low_saved;

uint8_t g_on_stop_back = 0;
void (*g_stop_back_complete)(void) = NULL;

lv_group_t *g_cookie_menu;

lv_group_t *g_cookie_set;

lv_group_t *g_cookie_cooking;

lv_group_t *g_cookie_setting;

lv_group_t *g_cookie_stop;

lv_group_t *g_cookie_stop_back;

lv_group_t *g_cookie_complete;

lv_group_t *g_west_menu;

lv_group_t *g_west_set;

lv_group_t *g_west_cooking;

lv_group_t *g_west_setting;

lv_group_t *g_west_stop;

lv_group_t *g_west_stop_back;

lv_group_t *g_west_complete;

lv_group_t *g_pizza_menu;

lv_group_t *g_pizza_set;

lv_group_t *g_pizza_cooking;

lv_group_t *g_pizza_setting;

lv_group_t *g_pizza_stop;

lv_group_t *g_pizza_stop_back;

lv_group_t *g_pizza_complete;

lv_group_t *g_menu_cook_menu;

lv_group_t *g_menu_cook_set;

lv_group_t *g_menu_cook_cooking;

lv_group_t *g_menu_cook_setting;

lv_group_t *g_menu_cook_stop;

lv_group_t *g_menu_cook_stop_back;

lv_group_t *g_menu_cook_complete;


#ifdef LV_USE_AIC_SIMULATOR

uint16_t g_sim_cavity_temp = 25;

#endif


uint16_t get_cavity_temp(void)

{

#ifdef LV_USE_AIC_SIMULATOR

    return g_sim_cavity_temp;

#else

    return ((uint16_t)uart_data_receive[Receive_data_QiangTi_Temp_H] << 8)

           | uart_data_receive[Receive_data_QiangTi_Temp_L];

#endif

}
lv_group_t *g_extra_color;
lv_group_t *g_color_cookoing;
lv_group_t *g_color_stop;
lv_group_t *g_color_stop_back;
lv_group_t *g_updown_bbq_setting;
lv_group_t *g_updown_bbq_stop;
lv_group_t *g_updown_bbq_stop_back;

lv_group_t *g_top_bbq_menu;
lv_group_t *g_top_bbq_set;
lv_group_t *g_top_bbq_cooking;
lv_group_t *g_top_bbq_setting;
lv_group_t *g_top_bbq_stop;
lv_group_t *g_top_bbq_stop_back;
lv_group_t *g_top_bbq_complete;

lv_group_t *g_bottom_bbq_menu;
lv_group_t *g_bottom_bbq_set;
lv_group_t *g_bottom_bbq_cooking;
lv_group_t *g_bottom_bbq_setting;
lv_group_t *g_bottom_bbq_stop;
lv_group_t *g_bottom_bbq_stop_back;
lv_group_t *g_bottom_bbq_complete;

lv_group_t *g_hot_bbq_menu;
lv_group_t *g_hot_bbq_set;
lv_group_t *g_hot_bbq_cooking;
lv_group_t *g_hot_bbq_setting;
lv_group_t *g_hot_bbq_stop;
lv_group_t *g_hot_bbq_stop_back;
lv_group_t *g_hot_bbq_complete;

lv_group_t *g_hotwind_bbq_menu;
lv_group_t *g_hotwind_bbq_set;
lv_group_t *g_hotwind_bbq_cooking;
lv_group_t *g_hotwind_bbq_setting;
lv_group_t *g_hotwind_bbq_stop;
lv_group_t *g_hotwind_bbq_stop_back;
lv_group_t *g_hotwind_bbq_complete;

lv_group_t *g_save_bbq_menu;
lv_group_t *g_save_bbq_set;
lv_group_t *g_save_bbq_cooking;
lv_group_t *g_save_bbq_setting;
lv_group_t *g_save_bbq_stop;
lv_group_t *g_save_bbq_stop_back;
lv_group_t *g_save_bbq_complete;

lv_group_t *g_central_bbq_menu;
lv_group_t *g_central_bbq_set;
lv_group_t *g_central_bbq_cooking;
lv_group_t *g_central_bbq_setting;
lv_group_t *g_central_bbq_stop;
lv_group_t *g_central_bbq_stop_back;
lv_group_t *g_central_bbq_complete;

lv_group_t *g_windchange_bbq_menu;
lv_group_t *g_windchange_bbq_set;
lv_group_t *g_windchange_bbq_cooking;
lv_group_t *g_windchange_bbq_setting;
lv_group_t *g_windchange_bbq_stop;
lv_group_t *g_windchange_bbq_stop_back;
lv_group_t *g_windchange_bbq_complete;

lv_group_t *current_group = NULL;  // 当前活跃的焦点组，nav_handle_key 操作的就是这个组

// 前向声明（page_pop/groups_create 互相引用）
void groups_create(void);
lv_group_t *group_create_for_page(lv_obj_t **btns, int count);
void bind_events(void);
void on_cook_updown_click(lv_event_t *e);
static void on_top_bbq_click(lv_event_t *e);
static void on_bottom_bbq_click(lv_event_t *e);
static void on_hot_bbq_click(lv_event_t *e);
static void on_hotwind_click(lv_event_t *e);
static void on_save_click(lv_event_t *e);
static void on_central_click(lv_event_t *e);
static void on_windchange_click(lv_event_t *e);
static void on_preheat_click(lv_event_t *e);
static void on_air_click(lv_event_t *e);
static void on_pizza_click(lv_event_t *e);
static void on_some_cook_click(lv_event_t *e);
static void on_frozen_click(lv_event_t *e);
static void on_slowcook_click(lv_event_t *e);
static void on_unfrozen_click(lv_event_t *e);
static void on_rising_click(lv_event_t *e);
static void on_corn_click(lv_event_t *e);
static void on_heatcontain_click(lv_event_t *e);
static void on_updown_next_click(lv_event_t *e);
void on_edit_focus(lv_event_t *e);
void validate_constraints(void);
static void on_preheat_toggle(lv_event_t *e);
static void on_delay_toggle(lv_event_t *e);
static void on_contain_toggle(lv_event_t *e);
static void on_delayset_focus(lv_event_t *e);
static void on_delayset_start_click(lv_event_t *e);
static void delayset_refresh_display(delayset_t *ds);
static void updown_set_apply_delay_label(updown_bbq_set_t *set);
static void on_delaycooking_cancel_click(lv_event_t *e);
static void rebuild_delaycooking(void);
static void delay_start_cook(void);
static void delay_cancel_to_stop_back(void);
static void on_sure_click(lv_event_t *e);
void cooking_timer_cb(lv_timer_t *timer);
static void topflag_update_visibility(void);
static void topflag_clock_cb(lv_timer_t *timer);
static void waitmenu_apply_clock(void);
static void waitmenu_clock_cache_reset(void);
static void color_exit_to_home(void);
static void jump_to_color_menu(void);
static void color_menu_rebuild(page_id_t child);
static void on_color_menu_next_click(lv_event_t *e);
static void jump_to_updown_bbq_complete(void);
static void jump_to_color_cookoing(void);
static void jump_to_color_complete(void);
static void on_color_start_click(lv_event_t *e);
void anim_bar_set_value(void *obj, int32_t v);
static void on_cook_stop_click(lv_event_t *e);
static void on_stop_start_click(lv_event_t *e);
static void on_stop_back_sure_click(lv_event_t *e);
static void on_updown_uptemp_click(lv_event_t *e);
static void on_updown_downtemp_click(lv_event_t *e);
static void on_updown_top_next_click(lv_event_t *e);
static void on_updown_low_next_click(lv_event_t *e);
static void on_stop_littal_click(lv_event_t *e);
static void jump_to_updown_bbq_menu_top(void);
static void jump_to_updown_bbq_menu_low(void);
static void on_stop_back_littal_click(lv_event_t *e);
static void jump_to_updown_bbq_stop(void);
static void jump_to_updown_bbq_stop_back(void);
static void stop_resume_cooking(void);
static void jump_to_color_stop(void);
static void jump_to_color_stop_back(void);
static void color_resume_cooking(void);
static void on_color_stop_click(lv_event_t *e);
static void on_color_stop_start_click(lv_event_t *e);
static void on_color_stop_back_sure_click(lv_event_t *e);
static void jump_to_updown_bbq_setting(void);
static void on_cook_setting_click(lv_event_t *e);
static void on_setting_edit_focus(lv_event_t *e);
static void on_setting_sure_click(lv_event_t *e);
static void update_setting_dir_icon(updown_bbq_setting_t *set);
void cook4menu_rebuild(page_id_t child);

/* updown_bbq setting 页进入时保存原始值，BACK 返回时恢复 */
static int updown_setting_saved_temp_up, updown_setting_saved_temp_down;
static int updown_setting_saved_hour, updown_setting_saved_min;
/* top_bbq setting 页保存值（在 nav_top_bbq.c 中定义） */
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



// ==============================
// 可编辑字段注册表
// ==============================

#define MAX_EDIT_FIELDS 8
static edit_field_t edit_fields[MAX_EDIT_FIELDS];
static int edit_count = 0;

void edit_clear(void)
{
    edit_count = 0;
}

void edit_register(lv_obj_t *label, lv_obj_t *ind_s, lv_obj_t *ind_l,
                   int *value, int min, int max, int step, const char *fmt)
{
    if (edit_count < MAX_EDIT_FIELDS) {
        edit_fields[edit_count].label = label;
        edit_fields[edit_count].ind_short = ind_s;
        edit_fields[edit_count].ind_long = ind_l;
        edit_fields[edit_count].value = value;
        edit_fields[edit_count].min = min;
        edit_fields[edit_count].max = max;
        edit_fields[edit_count].step = step;
        edit_fields[edit_count].fmt = fmt;
        edit_count++;
    }
}

static edit_field_t *find_edit_field(lv_obj_t *obj)
{
    for (int i = 0; i < edit_count; i++)
        if (edit_fields[i].label == obj)
            return &edit_fields[i];
    return NULL;
}

static void adjust_value(edit_field_t *f, int delta)
{
    int old_val = *f->value;
    int new_val = old_val + f->step * delta;

    /* 循环 */
    if (new_val > f->max) new_val = f->min;
    if (new_val < f->min) new_val = f->max;

    *f->value = new_val;
    lv_label_set_text_fmt(f->label, f->fmt, new_val);

    /* 温度线切换 */
    if (f->ind_short && f->ind_long) {
        lv_obj_add_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
        if (new_val < 100)
            lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
        lv_obj_invalidate(lv_scr_act());
    }

    /* dir 方向图标切换（上下烧烤菜单页） */
    validate_constraints();

    /* 设置页：上下温差 ≤ 20°C，超限则回弹 */
    if (current_group == g_updown_bbq_setting) {
        int diff = set_temp_up - set_temp_down;
        if (diff < 0) diff = -diff;
        updown_bbq_setting_t *set = updown_bbq_setting_get(&ui_manager);
        if (diff > 20) {
            int new_v;
            if (f->value == &set_temp_up)
                new_v = set_temp_up > set_temp_down ? set_temp_down - 20 : set_temp_down + 20;
            else
                new_v = set_temp_down > set_temp_up ? set_temp_up - 20 : set_temp_up + 20;
            if (new_v > 300) new_v = 300;
            if (new_v < 30) new_v = 30;
            *f->value = new_v;
            lv_label_set_text_fmt(f->label, f->fmt, new_v);
            if (f->ind_short && f->ind_long) {
                lv_obj_add_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
                if (new_v < 100)
                    lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
            }
        }
        update_setting_dir_icon(set);
    }
    /* 上层温度页：上下温差 ≤ 20°C，超限则循环 */
    if (current_group == g_updown_bbq_menu_top) {
        int diff = set_temp_up - set_temp_down;
        if (diff < 0) diff = -diff;
        if (diff > 20) {
            int new_v = set_temp_up > set_temp_down ? set_temp_down - 20 : set_temp_down + 20;
            if (new_v > 300) new_v = 300;
            if (new_v < 30) new_v = 30;
            *f->value = new_v;
            lv_label_set_text_fmt(f->label, f->fmt, new_v);
            if (f->ind_short && f->ind_long) {
                lv_obj_add_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
                if (new_v < 100)
                    lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
            }
            {
                updown_bbq_menu_top_t *m = updown_bbq_menu_top_get(&ui_manager);
                if (m) {
                    if (m->dir3) lv_obj_add_flag(m->dir3, LV_OBJ_FLAG_HIDDEN);
                    if (m->dir2) lv_obj_add_flag(m->dir2, LV_OBJ_FLAG_HIDDEN);
                    if (new_v < 100)
                        if (m->dir2) lv_obj_clear_flag(m->dir2, LV_OBJ_FLAG_HIDDEN);
                    else
                        if (m->dir3) lv_obj_clear_flag(m->dir3, LV_OBJ_FLAG_HIDDEN);
                }
            }
            lv_obj_invalidate(lv_scr_act());
        }
        {
            updown_bbq_menu_top_t *m2 = updown_bbq_menu_top_get(&ui_manager);
            if (m2) {
                if (m2->dir3) lv_obj_add_flag(m2->dir3, LV_OBJ_FLAG_HIDDEN);
                if (m2->dir2) lv_obj_add_flag(m2->dir2, LV_OBJ_FLAG_HIDDEN);
                if (set_temp_up < 100) { if (m2->dir2) lv_obj_clear_flag(m2->dir2, LV_OBJ_FLAG_HIDDEN); }
                else { if (m2->dir3) lv_obj_clear_flag(m2->dir3, LV_OBJ_FLAG_HIDDEN); }
            }
            lv_obj_invalidate(lv_scr_act());
        }
    }
    /* 下层温度页：上下温差 ≤ 20°C，超限则循环 */
    if (current_group == g_updown_bbq_menu_low) {
        int diff = set_temp_up - set_temp_down;
        if (diff < 0) diff = -diff;
        if (diff > 20) {
            int new_v = set_temp_down > set_temp_up ? set_temp_up - 20 : set_temp_up + 20;
            if (new_v > 300) new_v = 300;
            if (new_v < 30) new_v = 30;
            *f->value = new_v;
            lv_label_set_text_fmt(f->label, f->fmt, new_v);
            if (f->ind_short && f->ind_long) {
                lv_obj_add_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
                if (new_v < 100)
                    lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
            }
            {
                updown_bbq_menu_low_t *m = updown_bbq_menu_low_get(&ui_manager);
                if (m) {
                    if (m->dir3) lv_obj_add_flag(m->dir3, LV_OBJ_FLAG_HIDDEN);
                    if (m->dir2) lv_obj_add_flag(m->dir2, LV_OBJ_FLAG_HIDDEN);
                    if (new_v < 100)
                        if (m->dir2) lv_obj_clear_flag(m->dir2, LV_OBJ_FLAG_HIDDEN);
                    else
                        if (m->dir3) lv_obj_clear_flag(m->dir3, LV_OBJ_FLAG_HIDDEN);
                }
            }
            lv_obj_invalidate(lv_scr_act());
        }
        {
            updown_bbq_menu_low_t *m2 = updown_bbq_menu_low_get(&ui_manager);
            if (m2) {
                if (m2->dir3) lv_obj_add_flag(m2->dir3, LV_OBJ_FLAG_HIDDEN);
                if (m2->dir2) lv_obj_add_flag(m2->dir2, LV_OBJ_FLAG_HIDDEN);
                if (set_temp_down < 100) { if (m2->dir2) lv_obj_clear_flag(m2->dir2, LV_OBJ_FLAG_HIDDEN); }
                else { if (m2->dir3) lv_obj_clear_flag(m2->dir3, LV_OBJ_FLAG_HIDDEN); }
            }
            lv_obj_invalidate(lv_scr_act());
        }
    }
    /* 顶部烧烤设置页 dir/icon 即时更新 */
    if (current_group == g_top_bbq_setting) {
        top_bbq_setting_t *set = top_bbq_setting_get(&ui_manager);
        update_top_bbq_dir_icon(set);
    }

    /* 底部烧烤设置页 dir/icon 即时更新 */
    if (current_group == g_bottom_bbq_setting) {
        bottom_bbq_setting_t *set = bottom_bbq_setting_get(&ui_manager);
        update_bottom_bbq_dir_icon(set);
    }

    /* 热风烧烤设置页 dir/icon 即时更新 */
    if (current_group == g_hot_bbq_setting) {
        hot_bbq_setting_t *set = hot_bbq_setting_get(&ui_manager);
        update_hot_bbq_dir_icon(set);
    }
    if (current_group == g_hotwind_bbq_setting) {
        hotwind_bbq_setting_t *set = hotwind_bbq_setting_get(&ui_manager);
        update_hotwind_bbq_dir_icon(set);
    }
    if (current_group == g_save_bbq_setting) {
        save_bbq_setting_t *set = save_bbq_setting_get(&ui_manager);
        update_save_bbq_dir_icon(set);
    }
    if (current_group == g_central_bbq_setting) {
        central_bbq_setting_t *set = central_bbq_setting_get(&ui_manager);
        update_central_bbq_dir_icon(set);
    }
    if (current_group == g_windchange_bbq_setting) {
        windchange_bbq_setting_t *set = windchange_bbq_setting_get(&ui_manager);
        update_windchange_bbq_dir_icon(set);
    }
    if (current_group == g_cookie_setting) {
        cookie_setting_t *set = cookie_setting_get(&ui_manager);
        update_cookie_dir_icon(set);
    }
    if (current_group == g_west_setting) {
        west_setting_t *set = west_setting_get(&ui_manager);
        update_west_dir_icon(set);
    }
    if (current_group == g_pizza_setting) {
        pizza_setting_t *set = pizza_setting_get(&ui_manager);
        update_pizza_dir_icon(set);
    }
    if (current_group == g_menu_cook_setting) {
        menu_setting_t *set = menu_setting_get(&ui_manager);
        update_menu_dir_icon(set);
    }

    /* 空气炸设置页 dir/icon 即时更新 */
    if (current_group == g_air_setting) {
        air_setting_t *set = air_setting_get(&ui_manager);
        update_air_dir_icon(set);
    }

    /* 披萨设置页 dir/icon 即时更新 */
    if (current_group == g_pizza_2_setting) {
        pizza_2_setting_t *set = pizza_2_setting_get(&ui_manager);
        update_pizza_2_dir_icon(set);
    }

    /* 慢煮设置页 dir/icon 即时更新 */
    if (current_group == g_slowcook_setting) {
        slowcook_setting_t *set = slowcook_setting_get(&ui_manager);
        update_slowcook_dir_icon(set);
    }

    /* 解冻设置页 dir/icon 即时更新 */
    if (current_group == g_unfrozen_setting) {
        unfrozen_setting_t *set = unfrozen_setting_get(&ui_manager);
        update_unfrozen_dir_icon(set);
    }

    /* 发酵设置页 dir/icon 即时更新 */
    if (current_group == g_rising_setting) {
        rising_setting_t *set = rising_setting_get(&ui_manager);
        update_rising_dir_icon(set);
    }

    /* 干果设置页 dir/icon 即时更新 */
    if (current_group == g_corn_setting) {
        corn_setting_t *set = corn_setting_get(&ui_manager);
        update_corn_dir_icon(set);
    }

    /* 保温设置页 dir/icon 即时更新 */
    if (current_group == g_heatcontain_setting) {
        heatcontain_setting_t *set = heatcontain_setting_get(&ui_manager);
        update_heatcontain_dir_icon(set);
    }
}

void validate_constraints(void)
{
    /* 找到 minute 字段 */
    edit_field_t *min_field = NULL;
    for (int i = 0; i < edit_count; i++) {
        if (edit_fields[i].value == &set_min) {
            min_field = &edit_fields[i];
            break;
        }
    }
    if (!min_field) return;

    /* 找到 hour 字段，取其 max 作为动态最大小时 */
    int max_h = 4;
    for (int i = 0; i < edit_count; i++) {
        if (edit_fields[i].value == &set_hour) {
            max_h = edit_fields[i].max;
            break;
        }
    }

    /* 根据 hour 动态调整 minute 的循环范围 */
    if (set_hour == 0) {
        min_field->min = 1;    // hour=0时，最少1分钟（调试用）
        min_field->max = 59;
    } else if (set_hour == max_h) {
        min_field->min = 0;    // 最大小时时 minute 不可调
        min_field->max = 0;
    } else {
        min_field->min = 0;    // 0-59 正常循环
        min_field->max = 59;
    }

    /* 纠正 minute 值（如果当前值超出新范围） */
    if (set_hour == 0 && set_min < 1) {
        set_min = 1;
        lv_label_set_text_fmt(min_field->label, min_field->fmt, set_min);
    } else if (set_hour == max_h && set_min != 0) {
        set_min = 0;
        lv_label_set_text_fmt(min_field->label, min_field->fmt, set_min);
    }
}

// ==============================
// 公共 helper
// ==============================

static void set_status_label(lv_obj_t *label, int temp, int hour, int min)
{
    if (hour == 0)
        lv_label_set_text_fmt(label, "| 上下烧烤 | %d℃ | %02d分钟", temp, min);
    else
        lv_label_set_text_fmt(label, "| 上下烧烤 | %d℃ | %d小时%02d分钟", temp, hour, min);
}

static void set_status_label_min(lv_obj_t *label, int temp_up, int temp_down, int hour, int min)
{
    int t = temp_up < temp_down ? temp_up : temp_down;
    set_status_label(label, t, hour, min);
}

void set_time_label(lv_obj_t *label, int remaining_ms)
{
    if (remaining_ms < 0) remaining_ms = 0;
    int total_sec = remaining_ms / 1000;
    int h = total_sec / 3600;
    int m = (total_sec % 3600) / 60;
    int s = total_sec % 60;
    lv_label_set_text_fmt(label, "%02d:%02d:%02d", h, m, s);
}

void set_bar_progress(lv_obj_t *bar, int64_t elapsed_ms, int total_ms)
{
    if (total_ms <= 0) return;
    int progress = (int)(elapsed_ms * 100 / total_ms);
    if (progress > 100) progress = 100;
    if (progress < 0) progress = 0;
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, progress, LV_ANIM_OFF);
}

void apply_toggle_state(lv_obj_t *btn_off, lv_obj_t *btn_on, int on)
{
    if (on) {
        lv_obj_add_flag(btn_off, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(btn_on, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(btn_off, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_on, LV_OBJ_FLAG_HIDDEN);
    }
}

// 清除按钮数组的 FOCUSED 状态
void clear_focus_states(lv_obj_t **btns, int count)
{
    for (int i = 0; i < count; i++)
        if (btns[i])
            lv_obj_clear_state(btns[i], LV_STATE_FOCUSED);
}

// updown_bbq_set 温度组件显隐（2 位 / 3 位自动切换）
static void setup_set_temp_display(updown_bbq_set_t *set)
{
    lv_obj_add_flag(set->up2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->up2_dir_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->up2_icon_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->down2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->down2_dir_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->down2_icon_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->up3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->up3_dir_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->up3_icon_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->down3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->down3_dir_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->down3_icon_label, LV_OBJ_FLAG_HIDDEN);

    if (set_temp_up < 100) {
        lv_label_set_text_fmt(set->up2_tempnum_label, "%d", set_temp_up);
        lv_obj_clear_flag(set->up2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->up2_dir_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->up2_icon_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text_fmt(set->up3_tempnum_label, "%d", set_temp_up);
        lv_obj_clear_flag(set->up3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->up3_dir_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->up3_icon_label, LV_OBJ_FLAG_HIDDEN);
    }
    if (set_temp_down < 100) {
        lv_label_set_text_fmt(set->down2_tempnum_label, "%d", set_temp_down);
        lv_obj_clear_flag(set->down2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->down2_dir_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->down2_icon_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text_fmt(set->down3_tempnum_label, "%d", set_temp_down);
        lv_obj_clear_flag(set->down3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->down3_dir_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->down3_icon_label, LV_OBJ_FLAG_HIDDEN);
    }
}

// ==============================
// 页面栈操作
// ==============================

// 跳转子页前调用，记录"当前页"到栈顶
void page_push(page_id_t id)
{
    if (depth < MAX_STACK) {
        page_stack[depth++] = id;  // 写入栈顶，depth 自增
        printf("[nav] page push: depth=%d id=%d\n", depth, id);
    } else {
        printf("[nav] ERROR: page_stack overflow! depth=%d id=%d\n", depth, id);
    }
    topflag_update_visibility();
}

// 收到 KEY21 时调用，回到上一页
// 步骤：
//   ① 根页保护（depth≤1 说明只有根页，不能 pop）
//   ② depth-- 丢弃栈顶（"回到上一页"）
//   ③ child = 刚丢弃的那一页（即"从哪个页面回来的"）
//   ④ prev = 栈顶之下的那页（即"要回到哪个页面"）
//   ⑤ lv_obj_clean 清当前屏
//   ⑥ 根据 prev 重建对应页面的 UI + group
//   ⑦ 根据 child 恢复焦点到进入子页前的位置
//   ⑧ lv_scr_load_anim 显示重建后的页面
void page_pop(void)
{
    /* ① 根页保护 */
    if (depth <= 1) {
        printf("[nav] at root, cannot pop\n");
        return;
    }

    /* ②~④ 算出 child(子页) 和 prev(父页) */
    depth--;
    edit_clear();   /* 离开当前页清编辑注册表,防止悬空 label 指针残留(UAF) */
    page_id_t child = page_stack[depth];
    page_id_t prev = page_stack[depth - 1];
    printf("[nav] page pop: depth=%d, back to id=%d (from id=%d)\n", depth, prev, child);

    /* 页面栈变化后立即刷新 topflag 显隐（等值栈顶已是目标页） */
    topflag_update_visibility();

    /* 从 stop_back 确认页退出时清除暂停确认状态（防止残留导致 cooking 定时器走错分支） */
    if (g_on_stop_back) {
        switch (child) {
        case PAGE_UPDOWN_BBQ_STOP_BACK:
        case PAGE_TOP_BBQ_STOP_BACK:
        case PAGE_BOTTOM_BBQ_STOP_BACK:
        case PAGE_HOT_BBQ_STOP_BACK:
        case PAGE_HOTWIND_BBQ_STOP_BACK:
        case PAGE_SAVE_BBQ_STOP_BACK:
        case PAGE_CENTRAL_BBQ_STOP_BACK:
        case PAGE_WINDCHANGE_BBQ_STOP_BACK:
        case PAGE_AIR_STOP_BACK:
        case PAGE_PIZZA_2_STOP_BACK:
        case PAGE_SLOWCOOK_STOP_BACK:
        case PAGE_UNFROZEN_STOP_BACK:
        case PAGE_RISING_STOP_BACK:
        case PAGE_CORN_STOP_BACK:
        case PAGE_HEATCONTAIN_STOP_BACK:
        case PAGE_UPDOWN_BBQ_STOP_BACK_PROBE:
        case PAGE_HOT_BBQ_STOP_BACK_PROBE:
        case PAGE_BOTTOM_BBQ_STOP_BACK_PROBE:
        case PAGE_SLOWCOOK_STOP_BACK_PROBE:
        case PAGE_COOKIE_STOP_BACK:
        case PAGE_WEST_STOP_BACK:
        case PAGE_PIZZA_STOP_BACK:
        case PAGE_MENU_COOK_STOP_BACK:
        case PAGE_LASAGNA_STOP_BACK:
        case PAGE_STRUDEL_STOP_BACK:
        case PAGE_BREAD_STOP_BACK:
        case PAGE_PIZZA3_STOP_BACK:
        case PAGE_CHIP_STOP_BACK:
        case PAGE_CUSTOM_STOP_BACK:
        case PAGE_PREHEAT_STOP_BACK:
        case PAGE_WATER_CLEAN_STOP_BACK:
        case PAGE_HOTCLEANSAVE_STOP_BACK:
        case PAGE_HOTCLEANMIDDLE_STOP_BACK:
        case PAGE_HOTCLEANHIGH_STOP_BACK:
        case PAGE_COLOR_STOP_BACK:
            g_on_stop_back = 0;
            g_delay_cancel_btn = 0;
            break;
        default:
            break;
        }
    }

    /* ⑤ 清当前屏 */
    lv_obj_clean(lv_scr_act());

    /* ⑥ 重建父页 UI + group */
    switch (prev) {
    case PAGE_MAJOR_MENU:
        major_menu_create(&ui_manager);
        groups_create();
        bind_events();   // 新按钮需要重新绑定点击事件
        current_group = g_major_menu;

        /* ⑦ 根据 child 恢复焦点 */
        {
            major_menu_t *major = major_menu_get(&ui_manager);
            if (major) {
                if (child == PAGE_COOKMENU && major->cook_button)
                    lv_group_focus_obj(major->cook_button);
                else if (child == PAGE_SPECIAL_MENU && major->special_button)
                    lv_group_focus_obj(major->special_button);
                else if (child == PAGE_COOK4_MENU && major->cook4_button)
                    lv_group_focus_obj(major->cook4_button);
            }
        }
        lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to major_menu\n");
        break;

    case PAGE_COOKMENU:
        cookmenu_create(&ui_manager);
        {
            cookmenu_t *cook = cookmenu_get(&ui_manager);
            if (cook) {
                lv_obj_t *btns[] = {
                    cook->up_down_button, cook->top_bbq_button, cook->hot_bbq_button,
                    cook->hot_wind_button, cook->save_button, cook->bottom_button,
                    cook->central_button, cook->windchange_buttonn, cook->preheater_button,
                };
                /* 销毁旧 group + 创建新 group（旧按钮已被 lv_obj_clean 销毁） */
                if (g_cookmenu) lv_group_del(g_cookmenu);
                g_cookmenu = group_create_for_page(btns, sizeof(btns) / sizeof(btns[0]));
            }
            current_group = g_cookmenu;

            /* 根据 child 恢复焦点 */
            if (child == PAGE_UPDOWN_BBQ_MENU && cook->up_down_button)
                lv_group_focus_obj(cook->up_down_button);
            else if (child == PAGE_TOP_BBQ_MENU && cook->top_bbq_button)
                lv_group_focus_obj(cook->top_bbq_button);
            else if (child == PAGE_BOTTOM_BBQ_MENU && cook->bottom_button)
                lv_group_focus_obj(cook->bottom_button);
            else if (child == PAGE_HOT_BBQ_MENU && cook->hot_bbq_button)
                lv_group_focus_obj(cook->hot_bbq_button);
            else if (child == PAGE_HOTWIND_BBQ_MENU && cook->hot_wind_button)
                lv_group_focus_obj(cook->hot_wind_button);
            else if (child == PAGE_SAVE_BBQ_MENU && cook->save_button)
                lv_group_focus_obj(cook->save_button);
            else if (child == PAGE_CENTRAL_BBQ_MENU && cook->central_button)
                lv_group_focus_obj(cook->central_button);
            else if (child == PAGE_WINDCHANGE_BBQ_MENU && cook->windchange_buttonn)
                lv_group_focus_obj(cook->windchange_buttonn);
            else if (child == PAGE_PREHEAT_MENU && cook->preheater_button)
                lv_group_focus_obj(cook->preheater_button);

            /* 新按钮需要重新绑定事件 */
            if (cook && cook->up_down_button)
                lv_obj_add_event_cb(cook->up_down_button, on_cook_updown_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->top_bbq_button)
                lv_obj_add_event_cb(cook->top_bbq_button, on_top_bbq_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->bottom_button)
                lv_obj_add_event_cb(cook->bottom_button, on_bottom_bbq_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->hot_bbq_button)
                lv_obj_add_event_cb(cook->hot_bbq_button, on_hot_bbq_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->hot_wind_button)
                lv_obj_add_event_cb(cook->hot_wind_button, on_hotwind_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->save_button)
                lv_obj_add_event_cb(cook->save_button, on_save_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->central_button)
                lv_obj_add_event_cb(cook->central_button, on_central_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->windchange_buttonn)
                lv_obj_add_event_cb(cook->windchange_buttonn, on_windchange_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->preheater_button)
                lv_obj_add_event_cb(cook->preheater_button, on_preheat_click,
                                    LV_EVENT_CLICKED, NULL);

            /* 退出 updown_bbq 流程后清空临时值 */
            set_temp = 180;
            set_hour = 0;
            set_min = 30;
            g_send.cook_mode = MODE_NONE;
        }
        lv_scr_load_anim(cookmenu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to cookmenu\n");
        break;

    case PAGE_SPECIAL_MENU:
        special_menu_create(&ui_manager);
        {
            special_menu_t *sp = special_menu_get(&ui_manager);
            if (sp) {
                lv_obj_t *btns[] = {
                    sp->air_button, sp->piza_button, sp->frozen_cook_button,
                    sp->slow_cook_button, sp->unfrozen_button, sp->rising_button,
                    sp->corn_button, sp->heat_contain_button, sp->some_cook_button,
                };
                if (g_special_menu) lv_group_del(g_special_menu);
                g_special_menu = group_create_for_page(btns, sizeof(btns) / sizeof(btns[0]));

                lv_obj_add_event_cb(sp->air_button, on_air_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->piza_button, on_pizza_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->slow_cook_button, on_slowcook_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->unfrozen_button, on_unfrozen_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->rising_button, on_rising_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->corn_button, on_corn_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->heat_contain_button, on_heatcontain_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->frozen_cook_button, on_frozen_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->some_cook_button, on_some_cook_click,
                                    LV_EVENT_CLICKED, NULL);

                if (child == PAGE_AIR_MENU && sp->air_button)
                    lv_group_focus_obj(sp->air_button);
                else if (child == PAGE_PIZZA_2_MENU && sp->piza_button)
                    lv_group_focus_obj(sp->piza_button);
                else if (child == PAGE_SLOWCOOK_MENU && sp->slow_cook_button)
                    lv_group_focus_obj(sp->slow_cook_button);
                else if (child == PAGE_UNFROZEN_MENU && sp->unfrozen_button)
                    lv_group_focus_obj(sp->unfrozen_button);
                else if (child == PAGE_RISING_MENU && sp->rising_button)
                    lv_group_focus_obj(sp->rising_button);
                else if (child == PAGE_CORN_MENU && sp->corn_button)
                    lv_group_focus_obj(sp->corn_button);
                else if (child == PAGE_HEATCONTAIN_MENU && sp->heat_contain_button)
                    lv_group_focus_obj(sp->heat_contain_button);
                else if (child == PAGE_FROZEN_COOK && sp->frozen_cook_button)
                    lv_group_focus_obj(sp->frozen_cook_button);
                else if (child == PAGE_SOMECOOK && sp->some_cook_button)
                    lv_group_focus_obj(sp->some_cook_button);
            }
            current_group = g_special_menu;
        }
        lv_scr_load_anim(special_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        g_send.cook_mode = MODE_NONE;
        printf("[nav] back to special_menu\n");
        break;

    case PAGE_SOMECOOK:
        somecook_rebuild(child);
        break;

    case PAGE_UPDOWN_BBQ_MENU:
        updown_bbq_menu_create(&ui_manager);
        {
            updown_bbq_menu_t *bbq = updown_bbq_menu_get(&ui_manager);
            if (bbq) {
                lv_obj_t *btns[] = {
                    bbq->tempnum_label, bbq->hournum_label, bbq->minnum_label,
                    bbq->next_button,
                };
                if (g_updown_bbq_menu) lv_group_del(g_updown_bbq_menu);
                g_updown_bbq_menu = group_create_for_page(btns, 4);

                /* 注册编辑字段 */
                edit_clear();
                edit_register(bbq->tempnum_label, bbq->templine_short, bbq->temeline_long,
                              &set_temp, 30, 300, 5, "%d");
                edit_register(bbq->hournum_label, bbq->hourline, NULL,
                              &set_hour, 0, 4, 1, "%02d");
                edit_register(bbq->minnum_label, bbq->minline, NULL,
                              &set_min, 0, 59, 1, "%02d");

                /* 绑定 focus 高亮 */
                lv_obj_add_event_cb(bbq->tempnum_label, on_edit_focus, LV_EVENT_FOCUSED, NULL);
                lv_obj_add_event_cb(bbq->hournum_label, on_edit_focus, LV_EVENT_FOCUSED, NULL);
                lv_obj_add_event_cb(bbq->minnum_label, on_edit_focus, LV_EVENT_FOCUSED, NULL);
                lv_obj_add_event_cb(bbq->next_button, on_edit_focus, LV_EVENT_FOCUSED, NULL);

                /* 初始化数值显示 */
                lv_label_set_text_fmt(bbq->tempnum_label, "%d", set_temp);
                lv_label_set_text_fmt(bbq->hournum_label, "%02d", set_hour);
                lv_label_set_text_fmt(bbq->minnum_label, "%02d", set_min);

                /* 初始显示温度线 */
                lv_obj_add_flag(bbq->templine_short, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(bbq->temeline_long, LV_OBJ_FLAG_HIDDEN);
                if (set_temp < 100)
                    lv_obj_clear_flag(bbq->templine_short, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_clear_flag(bbq->temeline_long, LV_OBJ_FLAG_HIDDEN);

                /* 新按钮重新绑定事件 */
                if (bbq->next_button)
                    lv_obj_add_event_cb(bbq->next_button, on_updown_next_click,
                                        LV_EVENT_CLICKED, NULL);

                /* 初始化约束（设置 min/max 匹配当前 hour） */
                validate_constraints();

                /* 根据 child 恢复焦点 */
                if (bbq->next_button)
                    lv_group_focus_obj(bbq->next_button);
            }
            current_group = g_updown_bbq_menu;
        }
        lv_scr_load_anim(updown_bbq_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to updown_bbq_menu\n");
        break;

    case PAGE_UPDOWN_BBQ_COOKING:
        if (child == PAGE_UPDOWN_BBQ_COMPLETE)
            goto pop_to_major_menu;
        if (child == PAGE_UPDOWN_BBQ_SETTING) {
            /* 恢复进入 setting 前的原始值 */
            set_temp_up = updown_setting_saved_temp_up;
            set_temp_down = updown_setting_saved_temp_down;
            set_hour = updown_setting_saved_hour;
            set_min = updown_setting_saved_min;
            /* 从设置页返回 → 重建 cooking，恢复定时器 */
            updown_bbq_cooking_create(&ui_manager);
            {
                updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
                if (cook) {
                    lv_obj_t *btns[] = { cook->stop_button, cook->little_button };
                    if (g_updown_bbq_cooking) lv_group_del(g_updown_bbq_cooking);
                    g_updown_bbq_cooking = group_create_for_page(btns, 2);
                    lv_obj_add_event_cb(cook->stop_button, on_cook_stop_click,
                                        LV_EVENT_CLICKED, NULL);
                    lv_obj_add_event_cb(cook->little_button, on_cook_setting_click,
                                        LV_EVENT_CLICKED, NULL);

                    set_status_label_min(cook->updown_label, set_temp_up, set_temp_down, set_hour, set_min);
                    uint32_t elapsed = lv_tick_get() - cook_start_time;
                    int elapsed_sec = (elapsed + 500) / 1000;
                    int total_sec = cook_total_ms / 1000;
                    int remaining_sec = total_sec - elapsed_sec;
                    if (remaining_sec < 0) remaining_sec = 0;
                    int h = remaining_sec / 3600;
                    int m = (remaining_sec % 3600) / 60;
                    int s = remaining_sec % 60;
                    lv_label_set_text_fmt(cook->time_label, "%02d:%02d:%02d", h, m, s);
                    lv_bar_set_range(cook->bar, 0, 100);
                    int progress = (int)((int64_t)elapsed * 100 / (cook_total_ms ? cook_total_ms : 1));
                    if (progress > 100) progress = 100;
                    lv_bar_set_value(cook->bar, progress, LV_ANIM_OFF);
                    lv_anim_t a;
                    lv_anim_init(&a);
                    lv_anim_set_var(&a, cook->bar);
                    lv_anim_set_exec_cb(&a, anim_bar_set_value);
                    lv_anim_set_values(&a, progress, 100);
                    lv_anim_set_time(&a, ((int)(cook_total_ms - (int)elapsed) < 0) ? 0 : (cook_total_ms - (int)elapsed));
                    lv_anim_start(&a);
                }
                current_group = g_updown_bbq_cooking;
            }
            lv_scr_load_anim(updown_bbq_cooking_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0,
                             ui_manager.auto_del);
            printf("[nav] back from setting -> updown_bbq_cooking\n");
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                int rem = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
                g_send.remaining_ms = rem;
            }
        } else if (child == PAGE_UPDOWN_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            updown_bbq_cooking_create(&ui_manager);
            {
                updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
                if (cook) {
                    lv_obj_t *btns[] = { cook->stop_button, cook->little_button };
                    if (g_updown_bbq_cooking) lv_group_del(g_updown_bbq_cooking);
                    g_updown_bbq_cooking = group_create_for_page(btns, 2);
                    lv_obj_add_event_cb(cook->stop_button, on_cook_stop_click,
                                        LV_EVENT_CLICKED, NULL);
                    lv_obj_add_event_cb(cook->little_button, on_cook_setting_click,
                                        LV_EVENT_CLICKED, NULL);

                    set_status_label_min(cook->updown_label, set_temp_up, set_temp_down, set_hour, set_min);
                    uint32_t elapsed = lv_tick_get() - cook_start_time;
                    int elapsed_sec = (elapsed + 500) / 1000;
                    int total_sec = cook_total_ms / 1000;
                    int remaining_sec = total_sec - elapsed_sec;
                    if (remaining_sec < 0) remaining_sec = 0;
                    int h = remaining_sec / 3600;
                    int m = (remaining_sec % 3600) / 60;
                    int s = remaining_sec % 60;
                    lv_label_set_text_fmt(cook->time_label, "%02d:%02d:%02d", h, m, s);
                    lv_bar_set_range(cook->bar, 0, 100);
                    int progress = stop_back_progress(elapsed, cook_total_ms);
                    if (progress > 100) progress = 100;
                    lv_bar_set_value(cook->bar, progress, LV_ANIM_OFF);
                    printf("[debug] SB->COOK: elapsed=%u timer=%p prog=%d\n", elapsed, (void*)cook_timer, progress);
                    lv_anim_t a;
                    lv_anim_init(&a);
                    lv_anim_set_var(&a, cook->bar);
                    lv_anim_set_exec_cb(&a, anim_bar_set_value);
                    lv_anim_set_values(&a, progress, 100);
                    lv_anim_set_time(&a, ((int)(cook_total_ms - (int)elapsed) < 0) ? 0 : (cook_total_ms - (int)elapsed));
                    lv_anim_start(&a);
                }
                current_group = g_updown_bbq_cooking;
            }
            lv_scr_load_anim(updown_bbq_cooking_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0,
                             ui_manager.auto_del);
            printf("[nav] back from stop_back -> updown_bbq_cooking\n");
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                int rem = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
                g_send.remaining_ms = rem;
            }
        } else {
            goto rebuild_updown_bbq_set;
        }
        break;

    case PAGE_UPDOWN_BBQ_STOP:
        g_on_stop_back = 0;
        g_stop_back_complete = NULL;
        if (child == PAGE_UPDOWN_BBQ_SETTING) {
            set_temp_up = updown_setting_saved_temp_up;
            set_temp_down = updown_setting_saved_temp_down;
            set_hour = updown_setting_saved_hour;
            set_min = updown_setting_saved_min;
        }
        updown_bbq_stop_create(&ui_manager);
        {
            updown_bbq_stop_t *stop = updown_bbq_stop_get(&ui_manager);
            if (stop) {
                lv_obj_t *btns[] = { stop->start_button, stop->little_button };
                if (g_updown_bbq_stop) lv_group_del(g_updown_bbq_stop);
                g_updown_bbq_stop = group_create_for_page(btns, 2);
                lv_obj_add_event_cb(stop->start_button, on_stop_start_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(stop->little_button, on_cook_setting_click,
                                    LV_EVENT_CLICKED, NULL);

                /* 同步显示（与暂停时刻一致） */
                int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
                int total_sec = cook_total_ms / 1000;
                int remaining_sec = total_sec - elapsed_sec;
                if (remaining_sec < 0) remaining_sec = 0;
                int h = remaining_sec / 3600;
                int m = (remaining_sec % 3600) / 60;
                int s = remaining_sec % 60;
                lv_label_set_text_fmt(stop->time_label, "%02d:%02d:%02d", h, m, s);

                set_status_label_min(stop->statu_label, set_temp_up, set_temp_down, set_hour, set_min);

                lv_bar_set_range(stop->bar_1, 0, 100);
                if (cook_bar_saved > 100) cook_bar_saved = 100;
                lv_bar_set_value(stop->bar_1, cook_bar_saved, LV_ANIM_OFF);
            }
            current_group = g_updown_bbq_stop;
        }
        lv_scr_load_anim(updown_bbq_stop_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to updown_bbq_stop\n");
        g_send.iface_status = IFACE_PAUSE;
        break;

    case PAGE_UPDOWN_BBQ_STOP_BACK:
        g_on_stop_back = 1;
        g_stop_back_complete = jump_to_updown_bbq_complete;
        updown_bbq_stop_back_create(&ui_manager);
        {
            updown_bbq_stop_back_t *back = updown_bbq_stop_back_get(&ui_manager);
            if (back) {
                lv_obj_t *btns[] = { back->sure_button };
                if (g_updown_bbq_stop_back) lv_group_del(g_updown_bbq_stop_back);
                g_updown_bbq_stop_back = group_create_for_page(btns, 1);
                lv_obj_add_event_cb(back->sure_button, on_stop_back_sure_click,
                                    LV_EVENT_CLICKED, NULL);

                /* 同步 statu_label / bar_2（从耗时计算进度） */
                set_status_label_min(back->statu_label, set_temp_up, set_temp_down, set_hour, set_min);

                uint32_t elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
                int p = stop_back_progress(elapsed, cook_total_ms);
                if (p > 100) p = 100;
                lv_bar_set_range(back->bar_2, 0, 100);
                lv_bar_set_value(back->bar_2, p, LV_ANIM_OFF);

                if (g_send.iface_status == IFACE_COOKING)
                    lv_label_set_text(back->label_8, "烹饪中...");

                if (g_complete_to_stop_back) {
                    g_complete_to_stop_back = 0;
                    lv_label_set_text(back->label_8, "已完成");
                    lv_bar_set_value(back->bar_2, 100, LV_ANIM_OFF);
                }
            }
            current_group = g_updown_bbq_stop_back;
        }
        lv_scr_load_anim(updown_bbq_stop_back_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to updown_bbq_stop_back\n");
        break;

    case PAGE_UPDOWN_BBQ_SETTING:
        updown_bbq_setting_create(&ui_manager);
        {
            updown_bbq_setting_t *set = updown_bbq_setting_get(&ui_manager);
            if (set) {
                lv_obj_t *btns[] = {
                    set->tempup_label, set->tempdown_label,
                    set->hour_label, set->min_label,
                    set->sure_button,
                };
                if (g_updown_bbq_setting) lv_group_del(g_updown_bbq_setting);
                g_updown_bbq_setting = group_create_for_page(btns, 5);

                edit_clear();
                edit_register(set->tempup_label, set->shortup_templine_img, set->longup_templine_img,
                              &set_temp_up, 30, 300, 5, "%d");
                edit_register(set->tempdown_label, set->shordown_templine_img, set->longdown_templine_img,
                              &set_temp_down, 30, 300, 5, "%d");
                edit_register(set->hour_label, set->hourline_img, NULL,
                              &set_hour, 0, 4, 1, "%02d");
                edit_register(set->min_label, set->minline_label, NULL,
                              &set_min, 0, 59, 1, "%02d");

                lv_obj_add_event_cb(set->tempup_label, on_setting_edit_focus,
                                    LV_EVENT_FOCUSED, NULL);
                lv_obj_add_event_cb(set->tempdown_label, on_setting_edit_focus,
                                    LV_EVENT_FOCUSED, NULL);
                lv_obj_add_event_cb(set->hour_label, on_setting_edit_focus,
                                    LV_EVENT_FOCUSED, NULL);
                lv_obj_add_event_cb(set->min_label, on_setting_edit_focus,
                                    LV_EVENT_FOCUSED, NULL);
                lv_obj_add_event_cb(set->sure_button, on_setting_edit_focus,
                                    LV_EVENT_FOCUSED, NULL);
                lv_obj_add_event_cb(set->sure_button, on_setting_sure_click,
                                    LV_EVENT_CLICKED, NULL);

                uint32_t elapsed = lv_tick_get() - cook_start_time;
                int elapsed_sec = (elapsed + 500) / 1000;
                int total_sec = cook_total_ms / 1000;
                int remaining_sec = total_sec - elapsed_sec;
                if (remaining_sec < 0) remaining_sec = 0;
                int h = remaining_sec / 3600;
                int m = (remaining_sec % 3600) / 60;
                int s = remaining_sec % 60;
                if (g_send.iface_status == IFACE_COMPLETE) { h = m = s = 0; }
                lv_label_set_text_fmt(set->time_label, "%02d:%02d:%02d", h, m, s);
                lv_label_set_text_fmt(set->hour_label, "%02d", set_hour);
                lv_label_set_text_fmt(set->min_label, "%02d", set_min);
                lv_label_set_text_fmt(set->tempup_label, "%d", set_temp_up);
                lv_label_set_text_fmt(set->tempdown_label, "%d", set_temp_down);

                lv_obj_add_flag(set->shortup_templine_img, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->longup_templine_img, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->shordown_templine_img, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->longdown_templine_img, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->hourline_img, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->minline_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->dirup2_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->dirup3_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->icon2_label1, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->icon3_label1, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->dirdown2_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->dirdown3_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->icon2_label2, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->icon3_label2, LV_OBJ_FLAG_HIDDEN);

                lv_group_focus_obj(set->tempup_label);

                update_setting_dir_icon(set);
            }
            current_group = g_updown_bbq_setting;
        }
        lv_scr_load_anim(updown_bbq_setting_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to updown_bbq_setting\n");
        break;

    case PAGE_UPDOWN_BBQ_COMPLETE:
        if (child == PAGE_UPDOWN_BBQ_SETTING) {
            set_temp_up = updown_setting_saved_temp_up;
            set_temp_down = updown_setting_saved_temp_down;
            set_hour = updown_setting_saved_hour;
            set_min = updown_setting_saved_min;
        }
        if (child == PAGE_UPDOWN_BBQ_SETTING || child == PAGE_UPDOWN_BBQ_STOP_BACK) {
            updown_bbq_complete_create(&ui_manager);
            updown_bbq_complete_t *done = updown_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little_button };
                if (g_updown_bbq_complete) lv_group_del(g_updown_bbq_complete);
                g_updown_bbq_complete = group_create_for_page(btns, 1);
                lv_obj_add_event_cb(done->little_button, on_cook_setting_click,
                                    LV_EVENT_CLICKED, NULL);
                set_status_label_min(done->statu_label, set_temp_up, set_temp_down, set_hour, set_min);
                lv_bar_set_value(done->bar_3, 100, LV_ANIM_OFF);
                /* 保温场景（stop_back 返回）：恢复"保温中..."显示，计时继续 */
                if (g_keepwarm_active)
                    lv_label_set_text(done->complete_label, "保温中...");
                lv_group_focus_obj(done->little_button);
            }
            current_group = g_updown_bbq_complete;
            lv_scr_load_anim(updown_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            printf("[keepwarm] page_pop complete rebuild: child=%d active=%d sec=%d iface=%d contain=%d timer=%p\n",
                   child, g_keepwarm_active, g_keepwarm_sec, g_send.iface_status,
                   contain_on, (void *)cook_timer);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_UPDOWN_BBQ_MENU_PROBE:
        updown_bbq_probe_rebuild_menu(child);
        break;

    case PAGE_UPDOWN_BBQ_SET_PROBE:
        updown_bbq_probe_rebuild_set(child);
        break;

    case PAGE_UPDOWN_BBQ_COOKING_PROBE:
        if (child == PAGE_UPDOWN_BBQ_COMPLETE_PROBE)
            goto pop_to_major_menu;
        if (child == PAGE_UPDOWN_BBQ_STOP_BACK_PROBE) {
            g_on_stop_back = 0;
            updown_bbq_probe_rebuild_cooking(child);
        } else {
            updown_bbq_probe_rebuild_cooking(child);
        }
        break;

    case PAGE_UPDOWN_BBQ_STOP_PROBE:
        updown_bbq_probe_rebuild_stop();
        break;

    case PAGE_UPDOWN_BBQ_STOP_BACK_PROBE:
        updown_bbq_probe_rebuild_stop_back();
        break;

    case PAGE_UPDOWN_BBQ_COMPLETE_PROBE:
        if (child == PAGE_UPDOWN_BBQ_STOP_BACK_PROBE) {
            updown_bbq_complete_probe_create(&ui_manager);
            updown_bbq_complete_probe_t *done = updown_bbq_complete_probe_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->image_31 };
                if (g_updown_bbq_complete_probe) lv_group_del(g_updown_bbq_complete_probe);
                g_updown_bbq_complete_probe = group_create_for_page(btns, 1);
                updown_bbq_probe_complete_rebind(done->image_31);
                lv_group_focus_obj(done->image_31);
                lv_label_set_text_fmt(done->label_74, "| 上下烧烤 | %d℃ | %d℃", set_temp, probe_target_temp);
                lv_bar_set_value(done->bar_4, 100, LV_ANIM_OFF);
            }
            current_group = g_updown_bbq_complete_probe;
            lv_scr_load_anim(updown_bbq_complete_probe_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_HOT_BBQ_MENU_PROBE:
        hot_bbq_probe_rebuild_menu(child);
        break;
    case PAGE_HOT_BBQ_SET_PROBE:
        hot_bbq_probe_rebuild_set(child);
        break;
    case PAGE_HOT_BBQ_COOKING_PROBE:
        if (child == PAGE_HOT_BBQ_COMPLETE_PROBE)
            goto pop_to_major_menu;
        if (child == PAGE_HOT_BBQ_STOP_BACK_PROBE) {
            g_on_stop_back = 0;
            hot_bbq_probe_rebuild_cooking(child);
        } else {
            hot_bbq_probe_rebuild_cooking(child);
        }
        break;
    case PAGE_HOT_BBQ_STOP_PROBE:
        hot_bbq_probe_rebuild_stop();
        break;
    case PAGE_HOT_BBQ_STOP_BACK_PROBE:
        hot_bbq_probe_rebuild_stop_back();
        break;
    case PAGE_HOT_BBQ_COMPLETE_PROBE:
        if (child == PAGE_HOT_BBQ_STOP_BACK_PROBE) {
            hot_bbq_complete_probe_create(&ui_manager);
            hot_bbq_complete_probe_t *done = hot_bbq_complete_probe_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->image_15 };
                if (g_hot_bbq_complete_probe) lv_group_del(g_hot_bbq_complete_probe);
                g_hot_bbq_complete_probe = group_create_for_page(btns, 1);
                hot_bbq_probe_complete_rebind(done->image_15);
                lv_group_focus_obj(done->image_15);
                lv_label_set_text_fmt(done->status, "| 热风烧烤 | %d℃ | %d℃", set_temp, probe_target_temp);
                lv_bar_set_value(done->bar_4, 100, LV_ANIM_OFF);
            }
            current_group = g_hot_bbq_complete_probe;
            lv_scr_load_anim(hot_bbq_complete_probe_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_BOTTOM_BBQ_MENU_PROBE:
        bottom_bbq_probe_rebuild_menu(child);
        break;
    case PAGE_BOTTOM_BBQ_SET_PROBE:
        bottom_bbq_probe_rebuild_set(child);
        break;
    case PAGE_BOTTOM_BBQ_COOKING_PROBE:
        if (child == PAGE_BOTTOM_BBQ_COMPLETE_PROBE)
            goto pop_to_major_menu;
        if (child == PAGE_BOTTOM_BBQ_STOP_BACK_PROBE) {
            g_on_stop_back = 0;
            bottom_bbq_probe_rebuild_cooking(child);
        } else {
            bottom_bbq_probe_rebuild_cooking(child);
        }
        break;
    case PAGE_BOTTOM_BBQ_STOP_PROBE:
        bottom_bbq_probe_rebuild_stop();
        break;
    case PAGE_BOTTOM_BBQ_STOP_BACK_PROBE:
        bottom_bbq_probe_rebuild_stop_back();
        break;
    case PAGE_BOTTOM_BBQ_COMPLETE_PROBE:
        if (child == PAGE_BOTTOM_BBQ_STOP_BACK_PROBE) {
            bottom_bbq_complete_probe_create(&ui_manager);
            bottom_bbq_complete_probe_t *done = bottom_bbq_complete_probe_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->image_32 };
                if (g_bottom_bbq_complete_probe) lv_group_del(g_bottom_bbq_complete_probe);
                g_bottom_bbq_complete_probe = group_create_for_page(btns, 1);
                bottom_bbq_probe_complete_rebind(done->image_32);
                lv_group_focus_obj(done->image_32);
                lv_label_set_text_fmt(done->status, "| 底部烧烤 | %d℃ | %d℃", set_temp, probe_target_temp);
                lv_bar_set_value(done->bar_8, 100, LV_ANIM_OFF);
            }
            current_group = g_bottom_bbq_complete_probe;
            lv_scr_load_anim(bottom_bbq_complete_probe_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_SLOWCOOK_MENU_PROBE:
        slowcook_probe_rebuild_menu(child);
        break;
    case PAGE_SLOWCOOK_SET_PROBE:
        slowcook_probe_rebuild_set(child);
        break;
    case PAGE_SLOWCOOK_COOKING_PROBE:
        if (child == PAGE_SLOWCOOK_COMPLETE_PROBE)
            goto pop_to_major_menu;
        if (child == PAGE_SLOWCOOK_STOP_BACK_PROBE) {
            g_on_stop_back = 0;
            slowcook_probe_rebuild_cooking(child);
        } else {
            slowcook_probe_rebuild_cooking(child);
        }
        break;
    case PAGE_SLOWCOOK_STOP_PROBE:
        slowcook_probe_rebuild_stop();
        break;
    case PAGE_SLOWCOOK_STOP_BACK_PROBE:
        slowcook_probe_rebuild_stop_back();
        break;
    case PAGE_SLOWCOOK_COMPLETE_PROBE:
        if (child == PAGE_SLOWCOOK_STOP_BACK_PROBE) {
            slowcook_complete_probe_create(&ui_manager);
            slowcook_complete_probe_t *done = slowcook_complete_probe_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->image_48 };
                if (g_slowcook_complete_probe) lv_group_del(g_slowcook_complete_probe);
                g_slowcook_complete_probe = group_create_for_page(btns, 1);
                slowcook_probe_complete_rebind(done->image_48);
                lv_group_focus_obj(done->image_48);
                lv_label_set_text_fmt(done->status, "| 慢煮 | %d℃ | %d℃", set_temp, probe_target_temp);
                lv_bar_set_value(done->bar_12, 100, LV_ANIM_OFF);
            }
            current_group = g_slowcook_complete_probe;
            lv_scr_load_anim(slowcook_complete_probe_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_EXTRA_COLOR:
        if (child == PAGE_COLOR_STOP_BACK) {
            g_on_stop_back = 0;
            extra_color_create(&ui_manager);
            extra_color_t *ec = extra_color_get(&ui_manager);
            if (ec) {
                lv_obj_t *btns[] = { ec->start_button };
                if (g_extra_color) lv_group_del(g_extra_color);
                g_extra_color = group_create_for_page(btns, 1);
                lv_obj_add_event_cb(ec->start_button, on_color_start_click,
                                    LV_EVENT_CLICKED, NULL);
                if (ec->start_button) lv_group_focus_obj(ec->start_button);
            }
            current_group = g_extra_color;
            lv_scr_load_anim(extra_color_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        color_exit_to_home();
        break;

    case PAGE_COLOR_COOKING:
        if (child == PAGE_COLOR_STOP_BACK) {
            g_on_stop_back = 0;
            cook_is_color = 1;
            if (!cook_timer) {
                cook_start_time = lv_tick_get() - cook_elapsed_saved;
                cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
            }
            lv_obj_clean(lv_scr_act());
            color_cookoing_create(&ui_manager);
            color_cookoing_t *cc = color_cookoing_get(&ui_manager);
            if (cc) {
                lv_obj_t *btns[] = { cc->stop_button };
                if (g_color_cookoing) lv_group_del(g_color_cookoing);
                g_color_cookoing = group_create_for_page(btns, 1);
                lv_obj_add_event_cb(cc->stop_button, on_color_stop_click,
                                    LV_EVENT_CLICKED, NULL);
                uint32_t elapsed = lv_tick_get() - cook_start_time;
                int remaining_sec = (cook_total_ms - (int)elapsed) / 1000;
                if (remaining_sec < 0) remaining_sec = 0;
                int h = remaining_sec / 3600;
                int m = (remaining_sec % 3600) / 60;
                int s = remaining_sec % 60;
                lv_label_set_text_fmt(cc->time_label, "%02d:%02d:%02d", h, m, s);
                lv_label_set_text_fmt(cc->status_label, "| 额外上色 | %d℃ | 5分钟", set_temp);
                lv_bar_set_range(cc->bar, 0, 100);
                    int progress = stop_back_progress(elapsed, cook_total_ms);
                    if (progress > 100) progress = 100;
                lv_bar_set_value(cc->bar, progress, LV_ANIM_OFF);
                lv_anim_t a;
                lv_anim_init(&a);
                lv_anim_set_var(&a, cc->bar);
                lv_anim_set_exec_cb(&a, anim_bar_set_value);
                lv_anim_set_values(&a, progress, 100);
                lv_anim_set_time(&a, ((int)(cook_total_ms - (int)elapsed) < 0) ? 0 : (cook_total_ms - (int)elapsed));
                lv_anim_start(&a);
            }
            current_group = g_color_cookoing;
            lv_scr_load_anim(color_cookoing_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            printf("[nav] back from stop_back -> color_cookoing\n");
            break;
        }
        color_exit_to_home();
        break;
    case PAGE_COLOR_COOKING_COMPLETE:
        if (child == PAGE_COLOR_STOP_BACK) {
            colorcooking_complete_create(&ui_manager);
            colorcooking_complete_t *cc = colorcooking_complete_get(&ui_manager);
            if (cc) {
                lv_label_set_text_fmt(cc->status_label, "| 额外上色 | %d℃ | 5分钟", set_temp);
                lv_bar_set_range(cc->bar, 0, 100);
                lv_bar_set_value(cc->bar, 100, LV_ANIM_OFF);
            }
            current_group = NULL;
            lv_scr_load_anim(colorcooking_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        color_exit_to_home();
        break;

    case PAGE_UPDOWN_BBQ_MENU_TOP:
    case PAGE_UPDOWN_BBQ_MENU_LOW:
        goto rebuild_updown_bbq_set;   /* 上级是 updown set 页，按 child 恢复对应温度 */
    case PAGE_DELAYCOOKING:
        rebuild_delaycooking();
        break;
    case PAGE_DELAYSET:
    case PAGE_UPDOWN_BBQ_SET:
    rebuild_updown_bbq_set:
        if (child == PAGE_UPDOWN_BBQ_MENU_TOP)
            set_temp_up = updown_menu_top_saved;
        else if (child == PAGE_UPDOWN_BBQ_MENU_LOW)
            set_temp_down = updown_menu_low_saved;
        /* 关闭烹饪倒计时 */
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

        updown_bbq_set_create(&ui_manager);
        {
            updown_bbq_set_t *set = updown_bbq_set_get(&ui_manager);
            if (set) {
                lv_obj_t *btns[] = {
                    set->sure_button, set->uptemp_button, set->downtemp_button,
                    set->preheat_button, set->preheat_on_button,
                    set->delay_button, set->delay_on_button,
                    set->contain_button, set->contain_on_button,
                };
                if (g_updown_bbq_set) lv_group_del(g_updown_bbq_set);
                g_updown_bbq_set = group_create_for_page(btns, 9);

                /* 强制清除所有按钮的 FOCUSED 状态，防止渲染缓存残留 */
                clear_focus_states(btns, 9);
                lv_group_focus_obj(set->sure_button);

                /* 温度显示 */
                setup_set_temp_display(set);
                printf("%d,%d\n",set_temp_up,set_temp_down);

                /* 时间显示 */
                lv_label_set_text_fmt(set->hour_label, "%02d", set_hour);
                lv_label_set_text_fmt(set->min_label, "%02d", set_min);

                if (set_hour == 0) {
                    lv_obj_add_flag(set->hour_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(set->shi_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_pos(set->min_label, 312, 254);
                    lv_obj_set_pos(set->fen_label, 365, 269);
                } else {
                    lv_obj_clear_flag(set->hour_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->shi_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_pos(set->min_label, 395, 254);
                    lv_obj_set_pos(set->fen_label, 448, 269);
                }

                /* 恢复 toggle 状态（跟随变量） */
                apply_toggle_state(set->preheat_button, set->preheat_on_button, preheat_on);
                apply_toggle_state(set->delay_button, set->delay_on_button, delay_on);
                apply_toggle_state(set->contain_button, set->contain_on_button, contain_on);
                updown_set_apply_delay_label(set);

                /* 绑定 toggle 事件 */
                lv_obj_add_event_cb(set->preheat_button, on_preheat_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->preheat_on_button, on_preheat_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->delay_button, on_delay_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->delay_on_button, on_delay_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->contain_button, on_contain_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->contain_on_button, on_contain_toggle, LV_EVENT_CLICKED, NULL);

                /* 绑定 sure_button 事件 */
                lv_obj_add_event_cb(set->sure_button, on_sure_click, LV_EVENT_CLICKED, NULL);

                /* 绑定温度按钮事件 */
                lv_obj_add_event_cb(set->uptemp_button, on_updown_uptemp_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->downtemp_button, on_updown_downtemp_click,
                                    LV_EVENT_CLICKED, NULL);

                /* 焦点恢复 */
                if (child == PAGE_UPDOWN_BBQ_COOKING && set->sure_button)
                    lv_group_focus_obj(set->sure_button);
                else if (child == PAGE_UPDOWN_BBQ_MENU_TOP && set->uptemp_button)
                    lv_group_focus_obj(set->uptemp_button);
                else if (child == PAGE_UPDOWN_BBQ_MENU_LOW && set->downtemp_button)
                    lv_group_focus_obj(set->downtemp_button);
                else if (child == PAGE_DELAYSET) {
                    if (delay_on && set->delay_on_button)
                        lv_group_focus_obj(set->delay_on_button);
                    else if (set->delay_button)
                        lv_group_focus_obj(set->delay_button);
                }
            }
            current_group = g_updown_bbq_set;
        }
        lv_scr_load_anim(updown_bbq_set_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to updown_bbq_set\n");
        break;

    case PAGE_COLOR_STOP:
        g_on_stop_back = 0;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }   /* 暂停页不跑烹饪计时 */
        color_stop_create(&ui_manager);
        {
            color_stop_t *cs = color_stop_get(&ui_manager);
            if (cs) {
                lv_obj_t *btns[] = { cs->button_6 };
                if (g_color_stop) lv_group_del(g_color_stop);
                g_color_stop = group_create_for_page(btns, 1);
                lv_obj_add_event_cb(cs->button_6, on_color_stop_start_click,
                                    LV_EVENT_CLICKED, NULL);

                int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
                int total_sec = cook_total_ms / 1000;
                int remaining_sec = total_sec - elapsed_sec;
                if (remaining_sec < 0) remaining_sec = 0;
                int h = remaining_sec / 3600;
                int m = (remaining_sec % 3600) / 60;
                int s = remaining_sec % 60;
                lv_label_set_text_fmt(cs->time_label, "%02d:%02d:%02d", h, m, s);
                lv_label_set_text_fmt(cs->label_13, "| 额外上色 | %d℃ | 5分钟", set_temp);
                lv_bar_set_range(cs->bar_3, 0, 100);
                if (cook_bar_saved > 100) cook_bar_saved = 100;
                lv_bar_set_value(cs->bar_3, cook_bar_saved, LV_ANIM_OFF);
            }
            current_group = g_color_stop;
        }
        lv_scr_load_anim(color_stop_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to color_stop\n");
        break;

    case PAGE_COLOR_STOP_BACK:
        color_stop_back_create(&ui_manager);
        {
            color_stop_back_t *csb = color_stop_back_get(&ui_manager);
            if (csb) {
                lv_obj_t *btns[] = { csb->button_7 };
                if (g_color_stop_back) lv_group_del(g_color_stop_back);
                g_color_stop_back = group_create_for_page(btns, 1);
                lv_obj_add_event_cb(csb->button_7, on_color_stop_back_sure_click,
                                    LV_EVENT_CLICKED, NULL);

                lv_label_set_text_fmt(csb->label_17, "| 额外上色 | %d℃ | 5分钟", set_temp);
                uint32_t _elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
                int _p = stop_back_progress(_elapsed, cook_total_ms);
                if (_p > 100) _p = 100;
                lv_bar_set_range(csb->bar_4, 0, 100);
                lv_bar_set_value(csb->bar_4, _p, LV_ANIM_OFF);

                if (g_complete_to_stop_back) {
                    g_complete_to_stop_back = 0;
                    lv_label_set_text(csb->label_19, "已完成");
                    lv_bar_set_value(csb->bar_4, 100, LV_ANIM_OFF);
                }
            }
            current_group = g_color_stop_back;
        }
        lv_scr_load_anim(color_stop_back_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to color_stop_back\n");
        break;

    case PAGE_TOP_BBQ_MENU:
        top_bbq_rebuild_menu(child);
        break;

    case PAGE_TOP_BBQ_SET:
        top_bbq_rebuild_set(child);
        break;

    case PAGE_TOP_BBQ_COOKING:
        if (child == PAGE_TOP_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_TOP_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            top_bbq_rebuild_cooking(PAGE_TOP_BBQ_SETTING);
        } else if (child == PAGE_TOP_BBQ_SETTING) {
            set_temp = top_setting_saved_temp;
            set_hour = top_setting_saved_hour;
            set_min = top_setting_saved_min;
            top_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            top_bbq_rebuild_cooking(0);
        break;

    case PAGE_TOP_BBQ_SETTING:
        top_bbq_rebuild_setting();
        break;

    case PAGE_TOP_BBQ_STOP:
        if (child == PAGE_TOP_BBQ_SETTING) {
            set_temp = top_setting_saved_temp;
            set_hour = top_setting_saved_hour;
            set_min = top_setting_saved_min;
        }
        top_bbq_rebuild_stop();
        break;

    case PAGE_TOP_BBQ_STOP_BACK:
        top_bbq_rebuild_stop_back();
        break;

    case PAGE_TOP_BBQ_COMPLETE:
        if (child == PAGE_TOP_BBQ_SETTING) {
            set_temp = top_setting_saved_temp;
            set_hour = top_setting_saved_hour;
            set_min = top_setting_saved_min;
        }
        if (child == PAGE_TOP_BBQ_SETTING || child == PAGE_TOP_BBQ_STOP_BACK) {
            top_bbq_complete_create(&ui_manager);
            top_bbq_complete_t *done = top_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_27 };
                if (g_top_bbq_complete) lv_group_del(g_top_bbq_complete);
                g_top_bbq_complete = group_create_for_page(btns, 1);
                top_bbq_complete_rebind(done->button_27);
                lv_group_focus_obj(done->button_27);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_108, "保温中...");

                lv_label_set_text_fmt(done->label_107, "| 顶部烧烤 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_9, 100, LV_ANIM_OFF);
            }
            current_group = g_top_bbq_complete;
            lv_scr_load_anim(top_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_BOTTOM_BBQ_MENU:
        bottom_bbq_rebuild_menu(child);
        break;

    case PAGE_BOTTOM_BBQ_SET:
        bottom_bbq_rebuild_set(child);
        break;

    case PAGE_BOTTOM_BBQ_COOKING:
        if (child == PAGE_BOTTOM_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_BOTTOM_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            bottom_bbq_rebuild_cooking(PAGE_BOTTOM_BBQ_SETTING);
        } else if (child == PAGE_BOTTOM_BBQ_SETTING) {
            set_temp = bottom_bbq_setting_saved_temp;
            set_hour = bottom_bbq_setting_saved_hour;
            set_min = bottom_bbq_setting_saved_min;
            bottom_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            bottom_bbq_rebuild_cooking(0);
        break;

    case PAGE_BOTTOM_BBQ_SETTING:
        bottom_bbq_rebuild_setting();
        break;

    case PAGE_BOTTOM_BBQ_STOP:
        if (child == PAGE_BOTTOM_BBQ_SETTING) {
            set_temp = bottom_bbq_setting_saved_temp;
            set_hour = bottom_bbq_setting_saved_hour;
            set_min = bottom_bbq_setting_saved_min;
        }
        bottom_bbq_rebuild_stop();
        break;

    case PAGE_BOTTOM_BBQ_STOP_BACK:
        bottom_bbq_rebuild_stop_back();
        break;

    case PAGE_BOTTOM_BBQ_COMPLETE:
        if (child == PAGE_BOTTOM_BBQ_SETTING) {
            set_temp = bottom_bbq_setting_saved_temp;
            set_hour = bottom_bbq_setting_saved_hour;
            set_min = bottom_bbq_setting_saved_min;
        }
        if (child == PAGE_BOTTOM_BBQ_SETTING || child == PAGE_BOTTOM_BBQ_STOP_BACK) {
            bottom_bbq_complete_create(&ui_manager);
            bottom_bbq_complete_t *done = bottom_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_43 };
                if (g_bottom_bbq_complete) lv_group_del(g_bottom_bbq_complete);
                g_bottom_bbq_complete = group_create_for_page(btns, 1);
                bottom_bbq_complete_rebind(done->button_43);
                lv_group_focus_obj(done->button_43);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_156, "保温中...");

                lv_label_set_text_fmt(done->status, "| 底部烧烤 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_13, 100, LV_ANIM_OFF);
            }
            current_group = g_bottom_bbq_complete;
            lv_scr_load_anim(bottom_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_HOT_BBQ_MENU:
        hot_bbq_rebuild_menu(child);
        break;

    case PAGE_HOT_BBQ_SET:
        hot_bbq_rebuild_set(child);
        break;

    case PAGE_HOT_BBQ_COOKING:
        if (child == PAGE_HOT_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_HOT_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            hot_bbq_rebuild_cooking(PAGE_HOT_BBQ_SETTING);
        } else if (child == PAGE_HOT_BBQ_SETTING) {
            set_temp = hot_bbq_setting_saved_temp;
            set_hour = hot_bbq_setting_saved_hour;
            set_min = hot_bbq_setting_saved_min;
            hot_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            hot_bbq_rebuild_cooking(0);
        break;

    case PAGE_HOT_BBQ_SETTING:
        hot_bbq_rebuild_setting();
        break;

    case PAGE_HOT_BBQ_STOP:
        if (child == PAGE_HOT_BBQ_SETTING) {
            set_temp = hot_bbq_setting_saved_temp;
            set_hour = hot_bbq_setting_saved_hour;
            set_min = hot_bbq_setting_saved_min;
        }
        hot_bbq_rebuild_stop();
        break;

    case PAGE_HOT_BBQ_STOP_BACK:
        hot_bbq_rebuild_stop_back();
        break;

    case PAGE_HOT_BBQ_COMPLETE:
        if (child == PAGE_HOT_BBQ_SETTING) {
            set_temp = hot_bbq_setting_saved_temp;
            set_hour = hot_bbq_setting_saved_hour;
            set_min = hot_bbq_setting_saved_min;
        }
        if (child == PAGE_HOT_BBQ_SETTING || child == PAGE_HOT_BBQ_STOP_BACK) {
            hot_bbq_complete_create(&ui_manager);
            hot_bbq_complete_t *done = hot_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_59 };
                if (g_hot_bbq_complete) lv_group_del(g_hot_bbq_complete);
                g_hot_bbq_complete = group_create_for_page(btns, 1);
                hot_bbq_complete_rebind(done->button_59);
                lv_group_focus_obj(done->button_59);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_204, "保温中...");

                lv_label_set_text_fmt(done->status, "| 热风烧烤 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_17, 100, LV_ANIM_OFF);
            }
            current_group = g_hot_bbq_complete;
            lv_scr_load_anim(hot_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_HOTWIND_BBQ_MENU:
        hotwind_bbq_rebuild_menu(child);
        break;
    case PAGE_HOTWIND_BBQ_SET:
        hotwind_bbq_rebuild_set(child);
        break;
    case PAGE_HOTWIND_BBQ_COOKING:
        if (child == PAGE_HOTWIND_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_HOTWIND_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            hotwind_bbq_rebuild_cooking(PAGE_HOTWIND_BBQ_SETTING);
        } else if (child == PAGE_HOTWIND_BBQ_SETTING) {
            set_temp = hotwind_bbq_setting_saved_temp;
            set_hour = hotwind_bbq_setting_saved_hour;
            set_min = hotwind_bbq_setting_saved_min;
            hotwind_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            hotwind_bbq_rebuild_cooking(0);
        break;
    case PAGE_HOTWIND_BBQ_SETTING:
        hotwind_bbq_rebuild_setting();
        break;
    case PAGE_HOTWIND_BBQ_STOP:
        if (child == PAGE_HOTWIND_BBQ_SETTING) {
            set_temp = hotwind_bbq_setting_saved_temp;
            set_hour = hotwind_bbq_setting_saved_hour;
            set_min = hotwind_bbq_setting_saved_min;
        }
        hotwind_bbq_rebuild_stop();
        break;
    case PAGE_HOTWIND_BBQ_STOP_BACK:
        hotwind_bbq_rebuild_stop_back();
        break;
    case PAGE_HOTWIND_BBQ_COMPLETE:
        if (child == PAGE_HOTWIND_BBQ_SETTING) {
            set_temp = hotwind_bbq_setting_saved_temp;
            set_hour = hotwind_bbq_setting_saved_hour;
            set_min = hotwind_bbq_setting_saved_min;
        }
        if (child == PAGE_HOTWIND_BBQ_SETTING || child == PAGE_HOTWIND_BBQ_STOP_BACK) {
            hotwind_bbq_complete_create(&ui_manager);
            hotwind_bbq_complete_t *done = hotwind_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_74 };
                if (g_hotwind_bbq_complete) lv_group_del(g_hotwind_bbq_complete);
                g_hotwind_bbq_complete = group_create_for_page(btns, 1);
                hotwind_bbq_complete_rebind(done->button_74);
                lv_group_focus_obj(done->button_74);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_242, "保温中...");

                lv_label_set_text_fmt(done->status, "| 热风对流 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_21, 100, LV_ANIM_OFF);
            }
            current_group = g_hotwind_bbq_complete;
            lv_scr_load_anim(hotwind_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_SAVE_BBQ_MENU:
        save_bbq_rebuild_menu(child);
        break;
    case PAGE_SAVE_BBQ_SET:
        save_bbq_rebuild_set(child);
        break;
    case PAGE_SAVE_BBQ_COOKING:
        if (child == PAGE_SAVE_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_SAVE_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            save_bbq_rebuild_cooking(PAGE_SAVE_BBQ_SETTING);
        } else if (child == PAGE_SAVE_BBQ_SETTING) {
            set_temp = save_bbq_setting_saved_temp;
            set_hour = save_bbq_setting_saved_hour;
            set_min = save_bbq_setting_saved_min;
            save_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            save_bbq_rebuild_cooking(0);
        break;
    case PAGE_SAVE_BBQ_SETTING:
        save_bbq_rebuild_setting();
        break;
    case PAGE_SAVE_BBQ_STOP:
        if (child == PAGE_SAVE_BBQ_SETTING) {
            set_temp = save_bbq_setting_saved_temp;
            set_hour = save_bbq_setting_saved_hour;
            set_min = save_bbq_setting_saved_min;
        }
        save_bbq_rebuild_stop();
        break;
    case PAGE_SAVE_BBQ_STOP_BACK:
        save_bbq_rebuild_stop_back();
        break;
    case PAGE_SAVE_BBQ_COMPLETE:
        if (child == PAGE_SAVE_BBQ_SETTING) {
            set_temp = save_bbq_setting_saved_temp;
            set_hour = save_bbq_setting_saved_hour;
            set_min = save_bbq_setting_saved_min;
        }
        if (child == PAGE_SAVE_BBQ_SETTING || child == PAGE_SAVE_BBQ_STOP_BACK) {
            save_bbq_complete_create(&ui_manager);
            save_bbq_complete_t *done = save_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_89 };
                if (g_save_bbq_complete) lv_group_del(g_save_bbq_complete);
                g_save_bbq_complete = group_create_for_page(btns, 1);
                save_bbq_complete_rebind(done->button_89);
                lv_group_focus_obj(done->button_89);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_280, "保温中...");

                lv_label_set_text_fmt(done->status, "| 节能热风 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_25, 100, LV_ANIM_OFF);
            }
            current_group = g_save_bbq_complete;
            lv_scr_load_anim(save_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_CENTRAL_BBQ_MENU:
        central_bbq_rebuild_menu(child);
        break;
    case PAGE_CENTRAL_BBQ_SET:
        central_bbq_rebuild_set(child);
        break;
    case PAGE_CENTRAL_BBQ_COOKING:
        if (child == PAGE_CENTRAL_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_CENTRAL_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            central_bbq_rebuild_cooking(PAGE_CENTRAL_BBQ_SETTING);
        } else if (child == PAGE_CENTRAL_BBQ_SETTING) {
            set_temp = central_bbq_setting_saved_temp;
            set_hour = central_bbq_setting_saved_hour;
            set_min = central_bbq_setting_saved_min;
            central_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            central_bbq_rebuild_cooking(0);
        break;
    case PAGE_CENTRAL_BBQ_SETTING:
        central_bbq_rebuild_setting();
        break;
    case PAGE_CENTRAL_BBQ_STOP:
        if (child == PAGE_CENTRAL_BBQ_SETTING) {
            set_temp = central_bbq_setting_saved_temp;
            set_hour = central_bbq_setting_saved_hour;
            set_min = central_bbq_setting_saved_min;
        }
        central_bbq_rebuild_stop();
        break;
    case PAGE_CENTRAL_BBQ_STOP_BACK:
        central_bbq_rebuild_stop_back();
        break;
    case PAGE_CENTRAL_BBQ_COMPLETE:
        if (child == PAGE_CENTRAL_BBQ_SETTING) {
            set_temp = central_bbq_setting_saved_temp;
            set_hour = central_bbq_setting_saved_hour;
            set_min = central_bbq_setting_saved_min;
        }
        if (child == PAGE_CENTRAL_BBQ_SETTING || child == PAGE_CENTRAL_BBQ_STOP_BACK) {
            central_bbq_complete_create(&ui_manager);
            central_bbq_complete_t *done = central_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_104 };
                if (g_central_bbq_complete) lv_group_del(g_central_bbq_complete);
                g_central_bbq_complete = group_create_for_page(btns, 1);
                central_bbq_complete_rebind(done->button_104);
                lv_group_focus_obj(done->button_104);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_318, "保温中...");

                lv_label_set_text_fmt(done->status, "| 集中烧烤 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_29, 100, LV_ANIM_OFF);
            }
            current_group = g_central_bbq_complete;
            lv_scr_load_anim(central_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_WINDCHANGE_BBQ_MENU:
        windchange_bbq_rebuild_menu(child);
        break;
    case PAGE_WINDCHANGE_BBQ_SET:
        windchange_bbq_rebuild_set(child);
        break;
    case PAGE_WINDCHANGE_BBQ_COOKING:
        if (child == PAGE_WINDCHANGE_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_WINDCHANGE_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            windchange_bbq_rebuild_cooking(PAGE_WINDCHANGE_BBQ_SETTING);
        } else if (child == PAGE_WINDCHANGE_BBQ_SETTING) {
            set_temp = windchange_bbq_setting_saved_temp;
            set_hour = windchange_bbq_setting_saved_hour;
            set_min = windchange_bbq_setting_saved_min;
            windchange_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            windchange_bbq_rebuild_cooking(0);
        break;
    case PAGE_WINDCHANGE_BBQ_SETTING:
        windchange_bbq_rebuild_setting();
        break;
    case PAGE_WINDCHANGE_BBQ_STOP:
        if (child == PAGE_WINDCHANGE_BBQ_SETTING) {
            set_temp = windchange_bbq_setting_saved_temp;
            set_hour = windchange_bbq_setting_saved_hour;
            set_min = windchange_bbq_setting_saved_min;
        }
        windchange_bbq_rebuild_stop();
        break;
    case PAGE_WINDCHANGE_BBQ_STOP_BACK:
        windchange_bbq_rebuild_stop_back();
        break;
    case PAGE_WINDCHANGE_BBQ_COMPLETE:
        if (child == PAGE_WINDCHANGE_BBQ_SETTING) {
            set_temp = windchange_bbq_setting_saved_temp;
            set_hour = windchange_bbq_setting_saved_hour;
            set_min = windchange_bbq_setting_saved_min;
        }
        if (child == PAGE_WINDCHANGE_BBQ_SETTING || child == PAGE_WINDCHANGE_BBQ_STOP_BACK) {
            windchange_bbq_complete_create(&ui_manager);
            windchange_bbq_complete_t *done = windchange_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_119 };
                if (g_windchange_bbq_complete) lv_group_del(g_windchange_bbq_complete);
                g_windchange_bbq_complete = group_create_for_page(btns, 1);
                windchange_bbq_complete_rebind(done->button_119);
                lv_group_focus_obj(done->button_119);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_356, "保温中...");

                lv_label_set_text_fmt(done->status, "| 加强热风 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_33, 100, LV_ANIM_OFF);
            }
            current_group = g_windchange_bbq_complete;
            lv_scr_load_anim(windchange_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_COOK4_MENU:
        cook4menu_rebuild(child);
        break;
    case PAGE_COOKIE_MENU:
        cookie_rebuild_menu(child);
        break;
    case PAGE_COOKIE_SET:
        cookie_rebuild_set(child);
        break;
    case PAGE_COOKIE_COOKING:
        if (child == PAGE_COOKIE_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_COOKIE_STOP_BACK) {
            g_on_stop_back = 0;
            cookie_rebuild_cooking(PAGE_COOKIE_SETTING);
        } else if (child == PAGE_COOKIE_SETTING) {
            set_temp = cookie_setting_saved_temp;
            set_hour = cookie_setting_saved_hour;
            set_min = cookie_setting_saved_min;
            cookie_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            cookie_rebuild_cooking(0);
        break;
    case PAGE_COOKIE_SETTING:
        cookie_rebuild_setting();
        break;
    case PAGE_COOKIE_STOP:
        if (child == PAGE_COOKIE_SETTING) {
            set_temp = cookie_setting_saved_temp;
            set_hour = cookie_setting_saved_hour;
            set_min = cookie_setting_saved_min;
        }
        cookie_rebuild_stop();
        break;
    case PAGE_COOKIE_STOP_BACK:
        cookie_rebuild_stop_back();
        break;
    case PAGE_COOKIE_COMPLETE:
        if (child == PAGE_COOKIE_SETTING) {
            set_temp = cookie_setting_saved_temp;
            set_hour = cookie_setting_saved_hour;
            set_min = cookie_setting_saved_min;
        }
        if (child == PAGE_COOKIE_SETTING || child == PAGE_COOKIE_STOP_BACK) {
            cookie_complete_create(&ui_manager);
            cookie_complete_t *done = cookie_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_cookie_complete) lv_group_del(g_cookie_complete);
                g_cookie_complete = group_create_for_page(btns, 1);
                cookie_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_143, "保温中...");

                lv_label_set_text_fmt(done->status, "| 曲奇 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_7, 100, LV_ANIM_OFF);
            }
            current_group = g_cookie_complete;
            lv_scr_load_anim(cookie_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_WEST_MENU:
        west_rebuild_menu(child);
        break;
    case PAGE_WEST_SET:
        west_rebuild_set(child);
        break;
    case PAGE_WEST_COOKING:
        if (child == PAGE_WEST_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_WEST_STOP_BACK) {
            g_on_stop_back = 0;
            west_rebuild_cooking(PAGE_WEST_SETTING);
        } else if (child == PAGE_WEST_SETTING) {
            set_temp = west_setting_saved_temp;
            set_hour = west_setting_saved_hour;
            set_min = west_setting_saved_min;
            west_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            west_rebuild_cooking(0);
        break;
    case PAGE_WEST_SETTING:
        west_rebuild_setting();
        break;
    case PAGE_WEST_STOP:
        if (child == PAGE_WEST_SETTING) {
            set_temp = west_setting_saved_temp;
            set_hour = west_setting_saved_hour;
            set_min = west_setting_saved_min;
        }
        west_rebuild_stop();
        break;
    case PAGE_WEST_STOP_BACK:
        west_rebuild_stop_back();
        break;
    case PAGE_WEST_COMPLETE:
        if (child == PAGE_WEST_SETTING) {
            set_temp = west_setting_saved_temp;
            set_hour = west_setting_saved_hour;
            set_min = west_setting_saved_min;
        }
        if (child == PAGE_WEST_SETTING || child == PAGE_WEST_STOP_BACK) {
            west_complete_create(&ui_manager);
            west_complete_t *done = west_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_west_complete) lv_group_del(g_west_complete);
                g_west_complete = group_create_for_page(btns, 1);
                west_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_192, "保温中...");

                lv_label_set_text_fmt(done->status, "| 西式 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_11, 100, LV_ANIM_OFF);
            }
            current_group = g_west_complete;
            lv_scr_load_anim(west_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_PIZZA_MENU:
        pizza_rebuild_menu(child);
        break;
    case PAGE_PIZZA_SET:
        pizza_rebuild_set(child);
        break;
    case PAGE_PIZZA_COOKING:
        if (child == PAGE_PIZZA_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_PIZZA_STOP_BACK) {
            g_on_stop_back = 0;
            pizza_rebuild_cooking(PAGE_PIZZA_SETTING);
        } else if (child == PAGE_PIZZA_SETTING) {
            set_temp = pizza_setting_saved_temp;
            set_hour = pizza_setting_saved_hour;
            set_min = pizza_setting_saved_min;
            pizza_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            pizza_rebuild_cooking(0);
        break;
    case PAGE_PIZZA_SETTING:
        pizza_rebuild_setting();
        break;
    case PAGE_PIZZA_STOP:
        if (child == PAGE_PIZZA_SETTING) {
            set_temp = pizza_setting_saved_temp;
            set_hour = pizza_setting_saved_hour;
            set_min = pizza_setting_saved_min;
        }
        pizza_rebuild_stop();
        break;
    case PAGE_PIZZA_STOP_BACK:
        pizza_rebuild_stop_back();
        break;
    case PAGE_PIZZA_COMPLETE:
        if (child == PAGE_PIZZA_SETTING) {
            set_temp = pizza_setting_saved_temp;
            set_hour = pizza_setting_saved_hour;
            set_min = pizza_setting_saved_min;
        }
        if (child == PAGE_PIZZA_SETTING || child == PAGE_PIZZA_STOP_BACK) {
            pizza_complete_create(&ui_manager);
            pizza_complete_t *done = pizza_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_pizza_complete) lv_group_del(g_pizza_complete);
                g_pizza_complete = group_create_for_page(btns, 1);
                pizza_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_240, "保温中...");

                lv_label_set_text_fmt(done->status, "| 披萨 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_15, 100, LV_ANIM_OFF);
            }
            current_group = g_pizza_complete;
            lv_scr_load_anim(pizza_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_MENU_COOK_MENU:
        menu_rebuild_menu(child);
        break;
    case PAGE_MENU_COOK_SET:
        menu_rebuild_set(child);
        break;
    case PAGE_MENU_COOK_COOKING:
        if (child == PAGE_MENU_COOK_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_MENU_COOK_STOP_BACK) {
            g_on_stop_back = 0;
            menu_rebuild_cooking(PAGE_MENU_COOK_SETTING);
        } else if (child == PAGE_MENU_COOK_SETTING) {
            set_temp = menu_setting_saved_temp;
            set_hour = menu_setting_saved_hour;
            set_min = menu_setting_saved_min;
            menu_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            menu_rebuild_cooking(0);
        break;
    case PAGE_MENU_COOK_SETTING:
        menu_rebuild_setting();
        break;
    case PAGE_MENU_COOK_STOP:
        if (child == PAGE_MENU_COOK_SETTING) {
            set_temp = menu_setting_saved_temp;
            set_hour = menu_setting_saved_hour;
            set_min = menu_setting_saved_min;
        }
        menu_rebuild_stop();
        break;
    case PAGE_MENU_COOK_STOP_BACK:
        menu_rebuild_stop_back();
        break;
    case PAGE_MENU_COOK_COMPLETE:
        if (child == PAGE_MENU_COOK_SETTING) {
            set_temp = menu_setting_saved_temp;
            set_hour = menu_setting_saved_hour;
            set_min = menu_setting_saved_min;
        }
        if (child == PAGE_MENU_COOK_SETTING || child == PAGE_MENU_COOK_STOP_BACK) {
            menu_complete_create(&ui_manager);
            menu_complete_t *done = menu_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_menu_cook_complete) lv_group_del(g_menu_cook_complete);
                g_menu_cook_complete = group_create_for_page(btns, 1);
                menu_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_288, "保温中...");

                lv_label_set_text_fmt(done->status, "| 菜单 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_19, 100, LV_ANIM_OFF);
            }
            current_group = g_menu_cook_complete;
            lv_scr_load_anim(menu_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_AIR_MENU:
        air_rebuild_menu(child);
        break;
    case PAGE_AIR_SET:
        air_rebuild_set(child);
        break;
    case PAGE_AIR_COOKING:
        if (child == PAGE_AIR_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_AIR_STOP_BACK) {
            g_on_stop_back = 0;
            air_rebuild_cooking(PAGE_AIR_SETTING);
        } else if (child == PAGE_AIR_SETTING) {
            set_temp = air_setting_saved_temp;
            set_hour = air_setting_saved_hour;
            set_min = air_setting_saved_min;
            air_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            air_rebuild_cooking(0);
        break;
    case PAGE_AIR_SETTING:
        air_rebuild_setting();
        break;
    case PAGE_AIR_STOP:
        if (child == PAGE_AIR_SETTING) {
            set_temp = air_setting_saved_temp;
            set_hour = air_setting_saved_hour;
            set_min = air_setting_saved_min;
        }
        air_rebuild_stop();
        break;
    case PAGE_AIR_STOP_BACK:
        air_rebuild_stop_back();
        break;
    case PAGE_AIR_COMPLETE:
        if (child == PAGE_AIR_SETTING) {
            set_temp = air_setting_saved_temp;
            set_hour = air_setting_saved_hour;
            set_min = air_setting_saved_min;
        }
        if (child == PAGE_AIR_SETTING || child == PAGE_AIR_STOP_BACK) {
            air_complete_create(&ui_manager);
            air_complete_t *done = air_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_air_complete) lv_group_del(g_air_complete);
                g_air_complete = group_create_for_page(btns, 1);
                air_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_337, "保温中...");

                lv_label_set_text_fmt(done->status, "| 空气炸 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_23, 100, LV_ANIM_OFF);
            }
            current_group = g_air_complete;
            lv_scr_load_anim(air_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_PIZZA_2_MENU:
        pizza_2_rebuild_menu(child);
        break;
    case PAGE_PIZZA_2_SET:
        pizza_2_rebuild_set(child);
        break;
    case PAGE_PIZZA_2_COOKING:
        if (child == PAGE_PIZZA_2_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_PIZZA_2_STOP_BACK) {
            g_on_stop_back = 0;
            pizza_2_rebuild_cooking(PAGE_PIZZA_2_SETTING);
        } else if (child == PAGE_PIZZA_2_SETTING) {
            set_temp = pizza_2_setting_saved_temp;
            set_hour = pizza_2_setting_saved_hour;
            set_min = pizza_2_setting_saved_min;
            pizza_2_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            pizza_2_rebuild_cooking(0);
        break;
    case PAGE_PIZZA_2_SETTING:
        pizza_2_rebuild_setting();
        break;
    case PAGE_PIZZA_2_STOP:
        if (child == PAGE_PIZZA_2_SETTING) {
            set_temp = pizza_2_setting_saved_temp;
            set_hour = pizza_2_setting_saved_hour;
            set_min = pizza_2_setting_saved_min;
        }
        pizza_2_rebuild_stop();
        break;
    case PAGE_PIZZA_2_STOP_BACK:
        pizza_2_rebuild_stop_back();
        break;
    case PAGE_PIZZA_2_COMPLETE:
        if (child == PAGE_PIZZA_2_SETTING) {
            set_temp = pizza_2_setting_saved_temp;
            set_hour = pizza_2_setting_saved_hour;
            set_min = pizza_2_setting_saved_min;
        }
        if (child == PAGE_PIZZA_2_SETTING || child == PAGE_PIZZA_2_STOP_BACK) {
            pizza_2_complete_create(&ui_manager);
            pizza_2_complete_t *done = pizza_2_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_pizza_2_complete) lv_group_del(g_pizza_2_complete);
                g_pizza_2_complete = group_create_for_page(btns, 1);
                pizza_2_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_386, "保温中...");

                lv_label_set_text_fmt(done->status, "| 披萨 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_27, 100, LV_ANIM_OFF);
            }
            current_group = g_pizza_2_complete;
            lv_scr_load_anim(pizza_2_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_SLOWCOOK_MENU:
        slowcook_rebuild_menu(child);
        break;
    case PAGE_SLOWCOOK_SET:
        slowcook_rebuild_set(child);
        break;
    case PAGE_SLOWCOOK_COOKING:
        if (child == PAGE_SLOWCOOK_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_SLOWCOOK_STOP_BACK) {
            g_on_stop_back = 0;
            slowcook_rebuild_cooking(PAGE_SLOWCOOK_SETTING);
        } else if (child == PAGE_SLOWCOOK_SETTING) {
            set_temp = slowcook_setting_saved_temp;
            set_hour = slowcook_setting_saved_hour;
            set_min = slowcook_setting_saved_min;
            slowcook_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            slowcook_rebuild_cooking(0);
        break;
    case PAGE_SLOWCOOK_SETTING:
        slowcook_rebuild_setting();
        break;
    case PAGE_SLOWCOOK_STOP:
        if (child == PAGE_SLOWCOOK_SETTING) {
            set_temp = slowcook_setting_saved_temp;
            set_hour = slowcook_setting_saved_hour;
            set_min = slowcook_setting_saved_min;
        }
        slowcook_rebuild_stop();
        break;
    case PAGE_SLOWCOOK_STOP_BACK:
        slowcook_rebuild_stop_back();
        break;
    case PAGE_SLOWCOOK_COMPLETE:
        if (child == PAGE_SLOWCOOK_SETTING) {
            set_temp = slowcook_setting_saved_temp;
            set_hour = slowcook_setting_saved_hour;
            set_min = slowcook_setting_saved_min;
        }
        if (child == PAGE_SLOWCOOK_SETTING || child == PAGE_SLOWCOOK_STOP_BACK) {
            slowcook_complete_create(&ui_manager);
            slowcook_complete_t *done = slowcook_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_slowcook_complete) lv_group_del(g_slowcook_complete);
                g_slowcook_complete = group_create_for_page(btns, 1);
                slowcook_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_435, "保温中...");

                lv_label_set_text_fmt(done->status, "| 慢煮 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_31, 100, LV_ANIM_OFF);
            }
            current_group = g_slowcook_complete;
            lv_scr_load_anim(slowcook_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_UNFROZEN_MENU:
        unfrozen_rebuild_menu(child);
        break;
    case PAGE_UNFROZEN_SET:
        unfrozen_rebuild_set(child);
        break;
    case PAGE_UNFROZEN_COOKING:
        if (child == PAGE_UNFROZEN_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_UNFROZEN_STOP_BACK) {
            g_on_stop_back = 0;
            unfrozen_rebuild_cooking(PAGE_UNFROZEN_SETTING);
        } else if (child == PAGE_UNFROZEN_SETTING) {
            set_temp = unfrozen_setting_saved_temp;
            set_hour = unfrozen_setting_saved_hour;
            set_min = unfrozen_setting_saved_min;
            unfrozen_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            unfrozen_rebuild_cooking(0);
        break;
    case PAGE_UNFROZEN_SETTING:
        unfrozen_rebuild_setting();
        break;
    case PAGE_UNFROZEN_STOP:
        if (child == PAGE_UNFROZEN_SETTING) {
            set_temp = unfrozen_setting_saved_temp;
            set_hour = unfrozen_setting_saved_hour;
            set_min = unfrozen_setting_saved_min;
        }
        unfrozen_rebuild_stop();
        break;
    case PAGE_UNFROZEN_STOP_BACK:
        unfrozen_rebuild_stop_back();
        break;
    case PAGE_UNFROZEN_COMPLETE:
        if (child == PAGE_UNFROZEN_SETTING) {
            set_temp = unfrozen_setting_saved_temp;
            set_hour = unfrozen_setting_saved_hour;
            set_min = unfrozen_setting_saved_min;
        }
        if (child == PAGE_UNFROZEN_SETTING || child == PAGE_UNFROZEN_STOP_BACK) {
            unfrozen_complete_create(&ui_manager);
            unfrozen_complete_t *done = unfrozen_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_unfrozen_complete) lv_group_del(g_unfrozen_complete);
                g_unfrozen_complete = group_create_for_page(btns, 1);
                unfrozen_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_484, "保温中...");

                lv_label_set_text_fmt(done->status, "| 解冻 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_35, 100, LV_ANIM_OFF);
            }
            current_group = g_unfrozen_complete;
            lv_scr_load_anim(unfrozen_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_RISING_MENU:
        rising_rebuild_menu(child);
        break;
    case PAGE_RISING_SET:
        rising_rebuild_set(child);
        break;
    case PAGE_RISING_COOKING:
        if (child == PAGE_RISING_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_RISING_STOP_BACK) {
            g_on_stop_back = 0;
            rising_rebuild_cooking(PAGE_RISING_SETTING);
        } else if (child == PAGE_RISING_SETTING) {
            set_temp = rising_setting_saved_temp;
            set_hour = rising_setting_saved_hour;
            set_min = rising_setting_saved_min;
            rising_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            rising_rebuild_cooking(0);
        break;
    case PAGE_RISING_SETTING:
        rising_rebuild_setting();
        break;
    case PAGE_RISING_STOP:
        if (child == PAGE_RISING_SETTING) {
            set_temp = rising_setting_saved_temp;
            set_hour = rising_setting_saved_hour;
            set_min = rising_setting_saved_min;
        }
        rising_rebuild_stop();
        break;
    case PAGE_RISING_STOP_BACK:
        rising_rebuild_stop_back();
        break;
    case PAGE_RISING_COMPLETE:
        if (child == PAGE_RISING_SETTING) {
            set_temp = rising_setting_saved_temp;
            set_hour = rising_setting_saved_hour;
            set_min = rising_setting_saved_min;
        }
        if (child == PAGE_RISING_SETTING || child == PAGE_RISING_STOP_BACK) {
            rising_complete_create(&ui_manager);
            rising_complete_t *done = rising_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_rising_complete) lv_group_del(g_rising_complete);
                g_rising_complete = group_create_for_page(btns, 1);
                rising_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_533, "保温中...");

                lv_label_set_text_fmt(done->status, "| 发酵 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_39, 100, LV_ANIM_OFF);
            }
            current_group = g_rising_complete;
            lv_scr_load_anim(rising_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_CORN_MENU:
        corn_rebuild_menu(child);
        break;
    case PAGE_CORN_SET:
        corn_rebuild_set(child);
        break;
    case PAGE_CORN_COOKING:
        if (child == PAGE_CORN_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_CORN_STOP_BACK) {
            g_on_stop_back = 0;
            corn_rebuild_cooking(PAGE_CORN_SETTING);
        } else if (child == PAGE_CORN_SETTING) {
            set_temp = corn_setting_saved_temp;
            set_hour = corn_setting_saved_hour;
            set_min = corn_setting_saved_min;
            corn_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            corn_rebuild_cooking(0);
        break;
    case PAGE_CORN_SETTING:
        corn_rebuild_setting();
        break;
    case PAGE_CORN_STOP:
        if (child == PAGE_CORN_SETTING) {
            set_temp = corn_setting_saved_temp;
            set_hour = corn_setting_saved_hour;
            set_min = corn_setting_saved_min;
        }
        corn_rebuild_stop();
        break;
    case PAGE_CORN_STOP_BACK:
        corn_rebuild_stop_back();
        break;
    case PAGE_CORN_COMPLETE:
        if (child == PAGE_CORN_SETTING) {
            set_temp = corn_setting_saved_temp;
            set_hour = corn_setting_saved_hour;
            set_min = corn_setting_saved_min;
        }
        if (child == PAGE_CORN_SETTING || child == PAGE_CORN_STOP_BACK) {
            corn_complete_create(&ui_manager);
            corn_complete_t *done = corn_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_corn_complete) lv_group_del(g_corn_complete);
                g_corn_complete = group_create_for_page(btns, 1);
                corn_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_582, "保温中...");

                lv_label_set_text_fmt(done->status, "| 干果 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_43, 100, LV_ANIM_OFF);
            }
            current_group = g_corn_complete;
            lv_scr_load_anim(corn_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_HEATCONTAIN_MENU:
        heatcontain_rebuild_menu(child);
        break;
    case PAGE_HEATCONTAIN_SET:
        heatcontain_rebuild_set(child);
        break;
    case PAGE_HEATCONTAIN_COOKING:
        if (child == PAGE_HEATCONTAIN_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_HEATCONTAIN_STOP_BACK) {
            g_on_stop_back = 0;
            heatcontain_rebuild_cooking(PAGE_HEATCONTAIN_SETTING);
        } else if (child == PAGE_HEATCONTAIN_SETTING) {
            set_temp = heatcontain_setting_saved_temp;
            set_hour = heatcontain_setting_saved_hour;
            set_min = heatcontain_setting_saved_min;
            heatcontain_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            heatcontain_rebuild_cooking(0);
        break;
    case PAGE_HEATCONTAIN_SETTING:
        heatcontain_rebuild_setting();
        break;
    case PAGE_HEATCONTAIN_STOP:
        if (child == PAGE_HEATCONTAIN_SETTING) {
            set_temp = heatcontain_setting_saved_temp;
            set_hour = heatcontain_setting_saved_hour;
            set_min = heatcontain_setting_saved_min;
        }
        heatcontain_rebuild_stop();
        break;
    case PAGE_HEATCONTAIN_STOP_BACK:
        heatcontain_rebuild_stop_back();
        break;
    case PAGE_HEATCONTAIN_COMPLETE:
        if (child == PAGE_HEATCONTAIN_SETTING) {
            set_temp = heatcontain_setting_saved_temp;
            set_hour = heatcontain_setting_saved_hour;
            set_min = heatcontain_setting_saved_min;
        }
        if (child == PAGE_HEATCONTAIN_SETTING || child == PAGE_HEATCONTAIN_STOP_BACK) {
            heatcontain_complete_create(&ui_manager);
            heatcontain_complete_t *done = heatcontain_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_heatcontain_complete) lv_group_del(g_heatcontain_complete);
                g_heatcontain_complete = group_create_for_page(btns, 1);
                heatcontain_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_672, "保温中...");

                lv_label_set_text_fmt(done->status, "| 保温 | %d℃ | %02d分钟", set_temp, set_min);
                lv_bar_set_value(done->bar_48, 100, LV_ANIM_OFF);
            }
            current_group = g_heatcontain_complete;
            lv_scr_load_anim(heatcontain_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_FROZEN_COOK:
        frozen_rebuild(child);
        break;

    case PAGE_LASAGNA_MENU:
        lasagna_rebuild_menu(child);
        break;
    case PAGE_LASAGNA_SET:
        lasagna_rebuild_set(child);
        break;
    case PAGE_LASAGNA_COOKING:
        if (child == PAGE_LASAGNA_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_LASAGNA_STOP_BACK) {
            g_on_stop_back = 0;
            lasagna_rebuild_cooking(PAGE_LASAGNA_SETTING);
        } else if (child == PAGE_LASAGNA_SETTING) {
            set_hour = lasagna_setting_saved_hour;
            set_min = lasagna_setting_saved_min;
            lasagna_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            lasagna_rebuild_cooking(0);
        break;
    case PAGE_LASAGNA_SETTING:
        lasagna_rebuild_setting();
        break;
    case PAGE_LASAGNA_STOP:
        if (child == PAGE_LASAGNA_SETTING) {
            set_hour = lasagna_setting_saved_hour;
            set_min = lasagna_setting_saved_min;
        }
        lasagna_rebuild_stop();
        break;
    case PAGE_LASAGNA_STOP_BACK:
        lasagna_rebuild_stop_back();
        break;
    case PAGE_LASAGNA_COMPLETE:
        if (child == PAGE_LASAGNA_SETTING || child == PAGE_LASAGNA_STOP_BACK) {
            lasagna_complete_create(&ui_manager);
            lasagna_complete_t *done = lasagna_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_lasagna_complete) lv_group_del(g_lasagna_complete);
                g_lasagna_complete = group_create_for_page(btns, 1);
                lasagna_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_681, "保温中...");

                if (set_hour == 0)
                    lv_label_set_text_fmt(done->status, "| 千层面 | %d℃ | %02d分钟", set_temp, set_min);
                else
                    lv_label_set_text_fmt(done->status, "| 千层面 | %d℃ | %d小时%02d分钟", set_temp, set_hour, set_min);
                lv_bar_set_value(done->bar_51, 100, LV_ANIM_OFF);
            }
            current_group = g_lasagna_complete;
            lv_scr_load_anim(lasagna_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_STRUDEL_MENU:
        strudel_rebuild_menu(child);
        break;
    case PAGE_STRUDEL_SET:
        strudel_rebuild_set(child);
        break;
    case PAGE_STRUDEL_COOKING:
        if (child == PAGE_STRUDEL_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_STRUDEL_STOP_BACK) {
            g_on_stop_back = 0;
            strudel_rebuild_cooking(PAGE_STRUDEL_SETTING);
        } else if (child == PAGE_STRUDEL_SETTING) {
            set_hour = strudel_setting_saved_hour;
            set_min = strudel_setting_saved_min;
            strudel_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            strudel_rebuild_cooking(0);
        break;
    case PAGE_STRUDEL_SETTING:
        strudel_rebuild_setting();
        break;
    case PAGE_STRUDEL_STOP:
        if (child == PAGE_STRUDEL_SETTING) {
            set_hour = strudel_setting_saved_hour;
            set_min = strudel_setting_saved_min;
        }
        strudel_rebuild_stop();
        break;
    case PAGE_STRUDEL_STOP_BACK:
        strudel_rebuild_stop_back();
        break;
    case PAGE_STRUDEL_COMPLETE:
        if (child == PAGE_STRUDEL_SETTING || child == PAGE_STRUDEL_STOP_BACK) {
            strudel_complete_create(&ui_manager);
            strudel_complete_t *done = strudel_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_strudel_complete) lv_group_del(g_strudel_complete);
                g_strudel_complete = group_create_for_page(btns, 1);
                strudel_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_722, "保温中...");

                if (set_hour == 0)
                    lv_label_set_text_fmt(done->status, "| 果馅卷 | %d℃ | %02d分钟", set_temp, set_min);
                else
                    lv_label_set_text_fmt(done->status, "| 果馅卷 | %d℃ | %d小时%02d分钟", set_temp, set_hour, set_min);
                lv_bar_set_value(done->bar_55, 100, LV_ANIM_OFF);
            }
            current_group = g_strudel_complete;
            lv_scr_load_anim(strudel_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_BREAD_MENU:
        bread_rebuild_menu(child);
        break;
    case PAGE_BREAD_SET:
        bread_rebuild_set(child);
        break;
    case PAGE_BREAD_COOKING:
        if (child == PAGE_BREAD_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_BREAD_STOP_BACK) {
            g_on_stop_back = 0;
            bread_rebuild_cooking(PAGE_BREAD_SETTING);
        } else if (child == PAGE_BREAD_SETTING) {
            set_hour = bread_setting_saved_hour;
            set_min = bread_setting_saved_min;
            bread_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            bread_rebuild_cooking(0);
        break;
    case PAGE_BREAD_SETTING:
        bread_rebuild_setting();
        break;
    case PAGE_BREAD_STOP:
        if (child == PAGE_BREAD_SETTING) {
            set_hour = bread_setting_saved_hour;
            set_min = bread_setting_saved_min;
        }
        bread_rebuild_stop();
        break;
    case PAGE_BREAD_STOP_BACK:
        bread_rebuild_stop_back();
        break;
    case PAGE_BREAD_COMPLETE:
        if (child == PAGE_BREAD_SETTING || child == PAGE_BREAD_STOP_BACK) {
            bread_complete_create(&ui_manager);
            bread_complete_t *done = bread_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_bread_complete) lv_group_del(g_bread_complete);
                g_bread_complete = group_create_for_page(btns, 1);
                bread_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_763, "保温中...");

                if (set_hour == 0)
                    lv_label_set_text_fmt(done->status, "| 面包 | %d℃ | %02d分钟", set_temp, set_min);
                else
                    lv_label_set_text_fmt(done->status, "| 面包 | %d℃ | %d小时%02d分钟", set_temp, set_hour, set_min);
                lv_bar_set_value(done->bar_59, 100, LV_ANIM_OFF);
            }
            current_group = g_bread_complete;
            lv_scr_load_anim(bread_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_PIZZA3_MENU:
        pizza3_rebuild_menu(child);
        break;
    case PAGE_PIZZA3_SET:
        pizza3_rebuild_set(child);
        break;
    case PAGE_PIZZA3_COOKING:
        if (child == PAGE_PIZZA3_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_PIZZA3_STOP_BACK) {
            g_on_stop_back = 0;
            pizza3_rebuild_cooking(PAGE_PIZZA3_SETTING);
        } else if (child == PAGE_PIZZA3_SETTING) {
            set_hour = pizza3_setting_saved_hour;
            set_min = pizza3_setting_saved_min;
            pizza3_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            pizza3_rebuild_cooking(0);
        break;
    case PAGE_PIZZA3_SETTING:
        pizza3_rebuild_setting();
        break;
    case PAGE_PIZZA3_STOP:
        if (child == PAGE_PIZZA3_SETTING) {
            set_hour = pizza3_setting_saved_hour;
            set_min = pizza3_setting_saved_min;
        }
        pizza3_rebuild_stop();
        break;
    case PAGE_PIZZA3_STOP_BACK:
        pizza3_rebuild_stop_back();
        break;
    case PAGE_PIZZA3_COMPLETE:
        if (child == PAGE_PIZZA3_SETTING || child == PAGE_PIZZA3_STOP_BACK) {
            pizza3_complete_create(&ui_manager);
            pizza3_complete_t *done = pizza3_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_pizza3_complete) lv_group_del(g_pizza3_complete);
                g_pizza3_complete = group_create_for_page(btns, 1);
                pizza3_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_804, "保温中...");

                if (set_hour == 0)
                    lv_label_set_text_fmt(done->status, "| 披萨 | %d℃ | %02d分钟", set_temp, set_min);
                else
                    lv_label_set_text_fmt(done->status, "| 披萨 | %d℃ | %d小时%02d分钟", set_temp, set_hour, set_min);
                lv_bar_set_value(done->bar_63, 100, LV_ANIM_OFF);
            }
            current_group = g_pizza3_complete;
            lv_scr_load_anim(pizza3_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_CHIP_MENU:
        chip_rebuild_menu(child);
        break;
    case PAGE_CHIP_SET:
        chip_rebuild_set(child);
        break;
    case PAGE_CHIP_COOKING:
        if (child == PAGE_CHIP_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_CHIP_STOP_BACK) {
            g_on_stop_back = 0;
            chip_rebuild_cooking(PAGE_CHIP_SETTING);
        } else if (child == PAGE_CHIP_SETTING) {
            set_hour = chip_setting_saved_hour;
            set_min = chip_setting_saved_min;
            chip_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            chip_rebuild_cooking(0);
        break;
    case PAGE_CHIP_SETTING:
        chip_rebuild_setting();
        break;
    case PAGE_CHIP_STOP:
        if (child == PAGE_CHIP_SETTING) {
            set_hour = chip_setting_saved_hour;
            set_min = chip_setting_saved_min;
        }
        chip_rebuild_stop();
        break;
    case PAGE_CHIP_STOP_BACK:
        chip_rebuild_stop_back();
        break;
    case PAGE_CHIP_COMPLETE:
        if (child == PAGE_CHIP_SETTING || child == PAGE_CHIP_STOP_BACK) {
            chip_complete_create(&ui_manager);
            chip_complete_t *done = chip_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_chip_complete) lv_group_del(g_chip_complete);
                g_chip_complete = group_create_for_page(btns, 1);
                chip_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_845, "保温中...");

                if (set_hour == 0)
                    lv_label_set_text_fmt(done->status, "| 薯条 | %d℃ | %02d分钟", set_temp, set_min);
                else
                    lv_label_set_text_fmt(done->status, "| 薯条 | %d℃ | %d小时%02d分钟", set_temp, set_hour, set_min);
                lv_bar_set_value(done->bar_67, 100, LV_ANIM_OFF);
            }
            current_group = g_chip_complete;
            lv_scr_load_anim(chip_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_CUSTOM_MENU:
        custom_rebuild_menu(child);
        break;
    case PAGE_CUSTOM_SET:
        custom_rebuild_set(child);
        break;
    case PAGE_CUSTOM_COOKING:
        if (child == PAGE_CUSTOM_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_CUSTOM_STOP_BACK) {
            g_on_stop_back = 0;
            custom_rebuild_cooking(PAGE_CUSTOM_SETTING);
        } else if (child == PAGE_CUSTOM_SETTING) {
            set_hour = custom_setting_saved_hour;
            set_min = custom_setting_saved_min;
            custom_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            custom_rebuild_cooking(0);
        break;
    case PAGE_CUSTOM_SETTING:
        custom_rebuild_setting();
        break;
    case PAGE_CUSTOM_STOP:
        if (child == PAGE_CUSTOM_SETTING) {
            set_hour = custom_setting_saved_hour;
            set_min = custom_setting_saved_min;
        }
        custom_rebuild_stop();
        break;
    case PAGE_CUSTOM_STOP_BACK:
        custom_rebuild_stop_back();
        break;
    case PAGE_CUSTOM_COMPLETE:
        if (child == PAGE_CUSTOM_SETTING || child == PAGE_CUSTOM_STOP_BACK) {
            custom_complete_create(&ui_manager);
            custom_complete_t *done = custom_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_custom_complete) lv_group_del(g_custom_complete);
                g_custom_complete = group_create_for_page(btns, 1);
                custom_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_886, "保温中...");

                if (set_hour == 0)
                    lv_label_set_text_fmt(done->status, "| 自定义 | %d℃ | %02d分钟", set_temp, set_min);
                else
                    lv_label_set_text_fmt(done->status, "| 自定义 | %d℃ | %d小时%02d分钟", set_temp, set_hour, set_min);
                lv_bar_set_value(done->bar_71, 100, LV_ANIM_OFF);
            }
            current_group = g_custom_complete;
            lv_scr_load_anim(custom_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_CLEAN_MENU:
        clean_rebuild(child);
        break;
    case PAGE_HOTCLEAN_MENU:
        hotclean_rebuild(child);
        break;
    case PAGE_HOTCLEANSAVE_SET:
        hcs_rebuild_set(child);
        break;
    case PAGE_HOTCLEANSAVE_COOKING:
        if (child == PAGE_HOTCLEANSAVE_COMPLETE)
            goto pop_to_clean;
        if (child == PAGE_HOTCLEANSAVE_COOLING) {
            hcs_rebuild_cooking(child);
        } else if (child == PAGE_HOTCLEANSAVE_STOP_BACK) {
            g_on_stop_back = 0;
            hcs_rebuild_cooking(PAGE_HOTCLEANSAVE_STOP);
        } else
            hcs_rebuild_cooking(0);
        break;
    case PAGE_HOTCLEANSAVE_STOP:
        hcs_rebuild_stop();
        break;
    case PAGE_HOTCLEANSAVE_STOP_BACK:
        hcs_rebuild_stop_back();
        break;
    case PAGE_HOTCLEANSAVE_COOLING:
        if (child == PAGE_HOTCLEANSAVE_COMPLETE)
            goto pop_to_clean;
        hcs_rebuild_cooling();
        break;
    case PAGE_HOTCLEANSAVE_COMPLETE:
        if (child == PAGE_HOTCLEANSAVE_STOP_BACK) {
            hotcleansave_complete_create(&ui_manager);
            hotcleansave_complete_t *done = hotcleansave_complete_get(&ui_manager);
            if (done) {
                lv_bar_set_value(done->bar_1, 100, LV_ANIM_OFF);
            }
            current_group = g_hcs_complete;
            lv_scr_load_anim(hotcleansave_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_clean;
    case PAGE_HOTCLEANMIDDLE_SET:
        hcm_rebuild_set(child);
        break;
    case PAGE_HOTCLEANMIDDLE_COOKING:
        if (child == PAGE_HOTCLEANMIDDLE_COMPLETE)
            goto pop_to_clean;
        if (child == PAGE_HOTCLEANMIDDLE_COOLING) {
            hcm_rebuild_cooking(child);
        } else if (child == PAGE_HOTCLEANMIDDLE_STOP_BACK) {
            g_on_stop_back = 0;
            hcm_rebuild_cooking(PAGE_HOTCLEANMIDDLE_STOP);
        } else
            hcm_rebuild_cooking(0);
        break;
    case PAGE_HOTCLEANMIDDLE_STOP:
        hcm_rebuild_stop();
        break;
    case PAGE_HOTCLEANMIDDLE_STOP_BACK:
        hcm_rebuild_stop_back();
        break;
    case PAGE_HOTCLEANMIDDLE_COOLING:
        if (child == PAGE_HOTCLEANMIDDLE_COMPLETE)
            goto pop_to_clean;
        hcm_rebuild_cooling();
        break;
    case PAGE_HOTCLEANMIDDLE_COMPLETE:
        if (child == PAGE_HOTCLEANMIDDLE_STOP_BACK) {
            hotcleanmiddle_complete_create(&ui_manager);
            hotcleanmiddle_complete_t *done = hotcleanmiddle_complete_get(&ui_manager);
            if (done) {
                lv_bar_set_value(done->bar_5, 100, LV_ANIM_OFF);
            }
            current_group = g_hcm_complete;
            lv_scr_load_anim(hotcleanmiddle_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_clean;
    case PAGE_HOTCLEANHIGH_SET:
        hch_rebuild_set(child);
        break;
    case PAGE_HOTCLEANHIGH_COOKING:
        if (child == PAGE_HOTCLEANHIGH_COMPLETE)
            goto pop_to_clean;
        if (child == PAGE_HOTCLEANHIGH_COOLING) {
            hch_rebuild_cooking(child);
        } else if (child == PAGE_HOTCLEANHIGH_STOP_BACK) {
            g_on_stop_back = 0;
            hch_rebuild_cooking(PAGE_HOTCLEANHIGH_STOP);
        } else
            hch_rebuild_cooking(0);
        break;
    case PAGE_HOTCLEANHIGH_STOP:
        hch_rebuild_stop();
        break;
    case PAGE_HOTCLEANHIGH_STOP_BACK:
        hch_rebuild_stop_back();
        break;
    case PAGE_HOTCLEANHIGH_COOLING:
        if (child == PAGE_HOTCLEANHIGH_COMPLETE)
            goto pop_to_clean;
        hch_rebuild_cooling();
        break;
    case PAGE_HOTCLEANHIGH_COMPLETE:
        if (child == PAGE_HOTCLEANHIGH_STOP_BACK) {
            hotcleanhigh_complete_create(&ui_manager);
            hotcleanhigh_complete_t *done = hotcleanhigh_complete_get(&ui_manager);
            if (done) {
                lv_bar_set_value(done->bar_9, 100, LV_ANIM_OFF);
            }
            current_group = g_hch_complete;
            lv_scr_load_anim(hotcleanhigh_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_clean;
    case PAGE_WATER_CLEAN_SET:
        wc_rebuild_set(child);
        break;
    case PAGE_WATER_CLEAN_COOKING:
        if (child == PAGE_WATER_CLEAN_COMPLETE)
            goto pop_to_clean;
        if (child == PAGE_WATER_CLEAN_STOP_BACK) {
            g_on_stop_back = 0;
            wc_rebuild_cooking(PAGE_WATER_CLEAN_STOP);
        } else
            wc_rebuild_cooking(child);
        break;
    case PAGE_WATER_CLEAN_STOP:
        wc_rebuild_stop();
        break;
    case PAGE_WATER_CLEAN_STOP_BACK:
        wc_rebuild_stop_back();
        break;
    case PAGE_WATER_CLEAN_COMPLETE:
        if (child == PAGE_WATER_CLEAN_STOP_BACK) {
            waterclean_complete_create(&ui_manager);
            waterclean_complete_t *done = waterclean_complete_get(&ui_manager);
            if (done) {
                lv_bar_set_value(done->bar_4, 100, LV_ANIM_OFF);
            }
            current_group = g_wc_complete;
            lv_scr_load_anim(waterclean_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_clean;

    pop_to_clean:
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        set_hour = 0; set_min = 10;
        cook_elapsed_saved = 0; cook_bar_saved = 0;
        depth = 2;
        clean_rebuild(0);
        break;

    case PAGE_PREHEAT_MENU:
        preheat_rebuild_menu(child);
        break;
    case PAGE_COLOR_MENU:
        color_menu_rebuild(child);
        break;
    case PAGE_PREHEAT_COOKING:
        if (child == PAGE_PREHEAT_STOP_BACK) {
            g_on_stop_back = 0;
            if (!cook_timer) {
                cook_start_time = lv_tick_get() - cook_elapsed_saved;
                cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
            }
            preheat_rebuild_cooking();
            break;
        }
        goto pop_to_major_menu;
    case PAGE_PREHEAT_STOP:
        preheat_rebuild_stop();
        break;
    case PAGE_PREHEAT_STOP_BACK:
        preheat_rebuild_stop_back();
        break;
    case PAGE_PREHEAT_COMPLETE:
        if (child == PAGE_PREHEAT_STOP_BACK) {
            preheat_rebuild_complete();
            break;
        }
        goto pop_to_major_menu;

    pop_to_major_menu:
        /* 关闭定时器 */
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
        cook_is_color = 0;
        cook_elapsed_saved = 0; cook_bar_saved = 0;

        g_send.iface_status = IFACE_SETTING;
        g_send.cook_mode = MODE_NONE;
        g_send.cook_flag = 0;
        g_send.set_temp = 0;
        g_send.set_temp_lower = 0;
        g_send.remaining_ms = -1;

        if (is_probe_inserted()) {
            depth = 2;
            page_stack[1] = PAGE_MAJOR_MENU_TZ;
            major_menu_tz_rebuild(0);
        } else {
            depth = 2;
            page_stack[1] = PAGE_MAJOR_MENU;   /* goto 进入时 stack[1] 可能是任意 prev 页，显式修正 */
            lv_obj_clean(lv_scr_act());
            major_menu_create(&ui_manager);
            groups_create();
            bind_events();
            current_group = g_major_menu;
            lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0,
                             ui_manager.auto_del);
            printf("[nav] pop to major_menu\n");
        }
        break;

    case PAGE_MAJOR_MENU_TZ:
        major_menu_tz_rebuild(child);
        break;

    case PAGE_COOK_MENU_TZ:
        cook_menu_tz_rebuild(child);
        break;

    case PAGE_SPECIAL_MENU_TZ:
        special_menu_tz_rebuild(child);
        break;

    case PAGE_PROBETIP:
        probetip_cancel_auto_dismiss();
        goto pop_to_waitmenu;

    case PAGE_WAITMENU_24:
    pop_to_waitmenu:
        waitmenu_24_create(&ui_manager);
        waitmenu_clock_cache_reset();   /* 强制刷新为真实时间 */
        current_group = NULL;
        lv_scr_load_anim(waitmenu_24_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        waitmenu_apply_clock();   /* 立即刷新为真实时间，不等 500ms 定时器 */
        g_send.iface_status = IFACE_STANDBY;
        printf("[nav] back to waitmenu_24\n");
        break;

    default:
        printf("[nav] unknown page to restore: %d, fallback to waitmenu_24\n", prev);
        goto pop_to_waitmenu;
    }
}

// ==============================
// 焦点组创建
// ==============================

// 创建 g_major_menu（只在初始化时调一次，返回时也会调）
void groups_create(void)
{
    major_menu_t *major = major_menu_get(&ui_manager);
    if (!major) {
        printf("[nav] major_menu struct is NULL\n");
        return;
    }

    if (g_major_menu) lv_group_del(g_major_menu);
    g_major_menu = lv_group_create();
    if (!g_major_menu) {
        printf("[nav] failed to create major_menu group\n");
        return;
    }

    /* 按焦点顺序依次加入按钮 */
    if (major->cook_button)
        lv_group_add_obj(g_major_menu, major->cook_button);
    else
        printf("[nav] major->cook_button is NULL\n");

    if (major->cook4_button)
        lv_group_add_obj(g_major_menu, major->cook4_button);
    else
        printf("[nav] major->cook4_button is NULL\n");

    if (major->special_button)
        lv_group_add_obj(g_major_menu, major->special_button);
    else
        printf("[nav] major->special_button is NULL\n");

    printf("[nav] major_menu group created\n");
}

// 将 buttons 数组中的非 NULL 对象全部加入 group（统一 NULL 检查）
static void group_add_all_btns(lv_group_t *g, lv_obj_t **btns, int count)
{
    for (int i = 0; i < count; i++) {
        if (btns[i])
            lv_group_add_obj(g, btns[i]);
    }
}

// 创建 group 并加入所有按钮（跳转子页时调用）
lv_group_t *group_create_for_page(lv_obj_t **btns, int count)
{
    lv_group_t *g = lv_group_create();
    if (g)
        group_add_all_btns(g, btns, count);
    return g;
}

// ==============================
// 页面跳转
// ==============================

// major_menu → cookmenu
void jump_to_cookmenu(void)
{
    page_push(PAGE_COOKMENU);  // 推栈
    lv_obj_clean(lv_scr_act());
    cookmenu_create(&ui_manager);

    cookmenu_t *cook = cookmenu_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = {
            cook->up_down_button, cook->top_bbq_button, cook->hot_bbq_button,
            cook->hot_wind_button, cook->save_button, cook->bottom_button,
            cook->central_button, cook->windchange_buttonn, cook->preheater_button,
        };
        if (g_cookmenu) lv_group_del(g_cookmenu);  // 旧 group 可能引用已销毁的按钮
        g_cookmenu = group_create_for_page(btns, sizeof(btns) / sizeof(btns[0]));
    }

    current_group = g_cookmenu;

    /* 绑定 cookmenu 按钮的点击事件 */
    if (cook && cook->up_down_button)
        lv_obj_add_event_cb(cook->up_down_button, on_cook_updown_click,
                            LV_EVENT_CLICKED, NULL);
    if (cook && cook->top_bbq_button)
        lv_obj_add_event_cb(cook->top_bbq_button, on_top_bbq_click,
                            LV_EVENT_CLICKED, NULL);
    if (cook && cook->bottom_button)
        lv_obj_add_event_cb(cook->bottom_button, on_bottom_bbq_click,
                             LV_EVENT_CLICKED, NULL);
    if (cook && cook->hot_bbq_button)
        lv_obj_add_event_cb(cook->hot_bbq_button, on_hot_bbq_click,
                            LV_EVENT_CLICKED, NULL);
    if (cook && cook->hot_wind_button)
        lv_obj_add_event_cb(cook->hot_wind_button, on_hotwind_click,
                            LV_EVENT_CLICKED, NULL);
    if (cook && cook->save_button)
        lv_obj_add_event_cb(cook->save_button, on_save_click,
                            LV_EVENT_CLICKED, NULL);
    if (cook && cook->central_button)
        lv_obj_add_event_cb(cook->central_button, on_central_click,
                            LV_EVENT_CLICKED, NULL);
    if (cook && cook->windchange_buttonn)
        lv_obj_add_event_cb(cook->windchange_buttonn, on_windchange_click,
                            LV_EVENT_CLICKED, NULL);
    if (cook && cook->preheater_button)
        lv_obj_add_event_cb(cook->preheater_button, on_preheat_click,
                            LV_EVENT_CLICKED, NULL);

    lv_scr_load_anim(cookmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    printf("[nav] jump: major_menu -> cookmenu\n");
}

// major_menu → special_menu
static void jump_to_special_menu(void)
{
    page_push(PAGE_SPECIAL_MENU);
    lv_obj_clean(lv_scr_act());
    special_menu_create(&ui_manager);

    special_menu_t *sp = special_menu_get(&ui_manager);
    if (sp) {
        lv_obj_t *btns[] = {
            sp->air_button, sp->piza_button, sp->frozen_cook_button,
            sp->slow_cook_button, sp->unfrozen_button, sp->rising_button,
            sp->corn_button, sp->heat_contain_button, sp->some_cook_button,
        };

        if (g_special_menu) lv_group_del(g_special_menu);
        g_special_menu = group_create_for_page(btns, sizeof(btns) / sizeof(btns[0]));

        lv_obj_add_event_cb(sp->air_button, on_air_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(sp->piza_button, on_pizza_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(sp->slow_cook_button, on_slowcook_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(sp->unfrozen_button, on_unfrozen_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(sp->rising_button, on_rising_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(sp->corn_button, on_corn_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(sp->heat_contain_button, on_heatcontain_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(sp->frozen_cook_button, on_frozen_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(sp->some_cook_button, on_some_cook_click,
                            LV_EVENT_CLICKED, NULL);
    }

    current_group = g_special_menu;

    lv_scr_load_anim(special_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    printf("[nav] jump: major_menu -> special_menu\n");
}

// cookmenu → updown_bbq_menu
void jump_to_updown_bbq_menu(void)
{
    set_temp = 180; set_hour = 0; set_min = 30;
    page_push(PAGE_UPDOWN_BBQ_MENU);
    lv_obj_clean(lv_scr_act());
    updown_bbq_menu_create(&ui_manager);

    updown_bbq_menu_t *bbq = updown_bbq_menu_get(&ui_manager);
    if (bbq) {
        lv_obj_t *btns[] = {
            bbq->tempnum_label, bbq->hournum_label, bbq->minnum_label,
            bbq->next_button,
        };
        if (g_updown_bbq_menu) lv_group_del(g_updown_bbq_menu);
        g_updown_bbq_menu = group_create_for_page(btns, 4);

        /* 注册编辑字段 */
        edit_clear();
        edit_register(bbq->tempnum_label, bbq->templine_short, bbq->temeline_long,
                      &set_temp, 30, 300, 5, "%d");
        edit_register(bbq->hournum_label, bbq->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(bbq->minnum_label, bbq->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        /* 绑定 focus 高亮 */
        lv_obj_add_event_cb(bbq->tempnum_label, on_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(bbq->hournum_label, on_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(bbq->minnum_label, on_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(bbq->next_button, on_edit_focus, LV_EVENT_FOCUSED, NULL);

        /* 初始化数值显示（覆盖 UiBuilder 默认值） */
        lv_label_set_text_fmt(bbq->tempnum_label, "%d", set_temp);
        lv_label_set_text_fmt(bbq->hournum_label, "%02d", set_hour);
        lv_label_set_text_fmt(bbq->minnum_label, "%02d", set_min);

        /* 初始显示温度线 */
        lv_obj_add_flag(bbq->templine_short, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(bbq->temeline_long, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(bbq->templine_short, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(bbq->temeline_long, LV_OBJ_FLAG_HIDDEN);
    }

    /* 初始化约束（设置 min/max 匹配当前 hour） */
    validate_constraints();

    current_group = g_updown_bbq_menu;

    /* 默认焦点在 next_button */
    if (bbq && bbq->next_button)
        lv_group_focus_obj(bbq->next_button);

    /* 绑定 next_button 事件 */
    if (bbq && bbq->next_button)
        lv_obj_add_event_cb(bbq->next_button, on_updown_next_click,
                            LV_EVENT_CLICKED, NULL);

    lv_scr_load_anim(updown_bbq_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    g_send.cook_mode = MODE_UPDOWN_BBQ;
    printf("[nav] jump: cookmenu -> updown_bbq_menu\n");
}

// updown_bbq_menu → updown_bbq_set
static void jump_to_updown_bbq_set(void)
{
    set_temp_up = set_temp; set_temp_down = set_temp;
    page_push(PAGE_UPDOWN_BBQ_SET);
    lv_obj_clean(lv_scr_act());
    updown_bbq_set_create(&ui_manager);

    /* 复位 toggle 状态（全新进入，默认全关） */
    preheat_on = 0;
    delay_on = 0;
    contain_on = 0;

    updown_bbq_set_t *set = updown_bbq_set_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->sure_button, set->uptemp_button, set->downtemp_button,
            set->preheat_button, set->preheat_on_button,
            set->delay_button, set->delay_on_button,
            set->contain_button, set->contain_on_button,
        };
        if (g_updown_bbq_set) lv_group_del(g_updown_bbq_set);
        g_updown_bbq_set = group_create_for_page(btns, sizeof(btns) / sizeof(btns[0]));

        /* 强制清除所有按钮的 FOCUSED 状态，防止渲染缓存残留 */
        clear_focus_states(btns, sizeof(btns)/sizeof(btns[0]));
        lv_group_focus_obj(set->sure_button);

        /* 温度显示 */
        setup_set_temp_display(set);

        /* 时间显示 */
        lv_label_set_text_fmt(set->hour_label, "%02d", set_hour);
        lv_label_set_text_fmt(set->min_label, "%02d", set_min);

        if (set_hour == 0) {
            lv_obj_add_flag(set->hour_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(set->shi_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(set->min_label, 312, 254);
            lv_obj_set_pos(set->fen_label, 365, 269);
        } else {
            lv_obj_clear_flag(set->hour_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->shi_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(set->min_label, 395, 254);
            lv_obj_set_pos(set->fen_label, 448, 269);
        }

        /* 按钮状态重置（跟随变量） */
        apply_toggle_state(set->preheat_button, set->preheat_on_button, preheat_on);
        apply_toggle_state(set->delay_button, set->delay_on_button, delay_on);
        apply_toggle_state(set->contain_button, set->contain_on_button, contain_on);
        updown_set_apply_delay_label(set);

        /* 绑定 toggle 事件 */
        lv_obj_add_event_cb(set->preheat_button, on_preheat_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->preheat_on_button, on_preheat_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->delay_button, on_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->delay_on_button, on_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->contain_button, on_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->contain_on_button, on_contain_toggle, LV_EVENT_CLICKED, NULL);

        /* 绑定温度按钮事件 */
        lv_obj_add_event_cb(set->uptemp_button, on_updown_uptemp_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->downtemp_button, on_updown_downtemp_click,
                            LV_EVENT_CLICKED, NULL);

        /* 绑定 sure_button 事件 */
        lv_obj_add_event_cb(set->sure_button, on_sure_click, LV_EVENT_CLICKED, NULL);
    }

    current_group = g_updown_bbq_set;

    lv_scr_load_anim(updown_bbq_set_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    printf("[nav] jump: updown_bbq_menu -> updown_bbq_set\n");
}

// updown_bbq_set → updown_bbq_cooking
void jump_to_updown_bbq_cooking(void)
{
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    g_on_stop_back = 0;
    g_keepwarm_active = 0;
    g_stop_back_complete = NULL;
    page_push(PAGE_UPDOWN_BBQ_COOKING);
    lv_obj_clean(lv_scr_act());
    updown_bbq_cooking_create(&ui_manager);

    updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = {
            cook->stop_button, cook->little_button,
        };
        if (g_updown_bbq_cooking) lv_group_del(g_updown_bbq_cooking);
        g_updown_bbq_cooking = group_create_for_page(btns, sizeof(btns) / sizeof(btns[0]));

        /* 绑定停止按钮 */
        lv_obj_add_event_cb(cook->stop_button, on_cook_stop_click,
                            LV_EVENT_CLICKED, NULL);

        /* 绑定设置按钮 */
        lv_obj_add_event_cb(cook->little_button, on_cook_setting_click,
                            LV_EVENT_CLICKED, NULL);

        /* 更新界面显示 */
        set_status_label_min(cook->updown_label, set_temp_up, set_temp_down, set_hour, set_min);
    }

    /* 立即显示倒计时初始值（覆盖 UiBuilder 默认值） */
    if (cook)
        lv_label_set_text_fmt(cook->time_label, "%02d:%02d:%02d", set_hour, set_min, 0);

    /* 初始化上下温度（从 set 页带入） */
    set_temp_up = set_temp;
    set_temp_down = set_temp;

    /* 初始化进度条 + 启动动画（连续无级平滑） */
    cook_total_ms = (set_hour * 3600 + set_min * 60) * 1000;
    if (cook) {
        lv_bar_set_range(cook->bar, 0, 100);
        lv_bar_set_value(cook->bar, 3, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    /* 启动时间显示定时器 */
    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_updown_bbq_cooking;

    lv_scr_load_anim(updown_bbq_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = (uint16_t)set_temp_up;
    g_send.set_temp_lower = (uint16_t)set_temp_down;
    g_send.remaining_ms = cook_total_ms;

    printf("[nav] jump: updown_bbq_set -> updown_bbq_cooking\n");
}

// updown_bbq_cooking → updown_bbq_complete
static void jump_to_updown_bbq_complete(void)
{
    if (depth > 0 && page_stack[depth - 1] == PAGE_UPDOWN_BBQ_STOP_BACK)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_UPDOWN_BBQ_STOP)
        depth--;
    page_push(PAGE_UPDOWN_BBQ_COMPLETE);
    lv_obj_clean(lv_scr_act());
    updown_bbq_complete_create(&ui_manager);

    updown_bbq_complete_t *cook = updown_bbq_complete_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->little_button };
        if (g_updown_bbq_complete) lv_group_del(g_updown_bbq_complete);
        g_updown_bbq_complete = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cook->little_button, on_cook_setting_click,
                            LV_EVENT_CLICKED, NULL);
        set_status_label_min(cook->statu_label, set_temp_up, set_temp_down, set_hour, set_min);
        lv_bar_set_value(cook->bar_3, 100, LV_ANIM_OFF);
    }

    current_group = g_updown_bbq_complete;

    lv_scr_load_anim(updown_bbq_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    g_send.iface_status = IFACE_COMPLETE;
    g_send.remaining_ms = 0;

    /* 自动保温：保温开关开启时 complete 页停留 1 分钟无操作 → 保温（15 分钟） */
    if (contain_on) {
        g_keepwarm_active = 0;
        g_keepwarm_sec = 0;
        if (cook_timer) lv_timer_del(cook_timer);
        cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
        printf("[keepwarm] complete enter (contain on): active=%d sec=%d timer=%p\n",
               g_keepwarm_active, g_keepwarm_sec, (void *)cook_timer);
    } else {
        printf("[keepwarm] complete enter (contain off): no timer\n");
    }
    printf("[nav] jump: updown_bbq_cooking -> updown_bbq_complete\n");
}

// updown_bbq_set → updown_bbq_menu_top（设置上层温度）
static void on_updown_uptemp_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_menu_top();
}

static void on_updown_downtemp_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_menu_low();
}

static void on_updown_top_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr)) {
        updown_menu_top_saved = set_temp_up;
        page_pop();
    }
}

static void on_updown_low_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr)) {
        updown_menu_low_saved = set_temp_down;
        page_pop();
    }
}

static void jump_to_updown_bbq_menu_top(void)
{
    updown_menu_top_saved = set_temp_up;
    page_push(PAGE_UPDOWN_BBQ_MENU_TOP);
    lv_obj_clean(lv_scr_act());
    updown_bbq_menu_top_create(&ui_manager);

    updown_bbq_menu_top_t *menu = updown_bbq_menu_top_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = { menu->temp, menu->next };
        if (g_updown_bbq_menu_top) lv_group_del(g_updown_bbq_menu_top);
        g_updown_bbq_menu_top = group_create_for_page(btns, 2);

        edit_clear();
        edit_register(menu->temp, menu->line2, menu->line3,
                      &set_temp_up, 30, 300, 5, "%d");

        lv_obj_add_event_cb(menu->temp, on_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_edit_focus, LV_EVENT_FOCUSED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp_up);

        lv_obj_add_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp_up < 100) {
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        }

        lv_group_focus_obj(menu->next);
    }
    current_group = g_updown_bbq_menu_top;

    if (menu && menu->next)
        lv_obj_add_event_cb(menu->next, on_updown_top_next_click,
                            LV_EVENT_CLICKED, NULL);

    lv_scr_load_anim(updown_bbq_menu_top_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] jump: updown_bbq_set -> updown_bbq_menu_top\n");
}

static void jump_to_updown_bbq_menu_low(void)
{
    updown_menu_low_saved = set_temp_down;
    page_push(PAGE_UPDOWN_BBQ_MENU_LOW);
    lv_obj_clean(lv_scr_act());
    updown_bbq_menu_low_create(&ui_manager);

    updown_bbq_menu_low_t *menu = updown_bbq_menu_low_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = { menu->temp, menu->next };
        if (g_updown_bbq_menu_low) lv_group_del(g_updown_bbq_menu_low);
        g_updown_bbq_menu_low = group_create_for_page(btns, 2);

        edit_clear();
        edit_register(menu->temp, menu->line2, menu->line3,
                      &set_temp_down, 30, 300, 5, "%d");

        lv_obj_add_event_cb(menu->temp, on_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_edit_focus, LV_EVENT_FOCUSED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp_down);

        lv_obj_add_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp_down < 100) {
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        }

        lv_group_focus_obj(menu->next);
    }
    current_group = g_updown_bbq_menu_low;

    if (menu && menu->next)
        lv_obj_add_event_cb(menu->next, on_updown_low_next_click,
                            LV_EVENT_CLICKED, NULL);

    lv_scr_load_anim(updown_bbq_menu_low_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] jump: updown_bbq_set -> updown_bbq_menu_low\n");
}

// 额外上色设置页（复用 preheatmenu 结构）：label_69="额外上色"，温度 30-300，next → color cooking
static void color_menu_open(void)
{
    preheatmenu_create(&ui_manager);
    preheatmenu_t *menu = preheatmenu_get(&ui_manager);
    if (menu) {
        lv_label_set_text(menu->label_69, "额外上色");
        lv_obj_t *btns[] = { menu->temp, menu->next };
        if (g_preheat_menu) lv_group_del(g_preheat_menu);
        g_preheat_menu = group_create_for_page(btns, 2);

        edit_clear();
        edit_register(menu->temp, menu->line2, menu->line3,
                      &set_temp, 30, 300, 5, "%d");

        lv_obj_add_event_cb(menu->temp, on_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_edit_focus, LV_EVENT_FOCUSED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp);

        lv_obj_add_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_event_cb(menu->next, on_color_menu_next_click,
                            LV_EVENT_CLICKED, NULL);
        lv_group_focus_obj(menu->next);
    }
    current_group = g_preheat_menu;

    lv_scr_load_anim(preheatmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] color_menu open\n");
}

static void on_color_menu_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    jump_to_color_cookoing();
}

static void jump_to_color_menu(void)
{
    set_temp = 180;                              /* 默认温度 */
    g_color_from_probe = is_probe_inserted();   /* 记录进入时的探针状态（返回时区分） */
    page_push(PAGE_COLOR_MENU);
    lv_obj_clean(lv_scr_act());
    color_menu_open();
}

static void color_menu_rebuild(page_id_t child)
{
    color_menu_open();
}

// extra_color → color_cookoing（固定 5 分钟倒计时）
static void jump_to_color_cookoing(void)
{
    edit_clear();   /* 清除编辑注册表残留（color_menu_open 注册了 preheatmenu 的字段） */
    g_on_stop_back = 0;
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    page_push(PAGE_COLOR_COOKING);
    lv_obj_clean(lv_scr_act());
    color_cookoing_create(&ui_manager);

    color_cookoing_t *cc = color_cookoing_get(&ui_manager);
    if (cc) {
        lv_obj_t *btns[] = { cc->stop_button };
        if (g_color_cookoing) lv_group_del(g_color_cookoing);
        g_color_cookoing = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cc->stop_button, on_color_stop_click,
                            LV_EVENT_CLICKED, NULL);
    }

    /* 初始化显示 */
    if (cc) {
        lv_label_set_text_fmt(cc->status_label, "| 额外上色 | %d℃ | 5分钟", set_temp);
        lv_label_set_text_fmt(cc->time_label, "%02d:%02d:%02d", 0, 5, 0);
        lv_bar_set_range(cc->bar, 0, 100);
        lv_bar_set_value(cc->bar, 3, LV_ANIM_OFF);
    }

    /* 启动进度条动画 */
    cook_total_ms = 1 * 60 * 1000;
    if (cc) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cc->bar);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    /* 启动定时器 */
    cook_is_color = 1;
    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    g_send.iface_status = IFACE_COOKING;
    g_send.cook_mode = MODE_EXTRA_COLOR;
    g_send.set_temp = set_temp;   /* 跟随原烹饪模式温度 */
    g_send.set_temp_lower = set_temp_down;
    g_send.remaining_ms = cook_total_ms;
    g_send.cook_flag = 0;

    current_group = g_color_cookoing;

    lv_scr_load_anim(color_cookoing_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    printf("[nav] jump: extra_color -> color_cookoing\n");
}

// color_cookoing → colorcooking_complete
static void jump_to_color_complete(void)
{
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }   /* 防御:防残留 */
    cook_is_color = 0;
    if (depth > 0 && page_stack[depth - 1] == PAGE_COLOR_STOP_BACK)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_COLOR_STOP)
        depth--;
    /* 压缩本轮上色层（EXTRA_COLOR/COLOR_COOKING 仅本轮使用）：
       防止完成态连续再上色时栈无限累积（MAX_STACK=16） */
    if (depth > 0 && page_stack[depth - 1] == PAGE_COLOR_COOKING)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_EXTRA_COLOR)
        depth--;
    page_push(PAGE_COLOR_COOKING_COMPLETE);
    lv_obj_clean(lv_scr_act());
    colorcooking_complete_create(&ui_manager);

    {
        colorcooking_complete_t *cc = colorcooking_complete_get(&ui_manager);
        if (cc) {
            lv_label_set_text_fmt(cc->status_label, "| 额外上色 | %d℃ | 5分钟", set_temp);
            lv_bar_set_range(cc->bar, 0, 100);
            lv_bar_set_value(cc->bar, 100, LV_ANIM_OFF);
        }
    }
    current_group = NULL;

    lv_scr_load_anim(colorcooking_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COMPLETE;
    g_send.remaining_ms = 0;
    printf("[nav] jump: color_cookoing -> colorcooking_complete\n");
}

// ==============================
// 按键处理（状态机防抖）
// ==============================

typedef enum { KEY_IDLE, KEY_PRESSED } key_state_t;
static key_state_t key_state = KEY_IDLE;
static uint8_t active_key = 0;
static uint32_t active_key_time = 0;

#define ENC_REPEAT_MS   50    // 编码器长按时重复间隔

// color（额外上色）流程退出回主页：
// 探针模式进入 → 探针主菜单；否则普通主菜单。完整清理定时器/标志/栈。
static void color_exit_to_home(void)
{
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    g_on_stop_back = 0;
    g_extra_color_to_stop_back = 0;
    g_complete_to_stop_back = 0;
    g_cooling_to_stop_back = 0;
    g_keepwarm_active = 0;
    g_keepwarm_sec = 0;
    cook_is_color = 0;
    g_stop_back_complete = NULL;
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.cook_flag = 0;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    if (g_color_from_probe) {
        depth = 2;
        page_stack[1] = PAGE_MAJOR_MENU_TZ;
        lv_obj_clean(lv_scr_act());   /* 与普通分支对称，避免依赖 auto_del 回收旧屏 */
        major_menu_tz_rebuild(0);
        printf("[color] exit -> major_menu_tz (probe)\n");
    } else {
        depth = 2;
        page_stack[1] = PAGE_MAJOR_MENU;
        lv_obj_clean(lv_scr_act());
        major_menu_create(&ui_manager);
        groups_create();
        bind_events();
        current_group = g_major_menu;
        lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[color] exit -> major_menu\n");
    }
}

// MENU/CLEAN 键白名单：仅在菜单/待机/探针提示类页面有效，
// 设置温度时间页、运行状态页、完成页等一律无效音
static int menu_clean_key_allowed(void)
{
    if (depth <= 0) return 0;
    switch (page_stack[depth - 1]) {
    case PAGE_WAITMENU_24:
    case PAGE_PROBETIP:
    case PAGE_MAJOR_MENU:
    case PAGE_MAJOR_MENU_TZ:
    case PAGE_COOKMENU:
    case PAGE_COOK_MENU_TZ:
    case PAGE_SPECIAL_MENU:
    case PAGE_SPECIAL_MENU_TZ:
    case PAGE_COOK4_MENU:
    case PAGE_FROZEN_COOK:
    case PAGE_CLEAN_MENU:
    case PAGE_HOTCLEAN_MENU:
        return 1;
    default:
        return 0;
    }
}

static void process_key(uint8_t key)
{
    if (g_send.iface_status == IFACE_SLEEP) return;
    uart_data_receive[Receive_data_Touch_Key] = 0;

    switch (key) {
    case KEY1:              // 1: 开关机键，短按无操作
        break;
    case KEY_MENU:          // 3: 进入主菜单
        if (!menu_clean_key_allowed() || g_send.iface_status == IFACE_COOKING) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            break;
        }
        if (depth > 0) {
            page_id_t cur = page_stack[depth - 1];
            if (cur == PAGE_MAJOR_MENU || cur == PAGE_MAJOR_MENU_TZ) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;
                break;
            }
        }
        g_send.buzzer_req = BUZZER_KEY_VALID;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        g_on_stop_back = 0;
        g_complete_to_stop_back = 0;
        g_cooling_to_stop_back = 0;
        g_extra_color_to_stop_back = 0;
        g_keepwarm_active = 0;
        cook_is_color = 0;
        g_stop_back_complete = NULL;
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        if (is_probe_inserted()) {
            jump_to_major_menu_tz();
        } else {
            page_push(PAGE_MAJOR_MENU);
            lv_obj_clean(lv_scr_act());
            major_menu_create(&ui_manager);
            groups_create();
            bind_events();
            current_group = g_major_menu;
            lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0,
                             ui_manager.auto_del);
        }
        g_send.iface_status = IFACE_SETTING;
        g_send.cook_mode = MODE_NONE;
        g_send.set_temp = 0;
        g_send.set_temp_lower = 0;
        g_send.remaining_ms = -1;
        uart_print();
        break;
    case KEY_EXTRA_COLOR:   // 5: 进入额外上色
        if (g_send.iface_status == IFACE_COMPLETE) {
            /* 完成状态：特定模式完成页可再次上色（进 extra_color 确认页） */
            if (depth > 0 && page_stack[depth - 1] == PAGE_EXTRA_COLOR) {
                /* 已在 extra_color 确认页，防重复入栈 */
                g_send.buzzer_req = BUZZER_KEY_INVALID;
                break;
            }
            if (g_send.cook_mode != MODE_UPDOWN_BBQ &&
                g_send.cook_mode != MODE_HOTWIND_BBQ &&
                g_send.cook_mode != MODE_WINDCHANGE_BBQ &&
                g_send.cook_mode != MODE_PIZZA_2 &&
                g_send.cook_mode != MODE_COOK4 &&
                g_send.cook_mode != MODE_EXTRA_COLOR) {   /* 额外上色完成后可再次上色 */
                g_send.buzzer_req = BUZZER_KEY_INVALID;
                break;
            }
            g_send.buzzer_req = BUZZER_KEY_VALID;
            g_color_from_probe = is_probe_inserted();   /* 记录进入时的探针状态 */
            cook_elapsed_saved = 0; cook_bar_saved = 0;  /* 新一轮上色，清旧会话保存值 */
            page_push(PAGE_EXTRA_COLOR);
            lv_obj_clean(lv_scr_act());
            extra_color_create(&ui_manager);
        {
            extra_color_t *ec = extra_color_get(&ui_manager);
            if (ec) {
                lv_obj_t *btns[] = { ec->start_button };
                if (g_extra_color) lv_group_del(g_extra_color);
                g_extra_color = group_create_for_page(btns, 1);
                lv_obj_add_event_cb(ec->start_button, on_color_start_click,
                                    LV_EVENT_CLICKED, NULL);
            }
            current_group = g_extra_color;
        }
        lv_scr_load_anim(extra_color_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] jump: -> extra_color\n");
        uart_print();
        break;
        }
        /* 非完成状态：进入限制与 MENU/CLEAN 相同（白名单页面），进入额外上色设置页 */
        if (!menu_clean_key_allowed() || g_send.iface_status == IFACE_COOKING) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            uart_print();
            break;
        }
        g_send.buzzer_req = BUZZER_KEY_VALID;
        jump_to_color_menu();
        uart_print();
        break;
    case KEY_CLEAN:         // 7: 进入清洁菜单
        if (!menu_clean_key_allowed() || g_send.iface_status == IFACE_COOKING) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            break;
        }
        if (is_probe_inserted()) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            if (depth > 0 && page_stack[depth - 1] != PAGE_PROBETIP)
                jump_to_probetip("该功能不支持探针，请拔出探针！");
            break;
        }
        if (depth > 0) {
            page_id_t cur = page_stack[depth - 1];
            if (cur == PAGE_CLEAN_MENU) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;
                break;
            }
        }
        g_send.buzzer_req = BUZZER_KEY_VALID;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        g_on_stop_back = 0;
        g_complete_to_stop_back = 0;
        g_delay_cancel_to_stop_back = 0;
        g_delay_cancel_btn = 0;
        g_keepwarm_active = 0;
        g_keepwarm_sec = 0;
        delay_on = 0;
        contain_on = 0;
        preheat_on = 0;
        g_stop_back_complete = NULL;
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        jump_to_clean_menu();
        uart_print();
        break;
    case KEY_BACK:          // 21: 返回
        if (depth <= 1) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            uart_print();
            break;
        }
        g_send.buzzer_req = BUZZER_KEY_VALID;
        {
            page_id_t cur = page_stack[depth - 1];
            if (cur == PAGE_UPDOWN_BBQ_COOKING) {
                // 不暂停，后台继续cooking，直接跳stopback确认退出
                jump_to_updown_bbq_stop_back();
            } else if (cur == PAGE_UPDOWN_BBQ_STOP)
                jump_to_updown_bbq_stop_back();
            else if (cur == PAGE_UPDOWN_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_updown_bbq_stop_back();
            } else if (cur == PAGE_TOP_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_top_bbq_stop_back();
            } else if (cur == PAGE_BOTTOM_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_bottom_bbq_stop_back();
            } else if (cur == PAGE_HOT_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_hot_bbq_stop_back();
            } else if (cur == PAGE_HOTWIND_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_hotwind_bbq_stop_back();
            } else if (cur == PAGE_SAVE_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_save_bbq_stop_back();
            } else if (cur == PAGE_CENTRAL_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_central_bbq_stop_back();
            } else if (cur == PAGE_WINDCHANGE_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_windchange_bbq_stop_back();
            } else if (cur == PAGE_AIR_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_air_stop_back();
            } else if (cur == PAGE_COOKIE_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_cookie_stop_back();
            } else if (cur == PAGE_WEST_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_west_stop_back();
            } else if (cur == PAGE_PIZZA_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_pizza_stop_back();
            } else if (cur == PAGE_PIZZA_2_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_pizza_2_stop_back();
            } else if (cur == PAGE_MENU_COOK_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_menu_stop_back();
            } else if (cur == PAGE_SLOWCOOK_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_slowcook_stop_back();
            } else if (cur == PAGE_UNFROZEN_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_unfrozen_stop_back();
            } else if (cur == PAGE_RISING_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_rising_stop_back();
            } else if (cur == PAGE_CORN_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_corn_stop_back();
            } else if (cur == PAGE_HEATCONTAIN_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_heatcontain_stop_back();
            } else if (cur == PAGE_UPDOWN_BBQ_COOKING_PROBE) {
                jump_to_updown_bbq_stop_back_probe();
            } else if (cur == PAGE_UPDOWN_BBQ_STOP_PROBE)
                jump_to_updown_bbq_stop_back_probe();
            else if (cur == PAGE_HOT_BBQ_COOKING_PROBE) {
                jump_to_hot_bbq_stop_back_probe();
            } else if (cur == PAGE_HOT_BBQ_STOP_PROBE)
                jump_to_hot_bbq_stop_back_probe();
            else if (cur == PAGE_BOTTOM_BBQ_COOKING_PROBE) {
                jump_to_bottom_bbq_stop_back_probe();
            } else if (cur == PAGE_BOTTOM_BBQ_STOP_PROBE)
                jump_to_bottom_bbq_stop_back_probe();
            else if (cur == PAGE_SLOWCOOK_COOKING_PROBE) {
                jump_to_slowcook_stop_back_probe();
            } else if (cur == PAGE_SLOWCOOK_STOP_PROBE)
                jump_to_slowcook_stop_back_probe();
            else if (cur == PAGE_COLOR_COOKING)
                jump_to_color_stop_back();
            else if (cur == PAGE_COLOR_STOP)
                jump_to_color_stop_back();
            else if (cur == PAGE_EXTRA_COLOR) {
                g_extra_color_to_stop_back = 1;
                jump_to_color_stop_back();
            }
            else if (cur == PAGE_COLOR_COOKING_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_color_stop_back();
            }
            else if (cur == PAGE_TOP_BBQ_COOKING)
                jump_to_top_bbq_stop_back();
            else if (cur == PAGE_TOP_BBQ_STOP)
                jump_to_top_bbq_stop_back();
            else if (cur == PAGE_BOTTOM_BBQ_COOKING)
                jump_to_bottom_bbq_stop_back();
            else if (cur == PAGE_BOTTOM_BBQ_STOP)
                jump_to_bottom_bbq_stop_back();
            else if (cur == PAGE_HOT_BBQ_COOKING)
                jump_to_hot_bbq_stop_back();
            else if (cur == PAGE_HOT_BBQ_STOP)
                jump_to_hot_bbq_stop_back();
            else if (cur == PAGE_HOTWIND_BBQ_COOKING)
                jump_to_hotwind_bbq_stop_back();
            else if (cur == PAGE_HOTWIND_BBQ_STOP)
                jump_to_hotwind_bbq_stop_back();
            else if (cur == PAGE_SAVE_BBQ_COOKING)
                jump_to_save_bbq_stop_back();
            else if (cur == PAGE_SAVE_BBQ_STOP)
                jump_to_save_bbq_stop_back();
            else if (cur == PAGE_CENTRAL_BBQ_COOKING)
                jump_to_central_bbq_stop_back();
            else if (cur == PAGE_CENTRAL_BBQ_STOP)
                jump_to_central_bbq_stop_back();
            else if (cur == PAGE_WINDCHANGE_BBQ_COOKING)
                jump_to_windchange_bbq_stop_back();
            else if (cur == PAGE_WINDCHANGE_BBQ_STOP)
                jump_to_windchange_bbq_stop_back();
            else if (cur == PAGE_PREHEAT_COOKING)
                jump_to_preheat_stop_back();
            else if (cur == PAGE_DELAYCOOKING) {
                g_delay_cancel_to_stop_back = 1;
                delay_cancel_to_stop_back();
            }
            else if (cur == PAGE_SOMECOOK_COOKING) {
                somecook_cooking_handle_back();
            }
            else if (cur == PAGE_SOMECOOK) {
                if (current_group == g_somecook_edit) {
                    /* 步骤容器编辑组 BACK → 回对应大按钮（不弹栈） */
                    somecook_t *sc = somecook_get(&ui_manager);
                    lv_obj_t *df = lv_group_get_focused(current_group);
                    int step = 0;
                    if (sc && (df == sc->edit2 || df == sc->delete2)) step = 1;
                    else if (sc && (df == sc->edit3 || df == sc->delete3)) step = 2;
                    somecook_back_to_btns(step);
                } else {
                    page_pop();   /* 回 special_menu */
                }
            }
            else if (cur == PAGE_PREHEAT_STOP)
                jump_to_preheat_stop_back();
            else if (cur == PAGE_PREHEAT_COMPLETE) {
                if (g_send.cook_mode != MODE_PREHEAT) {
                    g_complete_to_stop_back = 1;
                    jump_to_preheat_stop_back();
                } else {
                    preheat_complete_exit();
                }
            }
            else if (cur == PAGE_COOKIE_COOKING)
                jump_to_cookie_stop_back();
            else if (cur == PAGE_COOKIE_STOP)
                jump_to_cookie_stop_back();
            else if (cur == PAGE_WEST_COOKING)
                jump_to_west_stop_back();
            else if (cur == PAGE_WEST_STOP)
                jump_to_west_stop_back();
            else if (cur == PAGE_PIZZA_COOKING)
                jump_to_pizza_stop_back();
            else if (cur == PAGE_PIZZA_STOP)
                jump_to_pizza_stop_back();
            else if (cur == PAGE_MENU_COOK_COOKING)
                jump_to_menu_stop_back();
            else if (cur == PAGE_MENU_COOK_STOP)
                jump_to_menu_stop_back();
            else if (cur == PAGE_AIR_COOKING)
                jump_to_air_stop_back();
            else if (cur == PAGE_AIR_STOP)
                jump_to_air_stop_back();
            else if (cur == PAGE_PIZZA_2_COOKING)
                jump_to_pizza_2_stop_back();
            else if (cur == PAGE_PIZZA_2_STOP)
                jump_to_pizza_2_stop_back();
            else if (cur == PAGE_SLOWCOOK_COOKING)
                jump_to_slowcook_stop_back();
            else if (cur == PAGE_SLOWCOOK_STOP)
                jump_to_slowcook_stop_back();
            else if (cur == PAGE_UNFROZEN_COOKING)
                jump_to_unfrozen_stop_back();
            else if (cur == PAGE_UNFROZEN_STOP)
                jump_to_unfrozen_stop_back();
            else if (cur == PAGE_RISING_COOKING)
                jump_to_rising_stop_back();
            else if (cur == PAGE_RISING_STOP)
                jump_to_rising_stop_back();
            else if (cur == PAGE_CORN_COOKING)
                jump_to_corn_stop_back();
            else if (cur == PAGE_CORN_STOP)
                jump_to_corn_stop_back();
            else if (cur == PAGE_HEATCONTAIN_COOKING)
                jump_to_heatcontain_stop_back();
            else if (cur == PAGE_HEATCONTAIN_STOP)
                jump_to_heatcontain_stop_back();
            else if (cur == PAGE_LASAGNA_COOKING)
                jump_to_lasagna_stop_back();
            else if (cur == PAGE_LASAGNA_STOP)
                jump_to_lasagna_stop_back();
            else if (cur == PAGE_STRUDEL_COOKING)
                jump_to_strudel_stop_back();
            else if (cur == PAGE_STRUDEL_STOP)
                jump_to_strudel_stop_back();
            else if (cur == PAGE_BREAD_COOKING)
                jump_to_bread_stop_back();
            else if (cur == PAGE_BREAD_STOP)
                jump_to_bread_stop_back();
            else if (cur == PAGE_PIZZA3_COOKING)
                jump_to_pizza3_stop_back();
            else if (cur == PAGE_PIZZA3_STOP)
                jump_to_pizza3_stop_back();
            else if (cur == PAGE_CHIP_COOKING)
                jump_to_chip_stop_back();
            else if (cur == PAGE_CHIP_STOP)
                jump_to_chip_stop_back();
            else if (cur == PAGE_CUSTOM_COOKING)
                jump_to_custom_stop_back();
            else if (cur == PAGE_CUSTOM_STOP)
                jump_to_custom_stop_back();
            else if (cur == PAGE_WATER_CLEAN_COOKING)
                jump_to_wc_stop_back();
            else if (cur == PAGE_WATER_CLEAN_STOP)
                jump_to_wc_stop_back();
            else if (cur == PAGE_HOTCLEANSAVE_COOKING)
                jump_to_hcs_stop_back();
            else if (cur == PAGE_HOTCLEANSAVE_STOP)
                jump_to_hcs_stop_back();
            else if (cur == PAGE_HOTCLEANSAVE_COOLING) {
                g_cooling_to_stop_back = 1;
                jump_to_hcs_stop_back();
                g_on_stop_back = 0;
            }
            else if (cur == PAGE_HOTCLEANMIDDLE_COOKING)
                jump_to_hcm_stop_back();
            else if (cur == PAGE_HOTCLEANMIDDLE_STOP)
                jump_to_hcm_stop_back();
            else if (cur == PAGE_HOTCLEANMIDDLE_COOLING) {
                g_cooling_to_stop_back = 1;
                jump_to_hcm_stop_back();
                g_on_stop_back = 0;
            }
            else if (cur == PAGE_HOTCLEANHIGH_COOKING)
                jump_to_hch_stop_back();
            else if (cur == PAGE_HOTCLEANHIGH_STOP)
                jump_to_hch_stop_back();
            else if (cur == PAGE_HOTCLEANHIGH_COOLING) {
                g_cooling_to_stop_back = 1;
                jump_to_hch_stop_back();
                g_on_stop_back = 0;
            }
            // B-2 clean complete
            else if (cur == PAGE_WATER_CLEAN_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_wc_stop_back();
            } else if (cur == PAGE_HOTCLEANSAVE_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_hcs_stop_back();
            } else if (cur == PAGE_HOTCLEANMIDDLE_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_hcm_stop_back();
            } else if (cur == PAGE_HOTCLEANHIGH_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_hch_stop_back();
            }
            // B-1 frozen bake complete
            else if (cur == PAGE_LASAGNA_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_lasagna_stop_back();
            } else if (cur == PAGE_STRUDEL_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_strudel_stop_back();
            } else if (cur == PAGE_BREAD_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_bread_stop_back();
            } else if (cur == PAGE_PIZZA3_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_pizza3_stop_back();
            } else if (cur == PAGE_CHIP_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_chip_stop_back();
            } else if (cur == PAGE_CUSTOM_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_custom_stop_back();
            }
            // B-3 probe complete
            else if (cur == PAGE_UPDOWN_BBQ_COMPLETE_PROBE) {
                g_complete_to_stop_back = 1;
                jump_to_updown_bbq_stop_back_probe();
            } else if (cur == PAGE_BOTTOM_BBQ_COMPLETE_PROBE) {
                g_complete_to_stop_back = 1;
                jump_to_bottom_bbq_stop_back_probe();
            } else if (cur == PAGE_HOT_BBQ_COMPLETE_PROBE) {
                g_complete_to_stop_back = 1;
                jump_to_hot_bbq_stop_back_probe();
            } else if (cur == PAGE_SLOWCOOK_COMPLETE_PROBE) {
                g_complete_to_stop_back = 1;
                jump_to_slowcook_stop_back_probe();
            }
            else
                page_pop();
        }
        uart_print();
        break;
    case KEY_ENCODER_CW: {  // 31: 焦点下移 / 数值+
#ifdef LV_USE_AIC_SIMULATOR
        if (current_group == g_preheat_cooking) {
            g_sim_cavity_temp += 5;
            if (g_sim_cavity_temp > 300) g_sim_cavity_temp = 300;
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (current_group == g_preheat_stop_back) {
            g_sim_cavity_temp += 5;
            if (g_sim_cavity_temp > 300) g_sim_cavity_temp = 300;
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (depth > 0 && (page_stack[depth-1] == PAGE_HOTCLEANSAVE_COOLING ||
            page_stack[depth-1] == PAGE_HOTCLEANMIDDLE_COOLING ||
            page_stack[depth-1] == PAGE_HOTCLEANHIGH_COOLING ||
            page_stack[depth-1] == PAGE_HOTCLEANSAVE_STOP_BACK ||
            page_stack[depth-1] == PAGE_HOTCLEANMIDDLE_STOP_BACK ||
            page_stack[depth-1] == PAGE_HOTCLEANHIGH_STOP_BACK)) {
            g_sim_cavity_temp += 5;
            if (g_sim_cavity_temp > 400) g_sim_cavity_temp = 400;
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
#endif
        if (!current_group) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            uart_print();
            break;
        }
        if (current_group == g_delayset) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            delayset_t *ds = delayset_get(&ui_manager);
            if (ds && df == ds->hour) {
                delay_hour++;
                if (delay_hour > 23) delay_hour = 0;
            } else if (ds && df == ds->min) {
                delay_min++;
                if (delay_min > 59) delay_min = 0;
            } else {
                g_send.buzzer_req = BUZZER_ENCODER;
                lv_group_focus_next(current_group);
                uart_print();
                break;
            }
            g_send.buzzer_req = BUZZER_ENCODER;
            delayset_refresh_display(ds);
            uart_print();
            break;
        }
        if (current_group == g_stepset) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            stepset_t *ss = stepset_get(&ui_manager);
            if (ss && df == ss->roller_main) {
                uint32_t sel = lv_roller_get_selected(ss->roller_main);
                uint32_t cnt = lv_roller_get_option_count(ss->roller_main);
                if (sel >= cnt - 1) {
                    g_send.buzzer_req = BUZZER_KEY_INVALID;
                    uart_print();
                    break;
                }
                lv_roller_set_selected(ss->roller_main, sel + 1, LV_ANIM_ON);
                lv_obj_send_event(ss->roller_main, LV_EVENT_VALUE_CHANGED, NULL);
                g_send.buzzer_req = BUZZER_ENCODER;
                uart_print();
                break;
            } else if (ss && df == ss->roller_mode) {
                uint32_t sel = lv_roller_get_selected(ss->roller_mode);
                uint32_t cnt = lv_roller_get_option_count(ss->roller_mode);
                if (sel >= cnt - 1) {
                    g_send.buzzer_req = BUZZER_KEY_INVALID;
                    uart_print();
                    break;
                }
                lv_roller_set_selected(ss->roller_mode, sel + 1, LV_ANIM_ON);
                lv_obj_send_event(ss->roller_mode, LV_EVENT_VALUE_CHANGED, NULL);
                g_send.buzzer_req = BUZZER_ENCODER;
                uart_print();
                break;
            }
            /* temp/hour/min/next:放行到通用逻辑(编辑字段调值 / 焦点移动) */
        }
        lv_obj_t *focused = lv_group_get_focused(current_group);
        edit_field_t *ef = find_edit_field(focused);
        if (ef) {
            if (ef->min == ef->max) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;
            } else {
                g_send.buzzer_req = BUZZER_ENCODER;
                adjust_value(ef, +1);
                printf("[nav] adjust +: %d\n", *ef->value);
            }
        } else {
            g_send.buzzer_req = BUZZER_ENCODER;
            lv_group_focus_next(current_group);
            printf("[nav] focus next\n");
        }
        uart_print();
        break;
    }
    case KEY_ENCODER_CCW: {  // 41: 焦点上移 / 数值-
#ifdef LV_USE_AIC_SIMULATOR
        if (current_group == g_preheat_cooking) {
            g_sim_cavity_temp -= 5;
            if (g_sim_cavity_temp > 300) g_sim_cavity_temp = 300;
            if (g_sim_cavity_temp < 0) g_sim_cavity_temp = 0;
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (current_group == g_preheat_stop_back) {
            g_sim_cavity_temp -= 5;
            if (g_sim_cavity_temp > 300) g_sim_cavity_temp = 300;
            if (g_sim_cavity_temp < 0) g_sim_cavity_temp = 0;
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (depth > 0 && (page_stack[depth-1] == PAGE_HOTCLEANSAVE_COOLING ||
            page_stack[depth-1] == PAGE_HOTCLEANMIDDLE_COOLING ||
            page_stack[depth-1] == PAGE_HOTCLEANHIGH_COOLING ||
            page_stack[depth-1] == PAGE_HOTCLEANSAVE_STOP_BACK ||
            page_stack[depth-1] == PAGE_HOTCLEANMIDDLE_STOP_BACK ||
            page_stack[depth-1] == PAGE_HOTCLEANHIGH_STOP_BACK)) {
            g_sim_cavity_temp -= 5;
            if (g_sim_cavity_temp > 400) g_sim_cavity_temp = 400;
            if (g_sim_cavity_temp < 0) g_sim_cavity_temp = 0;
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
#endif
        if (!current_group) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            uart_print();
            break;
        }
        if (current_group == g_delayset) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            delayset_t *ds = delayset_get(&ui_manager);
            if (ds && df == ds->hour) {
                delay_hour--;
                if (delay_hour < 0) delay_hour = 23;
            } else if (ds && df == ds->min) {
                delay_min--;
                if (delay_min < 0) delay_min = 59;
            } else {
                g_send.buzzer_req = BUZZER_ENCODER;
                lv_group_focus_prev(current_group);
                uart_print();
                break;
            }
            g_send.buzzer_req = BUZZER_ENCODER;
            delayset_refresh_display(ds);
            uart_print();
            break;
        }
        if (current_group == g_stepset) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            stepset_t *ss = stepset_get(&ui_manager);
            if (ss && df == ss->next) {
                /* 仿 updown menu:从 next 左转跳到 mainroller */
                lv_group_focus_obj(ss->roller_main);
                g_send.buzzer_req = BUZZER_ENCODER;
                uart_print();
                break;
            }
            if (ss && df == ss->roller_main) {
                uint32_t sel = lv_roller_get_selected(ss->roller_main);
                if (sel <= 0) {
                    g_send.buzzer_req = BUZZER_KEY_INVALID;
                    uart_print();
                    break;
                }
                lv_roller_set_selected(ss->roller_main, sel - 1, LV_ANIM_ON);
                lv_obj_send_event(ss->roller_main, LV_EVENT_VALUE_CHANGED, NULL);
                g_send.buzzer_req = BUZZER_ENCODER;
                uart_print();
                break;
            } else if (ss && df == ss->roller_mode) {
                uint32_t sel = lv_roller_get_selected(ss->roller_mode);
                if (sel <= 0) {
                    g_send.buzzer_req = BUZZER_KEY_INVALID;
                    uart_print();
                    break;
                }
                lv_roller_set_selected(ss->roller_mode, sel - 1, LV_ANIM_ON);
                lv_obj_send_event(ss->roller_mode, LV_EVENT_VALUE_CHANGED, NULL);
                g_send.buzzer_req = BUZZER_ENCODER;
                uart_print();
                break;
            }
            /* temp/hour/min/next:放行到通用逻辑(编辑字段调值 / 焦点移动) */
        }
        lv_obj_t *focused = lv_group_get_focused(current_group);
        edit_field_t *ef = find_edit_field(focused);
        g_send.buzzer_req = BUZZER_ENCODER;
        if (ef) {
            if (ef->min == ef->max) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;
            } else {
                g_send.buzzer_req = BUZZER_ENCODER;
                adjust_value(ef, -1);
                printf("[nav] adjust -: %d\n", *ef->value);
            }
        } else if (current_group == g_updown_bbq_menu) {
            updown_bbq_menu_t *bbq = updown_bbq_menu_get(&ui_manager);
            if (bbq && focused == bbq->next_button) {
                lv_group_focus_obj(bbq->tempnum_label);
                printf("[nav] focus wrap to tempnum\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[nav] focus prev\n");
            }
        } else if (current_group == g_updown_bbq_menu_probe) {
            updown_bbq_menu_probe_t *menu = updown_bbq_menu_probe_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[updown_bbq_probe] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[updown_bbq_probe] focus prev\n");
            }
        } else if (current_group == g_hot_bbq_menu_probe) {
            hot_bbq_menu_probe_t *menu = hot_bbq_menu_probe_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[hot_bbq_probe] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[hot_bbq_probe] focus prev\n");
            }
        } else if (current_group == g_bottom_bbq_menu_probe) {
            bottom_bbq_menu_probe_t *menu = bottom_bbq_menu_probe_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[bottom_bbq_probe] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[bottom_bbq_probe] focus prev\n");
            }
        } else if (current_group == g_slowcook_menu_probe) {
            slowcook_menu_probe_t *menu = slowcook_menu_probe_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[slowcook_probe] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[slowcook_probe] focus prev\n");
            }
        } else if (current_group == g_updown_bbq_setting) {
            updown_bbq_setting_t *set = updown_bbq_setting_get(&ui_manager);
            if (set && focused == set->sure_button) {
                lv_group_focus_obj(set->tempup_label);
                printf("[nav] setting focus wrap to tempup\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[nav] focus prev\n");
            }
        } else if (current_group == g_top_bbq_menu) {
            top_bbq_menu_t *menu = top_bbq_menu_get(&ui_manager);
            if (menu && focused == menu->next_button) {
                lv_group_focus_obj(menu->temp_label);
                printf("[top_bbq] focus wrap to temp_label\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[top_bbq] focus prev\n");
            }
        } else if (current_group == g_top_bbq_setting) {
            top_bbq_setting_t *set = top_bbq_setting_get(&ui_manager);
            if (set && focused == set->surebutton) {
                lv_group_focus_obj(set->temp);
                printf("[top_bbq] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[top_bbq] focus prev\n");
            }
        } else if (current_group == g_bottom_bbq_menu) {
            bottom_bbq_menu_t *menu = bottom_bbq_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[bottom_bbq] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[bottom_bbq] focus prev\n");
            }
        } else if (current_group == g_bottom_bbq_setting) {
            bottom_bbq_setting_t *set = bottom_bbq_setting_get(&ui_manager);
            if (set && focused == set->surebutton) {
                lv_group_focus_obj(set->temp);
                printf("[bottom_bbq] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[bottom_bbq] focus prev\n");
            }
        } else if (current_group == g_hot_bbq_menu) {
            hot_bbq_menu_t *menu = hot_bbq_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[hot_bbq] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[hot_bbq] focus prev\n");
            }
        } else if (current_group == g_hot_bbq_setting) {
            hot_bbq_setting_t *set = hot_bbq_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[hot_bbq] setting focus wrap to temp\n");
            } else {
            lv_group_focus_prev(current_group);
            printf("[hot_bbq] focus prev\n");
        }
    } else if (current_group == g_hotwind_bbq_menu) {
        hotwind_bbq_menu_t *menu = hotwind_bbq_menu_get(&ui_manager);
        if (menu && focused == menu->next) {
            lv_group_focus_obj(menu->temp);
            printf("[hotwind_bbq] focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[hotwind_bbq] focus prev\n");
        }
    } else if (current_group == g_hotwind_bbq_setting) {
        hotwind_bbq_setting_t *set = hotwind_bbq_setting_get(&ui_manager);
        if (set && focused == set->sure) {
            lv_group_focus_obj(set->temp);
            printf("[hotwind_bbq] setting focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[hotwind_bbq] focus prev\n");
        }
    } else if (current_group == g_save_bbq_menu) {
        save_bbq_menu_t *menu = save_bbq_menu_get(&ui_manager);
        if (menu && focused == menu->next) {
            lv_group_focus_obj(menu->temp);
            printf("[save_bbq] focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[save_bbq] focus prev\n");
        }
    } else if (current_group == g_save_bbq_setting) {
        save_bbq_setting_t *set = save_bbq_setting_get(&ui_manager);
        if (set && focused == set->sure) {
            lv_group_focus_obj(set->temp);
            printf("[save_bbq] setting focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[save_bbq] focus prev\n");
        }
    } else if (current_group == g_central_bbq_menu) {
        central_bbq_menu_t *menu = central_bbq_menu_get(&ui_manager);
        if (menu && focused == menu->next) {
            lv_group_focus_obj(menu->temp);
            printf("[central_bbq] focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[central_bbq] focus prev\n");
        }
    } else if (current_group == g_central_bbq_setting) {
        central_bbq_setting_t *set = central_bbq_setting_get(&ui_manager);
        if (set && focused == set->sure) {
            lv_group_focus_obj(set->temp);
            printf("[central_bbq] setting focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[central_bbq] focus prev\n");
        }
    } else if (current_group == g_windchange_bbq_menu) {
        windchange_bbq_menu_t *menu = windchange_bbq_menu_get(&ui_manager);
        if (menu && focused == menu->next) {
            lv_group_focus_obj(menu->temp);
            printf("[windchange_bbq] focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[windchange_bbq] focus prev\n");
        }
    } else if (current_group == g_windchange_bbq_setting) {
        windchange_bbq_setting_t *set = windchange_bbq_setting_get(&ui_manager);
        if (set && focused == set->sure) {
            lv_group_focus_obj(set->temp);
            printf("[windchange_bbq] setting focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[windchange_bbq] focus prev\n");
        }
        } else if (current_group == g_cookie_menu) {
            cookie_menu_t *menu = cookie_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[cookie] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[cookie] focus prev\n");
            }
        } else if (current_group == g_cookie_setting) {
            cookie_setting_t *set = cookie_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[cookie] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[cookie] setting focus prev\n");
            }
        } else if (current_group == g_west_menu) {
            west_menu_t *menu = west_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[west] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[west] focus prev\n");
            }
        } else if (current_group == g_west_setting) {
            west_setting_t *set = west_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[west] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[west] setting focus prev\n");
            }
        } else if (current_group == g_pizza_menu) {
            pizza_menu_t *menu = pizza_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[pizza] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[pizza] focus prev\n");
            }
        } else if (current_group == g_pizza_setting) {
            pizza_setting_t *set = pizza_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[pizza] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[pizza] setting focus prev\n");
            }
        } else if (current_group == g_menu_cook_menu) {
            menu_menu_t *menu = menu_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[menu] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[menu] focus prev\n");
            }
        } else if (current_group == g_menu_cook_setting) {
            menu_setting_t *set = menu_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[menu] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[menu] setting focus prev\n");
            }
        } else if (current_group == g_air_menu) {
            air_menu_t *menu = air_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[air] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[air] focus prev\n");
            }
        } else if (current_group == g_air_setting) {
            air_setting_t *set = air_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[air] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[air] setting focus prev\n");
            }
        } else if (current_group == g_pizza_2_menu) {
            pizza_2_menu_t *menu = pizza_2_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[pizza_2] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[pizza_2] focus prev\n");
            }
        } else if (current_group == g_pizza_2_setting) {
            pizza_2_setting_t *set = pizza_2_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[pizza_2] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[pizza_2] setting focus prev\n");
            }
        } else if (current_group == g_slowcook_menu) {
            slowcook_menu_t *menu = slowcook_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[slowcook] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[slowcook] focus prev\n");
            }
        } else if (current_group == g_slowcook_setting) {
            slowcook_setting_t *set = slowcook_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[slowcook] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[slowcook] setting focus prev\n");
            }
        } else if (current_group == g_unfrozen_menu) {
            unfrozen_menu_t *menu = unfrozen_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[unfrozen] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[unfrozen] focus prev\n");
            }
        } else if (current_group == g_unfrozen_setting) {
            unfrozen_setting_t *set = unfrozen_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[unfrozen] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[unfrozen] setting focus prev\n");
            }
        } else if (current_group == g_rising_menu) {
            rising_menu_t *menu = rising_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[rising] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[rising] focus prev\n");
            }
        } else if (current_group == g_rising_setting) {
            rising_setting_t *set = rising_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[rising] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[rising] setting focus prev\n");
            }
        } else if (current_group == g_corn_menu) {
            corn_menu_t *menu = corn_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[corn] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[corn] focus prev\n");
            }
        } else if (current_group == g_corn_setting) {
            corn_setting_t *set = corn_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[corn] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[corn] setting focus prev\n");
            }
        } else if (current_group == g_heatcontain_menu) {
            heatcontain_menu_t *menu = heatcontain_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[heatcontain] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[heatcontain] focus prev\n");
            }
        } else if (current_group == g_heatcontain_setting) {
            heatcontain_setting_t *set = heatcontain_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[heatcontain] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[heatcontain] setting focus prev\n");
            }
        } else if (current_group == g_lasagna_menu) {
            lasagna_menu_t *menu = lasagna_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->hour);
                printf("[lasagna] focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[lasagna] focus prev\n");
            }
        } else if (current_group == g_lasagna_setting) {
            lasagna_setting_t *set = lasagna_setting_get(&ui_manager);
            if (set && focused == set->button_200) {
                lv_group_focus_obj(set->hour);
                printf("[lasagna] setting focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[lasagna] setting focus prev\n");
            }
        } else if (current_group == g_strudel_menu) {
            strudel_menu_t *menu = strudel_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->hour);
                printf("[strudel] focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[strudel] focus prev\n");
            }
        } else if (current_group == g_strudel_setting) {
            strudel_setting_t *set = strudel_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->hour);
                printf("[strudel] setting focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[strudel] setting focus prev\n");
            }
        } else if (current_group == g_bread_menu) {
            bread_menu_t *menu = bread_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->hour);
                printf("[bread] focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[bread] focus prev\n");
            }
        } else if (current_group == g_bread_setting) {
            bread_setting_t *set = bread_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->hour);
                printf("[bread] setting focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[bread] setting focus prev\n");
            }
        } else if (current_group == g_pizza3_menu) {
            pizza3_menu_t *menu = pizza3_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->hour);
                printf("[pizza3] focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[pizza3] focus prev\n");
            }
        } else if (current_group == g_pizza3_setting) {
            pizza3_setting_t *set = pizza3_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->hour);
                printf("[pizza3] setting focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[pizza3] setting focus prev\n");
            }
        } else if (current_group == g_chip_menu) {
            chip_menu_t *menu = chip_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->hour);
                printf("[chip] focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[chip] focus prev\n");
            }
        } else if (current_group == g_chip_setting) {
            chip_setting_t *set = chip_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->hour);
                printf("[chip] setting focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[chip] setting focus prev\n");
            }
        } else if (current_group == g_custom_menu) {
            custom_menu_t *menu = custom_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->hour);
                printf("[custom] focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[custom] focus prev\n");
            }
        } else if (current_group == g_custom_setting) {
            custom_setting_t *set = custom_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->hour);
                printf("[custom] setting focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[custom] setting focus prev\n");
            }
        } else {
            lv_group_focus_prev(current_group);
            printf("[nav] focus prev\n");
        }
        uart_print();
        break;
    }
    case KEY_ENCODER_PRESS: { // 51: 确认 / 跳到下一焦点
        if (!current_group) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            break;
        }
        if (current_group == g_delayset) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            delayset_t *ds = delayset_get(&ui_manager);
            if (ds && df == ds->start) {
                g_send.buzzer_req = BUZZER_KEY_VALID;
                lv_obj_send_event(ds->start, LV_EVENT_CLICKED, NULL);
            } else {
                g_send.buzzer_req = BUZZER_KEY_VALID;
                lv_group_focus_next(current_group);
            }
            uart_print();
            break;
        }
        if (current_group == g_stepset) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            stepset_t *ss = stepset_get(&ui_manager);
            if (ss && df == ss->next) {
                g_send.buzzer_req = BUZZER_KEY_VALID;
                lv_obj_send_event(ss->next, LV_EVENT_CLICKED, NULL);
            } else {
                g_send.buzzer_req = BUZZER_KEY_VALID;
                lv_group_focus_next(current_group);
            }
            uart_print();
            break;
        }
        lv_obj_t *focused = lv_group_get_focused(current_group);
        edit_field_t *ef = find_edit_field(focused);
        if (ef) {
            g_send.buzzer_req = BUZZER_KEY_VALID;
            lv_group_focus_next(current_group);
            printf("[nav] press -> next focus\n");
        } else if (focused) {
            g_send.buzzer_req = BUZZER_KEY_VALID;
            lv_obj_send_event(focused, LV_EVENT_CLICKED, NULL);
            printf("[nav] press -> click\n");
        } else {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
        }
        uart_print();
        break;
    }
    default:
        printf("[nav] unknown key: %d\n", key);
        break;
    }
}

void nav_key1_long_press(void)
{
    probetip_cancel_auto_dismiss();   /* 取消陈旧的探针提示自动关闭定时器,防止跨会话误触发 */
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    g_on_stop_back = 0;
    g_complete_to_stop_back = 0;
    g_cooling_to_stop_back = 0;
    g_extra_color_to_stop_back = 0;
    g_keepwarm_active = 0;
    g_keepwarm_sec = 0;
    cook_is_color = 0;
    g_stop_back_complete = NULL;
    g_delay_cancel_btn = 0;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    delay_on = 0; preheat_on = 0; contain_on = 0;
    delay_hour = 0; delay_min = 0;
    g_delay_target = -1;
    g_somecook_running = 0;
    g_somecook_run_idx = 0;
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;

    if (g_send.iface_status != IFACE_SLEEP) {
        g_send.buzzer_req = BUZZER_POWER_OFF;
        g_send.iface_status = IFACE_SLEEP;
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        lv_obj_clean(lv_scr_act());
        waitmenu_24_create(&ui_manager);
        waitmenu_clock_cache_reset();   /* 强制刷新为真实时间 */
        current_group = NULL;
        lv_scr_load(waitmenu_24_get(&ui_manager)->obj);
        waitmenu_apply_clock();   /* 立即刷新为真实时间 */
#ifndef LV_USE_AIC_SIMULATOR
        backlight_set_level(10);
#endif
        printf("[KEY] KEY1 long press -> SLEEP (dim waitmenu)\n");
    } else {
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        if (is_probe_inserted()) {
            jump_to_major_menu_tz();
        } else {
            page_push(PAGE_MAJOR_MENU);
            lv_obj_clean(lv_scr_act());
            major_menu_create(&ui_manager);
            groups_create();
            bind_events();
            current_group = g_major_menu;
            lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, 0);
        }
        g_send.buzzer_req = BUZZER_POWER_ON;
        g_send.iface_status = IFACE_SETTING;
#ifndef LV_USE_AIC_SIMULATOR
        backlight_set_level(100);
#endif
        printf("[KEY] KEY1 long press -> WAKE (major_menu)\n");
    }
#ifdef LV_USE_AIC_SIMULATOR
    uart_print();
#endif
}

uint8_t nav_key1_hold_check(void)
{
    if (active_key == KEY1 && key_state == KEY_PRESSED) {
        uint32_t interval = lv_tick_get() - active_key_time;
        if (interval >= 2000) {
            active_key_time = lv_tick_get();
            nav_key1_long_press();
            return 1;
        }
    }
    return 0;
}

void nav_handle_key(uint8_t key)
{
    uint32_t now = lv_tick_get();

    switch (key_state) {
    case KEY_IDLE:
        if (key != 0) {
#ifdef LV_USE_AIC_SIMULATOR
            static const char *kn[] = {
                [1]="KEY1", [3]="MENU", [5]="COLOR", [21]="BACK",
                [31]="CW", [41]="CCW", [51]="PRESS"
            };
            printf("[KEY] %s (%d)\n", key<=51&&kn[key]?kn[key]:"?", key);
#endif
            active_key = key;
            active_key_time = now;
            key_state = KEY_PRESSED;
            process_key(key);
        }
        break;

    case KEY_PRESSED:
        if (key == 0) {
            // 松开 → 回到空闲
#ifdef LV_USE_AIC_SIMULATOR
            printf("[KEY] release\n");
#endif
            key_state = KEY_IDLE;
            active_key = 0;
        } else if (key == active_key) {
            // 同键按住
            uint32_t interval = now - active_key_time;
            int is_encoder = (key == KEY_ENCODER_CW || key == KEY_ENCODER_CCW);

            if (is_encoder && interval >= ENC_REPEAT_MS) {
                active_key_time = now;
                process_key(key);
            }
            if (active_key == KEY1 && interval >= 2000) {
                active_key_time = now;
                nav_key1_long_press();
            }
            // 触控键按住不重复（只有 KEY_IDLE 后的第一次触发）
        } else {
            // 键值变化（如编码器方向切换）
            active_key = key;
            active_key_time = now;
            process_key(key);
        }
        break;
    }
}

// ==============================
// 事件绑定
// ==============================

static void on_major_cook_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))  // 防重入：动画未完成时不跳转
        jump_to_cookmenu();
}

void jump_to_cook4_menu(void);
static void on_major_special_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_special_menu();
}

static void on_major_cook4_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_cook4_menu();
}

static void on_cook4_cookie_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_cookie_menu();
}

static void on_cook4_west_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_west_menu();
}

static void on_cook4_pizza_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_pizza_menu();
}

static void on_cook4_menu_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_menu_menu();
}

void jump_to_cook4_menu(void)
{
    page_push(PAGE_COOK4_MENU);
    lv_obj_clean(lv_scr_act());
    cook4menu_create(&ui_manager);

    cook4menu_t *c4 = cook4menu_get(&ui_manager);
    if (c4) {
        lv_obj_t *btns[] = { c4->cookie, c4->west, c4->piza, c4->menu };
        if (g_cook4_menu) lv_group_del(g_cook4_menu);
        g_cook4_menu = group_create_for_page(btns, 4);

        lv_obj_add_event_cb(c4->cookie, on_cook4_cookie_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(c4->west, on_cook4_west_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(c4->piza, on_cook4_pizza_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(c4->menu, on_cook4_menu_click,
                            LV_EVENT_CLICKED, NULL);
    }
    current_group = g_cook4_menu;

    lv_scr_load_anim(cook4menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] major_menu -> cook4_menu\n");
    g_send.cook_mode = MODE_COOK4;
}

void cook4menu_rebuild(page_id_t child)
{
    lv_obj_clean(lv_scr_act());
    cook4menu_create(&ui_manager);

    cook4menu_t *c4 = cook4menu_get(&ui_manager);
    if (c4) {
        lv_obj_t *btns[] = { c4->cookie, c4->west, c4->piza, c4->menu };
        if (g_cook4_menu) lv_group_del(g_cook4_menu);
        g_cook4_menu = group_create_for_page(btns, 4);

        lv_obj_add_event_cb(c4->cookie, on_cook4_cookie_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(c4->west, on_cook4_west_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(c4->piza, on_cook4_pizza_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(c4->menu, on_cook4_menu_click,
                            LV_EVENT_CLICKED, NULL);

        if (child == PAGE_COOKIE_MENU && c4->cookie)
            lv_group_focus_obj(c4->cookie);
        else if (child == PAGE_WEST_MENU && c4->west)
            lv_group_focus_obj(c4->west);
        else if (child == PAGE_PIZZA_MENU && c4->piza)
            lv_group_focus_obj(c4->piza);
        else if (child == PAGE_MENU_COOK_MENU && c4->menu)
            lv_group_focus_obj(c4->menu);
    }
    current_group = g_cook4_menu;

    lv_scr_load_anim(cook4menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] back to cook4_menu\n");
    g_send.cook_mode = MODE_COOK4;
}

void on_cook_updown_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_menu();
}

static void on_top_bbq_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_top_bbq_menu();
}

static void on_bottom_bbq_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_bottom_bbq_menu();
}

static void on_hot_bbq_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hot_bbq_menu();
}
static void on_hotwind_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hotwind_bbq_menu();
}
static void on_save_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_save_bbq_menu();
}
static void on_central_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_central_bbq_menu();
}
static void on_windchange_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_windchange_bbq_menu();
}
static void on_preheat_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_preheat_menu();
}

static void on_air_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_air_menu();
}

static void on_pizza_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_pizza_2_menu();
}

static void on_some_cook_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_somecook();
}

static void on_slowcook_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_slowcook_menu();
}

static void on_unfrozen_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_unfrozen_menu();
}

static void on_rising_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_rising_menu();
}

static void on_corn_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_corn_menu();
}

static void on_heatcontain_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_heatcontain_menu();
}

static void on_frozen_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_frozen_cook();
}

static void on_updown_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_set();
}

void on_edit_focus(lv_event_t *e)
{
    /* 隐藏所有指示线 */
    for (int i = 0; i < edit_count; i++) {
        if (edit_fields[i].ind_short)
            lv_obj_add_flag(edit_fields[i].ind_short, LV_OBJ_FLAG_HIDDEN);
        if (edit_fields[i].ind_long)
            lv_obj_add_flag(edit_fields[i].ind_long, LV_OBJ_FLAG_HIDDEN);
    }
    /* 显示当前焦点的指示线 */
    lv_obj_t *target = lv_event_get_target(e);
    edit_field_t *f = find_edit_field(target);
    if (f) {
        if (f->ind_short && f->ind_long) {
            if (*f->value < 100)
                lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
            else
                lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
        } else if (f->ind_short) {
            lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
        }
    }
    /* 强制整个页面重绘 */
    lv_obj_invalidate(lv_scr_act());
}

static void on_preheat_toggle(lv_event_t *e)
{
    updown_bbq_set_t *set = updown_bbq_set_get(&ui_manager);
    if (!set) return;
    preheat_on = !preheat_on;
    if (preheat_on) {
        /* 互斥：开预热关延时 */
        delay_on = 0;
        apply_toggle_state(set->delay_button, set->delay_on_button, delay_on);
        lv_obj_add_flag(set->preheat_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->preheat_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->preheat_on_button);
    } else {
        lv_obj_add_flag(set->preheat_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->preheat_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->preheat_button);
    }
}

int delay_hour = 19;
int delay_min = 0;

// 进入 delayset 的时刻（今天/明天切换基准，不随实时时间变化）
static int delayset_enter_hour = -1;
static int delayset_enter_min = 0;

// 刷新 delayset 页的时间显示（hour 显示 0-23，超 23:59 切换明天）
// 今天/明天标签：调节期按"进入时刻"双向比较（已过→明天），确定后 delay_hour>=24 恒明天。
// 刷新 delayset 页的时间显示（hour 显示 0-23，min 0-59）
// 今天/明天标签：与进入时刻纯比较（小时+分钟分开判断），小于 → 明天，否则 → 今天
static void delayset_refresh_display(delayset_t *ds)
{
    if (!ds) return;
    lv_label_set_text_fmt(ds->hour, "%02d", delay_hour);
    lv_label_set_text_fmt(ds->min, "%02d", delay_min);
    const char *day;
    if (delayset_enter_hour >= 0 &&
        (delay_hour < delayset_enter_hour ||
         (delay_hour == delayset_enter_hour && delay_min < delayset_enter_min)))
        day = "明天";   /* 小时或分钟小于时间戳 → 明天 */
    else
        day = "今天";   /* 大于或相等 → 今天 */
    lv_label_set_text(ds->day, day);
}

// delayset 焦点切换：显示对应字段的下划线
static void on_delayset_focus(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    delayset_t *ds = delayset_get(&ui_manager);
    if (!ds) return;
    lv_obj_add_flag(ds->image_9, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ds->image_10, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ds->startline, LV_OBJ_FLAG_HIDDEN);
    if (obj == ds->hour)
        lv_obj_clear_flag(ds->image_9, LV_OBJ_FLAG_HIDDEN);
    else if (obj == ds->min)
        lv_obj_clear_flag(ds->image_10, LV_OBJ_FLAG_HIDDEN);
    else if (obj == ds->start)
        lv_obj_clear_flag(ds->startline, LV_OBJ_FLAG_HIDDEN);
}

// delayset 点开始：与实时时间比较校正（已过 → 明天），用确定时日期一次性算好
// 预约目标 g_delay_target 并保存（此后 delaycooking 重建不再重算，避免跨天漂移）
static void on_delayset_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    rtc_time_t now;
    if (rtc_get_time(&now) == 0) {
        if (delay_hour < 24 &&
            (delay_hour < now.hour ||
             (delay_hour == now.hour && delay_min < now.min)))
            delay_hour += 24;                       /* 实时已过 → 明天 */
        int64_t base = (int64_t)rtc_days_from_epoch(now.year, now.month, now.day) * 86400000LL;
        g_delay_target = base
                       + (delay_hour >= 24 ? 86400000LL : 0)
                       + (int64_t)(delay_hour % 24) * 3600000LL
                       + (int64_t)delay_min * 60000LL;
    } else {
        g_delay_target = -1;                        /* RTC 失败哨兵，到点检测不触发 */
    }
    delay_on = 1;
    /* 互斥：开延时关预热（回 set 页重建时显示关） */
    preheat_on = 0;
    page_pop();
}

// 设置 updown set 页 ondelay 按钮文字（"今天19:00开始"，taiwan 字体滚动显示）
static void updown_set_apply_delay_label(updown_bbq_set_t *set)
{
    if (!set) return;
    lv_obj_t *lbl = lv_obj_get_child(set->delay_on_button, 0);
    if (lbl) {
        lv_obj_set_style_text_font(lbl, &c_taiwanpearl_regular_24,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_label_set_text_fmt(lbl, "%s%02d:%02d开始",
                              delay_hour >= 24 ? "明天" : "今天",
                              delay_hour % 24, delay_min);
        lv_obj_set_width(lbl, 110);
    }
}

// 延时开关：关态点击进入 delayset 设置时间；开态点击直接关闭
static void on_delay_toggle(lv_event_t *e)
{
    updown_bbq_set_t *set = updown_bbq_set_get(&ui_manager);
    if (!set) return;
    lv_obj_t *tgt = lv_event_get_target(e);
    if (tgt == set->delay_button) {
        jump_to_delayset();
        return;
    }
    /* delay_on_button：直接关闭 */
    delay_on = 0;
    lv_obj_add_flag(set->delay_on_button, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(set->delay_button, LV_OBJ_FLAG_HIDDEN);
    lv_group_focus_obj(set->delay_button);
}

// 按预约来源页设置 cook_mode（delay 流程所有显示/分发依赖 cook_mode）
static void apply_delay_cook_mode(page_id_t src)
{
    switch (src) {
    case PAGE_UPDOWN_BBQ_SET: case PAGE_UPDOWN_BBQ_SET_PROBE:
        g_send.cook_mode = MODE_UPDOWN_BBQ; break;
    case PAGE_TOP_BBQ_SET:        g_send.cook_mode = MODE_TOP_BBQ; break;
    case PAGE_BOTTOM_BBQ_SET: case PAGE_BOTTOM_BBQ_SET_PROBE:
        g_send.cook_mode = MODE_BOTTOM_BBQ; break;
    case PAGE_HOT_BBQ_SET: case PAGE_HOT_BBQ_SET_PROBE:
        g_send.cook_mode = MODE_HOT_BBQ; break;
    case PAGE_HOTWIND_BBQ_SET:    g_send.cook_mode = MODE_HOTWIND_BBQ; break;
    case PAGE_SAVE_BBQ_SET:       g_send.cook_mode = MODE_SAVE_BBQ; break;
    case PAGE_CENTRAL_BBQ_SET:    g_send.cook_mode = MODE_CENTRAL_BBQ; break;
    case PAGE_WINDCHANGE_BBQ_SET: g_send.cook_mode = MODE_WINDCHANGE_BBQ; break;
    case PAGE_AIR_SET:            g_send.cook_mode = MODE_AIR; break;
    case PAGE_PIZZA_2_SET:        g_send.cook_mode = MODE_PIZZA_2; break;
    case PAGE_SLOWCOOK_SET: case PAGE_SLOWCOOK_SET_PROBE:
        g_send.cook_mode = MODE_SLOWCOOK; break;
    case PAGE_UNFROZEN_SET:       g_send.cook_mode = MODE_UNFROZEN; break;
    case PAGE_RISING_SET:         g_send.cook_mode = MODE_RISING; break;
    case PAGE_CORN_SET:           g_send.cook_mode = MODE_CORN; break;
    case PAGE_HEATCONTAIN_SET:    g_send.cook_mode = MODE_HEATCONTAIN; break;
    case PAGE_COOKIE_SET: case PAGE_WEST_SET: case PAGE_PIZZA_SET: case PAGE_MENU_COOK_SET:
        g_send.cook_mode = MODE_COOK4; break;
    case PAGE_LASAGNA_SET: case PAGE_STRUDEL_SET: case PAGE_BREAD_SET:
    case PAGE_PIZZA3_SET: case PAGE_CHIP_SET: case PAGE_CUSTOM_SET:
        g_send.cook_mode = MODE_FROZEN_BAKE; break;
    default: break;
    }
}

void jump_to_delayset(void)
{
    edit_clear();
    /* 记录预约来源页（cook4 多组共用 MODE_COOK4，靠来源页区分） */
    if (depth > 0)
        g_delay_source_page = page_stack[depth - 1];
    apply_delay_cook_mode(g_delay_source_page);
    /* 记录进入时刻（今天/明天切换基准）+ 默认当前时间+5 分钟（当前 23:59 → 明天 00:04） */
    rtc_time_t now;
    if (rtc_get_time(&now) == 0) {
        delayset_enter_hour = now.hour;
        delayset_enter_min = now.min;
        delay_hour = now.hour;
        delay_min = now.min + 5;
        if (delay_min > 59) { delay_min -= 60; delay_hour++; }
        if (delay_hour > 23) delay_hour = 0;   /* 23:59+5min → 00:04,内部回 0,标签自动"明天" */
    }
    page_push(PAGE_DELAYSET);
    lv_obj_clean(lv_scr_act());
    delayset_create(&ui_manager);

    delayset_t *ds = delayset_get(&ui_manager);
    if (ds) {
        lv_obj_t *btns[] = { ds->hour, ds->min, ds->start };
        if (g_delayset) lv_group_del(g_delayset);
        g_delayset = group_create_for_page(btns, 3);
        clear_focus_states(btns, 3);

        lv_obj_add_flag(ds->start, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(ds->start, on_delayset_start_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(ds->hour, on_delayset_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(ds->min, on_delayset_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(ds->start, on_delayset_focus, LV_EVENT_FOCUSED, NULL);

        lv_group_focus_obj(ds->start);

        lv_label_set_text(ds->name, mode_display_name());
        delayset_refresh_display(ds);
    }
    current_group = g_delayset;

    lv_scr_load_anim(delayset_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] jump: updown_bbq_set -> delayset\n");
}

// delaycooking 取消：取消延时 → 跳 stop_back 确认（预约中态）
static void on_delaycooking_cancel_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    g_delay_cancel_to_stop_back = 1;
    g_delay_cancel_btn = 1;
    delay_cancel_to_stop_back();
}

// 按 g_send.cook_mode 返回模式显示名（无 "| |"）
const char *mode_display_name(void)
{
    switch (g_send.cook_mode) {
    case MODE_UPDOWN_BBQ:     return "上下烧烤";
    case MODE_TOP_BBQ:        return "顶部烧烤";
    case MODE_BOTTOM_BBQ:     return "底部烧烤";
    case MODE_HOT_BBQ:        return "热风烧烤";
    case MODE_HOTWIND_BBQ:    return "热风";
    case MODE_SAVE_BBQ:       return "节能热风";
    case MODE_CENTRAL_BBQ:    return "集中烧烤";
    case MODE_WINDCHANGE_BBQ: return "热风对流";
    case MODE_AIR:            return "空气炸";
    case MODE_PIZZA_2:        return "披萨";
    case MODE_SLOWCOOK:       return "慢煮";
    case MODE_UNFROZEN:       return "解冻";
    case MODE_RISING:         return "发酵";
    case MODE_CORN:           return "干果";
    case MODE_HEATCONTAIN:    return "保温";
    case MODE_FROZEN_BAKE:
        switch (g_delay_source_page) {
        case PAGE_LASAGNA_SET: return "千层面";
        case PAGE_STRUDEL_SET: return "果馅卷";
        case PAGE_BREAD_SET:   return "面包";
        case PAGE_PIZZA3_SET:  return "披萨";
        case PAGE_CHIP_SET:    return "薯条";
        case PAGE_CUSTOM_SET:  return "自定义";
        default:               return "冷冻烘焙";
        }
    case MODE_COOK4:
        switch (g_delay_source_page) {
        case PAGE_COOKIE_SET:    return "曲奇";
        case PAGE_WEST_SET:      return "西式";
        case PAGE_PIZZA_SET:     return "披萨";
        case PAGE_MENU_COOK_SET: return "菜单";
        default:                 return "上下烧烤";
        }
    default:                  return "上下烧烤";
    }
}

// 按 g_send.cook_mode 设置模式图标（与预热页映射一致）
void mode_apply_icon(lv_obj_t *icon)
{
    if (!icon) return;
    switch (g_send.cook_mode) {
    case MODE_UPDOWN_BBQ:     lv_img_set_src(icon, LVGL_IMAGE_PATH(updown_img.png)); break;
    case MODE_TOP_BBQ:        lv_img_set_src(icon, LVGL_IMAGE_PATH(topicon.png)); break;
    case MODE_BOTTOM_BBQ:     lv_img_set_src(icon, LVGL_IMAGE_PATH(dwbbqicon.png)); break;
    case MODE_HOT_BBQ:        lv_img_set_src(icon, LVGL_IMAGE_PATH(hotbbqicon.png)); break;
    case MODE_HOTWIND_BBQ:    lv_img_set_src(icon, LVGL_IMAGE_PATH(hotwindicon.png)); break;
    case MODE_SAVE_BBQ:       lv_img_set_src(icon, LVGL_IMAGE_PATH(savewindicon.png)); break;
    case MODE_CENTRAL_BBQ:    lv_img_set_src(icon, LVGL_IMAGE_PATH(centralbbqicon.png)); break;
    case MODE_WINDCHANGE_BBQ: lv_img_set_src(icon, LVGL_IMAGE_PATH(windchange.png)); break;
    case MODE_AIR:            lv_img_set_src(icon, LVGL_IMAGE_PATH(airicon.png)); break;
    case MODE_PIZZA_2:        lv_img_set_src(icon, LVGL_IMAGE_PATH(pizza2icon.png)); break;
    case MODE_SLOWCOOK:       lv_img_set_src(icon, LVGL_IMAGE_PATH(slowcookicon.png)); break;
    case MODE_UNFROZEN:       lv_img_set_src(icon, LVGL_IMAGE_PATH(unfrozenicon.png)); break;
    case MODE_RISING:         lv_img_set_src(icon, LVGL_IMAGE_PATH(risingicon.png)); break;
    case MODE_CORN:           lv_img_set_src(icon, LVGL_IMAGE_PATH(cornicon.png)); break;
    case MODE_HEATCONTAIN:    lv_img_set_src(icon, LVGL_IMAGE_PATH(heatcontainicon.png)); break;
    case MODE_FROZEN_BAKE:
        switch (g_delay_source_page) {
        case PAGE_LASAGNA_SET: lv_img_set_src(icon, LVGL_IMAGE_PATH(lasagnaicon.png)); break;
        case PAGE_STRUDEL_SET: lv_img_set_src(icon, LVGL_IMAGE_PATH(strudelicon.png)); break;
        case PAGE_BREAD_SET:   lv_img_set_src(icon, LVGL_IMAGE_PATH(breadicon_1.png)); break;
        case PAGE_PIZZA3_SET:  lv_img_set_src(icon, LVGL_IMAGE_PATH(pizza3icon.png)); break;
        case PAGE_CHIP_SET:    lv_img_set_src(icon, LVGL_IMAGE_PATH(chipicon.png)); break;
        case PAGE_CUSTOM_SET:  lv_img_set_src(icon, LVGL_IMAGE_PATH(customicon.png)); break;
        default:               lv_img_set_src(icon, LVGL_IMAGE_PATH(frozencookfr.png)); break;
        }
        break;
    case MODE_COOK4:
        switch (g_delay_source_page) {
        case PAGE_COOKIE_SET:    lv_img_set_src(icon, LVGL_IMAGE_PATH(cookieicon.png)); break;
        case PAGE_WEST_SET:      lv_img_set_src(icon, LVGL_IMAGE_PATH(westicon.png)); break;
        case PAGE_PIZZA_SET:     lv_img_set_src(icon, LVGL_IMAGE_PATH(pizzaicon.png)); break;
        case PAGE_MENU_COOK_SET: lv_img_set_src(icon, LVGL_IMAGE_PATH(menuicon.png)); break;
        default:                 lv_img_set_src(icon, LVGL_IMAGE_PATH(updown_img.png)); break;
        }
        break;
    default:                  lv_img_set_src(icon, LVGL_IMAGE_PATH(updown_img.png)); break;
    }
}

// 14 模式 set 页 ondelay 按钮（背景 switchbg30/80 + "今天19:00开始"滚动文字）
void mode_set_apply_delay_label(lv_obj_t *ondelay_btn)
{
    if (!ondelay_btn) return;
    lv_obj_set_style_bg_img_src(ondelay_btn, LVGL_IMAGE_PATH(switchbg30.png),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ondelay_btn, LVGL_IMAGE_PATH(switchbg80.png),
                                LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_t *lbl = lv_obj_get_child(ondelay_btn, 0);
    if (lbl) {
        lv_obj_set_style_text_font(lbl, &c_taiwanpearl_regular_24,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_label_set_text_fmt(lbl, "%s%02d:%02d开始",
                              delay_hour >= 24 ? "明天" : "今天",
                              delay_hour % 24, delay_min);
        lv_obj_set_width(lbl, 110);
    }
}

// 延时预约到点：按 cook_mode 进入对应模式烹饪（探针插入走探针烹饪）
static void delay_start_cook(void)
{
    if (is_probe_inserted()) {
        switch (g_send.cook_mode) {
        case MODE_UPDOWN_BBQ: jump_to_updown_bbq_cooking_probe(); return;
        case MODE_HOT_BBQ:    jump_to_hot_bbq_cooking_probe(); return;
        case MODE_BOTTOM_BBQ: jump_to_bottom_bbq_cooking_probe(); return;
        case MODE_SLOWCOOK:   jump_to_slowcook_cooking_probe(); return;
        default: break; /* 非探针模式：探针插入不影响，按普通分发 */
        }
    }
    if (g_send.cook_mode == MODE_FROZEN_BAKE) {
        /* 冷冻食谱六组共用 MODE_FROZEN_BAKE：按预约来源页分发 */
        switch (g_delay_source_page) {
        case PAGE_LASAGNA_SET: jump_to_lasagna_cooking(); break;
        case PAGE_STRUDEL_SET: jump_to_strudel_cooking(); break;
        case PAGE_BREAD_SET:   jump_to_bread_cooking(); break;
        case PAGE_PIZZA3_SET:  jump_to_pizza3_cooking(); break;
        case PAGE_CHIP_SET:    jump_to_chip_cooking(); break;
        case PAGE_CUSTOM_SET:  jump_to_custom_cooking(); break;
        default:               jump_to_lasagna_cooking(); break;
        }
    } else if (g_send.cook_mode == MODE_COOK4) {
        /* cook4 四组共用 MODE_COOK4：按预约来源页分发 */
        switch (g_delay_source_page) {
        case PAGE_COOKIE_SET: jump_to_cookie_cooking(); break;
        case PAGE_WEST_SET:   jump_to_west_cooking(); break;
        case PAGE_PIZZA_SET:  jump_to_pizza_cooking(); break;
        case PAGE_MENU_COOK_SET: jump_to_menu_cooking(); break;
        default:              jump_to_cookie_cooking(); break;
        }
    } else {
        switch (g_send.cook_mode) {
        case MODE_UPDOWN_BBQ:     jump_to_updown_bbq_cooking(); break;
        case MODE_TOP_BBQ:        jump_to_top_bbq_cooking(); break;
        case MODE_BOTTOM_BBQ:     jump_to_bottom_bbq_cooking(); break;
        case MODE_HOT_BBQ:        jump_to_hot_bbq_cooking(); break;
        case MODE_HOTWIND_BBQ:    jump_to_hotwind_bbq_cooking(); break;
        case MODE_SAVE_BBQ:       jump_to_save_bbq_cooking(); break;
        case MODE_CENTRAL_BBQ:    jump_to_central_bbq_cooking(); break;
        case MODE_WINDCHANGE_BBQ: jump_to_windchange_bbq_cooking(); break;
        case MODE_AIR:            jump_to_air_cooking(); break;
        case MODE_PIZZA_2:        jump_to_pizza_2_cooking(); break;
        case MODE_SLOWCOOK:       jump_to_slowcook_cooking(); break;
        case MODE_UNFROZEN:       jump_to_unfrozen_cooking(); break;
        case MODE_RISING:         jump_to_rising_cooking(); break;
        case MODE_CORN:           jump_to_corn_cooking(); break;
        case MODE_HEATCONTAIN:    jump_to_heatcontain_cooking(); break;
        default:                  jump_to_updown_bbq_cooking(); break;
        }
    }
}

// delaycooking 取消/返回：按 cook_mode 跳对应模式 stop_back
static void delay_cancel_to_stop_back(void)
{
    if (is_probe_inserted()) {
        switch (g_send.cook_mode) {
        case MODE_UPDOWN_BBQ: jump_to_updown_bbq_stop_back_probe(); return;
        case MODE_HOT_BBQ:    jump_to_hot_bbq_stop_back_probe(); return;
        case MODE_BOTTOM_BBQ: jump_to_bottom_bbq_stop_back_probe(); return;
        case MODE_SLOWCOOK:   jump_to_slowcook_stop_back_probe(); return;
        default: break; /* 非探针模式：探针插入不影响，按普通分发 */
        }
    }
    if (g_send.cook_mode == MODE_FROZEN_BAKE) {
        /* 冷冻食谱六组共用 MODE_FROZEN_BAKE：按预约来源页分发 */
        switch (g_delay_source_page) {
        case PAGE_LASAGNA_SET: jump_to_lasagna_stop_back(); break;
        case PAGE_STRUDEL_SET: jump_to_strudel_stop_back(); break;
        case PAGE_BREAD_SET:   jump_to_bread_stop_back(); break;
        case PAGE_PIZZA3_SET:  jump_to_pizza3_stop_back(); break;
        case PAGE_CHIP_SET:    jump_to_chip_stop_back(); break;
        case PAGE_CUSTOM_SET:  jump_to_custom_stop_back(); break;
        default:               jump_to_lasagna_stop_back(); break;
        }
    } else if (g_send.cook_mode == MODE_COOK4) {
        switch (g_delay_source_page) {
        case PAGE_COOKIE_SET:    jump_to_cookie_stop_back(); break;
        case PAGE_WEST_SET:      jump_to_west_stop_back(); break;
        case PAGE_PIZZA_SET:     jump_to_pizza_stop_back(); break;
        case PAGE_MENU_COOK_SET: jump_to_menu_stop_back(); break;
        default:                 jump_to_cookie_stop_back(); break;
        }
    } else {
        switch (g_send.cook_mode) {
        case MODE_UPDOWN_BBQ:     jump_to_updown_bbq_stop_back(); break;
        case MODE_TOP_BBQ:        jump_to_top_bbq_stop_back(); break;
        case MODE_BOTTOM_BBQ:     jump_to_bottom_bbq_stop_back(); break;
        case MODE_HOT_BBQ:        jump_to_hot_bbq_stop_back(); break;
        case MODE_HOTWIND_BBQ:    jump_to_hotwind_bbq_stop_back(); break;
        case MODE_SAVE_BBQ:       jump_to_save_bbq_stop_back(); break;
        case MODE_CENTRAL_BBQ:    jump_to_central_bbq_stop_back(); break;
        case MODE_WINDCHANGE_BBQ: jump_to_windchange_bbq_stop_back(); break;
        case MODE_AIR:            jump_to_air_stop_back(); break;
        case MODE_PIZZA_2:        jump_to_pizza_2_stop_back(); break;
        case MODE_SLOWCOOK:       jump_to_slowcook_stop_back(); break;
        case MODE_UNFROZEN:       jump_to_unfrozen_stop_back(); break;
        case MODE_RISING:         jump_to_rising_stop_back(); break;
        case MODE_CORN:           jump_to_corn_stop_back(); break;
        case MODE_HEATCONTAIN:    jump_to_heatcontain_stop_back(); break;
        default:                  jump_to_updown_bbq_stop_back(); break;
        }
    }
}

// 重建 delaycooking 预约页（stop_back 返回场景复用，不重复压栈）
static void rebuild_delaycooking(void)
{
    g_on_stop_back = 0;
    g_stop_back_complete = NULL;
    apply_delay_cook_mode(g_delay_source_page);
    lv_obj_clean(lv_scr_act());
    delaycooking_create(&ui_manager);

    delaycooking_t *dc = delaycooking_get(&ui_manager);
    if (dc) {
        lv_obj_t *btns[] = { dc->cancel };
        if (g_delaycooking) lv_group_del(g_delaycooking);
        g_delaycooking = group_create_for_page(btns, 1);
        clear_focus_states(btns, 1);
        lv_group_focus_obj(dc->cancel);

        lv_obj_add_event_cb(dc->cancel, on_delaycooking_cancel_click,
                            LV_EVENT_CLICKED, NULL);

        if (g_delay_source_page == PAGE_UPDOWN_BBQ_SET_PROBE ||
            g_delay_source_page == PAGE_HOT_BBQ_SET_PROBE ||
            g_delay_source_page == PAGE_BOTTOM_BBQ_SET_PROBE ||
            g_delay_source_page == PAGE_SLOWCOOK_SET_PROBE) {
            /* 探针来源：与探针烹饪页一致的格式（温度 + 探针目标温） */
            lv_label_set_text_fmt(dc->status, "| %s | %d℃ | %d℃",
                                  mode_display_name(), set_temp, probe_target_temp);
        } else if (g_send.cook_mode == MODE_UPDOWN_BBQ)
            set_status_label_min(dc->status, set_temp_up, set_temp_down, set_hour, set_min);
        else {
            if (set_hour == 0)
                lv_label_set_text_fmt(dc->status, "| %s | %d℃ | %02d分钟",
                                      mode_display_name(), set_temp, set_min);
            else
                lv_label_set_text_fmt(dc->status, "| %s | %d℃ | %d小时%02d分钟",
                                      mode_display_name(), set_temp, set_hour, set_min);
        }
        mode_apply_icon(dc->icon);
        lv_label_set_text(dc->label_14, "预约中...");
        lv_label_set_text_fmt(dc->tip2, "%s%02d:%02d",
                              delay_hour >= 24 ? "明天" : "今天",
                              delay_hour % 24, delay_min);
    }
    current_group = g_delaycooking;

    /* 预约目标 g_delay_target 已在 on_delayset_start_click 用确定时日期算好并保存，
       此处不再重算（避免 delaycooking 重建/跨 0 点导致目标漂移一天） */
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    g_send.iface_status = IFACE_DELAY_RESERVE;
    g_send.remaining_ms = 0;
    g_send.cook_flag = 0;

    lv_scr_load_anim(delaycooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] back to delaycooking\n");
}

void jump_to_delaycooking(void)
{
    edit_clear();
    page_push(PAGE_DELAYCOOKING);
    rebuild_delaycooking();
    printf("[nav] jump: updown_bbq_set -> delaycooking\n");
}

static void on_contain_toggle(lv_event_t *e)
{
    updown_bbq_set_t *set = updown_bbq_set_get(&ui_manager);
    if (!set) return;
    contain_on = !contain_on;
    if (contain_on) {
        lv_obj_add_flag(set->contain_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->contain_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->contain_on_button);
    } else {
        lv_obj_add_flag(set->contain_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->contain_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->contain_button);
    }
}

void anim_bar_set_value(void *obj, int32_t v)
{
    lv_bar_set_value((lv_obj_t *)obj, v, LV_ANIM_OFF);
}

static void on_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr)) {
        if (delay_on)
            jump_to_delaycooking();
        else if (preheat_on)
            jump_to_preheat_cooking();
        else
            jump_to_updown_bbq_cooking();
    }
}

static void on_color_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_color_cookoing();
}

static void auto_pause_on_door(void)
{
    if (!is_door_open()) return;
    if (depth < 1) return;
    page_id_t cur = page_stack[depth - 1];
    if (cur == PAGE_UPDOWN_BBQ_COOKING) jump_to_updown_bbq_stop();
    else if (cur == PAGE_UPDOWN_BBQ_COOKING_PROBE) jump_to_updown_bbq_stop_probe();
    else if (cur == PAGE_HOT_BBQ_COOKING_PROBE) jump_to_hot_bbq_stop_probe();
    else if (cur == PAGE_BOTTOM_BBQ_COOKING_PROBE) jump_to_bottom_bbq_stop_probe();
    else if (cur == PAGE_SLOWCOOK_COOKING_PROBE) jump_to_slowcook_stop_probe();
    else if (cur == PAGE_TOP_BBQ_COOKING) jump_to_top_bbq_stop();
    else if (cur == PAGE_BOTTOM_BBQ_COOKING) jump_to_bottom_bbq_stop();
    else if (cur == PAGE_HOT_BBQ_COOKING) jump_to_hot_bbq_stop();
    else if (cur == PAGE_HOTWIND_BBQ_COOKING) jump_to_hotwind_bbq_stop();
    else if (cur == PAGE_SAVE_BBQ_COOKING) jump_to_save_bbq_stop();
    else if (cur == PAGE_CENTRAL_BBQ_COOKING) jump_to_central_bbq_stop();
    else if (cur == PAGE_WINDCHANGE_BBQ_COOKING) jump_to_windchange_bbq_stop();
    else if (cur == PAGE_UPDOWN_BBQ_SETTING) jump_to_updown_bbq_stop();
    else if (cur == PAGE_TOP_BBQ_SETTING) jump_to_top_bbq_stop();
    else if (cur == PAGE_BOTTOM_BBQ_SETTING) jump_to_bottom_bbq_stop();
    else if (cur == PAGE_HOT_BBQ_SETTING) jump_to_hot_bbq_stop();
    else if (cur == PAGE_HOTWIND_BBQ_SETTING) jump_to_hotwind_bbq_stop();
    else if (cur == PAGE_SAVE_BBQ_SETTING) jump_to_save_bbq_stop();
    else if (cur == PAGE_CENTRAL_BBQ_SETTING) jump_to_central_bbq_stop();
    else if (cur == PAGE_WINDCHANGE_BBQ_SETTING) jump_to_windchange_bbq_stop();
    else if (cur == PAGE_COOKIE_SETTING) jump_to_cookie_stop();
    else if (cur == PAGE_WEST_SETTING) jump_to_west_stop();
    else if (cur == PAGE_PIZZA_SETTING) jump_to_pizza_stop();
    else if (cur == PAGE_MENU_COOK_SETTING) jump_to_menu_stop();
    else if (cur == PAGE_AIR_SETTING) jump_to_air_stop();
    else if (cur == PAGE_PIZZA_2_SETTING) jump_to_pizza_2_stop();
    else if (cur == PAGE_SLOWCOOK_SETTING) jump_to_slowcook_stop();
    else if (cur == PAGE_UNFROZEN_SETTING) jump_to_unfrozen_stop();
    else if (cur == PAGE_RISING_SETTING) jump_to_rising_stop();
    else if (cur == PAGE_CORN_SETTING) jump_to_corn_stop();
    else if (cur == PAGE_HEATCONTAIN_SETTING) jump_to_heatcontain_stop();
    else if (cur == PAGE_LASAGNA_SETTING) jump_to_lasagna_stop();
    else if (cur == PAGE_STRUDEL_SETTING) jump_to_strudel_stop();
    else if (cur == PAGE_BREAD_SETTING) jump_to_bread_stop();
    else if (cur == PAGE_PIZZA3_SETTING) jump_to_pizza3_stop();
    else if (cur == PAGE_CHIP_SETTING) jump_to_chip_stop();
    else if (cur == PAGE_CUSTOM_SETTING) jump_to_custom_stop();
    else if (cur == PAGE_PREHEAT_COOKING) jump_to_preheat_stop();
    else if (cur == PAGE_COOKIE_COOKING) jump_to_cookie_stop();
    else if (cur == PAGE_WEST_COOKING) jump_to_west_stop();
    else if (cur == PAGE_PIZZA_COOKING) jump_to_pizza_stop();
    else if (cur == PAGE_MENU_COOK_COOKING) jump_to_menu_stop();
    else if (cur == PAGE_AIR_COOKING) jump_to_air_stop();
    else if (cur == PAGE_PIZZA_2_COOKING) jump_to_pizza_2_stop();
    else if (cur == PAGE_SLOWCOOK_COOKING) jump_to_slowcook_stop();
    else if (cur == PAGE_UNFROZEN_COOKING) jump_to_unfrozen_stop();
    else if (cur == PAGE_RISING_COOKING) jump_to_rising_stop();
    else if (cur == PAGE_CORN_COOKING) jump_to_corn_stop();
    else if (cur == PAGE_HEATCONTAIN_COOKING) jump_to_heatcontain_stop();
    else if (cur == PAGE_LASAGNA_COOKING) jump_to_lasagna_stop();
    else if (cur == PAGE_STRUDEL_COOKING) jump_to_strudel_stop();
    else if (cur == PAGE_BREAD_COOKING) jump_to_bread_stop();
    else if (cur == PAGE_PIZZA3_COOKING) jump_to_pizza3_stop();
    else if (cur == PAGE_CHIP_COOKING) jump_to_chip_stop();
    else if (cur == PAGE_CUSTOM_COOKING) jump_to_custom_stop();
    else if (cur == PAGE_WATER_CLEAN_COOKING) jump_to_wc_stop();
    else if (cur == PAGE_HOTCLEANSAVE_COOKING) jump_to_hcs_stop();
    else if (cur == PAGE_HOTCLEANMIDDLE_COOKING) jump_to_hcm_stop();
    else if (cur == PAGE_HOTCLEANHIGH_COOKING) jump_to_hch_stop();
    else if (cur == PAGE_COLOR_COOKING) jump_to_color_stop();
}

void cooking_timer_cb(lv_timer_t *timer)
{
    /* complete 结束态：timer 仅服务保温计数，其他页面不驱动任何完成检测 */
    if (g_send.iface_status == IFACE_COMPLETE) {
        /* 按当前 complete 组取"已完成/保温中"标签 */
        lv_obj_t *kw_label = NULL;
        if (current_group == g_updown_bbq_complete) {
            updown_bbq_complete_t *c = updown_bbq_complete_get(&ui_manager);
            kw_label = c ? c->complete_label : NULL;
        } else if (current_group == g_top_bbq_complete) {
            top_bbq_complete_t *c = top_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_108 : NULL;
        } else if (current_group == g_bottom_bbq_complete) {
            bottom_bbq_complete_t *c = bottom_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_156 : NULL;
        } else if (current_group == g_hot_bbq_complete) {
            hot_bbq_complete_t *c = hot_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_204 : NULL;
        } else if (current_group == g_hotwind_bbq_complete) {
            hotwind_bbq_complete_t *c = hotwind_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_242 : NULL;
        } else if (current_group == g_save_bbq_complete) {
            save_bbq_complete_t *c = save_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_280 : NULL;
        } else if (current_group == g_central_bbq_complete) {
            central_bbq_complete_t *c = central_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_318 : NULL;
        } else if (current_group == g_windchange_bbq_complete) {
            windchange_bbq_complete_t *c = windchange_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_356 : NULL;
        } else if (current_group == g_air_complete) {
            air_complete_t *c = air_complete_get(&ui_manager);
            kw_label = c ? c->label_337 : NULL;
        } else if (current_group == g_pizza_2_complete) {
            pizza_2_complete_t *c = pizza_2_complete_get(&ui_manager);
            kw_label = c ? c->label_386 : NULL;
        } else if (current_group == g_slowcook_complete) {
            slowcook_complete_t *c = slowcook_complete_get(&ui_manager);
            kw_label = c ? c->label_435 : NULL;
        } else if (current_group == g_unfrozen_complete) {
            unfrozen_complete_t *c = unfrozen_complete_get(&ui_manager);
            kw_label = c ? c->label_484 : NULL;
        } else if (current_group == g_rising_complete) {
            rising_complete_t *c = rising_complete_get(&ui_manager);
            kw_label = c ? c->label_533 : NULL;
        } else if (current_group == g_corn_complete) {
            corn_complete_t *c = corn_complete_get(&ui_manager);
            kw_label = c ? c->label_582 : NULL;
        } else if (current_group == g_heatcontain_complete) {
            heatcontain_complete_t *c = heatcontain_complete_get(&ui_manager);
            kw_label = c ? c->label_672 : NULL;
        } else if (current_group == g_lasagna_complete) {
            lasagna_complete_t *c = lasagna_complete_get(&ui_manager);
            kw_label = c ? c->label_681 : NULL;
        } else if (current_group == g_strudel_complete) {
            strudel_complete_t *c = strudel_complete_get(&ui_manager);
            kw_label = c ? c->label_722 : NULL;
        } else if (current_group == g_bread_complete) {
            bread_complete_t *c = bread_complete_get(&ui_manager);
            kw_label = c ? c->label_763 : NULL;
        } else if (current_group == g_pizza3_complete) {
            pizza3_complete_t *c = pizza3_complete_get(&ui_manager);
            kw_label = c ? c->label_804 : NULL;
        } else if (current_group == g_chip_complete) {
            chip_complete_t *c = chip_complete_get(&ui_manager);
            kw_label = c ? c->label_845 : NULL;
        } else if (current_group == g_custom_complete) {
            custom_complete_t *c = custom_complete_get(&ui_manager);
            kw_label = c ? c->label_886 : NULL;
        } else if (current_group == g_cookie_complete) {
            cookie_complete_t *c = cookie_complete_get(&ui_manager);
            kw_label = c ? c->label_143 : NULL;
        } else if (current_group == g_west_complete) {
            west_complete_t *c = west_complete_get(&ui_manager);
            kw_label = c ? c->label_192 : NULL;
        } else if (current_group == g_pizza_complete) {
            pizza_complete_t *c = pizza_complete_get(&ui_manager);
            kw_label = c ? c->label_240 : NULL;
        } else if (current_group == g_menu_cook_complete) {
            menu_complete_t *c = menu_complete_get(&ui_manager);
            kw_label = c ? c->label_288 : NULL;
        }
        if (kw_label && contain_on) {
            /* 自动保温：停留 1 分钟无操作 → 保温中（15 分钟）→ 结束回已完成 */
            g_keepwarm_sec++;
            if (!g_keepwarm_active) {
                if (g_keepwarm_sec >= 60) {
                    g_keepwarm_active = 1;
                    g_keepwarm_sec = 0;
                    lv_label_set_text(kw_label, "保温中...");
                    printf("[keepwarm] START: sec=%d active=%d buf17=%d timer=%p\n",
                           g_keepwarm_sec, g_keepwarm_active,
                           g_keepwarm_active ? 0x01 : 0x00, (void *)cook_timer);
                }
            } else {
                if (g_keepwarm_sec >= 60) {
                    g_keepwarm_active = 0;
                    lv_label_set_text(kw_label, "已完成");
                    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
                    printf("[keepwarm] END: sec=%d active=%d buf17=%d timer=%p\n",
                           g_keepwarm_sec, g_keepwarm_active,
                           g_keepwarm_active ? 0x01 : 0x00, (void *)cook_timer);
                }
            }
        }
        return;
    }
    if (g_on_stop_back) {
        if (g_send.iface_status == IFACE_DELAY_RESERVE &&
            (current_group == g_updown_bbq_stop_back ||
         current_group == g_top_bbq_stop_back ||
         current_group == g_bottom_bbq_stop_back ||
         current_group == g_hot_bbq_stop_back ||
         current_group == g_hotwind_bbq_stop_back ||
         current_group == g_save_bbq_stop_back ||
         current_group == g_central_bbq_stop_back ||
         current_group == g_windchange_bbq_stop_back ||
         current_group == g_air_stop_back ||
         current_group == g_pizza_2_stop_back ||
         current_group == g_slowcook_stop_back ||
         current_group == g_unfrozen_stop_back ||
         current_group == g_rising_stop_back ||
         current_group == g_corn_stop_back ||
         current_group == g_heatcontain_stop_back ||
         current_group == g_updown_bbq_stop_back_probe ||
         current_group == g_hot_bbq_stop_back_probe ||
         current_group == g_bottom_bbq_stop_back_probe ||
         current_group == g_slowcook_stop_back_probe ||
         current_group == g_cookie_stop_back ||
         current_group == g_west_stop_back ||
         current_group == g_pizza_stop_back ||
         current_group == g_menu_cook_stop_back ||
         current_group == g_lasagna_stop_back ||
         current_group == g_strudel_stop_back ||
         current_group == g_bread_stop_back ||
         current_group == g_pizza3_stop_back ||
         current_group == g_chip_stop_back ||
         current_group == g_custom_stop_back)) {
            /* 预约取消确认页（预约中态）：到点自动开始烹饪，不走时长完成检测 */
            if (g_delay_target > 0 && rtc_now_ms() >= g_delay_target) {
                if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
                delay_on = 0;
                delay_start_cook();
                if (current_group == g_updown_bbq_stop_back ||
         current_group == g_top_bbq_stop_back ||
         current_group == g_bottom_bbq_stop_back ||
         current_group == g_hot_bbq_stop_back ||
         current_group == g_hotwind_bbq_stop_back ||
         current_group == g_save_bbq_stop_back ||
         current_group == g_central_bbq_stop_back ||
         current_group == g_windchange_bbq_stop_back ||
         current_group == g_air_stop_back ||
         current_group == g_pizza_2_stop_back ||
         current_group == g_slowcook_stop_back ||
         current_group == g_unfrozen_stop_back ||
         current_group == g_rising_stop_back ||
         current_group == g_corn_stop_back ||
         current_group == g_heatcontain_stop_back ||
         current_group == g_updown_bbq_stop_back_probe ||
         current_group == g_hot_bbq_stop_back_probe ||
         current_group == g_bottom_bbq_stop_back_probe ||
         current_group == g_slowcook_stop_back_probe ||
         current_group == g_cookie_stop_back ||
         current_group == g_west_stop_back ||
         current_group == g_pizza_stop_back ||
         current_group == g_menu_cook_stop_back ||
         current_group == g_lasagna_stop_back ||
         current_group == g_strudel_stop_back ||
         current_group == g_bread_stop_back ||
         current_group == g_pizza3_stop_back ||
         current_group == g_chip_stop_back ||
         current_group == g_custom_stop_back) {
                    /* 门开被拦截：继续检查 */
                    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
                }
            }
            return;
        }
        if (current_group == g_preheat_stop_back) {
            /* 预热按腔温驱动：刷新进度条并到温自动完成 */
            preheat_stop_back_t *back = preheat_stop_back_get(&ui_manager);
            if (back) {
                uint16_t cavity = get_cavity_temp();
                int range = set_temp - preheat_start_cavity;
                int p = range <= 0 ? 100 : 13 + (int)((int64_t)87 * (cavity - preheat_start_cavity) / range);
                if (p < 13) p = 13;
                if (p > 100) p = 100;
                lv_bar_set_value(back->bar_1, p, LV_ANIM_OFF);
                if (back->bartemp) {
                    int disp = cavity > set_temp ? set_temp : cavity;
                    lv_label_set_text_fmt(back->bartemp, "%d℃", disp);
                    int bx = 122 + (637 * p) / 100 - 80;
                    lv_obj_set_pos(back->bartemp, bx, 323);
                }
            }
            if (get_cavity_temp() >= set_temp && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                g_on_stop_back = 0;
                if (g_stop_back_complete) {
                    void (*fn)(void) = g_stop_back_complete;
                    g_stop_back_complete = NULL;
                    fn();
                }
            }
            return;
        }
        if (current_group == g_hcs_stop_back ||
            current_group == g_hcm_stop_back ||
            current_group == g_hch_stop_back) {
            /* hotclean stop_back（cooking 进入）：计时完成后进入冷却阶段 */
            uint32_t e = cook_timer ? (lv_tick_get() - cook_start_time) : (uint32_t)cook_elapsed_saved;
            if (e >= (uint32_t)cook_total_ms && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_on_stop_back = 0;
                g_stop_back_complete = NULL;
                if (current_group == g_hcs_stop_back) jump_to_hcs_cooling();
                else if (current_group == g_hcm_stop_back) jump_to_hcm_cooling();
                else jump_to_hch_cooling();
                return;
            }
            /* 未完成：fall through 到下方 elapsed 进度更新（hcs/hcm/hch bar）*/
        }
        uint32_t elapsed = lv_tick_get() - cook_start_time;
        if (current_group == g_updown_bbq_stop_back) {
            updown_bbq_stop_back_t *back = updown_bbq_stop_back_get(&ui_manager);
            if (back) {
                int p = stop_back_progress(elapsed, cook_total_ms);
                if (p > 100) p = 100;
                lv_bar_set_value(back->bar_2, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_updown_bbq_stop_back_probe) {
            updown_bbq_stop_back_probe_t *back = updown_bbq_stop_back_probe_get(&ui_manager);
            if (back) {
                int probe = get_probe_temp();
                int range = probe_target_temp - cook_start_probe;
                int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
                if (p > 100) p = 100;
                if (p < 3) p = 3;
                lv_bar_set_value(back->bar_3, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_hot_bbq_stop_back_probe) {
            hot_bbq_stop_back_probe_t *back = hot_bbq_stop_back_probe_get(&ui_manager);
            if (back) {
                int probe = get_probe_temp();
                int range = probe_target_temp - cook_start_probe;
                int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
                if (p > 100) p = 100; if (p < 3) p = 3;
                lv_bar_set_value(back->bar_3, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_bottom_bbq_stop_back_probe) {
            bottom_bbq_stop_back_probe_t *back = bottom_bbq_stop_back_probe_get(&ui_manager);
            if (back) {
                int probe = get_probe_temp();
                int range = probe_target_temp - cook_start_probe;
                int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
                if (p > 100) p = 100; if (p < 3) p = 3;
                lv_bar_set_value(back->bar_7, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_slowcook_stop_back_probe) {
            slowcook_stop_back_probe_t *back = slowcook_stop_back_probe_get(&ui_manager);
            if (back) {
                int probe = get_probe_temp();
                int range = probe_target_temp - cook_start_probe;
                int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
                if (p > 100) p = 100; if (p < 3) p = 3;
                lv_bar_set_value(back->bar_11, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_top_bbq_stop_back) {
            top_bbq_stop_back_t *back = top_bbq_stop_back_get(&ui_manager);
            if (back) {
                int p = stop_back_progress(elapsed, cook_total_ms);
                if (p > 100) p = 100;
                lv_bar_set_value(back->bar_8, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_hot_bbq_stop_back) {
            hot_bbq_stop_back_t *back = hot_bbq_stop_back_get(&ui_manager);
            if (back) {
                int p = stop_back_progress(elapsed, cook_total_ms);
                if (p > 100) p = 100;
                lv_bar_set_value(back->bar_16, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_bottom_bbq_stop_back) {
            bottom_bbq_stop_back_t *back = bottom_bbq_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_12, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_hotwind_bbq_stop_back) {
            hotwind_bbq_stop_back_t *back = hotwind_bbq_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_20, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_save_bbq_stop_back) {
            save_bbq_stop_back_t *back = save_bbq_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_24, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_central_bbq_stop_back) {
            central_bbq_stop_back_t *back = central_bbq_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_28, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_windchange_bbq_stop_back) {
            windchange_bbq_stop_back_t *back = windchange_bbq_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_32, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_cookie_stop_back) {
            cookie_stop_back_t *back = cookie_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_6, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_west_stop_back) {
            west_stop_back_t *back = west_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_10, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_pizza_stop_back) {
            pizza_stop_back_t *back = pizza_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_14, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_menu_cook_stop_back) {
            menu_stop_back_t *back = menu_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_18, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_air_stop_back) {
            air_stop_back_t *back = air_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_22, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_pizza_2_stop_back) {
            pizza_2_stop_back_t *back = pizza_2_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_26, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_slowcook_stop_back) {
            slowcook_stop_back_t *back = slowcook_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_30, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_unfrozen_stop_back) {
            unfrozen_stop_back_t *back = unfrozen_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_34, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_rising_stop_back) {
            rising_stop_back_t *back = rising_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_38, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_corn_stop_back) {
            corn_stop_back_t *back = corn_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_42, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_heatcontain_stop_back) {
            heatcontain_stop_back_t *back = heatcontain_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_46, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_lasagna_stop_back) {
            lasagna_stop_back_t *back = lasagna_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_50, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_strudel_stop_back) {
            strudel_stop_back_t *back = strudel_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_54, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_bread_stop_back) {
            bread_stop_back_t *back = bread_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_58, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_pizza3_stop_back) {
            pizza3_stop_back_t *back = pizza3_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_62, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_chip_stop_back) {
            chip_stop_back_t *back = chip_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_66, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_custom_stop_back) {
            custom_stop_back_t *back = custom_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_70, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_wc_stop_back) {
            waterclean_stop_back_t *back = waterclean_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_1, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_hcs_stop_back) {
            hotcleansave_stop_back_t *back = hotcleansave_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_4, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_hcm_stop_back) {
            hotcleanmiddle_stop_back_t *back = hotcleanmiddle_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_2, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_hch_stop_back) {
            hotcleanhigh_stop_back_t *back = hotcleanhigh_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_3, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_color_stop_back) {
            color_stop_back_t *csb = color_stop_back_get(&ui_manager);
            if (csb) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(csb->bar_4, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_updown_bbq_stop_back_probe) {
            int probe = get_probe_temp();
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                g_send.cook_flag = 0;
                g_on_stop_back = 0;
                if (g_stop_back_complete) {
                    void (*fn)(void) = g_stop_back_complete;
                    g_stop_back_complete = NULL;
                    fn();
                }
            }
        }
        if (current_group == g_hot_bbq_stop_back_probe) {
            int probe = get_probe_temp();
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
                lv_timer_del(cook_timer); cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE; g_send.cook_flag = 0;
                g_on_stop_back = 0;
                if (g_stop_back_complete) { void (*fn)(void) = g_stop_back_complete; g_stop_back_complete = NULL; fn(); }
            }
        }
        if (current_group == g_bottom_bbq_stop_back_probe) {
            int probe = get_probe_temp();
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
                lv_timer_del(cook_timer); cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE; g_send.cook_flag = 0;
                g_on_stop_back = 0;
                if (g_stop_back_complete) { void (*fn)(void) = g_stop_back_complete; g_stop_back_complete = NULL; fn(); }
            }
        }
        if (current_group == g_slowcook_stop_back_probe) {
            int probe = get_probe_temp();
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
                lv_timer_del(cook_timer); cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE; g_send.cook_flag = 0;
                g_on_stop_back = 0;
                if (g_stop_back_complete) { void (*fn)(void) = g_stop_back_complete; g_stop_back_complete = NULL; fn(); }
            }
        }
        if (elapsed >= (uint32_t)cook_total_ms && cook_timer
            && g_send.iface_status != IFACE_DELAY_RESERVE
            && current_group != g_updown_bbq_stop_back_probe
            && current_group != g_hot_bbq_stop_back_probe
            && current_group != g_bottom_bbq_stop_back_probe
            && current_group != g_slowcook_stop_back_probe
            && current_group != g_preheat_stop_back
            && current_group != g_hcs_stop_back
            && current_group != g_hcm_stop_back
            && current_group != g_hch_stop_back) {
            lv_timer_del(cook_timer);
            cook_timer = NULL;
            g_send.buzzer_req = BUZZER_COOK_DONE;
            g_on_stop_back = 0;
            cook_is_color = 0;
            if (g_somecook_running) {
                somecook_cooking_next_step();
            } else if (g_stop_back_complete) {
                void (*fn)(void) = g_stop_back_complete;
                g_stop_back_complete = NULL;
                fn();
            }
        }
        return;
    }
    if (current_group == g_somecook_cooking) {
        somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
        if (sc && cook_timer) {   /* cooking/stopback 态计时中 */
            somecook_cooking_update_timer(sc);
            /* 当前段到点 → 切下一段 / 全部完成 */
            uint32_t elapsed = lv_tick_get() - cook_start_time;
            if (elapsed >= (uint32_t)cook_total_ms) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                somecook_cooking_next_step();
            }
        }
    }
    if (is_door_open()) {
        if (g_somecook_running) {
            somecook_cooking_auto_pause();
            return;
        }
        auto_pause_on_door();
        return;
    }
    if (depth > 0 && (page_stack[depth-1] == PAGE_HOTCLEANSAVE_COOLING ||
                      (page_stack[depth-1] == PAGE_HOTCLEANSAVE_STOP_BACK && !g_on_stop_back))) {
        hotcleansave_cooling_t *cool = hotcleansave_cooling_get(&ui_manager);
        if (cool) {
            uint16_t cavity = get_cavity_temp();
            if (cavity <= 270 && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                jump_to_hcs_complete();
            }
        }
        return;
    }
    if (depth > 0 && (page_stack[depth-1] == PAGE_HOTCLEANMIDDLE_COOLING ||
                      (page_stack[depth-1] == PAGE_HOTCLEANMIDDLE_STOP_BACK && !g_on_stop_back))) {
        hotcleanmiddle_cooling_t *cool = hotcleanmiddle_cooling_get(&ui_manager);
        if (cool) {
            uint16_t cavity = get_cavity_temp();
            if (cavity <= 270 && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                jump_to_hcm_complete();
            }
        }
        return;
    }
    if (depth > 0 && (page_stack[depth-1] == PAGE_HOTCLEANHIGH_COOLING ||
                      (page_stack[depth-1] == PAGE_HOTCLEANHIGH_STOP_BACK && !g_on_stop_back))) {
        hotcleanhigh_cooling_t *cool = hotcleanhigh_cooling_get(&ui_manager);
        if (cool) {
            uint16_t cavity = get_cavity_temp();
            if (cavity <= 270 && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                jump_to_hch_complete();
            }
        }
        return;
    }
    if (current_group == g_updown_bbq_stop_back) {
        uint32_t elapsed = lv_tick_get() - cook_start_time;
        if (elapsed >= (uint32_t)cook_total_ms && cook_timer) {
            lv_timer_del(cook_timer);
            cook_timer = NULL;
            g_send.buzzer_req = BUZZER_COOK_DONE;
            jump_to_updown_bbq_complete();
        }
        return;
    }
    if (current_group == g_updown_bbq_cooking_probe) {
        int probe = get_probe_temp();
        updown_bbq_cooking_probe_t *cook = updown_bbq_cooking_probe_get(&ui_manager);
        if (cook) {
            int range = probe_target_temp - cook_start_probe;
            int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
            if (p > 100) p = 100;
            if (p < 3) p = 3;
            lv_bar_set_value(cook->bar_1, p, LV_ANIM_OFF);
            int display = probe > probe_target_temp ? probe_target_temp : probe;
            lv_label_set_text_fmt(cook->temp, "%d℃", display);
            lv_obj_invalidate(lv_scr_act());
        }
            if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
            lv_timer_del(cook_timer);
            cook_timer = NULL;
            g_send.buzzer_req = BUZZER_COOK_DONE;
            g_send.cook_flag = 0;
            jump_to_updown_bbq_complete_probe();
        }
        return;
    }
    if (current_group == g_hot_bbq_cooking_probe) {
        int probe = get_probe_temp();
        hot_bbq_cooking_probe_t *cook = hot_bbq_cooking_probe_get(&ui_manager);
        if (cook) {
            int range = probe_target_temp - cook_start_probe;
            int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
            if (p > 100) p = 100; if (p < 3) p = 3;
            lv_bar_set_value(cook->bar_1, p, LV_ANIM_OFF);
            int display = probe > probe_target_temp ? probe_target_temp : probe;
            lv_label_set_text_fmt(cook->temp, "%d℃", display);
            lv_obj_invalidate(lv_scr_act());
        }
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
            lv_timer_del(cook_timer); cook_timer = NULL;
            g_send.buzzer_req = BUZZER_COOK_DONE; g_send.cook_flag = 0;
            jump_to_hot_bbq_complete_probe();
        }
        return;
    }
    if (current_group == g_bottom_bbq_cooking_probe) {
        int probe = get_probe_temp();
        bottom_bbq_cooking_probe_t *cook = bottom_bbq_cooking_probe_get(&ui_manager);
        if (cook) {
            int range = probe_target_temp - cook_start_probe;
            int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
            if (p > 100) p = 100; if (p < 3) p = 3;
            lv_bar_set_value(cook->bar_5, p, LV_ANIM_OFF);
            int display = probe > probe_target_temp ? probe_target_temp : probe;
            lv_label_set_text_fmt(cook->temp, "%d℃", display);
            lv_obj_invalidate(lv_scr_act());
        }
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
            lv_timer_del(cook_timer); cook_timer = NULL;
            g_send.buzzer_req = BUZZER_COOK_DONE; g_send.cook_flag = 0;
            jump_to_bottom_bbq_complete_probe();
        }
        return;
    }
    if (current_group == g_slowcook_cooking_probe) {
        int probe = get_probe_temp();
        slowcook_cooking_probe_t *cook = slowcook_cooking_probe_get(&ui_manager);
        if (cook) {
            int range = probe_target_temp - cook_start_probe;
            int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
            if (p > 100) p = 100; if (p < 3) p = 3;
            lv_bar_set_value(cook->bar_9, p, LV_ANIM_OFF);
            int display = probe > probe_target_temp ? probe_target_temp : probe;
            lv_label_set_text_fmt(cook->temp, "%d℃", display);
            lv_obj_invalidate(lv_scr_act());
        }
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
            lv_timer_del(cook_timer); cook_timer = NULL;
            g_send.buzzer_req = BUZZER_COOK_DONE; g_send.cook_flag = 0;
            jump_to_slowcook_complete_probe();
        }
        return;
    }
    if (current_group == g_delaycooking) {
        if (g_delay_target > 0 && rtc_now_ms() >= g_delay_target) {
            delay_on = 0;
            if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
            delay_start_cook();
            if (current_group == g_delaycooking) {
                /* 门开被拦截：重建定时器继续检查 */
                cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
            }
        }
        return;
    }
    if (current_group == g_preheat_cooking) {
        preheatcooking_t *cook = preheatcooking_get(&ui_manager);
        if (cook) {
            uint16_t cavity = get_cavity_temp();
            int range = set_temp - preheat_start_cavity;
            int p = range <= 0 ? 100 : 13 + (int)((int64_t)87 * (cavity - preheat_start_cavity) / range);
            if (p < 13) p = 13;
            if (p > 100) p = 100;
            lv_bar_set_value(cook->bar_1, p, LV_ANIM_OFF);
            if (cook->bartemp) {
                int disp = cavity > set_temp ? set_temp : cavity;
                lv_label_set_text_fmt(cook->bartemp, "%d℃", disp);
                int bx = 122 + (637 * p) / 100 - 80;
                lv_obj_set_pos(cook->bartemp, bx, 323);
            }
            if (p >= 100 && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                jump_to_preheat_complete();
            }
        }
        return;
    }
    if (current_group == g_preheat_stop_back) {
        preheat_stop_back_t *back = preheat_stop_back_get(&ui_manager);
        if (back) {
            uint16_t cavity = get_cavity_temp();
            int range = set_temp - preheat_start_cavity;
            int p = range <= 0 ? 100 : 13 + (int)((int64_t)87 * (cavity - preheat_start_cavity) / range);
            if (p < 13) p = 13;
            if (p > 100) p = 100;
            lv_bar_set_value(back->bar_1, p, LV_ANIM_OFF);
            if (back->bartemp) {
                int disp = cavity > set_temp ? set_temp : cavity;
                lv_label_set_text_fmt(back->bartemp, "%d℃", disp);
                int bx = 122 + (637 * p) / 100 - 80;
                lv_obj_set_pos(back->bartemp, bx, 323);
            }
            if (p >= 100 && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                jump_to_preheat_complete();
            }
        }
        return;
    }

    lv_obj_t *time_label = NULL;

    if (current_group == g_updown_bbq_setting) {
        updown_bbq_setting_t *set = updown_bbq_setting_get(&ui_manager);
        if (set) time_label = set->time_label;
    } else if (current_group == g_updown_bbq_cooking) {
        updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->time_label;
    } else if (current_group == g_top_bbq_setting) {
        top_bbq_setting_t *set = top_bbq_setting_get(&ui_manager);
        if (set) time_label = set->time_label;
    } else if (current_group == g_color_cookoing) {
        color_cookoing_t *cc = color_cookoing_get(&ui_manager);
        if (cc) time_label = cc->time_label;
    } else if (current_group == g_top_bbq_cooking) {
        top_bbq_cooking_t *cook = top_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->label_80;
    } else if (current_group == g_bottom_bbq_setting) {
        bottom_bbq_setting_t *set = bottom_bbq_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_bottom_bbq_cooking) {
        bottom_bbq_cooking_t *cook = bottom_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_hot_bbq_setting) {
        hot_bbq_setting_t *set = hot_bbq_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_hot_bbq_cooking) {
        hot_bbq_cooking_t *cook = hot_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_hotwind_bbq_setting) {
        hotwind_bbq_setting_t *set = hotwind_bbq_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_hotwind_bbq_cooking) {
        hotwind_bbq_cooking_t *cook = hotwind_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_save_bbq_setting) {
        save_bbq_setting_t *set = save_bbq_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_save_bbq_cooking) {
        save_bbq_cooking_t *cook = save_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_central_bbq_setting) {
        central_bbq_setting_t *set = central_bbq_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_central_bbq_cooking) {
        central_bbq_cooking_t *cook = central_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_air_cooking) {
        air_cooking_t *cook = air_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_air_setting) {
        air_setting_t *set = air_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_pizza_2_cooking) {
        pizza_2_cooking_t *cook = pizza_2_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_pizza_2_setting) {
        pizza_2_setting_t *set = pizza_2_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_slowcook_cooking) {
        slowcook_cooking_t *cook = slowcook_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_slowcook_setting) {
        slowcook_setting_t *set = slowcook_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_unfrozen_cooking) {
        unfrozen_cooking_t *cook = unfrozen_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_unfrozen_setting) {
        unfrozen_setting_t *set = unfrozen_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_rising_cooking) {
        rising_cooking_t *cook = rising_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_rising_setting) {
        rising_setting_t *set = rising_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_corn_cooking) {
        corn_cooking_t *cook = corn_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_corn_setting) {
        corn_setting_t *set = corn_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_heatcontain_cooking) {
        heatcontain_cooking_t *cook = heatcontain_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_heatcontain_setting) {
        heatcontain_setting_t *set = heatcontain_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_lasagna_cooking) {
        lasagna_cooking_t *cook = lasagna_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_lasagna_setting) {
        lasagna_setting_t *set = lasagna_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_strudel_cooking) {
        strudel_cooking_t *cook = strudel_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_strudel_setting) {
        strudel_setting_t *set = strudel_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_bread_cooking) {
        bread_cooking_t *cook = bread_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_bread_setting) {
        bread_setting_t *set = bread_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_pizza3_cooking) {
        pizza3_cooking_t *cook = pizza3_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_pizza3_setting) {
        pizza3_setting_t *set = pizza3_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_chip_cooking) {
        chip_cooking_t *cook = chip_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_chip_setting) {
        chip_setting_t *set = chip_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_custom_cooking) {
        custom_cooking_t *cook = custom_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_custom_setting) {
        custom_setting_t *set = custom_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_wc_cooking) {
        waterclean_cooking_t *cook = waterclean_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_wc_stop) {
        waterclean_stop_t *stop = waterclean_stop_get(&ui_manager);
        if (stop) time_label = stop->timelabel;
    } else if (current_group == g_hcs_cooking) {
        hotcleansave_cooking_t *cook = hotcleansave_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_hcs_stop) {
        hotcleansave_stop_t *stop = hotcleansave_stop_get(&ui_manager);
        if (stop) time_label = stop->timelabel;
    } else if (current_group == g_hcm_cooking) {
        hotcleanmiddle_cooking_t *cook = hotcleanmiddle_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_hcm_stop) {
        hotcleanmiddle_stop_t *stop = hotcleanmiddle_stop_get(&ui_manager);
        if (stop) time_label = stop->timelabel;
    } else if (current_group == g_hch_cooking) {
        hotcleanhigh_cooking_t *cook = hotcleanhigh_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_hch_stop) {
        hotcleanhigh_stop_t *stop = hotcleanhigh_stop_get(&ui_manager);
        if (stop) time_label = stop->timelabel;
    } else if (current_group == g_windchange_bbq_setting) {
        windchange_bbq_setting_t *set = windchange_bbq_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_windchange_bbq_cooking) {
        windchange_bbq_cooking_t *cook = windchange_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_cookie_cooking) {
        cookie_cooking_t *cook = cookie_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_cookie_setting) {
        cookie_setting_t *set = cookie_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_west_cooking) {
        west_cooking_t *cook = west_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_west_setting) {
        west_setting_t *set = west_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_pizza_cooking) {
        pizza_cooking_t *cook = pizza_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_pizza_setting) {
        pizza_setting_t *set = pizza_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_menu_cook_cooking) {
        menu_cooking_t *cook = menu_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_menu_cook_setting) {
        menu_setting_t *set = menu_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else {
        /* 不在已识别的烹饪/设置/保温链内：可能页面已切走，禁止访问悬空对象 */
        time_label = NULL;
    }

    if (!time_label) return;

    uint32_t elapsed = lv_tick_get() - cook_start_time;
    if (elapsed >= (uint32_t)cook_total_ms) {
        lv_timer_del(cook_timer);
        cook_timer = NULL;
        g_send.buzzer_req = BUZZER_COOK_DONE;
        if (current_group == g_updown_bbq_setting) {
            if (depth > 0) depth--;
            lv_obj_clean(lv_scr_act());
        } else if (current_group == g_top_bbq_setting || current_group == g_top_bbq_cooking) {
            /* top_bbq 计时结束，直接跳 complete */
            int top_depth = depth;
            while (top_depth > 1 && page_stack[top_depth - 1] != PAGE_TOP_BBQ_COOKING &&
                   page_stack[top_depth - 1] != PAGE_TOP_BBQ_SETTING) {
                top_depth--;
            }
            if (top_depth > 1) depth = top_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_top_bbq_complete();
            return;
        } else if (current_group == g_bottom_bbq_setting || current_group == g_bottom_bbq_cooking) {
            int btm_depth = depth;
            while (btm_depth > 1 && page_stack[btm_depth - 1] != PAGE_BOTTOM_BBQ_COOKING &&
                   page_stack[btm_depth - 1] != PAGE_BOTTOM_BBQ_SETTING) {
                btm_depth--;
            }
            if (btm_depth > 1) depth = btm_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_bottom_bbq_complete();
            return;
        } else if (current_group == g_hot_bbq_setting || current_group == g_hot_bbq_cooking) {
            int hot_depth = depth;
            while (hot_depth > 1 && page_stack[hot_depth - 1] != PAGE_HOT_BBQ_COOKING &&
                   page_stack[hot_depth - 1] != PAGE_HOT_BBQ_SETTING) {
                hot_depth--;
            }
            if (hot_depth > 1) depth = hot_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_hot_bbq_complete();
            return;
        } else if (current_group == g_hotwind_bbq_setting || current_group == g_hotwind_bbq_cooking) {
            int hotwind_depth = depth;
            while (hotwind_depth > 1 && page_stack[hotwind_depth - 1] != PAGE_HOTWIND_BBQ_COOKING &&
                   page_stack[hotwind_depth - 1] != PAGE_HOTWIND_BBQ_SETTING) {
                hotwind_depth--;
            }
            if (hotwind_depth > 1) depth = hotwind_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_hotwind_bbq_complete();
            return;
        } else if (current_group == g_save_bbq_setting || current_group == g_save_bbq_cooking) {
            int save_depth = depth;
            while (save_depth > 1 && page_stack[save_depth - 1] != PAGE_SAVE_BBQ_COOKING &&
                   page_stack[save_depth - 1] != PAGE_SAVE_BBQ_SETTING) {
                save_depth--;
            }
            if (save_depth > 1) depth = save_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_save_bbq_complete();
            return;
        } else if (current_group == g_central_bbq_setting || current_group == g_central_bbq_cooking) {
            int central_depth = depth;
            while (central_depth > 1 && page_stack[central_depth - 1] != PAGE_CENTRAL_BBQ_COOKING &&
                   page_stack[central_depth - 1] != PAGE_CENTRAL_BBQ_SETTING) {
                central_depth--;
            }
            if (central_depth > 1) depth = central_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_central_bbq_complete();
            return;
        } else if (current_group == g_windchange_bbq_setting || current_group == g_windchange_bbq_cooking) {
            int windchange_depth = depth;
            while (windchange_depth > 1 && page_stack[windchange_depth - 1] != PAGE_WINDCHANGE_BBQ_COOKING &&
                   page_stack[windchange_depth - 1] != PAGE_WINDCHANGE_BBQ_SETTING) {
                windchange_depth--;
            }
            if (windchange_depth > 1) depth = windchange_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_windchange_bbq_complete();
            return;
        } else if (current_group == g_cookie_cooking || current_group == g_cookie_setting) {
            int c_depth = depth;
            while (c_depth > 1 && page_stack[c_depth - 1] != PAGE_COOKIE_COOKING &&
                   page_stack[c_depth - 1] != PAGE_COOKIE_SETTING) {
                c_depth--;
            }
            if (c_depth > 1) depth = c_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_cookie_complete();
            return;
        } else if (current_group == g_west_cooking || current_group == g_west_setting) {
            int w_depth = depth;
            while (w_depth > 1 && page_stack[w_depth - 1] != PAGE_WEST_COOKING &&
                   page_stack[w_depth - 1] != PAGE_WEST_SETTING) {
                w_depth--;
            }
            if (w_depth > 1) depth = w_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_west_complete();
            return;
        } else if (current_group == g_pizza_cooking || current_group == g_pizza_setting) {
            int p_depth = depth;
            while (p_depth > 1 && page_stack[p_depth - 1] != PAGE_PIZZA_COOKING &&
                   page_stack[p_depth - 1] != PAGE_PIZZA_SETTING) {
                p_depth--;
            }
            if (p_depth > 1) depth = p_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_pizza_complete();
            return;
        } else if (current_group == g_air_cooking || current_group == g_air_setting) {
            int a_depth = depth;
            while (a_depth > 1 && page_stack[a_depth - 1] != PAGE_AIR_COOKING &&
                   page_stack[a_depth - 1] != PAGE_AIR_SETTING) {
                a_depth--;
            }
            if (a_depth > 1) depth = a_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_air_complete();
            return;
        } else if (current_group == g_pizza_2_cooking || current_group == g_pizza_2_setting) {
            int p2_depth = depth;
            while (p2_depth > 1 && page_stack[p2_depth - 1] != PAGE_PIZZA_2_COOKING &&
                   page_stack[p2_depth - 1] != PAGE_PIZZA_2_SETTING) {
                p2_depth--;
            }
            if (p2_depth > 1) depth = p2_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_pizza_2_complete();
            return;
        } else if (current_group == g_slowcook_cooking || current_group == g_slowcook_setting) {
            int sc_depth = depth;
            while (sc_depth > 1 && page_stack[sc_depth - 1] != PAGE_SLOWCOOK_COOKING &&
                   page_stack[sc_depth - 1] != PAGE_SLOWCOOK_SETTING) {
                sc_depth--;
            }
            if (sc_depth > 1) depth = sc_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_slowcook_complete();
            return;
        } else if (current_group == g_unfrozen_cooking || current_group == g_unfrozen_setting) {
            int uf_depth = depth;
            while (uf_depth > 1 && page_stack[uf_depth - 1] != PAGE_UNFROZEN_COOKING &&
                   page_stack[uf_depth - 1] != PAGE_UNFROZEN_SETTING) {
                uf_depth--;
            }
            if (uf_depth > 1) depth = uf_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_unfrozen_complete();
            return;
        } else if (current_group == g_rising_cooking || current_group == g_rising_setting) {
            int rs_depth = depth;
            while (rs_depth > 1 && page_stack[rs_depth - 1] != PAGE_RISING_COOKING &&
                   page_stack[rs_depth - 1] != PAGE_RISING_SETTING) {
                rs_depth--;
            }
            if (rs_depth > 1) depth = rs_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_rising_complete();
            return;
        } else if (current_group == g_corn_cooking || current_group == g_corn_setting) {
            int cn_depth = depth;
            while (cn_depth > 1 && page_stack[cn_depth - 1] != PAGE_CORN_COOKING &&
                   page_stack[cn_depth - 1] != PAGE_CORN_SETTING) {
                cn_depth--;
            }
            if (cn_depth > 1) depth = cn_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_corn_complete();
            return;
        } else if (current_group == g_heatcontain_cooking || current_group == g_heatcontain_setting) {
            int hc_depth = depth;
            while (hc_depth > 1 && page_stack[hc_depth - 1] != PAGE_HEATCONTAIN_COOKING &&
                   page_stack[hc_depth - 1] != PAGE_HEATCONTAIN_SETTING) {
                hc_depth--;
            }
            if (hc_depth > 1) depth = hc_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_heatcontain_complete();
            return;
        } else if (current_group == g_lasagna_cooking || current_group == g_lasagna_setting) {
            int ls_depth = depth;
            while (ls_depth > 1 && page_stack[ls_depth - 1] != PAGE_LASAGNA_COOKING &&
                   page_stack[ls_depth - 1] != PAGE_LASAGNA_SETTING) {
                ls_depth--;
            }
            if (ls_depth > 1) depth = ls_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_lasagna_complete();
            return;
        } else if (current_group == g_strudel_cooking || current_group == g_strudel_setting) {
            int st_depth = depth;
            while (st_depth > 1 && page_stack[st_depth - 1] != PAGE_STRUDEL_COOKING &&
                   page_stack[st_depth - 1] != PAGE_STRUDEL_SETTING) {
                st_depth--;
            }
            if (st_depth > 1) depth = st_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_strudel_complete();
            return;
        } else if (current_group == g_bread_cooking || current_group == g_bread_setting) {
            int br_depth = depth;
            while (br_depth > 1 && page_stack[br_depth - 1] != PAGE_BREAD_COOKING &&
                   page_stack[br_depth - 1] != PAGE_BREAD_SETTING) {
                br_depth--;
            }
            if (br_depth > 1) depth = br_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_bread_complete();
            return;
        } else if (current_group == g_pizza3_cooking || current_group == g_pizza3_setting) {
            int p3_depth = depth;
            while (p3_depth > 1 && page_stack[p3_depth - 1] != PAGE_PIZZA3_COOKING &&
                   page_stack[p3_depth - 1] != PAGE_PIZZA3_SETTING) {
                p3_depth--;
            }
            if (p3_depth > 1) depth = p3_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_pizza3_complete();
            return;
        } else if (current_group == g_chip_cooking || current_group == g_chip_setting) {
            int ch_depth = depth;
            while (ch_depth > 1 && page_stack[ch_depth - 1] != PAGE_CHIP_COOKING &&
                   page_stack[ch_depth - 1] != PAGE_CHIP_SETTING) {
                ch_depth--;
            }
            if (ch_depth > 1) depth = ch_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_chip_complete();
            return;
        } else if (current_group == g_custom_cooking || current_group == g_custom_setting) {
            int cu_depth = depth;
            while (cu_depth > 1 && page_stack[cu_depth - 1] != PAGE_CUSTOM_COOKING &&
                   page_stack[cu_depth - 1] != PAGE_CUSTOM_SETTING) {
                cu_depth--;
            }
            if (cu_depth > 1) depth = cu_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_custom_complete();
            return;
        } else if (current_group == g_wc_cooking) {
            int wc_depth = depth;
            while (wc_depth > 1 && page_stack[wc_depth - 1] != PAGE_WATER_CLEAN_COOKING) {
                wc_depth--;
            }
            if (wc_depth > 1) depth = wc_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_wc_complete();
            return;
        } else if (current_group == g_hcs_cooking) {
            int hcs_depth = depth;
            while (hcs_depth > 1 && page_stack[hcs_depth - 1] != PAGE_HOTCLEANSAVE_COOKING) {
                hcs_depth--;
            }
            if (hcs_depth > 1) depth = hcs_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_hcs_cooling();
            return;
        } else if (current_group == g_hcm_cooking) {
            int hcm_depth = depth;
            while (hcm_depth > 1 && page_stack[hcm_depth - 1] != PAGE_HOTCLEANMIDDLE_COOKING) {
                hcm_depth--;
            }
            if (hcm_depth > 1) depth = hcm_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_hcm_cooling();
            return;
        } else if (current_group == g_hch_cooking) {
            int hch_depth = depth;
            while (hch_depth > 1 && page_stack[hch_depth - 1] != PAGE_HOTCLEANHIGH_COOKING) {
                hch_depth--;
            }
            if (hch_depth > 1) depth = hch_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_hch_cooling();
            return;
        } else if (current_group == g_menu_cook_cooking || current_group == g_menu_cook_setting) {
            int m_depth = depth;
            while (m_depth > 1 && page_stack[m_depth - 1] != PAGE_MENU_COOK_COOKING &&
                   page_stack[m_depth - 1] != PAGE_MENU_COOK_SETTING) {
                m_depth--;
            }
            if (m_depth > 1) depth = m_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_menu_complete();
            return;
        }
        if (current_group == g_color_cookoing) {
            jump_to_color_complete();
        } else if (current_group == g_updown_bbq_cooking ||
                   current_group == g_updown_bbq_setting) {
            jump_to_updown_bbq_complete();
        } else {
            /* 未识别页面：安全返回，不强制跳转（防止在错误页面跳转/破坏页面栈） */
            return;
        }
        return;
    }

    /* 时间显示（半秒四舍五入） */
    int elapsed_sec = (elapsed + 500) / 1000;
    int total_sec = cook_total_ms / 1000;
    int remaining_sec = total_sec - elapsed_sec;
    if (remaining_sec < 0) remaining_sec = 0;
    int h = remaining_sec / 3600;
    int m = (remaining_sec % 3600) / 60;
    int s = remaining_sec % 60;
    lv_label_set_text_fmt(time_label, "%02d:%02d:%02d", h, m, s);
    g_send.remaining_ms = remaining_sec * 1000;
}

// 为 major_menu 的三个按钮绑定 LV_EVENT_CLICKED 回调
// 注意：每次 groups_create 后必须调用 bind_events，新按钮需要重新绑定
void bind_events(void)
{
    major_menu_t *major = major_menu_get(&ui_manager);
    if (!major) {
        printf("[nav] bind_events: major_menu is NULL\n");
        return;
    }

    if (major->cook_button)
        lv_obj_add_event_cb(major->cook_button, on_major_cook_click,
                            LV_EVENT_CLICKED, NULL);
    else
        printf("[nav] cannot bind cook_button: NULL\n");

    if (major->special_button)
        lv_obj_add_event_cb(major->special_button, on_major_special_click,
                            LV_EVENT_CLICKED, NULL);
    else
        printf("[nav] cannot bind special_button: NULL\n");

    if (major->cook4_button)
        lv_obj_add_event_cb(major->cook4_button, on_major_cook4_click,
                            LV_EVENT_CLICKED, NULL);
    else
        printf("[nav] cannot bind cook4_button: NULL\n");
}

// ==============================
// 暂停 / 恢复
// ==============================

// cooking → stop（暂停）
static void jump_to_updown_bbq_stop(void)
{
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

    /* 保存 cooking bar 的实际值（3→100，与 cooking 页动画一致） */
    cook_bar_saved = 3 + (int)((int64_t)cook_elapsed_saved * 97 / (cook_total_ms ? cook_total_ms : 1));
    if (cook_bar_saved > 100) cook_bar_saved = 100;

    page_push(PAGE_UPDOWN_BBQ_STOP);
    lv_obj_clean(lv_scr_act());
    updown_bbq_stop_create(&ui_manager);

    updown_bbq_stop_t *stop = updown_bbq_stop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start_button, stop->little_button };
        if (g_updown_bbq_stop) lv_group_del(g_updown_bbq_stop);
        g_updown_bbq_stop = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(stop->start_button, on_stop_start_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(stop->little_button, on_cook_setting_click,
                            LV_EVENT_CLICKED, NULL);

        /* 同步 time_label / statu_label / bar_1 */
        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(stop->time_label, "%02d:%02d:%02d", h, m, s);

        set_status_label_min(stop->statu_label, set_temp_up, set_temp_down, set_hour, set_min);

        lv_bar_set_range(stop->bar_1, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_1, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_updown_bbq_stop;

    lv_scr_load_anim(updown_bbq_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = (cook_total_ms > (int)cook_elapsed_saved)
                          ? cook_total_ms - (int)cook_elapsed_saved : 0;
    printf("[nav] jump: cooking -> updown_bbq_stop (pause)\n");
}

// stop/cooking → stop_back（确认退出）
static void jump_to_updown_bbq_stop_back(void)
{
    int cooking_bar_val = 0;
    if (cook_timer && depth > 0 && page_stack[depth - 1] == PAGE_UPDOWN_BBQ_COOKING) {
        updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
        if (cook && cook->bar) cooking_bar_val = lv_bar_get_value(cook->bar);
    }

    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_updown_bbq_complete;

    page_push(PAGE_UPDOWN_BBQ_STOP_BACK);
    lv_obj_clean(lv_scr_act());
    updown_bbq_stop_back_create(&ui_manager);

    updown_bbq_stop_back_t *back = updown_bbq_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure_button };
        if (g_updown_bbq_stop_back) lv_group_del(g_updown_bbq_stop_back);
        g_updown_bbq_stop_back = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(back->sure_button, on_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        /* 同步 statu_label / bar_2（从耗时计算进度） */
        set_status_label_min(back->statu_label, set_temp_up, set_temp_down, set_hour, set_min);

        int p = cooking_bar_val;
        if (p <= 0) {
            uint32_t elapsed = cook_elapsed_saved;
            p = stop_back_progress(elapsed, cook_total_ms);
        }
        if (p > 100) p = 100;
        lv_bar_set_range(back->bar_2, 0, 100);
        lv_bar_set_value(back->bar_2, p, LV_ANIM_OFF);

        if (g_complete_to_stop_back) {
            printf("[keepwarm] stop_back enter (from complete): keepwarm=%d iface=%d\n",
                   g_keepwarm_active, g_send.iface_status);
            g_complete_to_stop_back = 0;
            if (g_keepwarm_active)
                lv_label_set_text(back->label_8, "保温中...");
            else
                lv_label_set_text(back->label_8, "已完成");
            lv_bar_set_value(back->bar_2, 100, LV_ANIM_OFF);
        }

        if (g_delay_cancel_to_stop_back) {
            g_delay_cancel_to_stop_back = 0;
            lv_label_set_text(back->label_8, "预约中...");
            lv_label_set_text(back->label_11, g_delay_cancel_btn ? "回到上一页" : "回到主页");
            lv_obj_add_flag(back->bar_2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(back->image_6, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(back->littal_button, LV_OBJ_FLAG_HIDDEN);
            printf("[keepwarm] stop_back enter (delay cancel): iface=%d active=%d\n",
                   g_send.iface_status, g_keepwarm_active);
        }

        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(back->label_8, "烹饪中...");
        printf("[keepwarm] stop_back enter: keepwarm=%d iface=%d label=%s\n",
               g_keepwarm_active, g_send.iface_status,
               lv_label_get_text(back->label_8) ? lv_label_get_text(back->label_8) : "?");
    }
    current_group = g_updown_bbq_stop_back;

    lv_scr_load_anim(updown_bbq_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] jump: stop/cooking -> updown_bbq_stop_back\n");
}

// cooking 暂停按钮点击
static void on_cook_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_stop();
}

// stop 开始按钮点击 → 恢复计时
static void on_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        stop_resume_cooking();
}

// stop 小按钮点击 → 确认退出页
static void on_stop_littal_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_stop_back();
}

// stop_back 小按钮点击 → 返回暂停页
static void on_stop_back_littal_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        page_pop();
}

// stop 恢复 cooking（不经过 page_pop，直接重建）
static void stop_resume_cooking(void)
{
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    depth--;  /* pop STOP 栈顶 */
    lv_obj_clean(lv_scr_act());
    updown_bbq_cooking_create(&ui_manager);

    updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop_button, cook->little_button };
        if (g_updown_bbq_cooking) lv_group_del(g_updown_bbq_cooking);
        g_updown_bbq_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop_button, on_cook_stop_click,
                            LV_EVENT_CLICKED, NULL);

        lv_obj_add_event_cb(cook->little_button, on_cook_setting_click,
                            LV_EVENT_CLICKED, NULL);

        /* 恢复状态标签 */
        set_status_label_min(cook->updown_label, set_temp_up, set_temp_down, set_hour, set_min);

        /* 恢复时间显示 */
        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(cook->time_label, "%02d:%02d:%02d", h, m, s);

        /* 进度条从保存的实际值 → 100 */
        lv_bar_set_range(cook->bar, 0, 100);
        lv_bar_set_value(cook->bar, cook_bar_saved, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, cook_bar_saved, 100);
        lv_anim_set_time(&a, ((int)(cook_total_ms - (int)cook_elapsed_saved) < 0) ? 0 : (cook_total_ms - (int)cook_elapsed_saved));
        lv_anim_start(&a);
    }

    /* 恢复定时器 */
    cook_start_time = lv_tick_get() - cook_elapsed_saved;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_updown_bbq_cooking;

    lv_scr_load_anim(updown_bbq_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    {
        int rem = cook_total_ms - (int)cook_elapsed_saved;
        if (rem < 0) rem = 0;
        g_send.iface_status = IFACE_COOKING;
        g_send.remaining_ms = rem;
    }
    printf("[nav] resume: stop -> updown_bbq_cooking\n");
}

// stop_back 确定 → 退出到 major_menu
// delay 取消确认（stop_back 确定）→ 取消预约并回来源 set 页（温度/时间等设置保持）
void delay_cancel_exit_to_set(void)
{
    /* 栈守卫:仅当栈结构为 [.., DELAYCOOKING, STOP_BACK]（delay 取消场景）才执行;
       标志残留误调时只清标志,不动页面栈,防止破坏栈 */
    if (depth < 3 || page_stack[depth - 2] != PAGE_DELAYCOOKING) {
        g_delay_cancel_btn = 0;
        g_delay_cancel_to_stop_back = 0;
        return;
    }
    g_delay_cancel_to_stop_back = 0;
    g_delay_cancel_btn = 0;
    delay_on = 0;
    page_pop();   /* pop STOP_BACK（内部 rebuild_delaycooking 会重建 cook_timer/iface） */
    page_pop();   /* pop DELAYCOOKING → 重建 set 屏 */
    /* 清理必须放在两次 pop 之后，否则会被 rebuild_delaycooking 抵消 */
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    cook_total_ms = 0;
    cook_start_time = 0;
    cook_elapsed_saved = 0;
    cook_bar_saved = 0;
    g_on_stop_back = 0;
    g_complete_to_stop_back = 0;
    g_stop_back_complete = NULL;
    g_delay_target = -1;
    g_send.iface_status = IFACE_SETTING;
    g_send.remaining_ms = -1;
    g_send.cook_flag = 0;
}

static void on_stop_back_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    if (g_delay_cancel_btn) {
        delay_cancel_exit_to_set();
        return;
    }
    g_on_stop_back = 0;
    g_keepwarm_active = 0;

    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    cook_elapsed_saved = 0; cook_bar_saved = 0;

    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;

    depth = 2;  /* 保留 WAITMENU_24 + MAJOR_MENU */
    lv_obj_clean(lv_scr_act());
    major_menu_create(&ui_manager);
    groups_create();
    bind_events();
    current_group = g_major_menu;
    lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] stop_back sure -> major_menu\n");
}

// ==============================
// 额外上色暂停 / 恢复
// ==============================

// color_cookoing → color_stop（暂停）
static void jump_to_color_stop(void)
{
    g_on_stop_back = 0;
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

    {
        color_cookoing_t *cc = color_cookoing_get(&ui_manager);
        cook_bar_saved = cc ? lv_bar_get_value(cc->bar) : 0;
    }

    page_push(PAGE_COLOR_STOP);
    lv_obj_clean(lv_scr_act());
    color_stop_create(&ui_manager);

    color_stop_t *cs = color_stop_get(&ui_manager);
    if (cs) {
        lv_obj_t *btns[] = { cs->button_6 };
        if (g_color_stop) lv_group_del(g_color_stop);
        g_color_stop = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cs->button_6, on_color_stop_start_click,
                            LV_EVENT_CLICKED, NULL);

        /* 同步 time_label / label_13 / bar_3 */
        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(cs->time_label, "%02d:%02d:%02d", h, m, s);
        lv_label_set_text_fmt(cs->label_13, "| 额外上色 | %d℃ | 5分钟", set_temp);
        lv_bar_set_range(cs->bar_3, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(cs->bar_3, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_color_stop;

    lv_scr_load_anim(color_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = (cook_total_ms > (int)cook_elapsed_saved) ? cook_total_ms - (int)cook_elapsed_saved : 0;
    printf("[nav] jump: color_cookoing -> color_stop (pause)\n");
}

// color_stop → color_stop_back（确认退出）
static void jump_to_color_stop_back(void)
{
    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_color_complete;
    page_push(PAGE_COLOR_STOP_BACK);
    lv_obj_clean(lv_scr_act());
    color_stop_back_create(&ui_manager);

    color_stop_back_t *csb = color_stop_back_get(&ui_manager);
    if (csb) {
        lv_obj_t *btns[] = { csb->button_7 };
        if (g_color_stop_back) lv_group_del(g_color_stop_back);
        g_color_stop_back = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(csb->button_7, on_color_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(csb->label_17, "| 额外上色 | %d℃ | 5分钟", set_temp);
        uint32_t _elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
        int _p = stop_back_progress(_elapsed, cook_total_ms);
        if (_p > 100) _p = 100;
        lv_bar_set_range(csb->bar_4, 0, 100);
        lv_bar_set_value(csb->bar_4, _p, LV_ANIM_OFF);

        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(csb->label_19, "烹饪中...");

        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            lv_label_set_text(csb->label_19, "已完成");
            lv_bar_set_value(csb->bar_4, 100, LV_ANIM_OFF);
        }

        if (g_extra_color_to_stop_back) {
            g_extra_color_to_stop_back = 0;
            lv_label_set_text(csb->label_17, "| 5分钟 |");
            lv_label_set_text(csb->label_19, "额外上色");
            lv_bar_set_value(csb->bar_4, 0, LV_ANIM_OFF);
        }
    }
    current_group = g_color_stop_back;

    lv_scr_load_anim(color_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] jump: stop/cooking -> color_stop_back\n");
}

// color_cookoing 暂停按钮点击
static void on_color_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_color_stop();
}

// color_stop 开始按钮点击 → 恢复计时
static void on_color_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        color_resume_cooking();
}

// color_stop 恢复 cooking（不经过 page_pop，直接重建）
static void color_resume_cooking(void)
{
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    g_on_stop_back = 0;
    depth--;  /* pop COLOR_STOP 栈顶 */
    lv_obj_clean(lv_scr_act());
    color_cookoing_create(&ui_manager);

    color_cookoing_t *cc = color_cookoing_get(&ui_manager);
    if (cc) {
        lv_obj_t *btns[] = { cc->stop_button };
        if (g_color_cookoing) lv_group_del(g_color_cookoing);
        g_color_cookoing = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cc->stop_button, on_color_stop_click,
                            LV_EVENT_CLICKED, NULL);

        /* 恢复时间显示 */
        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(cc->time_label, "%02d:%02d:%02d", h, m, s);
        lv_label_set_text_fmt(cc->status_label, "| 额外上色 | %d℃ | 5分钟", set_temp);

        /* 进度条从保存的值 → 100 */
        lv_bar_set_range(cc->bar, 0, 100);
        lv_bar_set_value(cc->bar, cook_bar_saved, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cc->bar);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, cook_bar_saved, 100);
        lv_anim_set_time(&a, ((int)(cook_total_ms - (int)cook_elapsed_saved) < 0) ? 0 : (cook_total_ms - (int)cook_elapsed_saved));
        lv_anim_start(&a);
    }

    /* 恢复定时器 */
    cook_is_color = 1;
    cook_start_time = lv_tick_get() - cook_elapsed_saved;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_color_cookoing;

    lv_scr_load_anim(color_cookoing_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    g_send.remaining_ms = (cook_total_ms > (int)cook_elapsed_saved) ? cook_total_ms - (int)cook_elapsed_saved : 0;
    printf("[nav] resume: color_stop -> color_cookoing\n");
}

// color_stop_back 确定 → 退出到 major_menu
static void on_color_stop_back_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    color_exit_to_home();
    printf("[nav] color_stop_back sure -> home\n");
}

// ==============================
// Cooking 设置页
// ==============================

// cooking little_button → 设置页（计时器继续运行，实时更新 time_label）
static void jump_to_updown_bbq_setting(void)
{
    updown_setting_saved_temp_up = set_temp_up;
    updown_setting_saved_temp_down = set_temp_down;
    updown_setting_saved_hour = set_hour;
    updown_setting_saved_min = set_min;

    page_push(PAGE_UPDOWN_BBQ_SETTING);
    lv_obj_clean(lv_scr_act());
    updown_bbq_setting_create(&ui_manager);

    updown_bbq_setting_t *set = updown_bbq_setting_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->tempup_label, set->tempdown_label,
            set->hour_label, set->min_label,
            set->sure_button,
        };
        if (g_updown_bbq_setting) lv_group_del(g_updown_bbq_setting);
        g_updown_bbq_setting = group_create_for_page(btns, 5);

        edit_clear();
        edit_register(set->tempup_label, set->shortup_templine_img, set->longup_templine_img,
                      &set_temp_up, 30, 300, 5, "%d");
        edit_register(set->tempdown_label, set->shordown_templine_img, set->longdown_templine_img,
                      &set_temp_down, 30, 300, 5, "%d");
        edit_register(set->hour_label, set->hourline_img, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(set->min_label, set->minline_label, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(set->tempup_label, on_setting_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->tempdown_label, on_setting_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->hour_label, on_setting_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->min_label, on_setting_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure_button, on_setting_edit_focus,
                            LV_EVENT_FOCUSED, NULL);

        lv_obj_add_event_cb(set->sure_button, on_setting_sure_click,
                            LV_EVENT_CLICKED, NULL);

        /* 同步显示（从 cooking 或 stop 进入） */
        uint32_t elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
        int elapsed_sec = (elapsed + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        if (g_send.iface_status == IFACE_COMPLETE) { h = m = s = 0; }
        lv_label_set_text_fmt(set->time_label, "%02d:%02d:%02d", h, m, s);
        lv_label_set_text_fmt(set->hour_label, "%02d", set_hour);
        lv_label_set_text_fmt(set->min_label, "%02d", set_min);
        lv_label_set_text_fmt(set->tempup_label, "%d", set_temp_up);
        lv_label_set_text_fmt(set->tempdown_label, "%d", set_temp_down);

        /* 初始隐藏所有指示线 + dir/icon */
        lv_obj_add_flag(set->shortup_templine_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->longup_templine_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->shordown_templine_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->longdown_templine_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hourline_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->minline_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->dirup2_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->dirup3_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2_label1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon3_label1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->dirdown2_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->dirdown3_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2_label2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon3_label2, LV_OBJ_FLAG_HIDDEN);

        lv_group_focus_obj(set->tempup_label);

        update_setting_dir_icon(set);
    }
    current_group = g_updown_bbq_setting;

    lv_scr_load_anim(updown_bbq_setting_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    /* complete 结束态（保温场景从 complete 进入设置页）保持 4，避免完成检测误触发 */
    if (g_send.iface_status != IFACE_COMPLETE)
        g_send.iface_status = (cook_timer != NULL) ? IFACE_COOKING : IFACE_SETTING;
    printf("[keepwarm] setting enter: iface=%d complete_keep=%d timer=%p\n",
           g_send.iface_status, (g_send.iface_status == IFACE_COMPLETE), (void *)cook_timer);
    printf("[nav] jump: cooking -> updown_bbq_setting\n");
}

// cooking little_button 点击
static void on_cook_setting_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_setting();
}

// 设置页 dir/icon 即时更新
static void update_setting_dir_icon(updown_bbq_setting_t *set)
{
    if (!set) return;
    lv_obj_add_flag(set->dirup2_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->dirup3_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->icon2_label1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->icon3_label1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->dirdown2_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->dirdown3_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->icon2_label2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->icon3_label2, LV_OBJ_FLAG_HIDDEN);

    if (set_temp_up < 100) {
        lv_obj_clear_flag(set->dirup2_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->icon2_label1, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(set->dirup3_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->icon3_label1, LV_OBJ_FLAG_HIDDEN);
    }
    if (set_temp_down < 100) {
        lv_obj_clear_flag(set->dirdown2_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->icon2_label2, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(set->dirdown3_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->icon3_label2, LV_OBJ_FLAG_HIDDEN);
    }
}

// 设置页 focus 回调（线随焦点，dir/icon 按数值固定显示）
static void on_setting_edit_focus(lv_event_t *e)
{
    on_edit_focus(e);
    update_setting_dir_icon(updown_bbq_setting_get(&ui_manager));
}

// 设置页确定 → 应用新参数回到 cooking
static void on_setting_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;

    /* 温差钳位 */
    if (set_temp_up - set_temp_down > 20) set_temp_up = set_temp_down + 20;
    else if (set_temp_down - set_temp_up > 20) set_temp_down = set_temp_up + 20;

    set_temp = set_temp_up;
    cook_total_ms = (set_hour * 3600 + set_min * 60) * 1000;

    if (depth > 1) depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_UPDOWN_BBQ_STOP)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_UPDOWN_BBQ_COMPLETE)
        depth--;
    lv_obj_clean(lv_scr_act());
    updown_bbq_cooking_create(&ui_manager);

    updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop_button, cook->little_button };
        if (g_updown_bbq_cooking) lv_group_del(g_updown_bbq_cooking);
        g_updown_bbq_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop_button, on_cook_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little_button, on_cook_setting_click,
                            LV_EVENT_CLICKED, NULL);

        set_status_label_min(cook->updown_label, set_temp_up, set_temp_down, set_hour, set_min);
        lv_label_set_text_fmt(cook->time_label, "%02d:%02d:%02d", set_hour, set_min, 0);

        lv_bar_set_range(cook->bar, 0, 100);
        lv_bar_set_value(cook->bar, 3, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_updown_bbq_cooking;

    lv_scr_load_anim(updown_bbq_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = (uint16_t)set_temp_up;
    g_send.set_temp_lower = (uint16_t)set_temp_down;
    g_send.remaining_ms = cook_total_ms;
    printf("[nav] setting sure -> updown_bbq_cooking\n");
}

// ==============================
// 系统定时器（每500ms，常驻检测）
// ==============================

static void system_timer_cb(lv_timer_t *timer)
{
    static int probe_last = 0;
    static uint32_t probe_last_time = 0;
    int probe_now = is_probe_inserted();

    // 门状态边沿检测（预热完成等待放食材阶段：门开又关 → 重建 complete 显示 sure）
    static int door_last = -1;
    int door_now = is_door_open();
    if (door_now != door_last) {
        door_last = door_now;
        if (preheat_wait_door && !door_now) {
            preheat_wait_door = 0;
            if (depth > 0 && (page_stack[depth - 1] == PAGE_PREHEAT_COMPLETE ||
                              page_stack[depth - 1] == PAGE_PREHEAT_STOP_BACK))
                jump_to_preheat_complete();
        }
    }

    if (probe_now == probe_last)
        return;

    uint32_t now = lv_tick_get();
    if (now - probe_last_time < 1000) {
        probe_last = probe_now;
        return;
    }
    probe_last_time = now;
    probe_last = probe_now;

    if (depth > 0 && page_stack[depth - 1] == PAGE_PROBETIP) {
        probetip_t *tip = probetip_get(&ui_manager);
        if (tip && tip->button_1) {
            lv_obj_t *label = lv_obj_get_child(tip->button_1, 0);
            if (label) lv_label_set_text(label, probe_now ? "探针已插入" : "探针已拔出");
        }
        return;
    }

    if (g_send.iface_status == IFACE_SLEEP) {
        jump_to_probetip(probe_now ? "探针已插入" : "探针已拔出");
        return;
    }

    probetip_cancel_auto_dismiss();   /* 取消陈旧的探针提示自动关闭定时器,防止跨会话误触发 */
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    set_temp = 180; set_temp_up = 180; set_temp_down = 180;
    set_hour = 0; set_min = 30;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    cook_total_ms = 0; cook_is_color = 0;
    preheat_on = 0; delay_on = 0; contain_on = 0;
    g_on_stop_back = 0;
    g_complete_to_stop_back = 0;
    g_cooling_to_stop_back = 0;
    g_extra_color_to_stop_back = 0;
    g_stop_back_complete = NULL;
    g_delay_cancel_btn = 0;
    g_somecook_running = 0;
    g_somecook_run_idx = 0;
    probe_target_temp = 80;
    g_send.iface_status = IFACE_STANDBY;
    g_send.cook_mode = MODE_NONE;
    g_send.cook_flag = 0;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;

    depth = 0;
    page_push(PAGE_WAITMENU_24);
    lv_obj_clean(lv_scr_act());
    waitmenu_24_create(&ui_manager);
    waitmenu_clock_cache_reset();   /* 强制刷新为真实时间 */
    current_group = NULL;
    lv_scr_load(waitmenu_24_get(&ui_manager)->obj);
    waitmenu_apply_clock();   /* 立即刷新为真实时间 */
    jump_to_probetip(probe_now ? "探针已插入" : "探针已拔出");
}

// ==============================
// 初始化入口
// ==============================

void nav_init(void)
{
    waitmenu_24_t *wait = waitmenu_24_get(&ui_manager);
    if (!wait) {
        printf("[nav] waitmenu_24 struct is NULL, skip nav_init\n");
        return;
    }
    if (!wait->obj) {
        printf("[nav] waitmenu_24->obj is NULL, skip nav_init\n");
        return;
    }

    printf("[nav] init start\n");
    depth = 0;
    page_push(PAGE_WAITMENU_24);        // 根页 = waitmenu_24
    page_push(PAGE_MAJOR_MENU);         // 上电自动进入 major_menu
    lv_obj_clean(lv_scr_act());
    major_menu_create(&ui_manager);
    groups_create();
    bind_events();
    current_group = g_major_menu;
    lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    g_send.iface_status = IFACE_SETTING;
    lv_timer_create(system_timer_cb, 500, NULL);

    /* topflag 顶层状态页：生成代码已改挂 lv_layer_top，所有页面之上（wait 页面除外） */
    topflagpage_create(&ui_manager);
    {
        topflagpage_t *tf = topflagpage_get(&ui_manager);
        if (tf) {
            /* 暂时只显示 currenttime */
            if (tf->demo)   lv_obj_add_flag(tf->demo, LV_OBJ_FLAG_HIDDEN);
            if (tf->timer)  lv_obj_add_flag(tf->timer, LV_OBJ_FLAG_HIDDEN);
            if (tf->light)  lv_obj_add_flag(tf->light, LV_OBJ_FLAG_HIDDEN);
        }
    }
    topflag_update_visibility();
    lv_timer_create(topflag_clock_cb, 500, NULL);
    printf("[nav] init done -> major_menu\n");
}

// topflag 顶层状态页显隐：除 wait 页面外都显示。
// 由 page_push / page_pop 驱动（事件驱动，无轮询延迟）
static void topflag_update_visibility(void)
{
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->obj) return;
    int is_wait = (depth > 0 && page_stack[depth - 1] == PAGE_WAITMENU_24);
    if (is_wait) lv_obj_add_flag(tf->obj, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_clear_flag(tf->obj, LV_OBJ_FLAG_HIDDEN);
}

// 待机页时钟缓存（waitmenu_apply_clock 使用）
static lv_obj_t *lw_obj = NULL;
static uint8_t lw_hour = 0xFF, lw_min = 0xFF;
static uint8_t lw_year = 0, lw_month = 0, lw_day = 0;
static int lw_wday = -1;

// 显式重置缓存：waitmenu_24_create 后调用，强制刷新为真实时间。
// 不依赖指针相等判定——auto_del 下 malloc 地址复用会导致缓存不失效（显示默认假文本/陈旧星期）
static void waitmenu_clock_cache_reset(void)
{
    lw_obj = NULL;
    lw_hour = 0xFF; lw_min = 0xFF;
    lw_year = 0; lw_month = 0; lw_day = 0; lw_wday = -1;
}

// 待机页 waitmenu_24 时间/星期/年月日 实时刷新：
// 页面重建（obj 指针变化或显式缓存重置）时清缓存强制刷新为真实时间；平时按数值变化更新。
// 有效性判断使用"页面栈顶 == 待机页"（业务状态，可靠），
// 不能用 obj == lv_scr_act 指针比较——离开待机页后 wait->obj 悬空，
// malloc 地址复用时可能误判通过导致 UAF 写入（模拟器卡死根因）。
static void waitmenu_apply_clock(void)
{
    rtc_time_t t;
    if (rtc_get_time(&t) != 0) return;
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WAITMENU_24) return;
    waitmenu_24_t *wait = waitmenu_24_get(&ui_manager);
    if (!wait || !wait->obj) return;
    if (wait->obj != lw_obj) {
        lw_obj = wait->obj;
        lw_hour = 0xFF; lw_min = 0xFF;
        lw_year = 0; lw_month = 0; lw_day = 0; lw_wday = -1;
    }
    if (t.hour != lw_hour || t.min != lw_min) {
        lw_hour = t.hour; lw_min = t.min;
        char buf[6];
        snprintf(buf, sizeof(buf), "%02d:%02d", t.hour, t.min);
        if (wait->time_label) lv_label_set_text(wait->time_label, buf);
    }
    /* week_label = 星期 + 年月日 合并文本（生成默认："星期日, 2025年12月15日"） */
    if (t.wday != lw_wday || t.year != lw_year || t.month != lw_month || t.day != lw_day) {
        lw_wday = t.wday;
        lw_year = t.year; lw_month = t.month; lw_day = t.day;
        static const char *week_cn[] = {"星期日", "星期一", "星期二",
                                        "星期三", "星期四", "星期五", "星期六"};
        char buf[32];
        snprintf(buf, sizeof(buf), "%s, %d年%d月%d日",
                 (t.wday >= 0 && t.wday <= 6) ? week_cn[t.wday] : "",
                 t.year, t.month, t.day);
        if (wait->week_label) lv_label_set_text(wait->week_label, buf);
    }
}

// topflag 时间走动：每 500ms 刷新 currenttime 为 "HH:MM"（分钟变化才更新）；
// 同时刷新待机页 waitmenu_24 的 时间/星期/年月日
static void topflag_clock_cb(lv_timer_t *timer)
{
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->currenttime) return;
    rtc_time_t t;
    if (rtc_get_time(&t) != 0) return;

    /* topflag 右上角时间 */
    static uint8_t last_hour = 0xFF, last_min = 0xFF;
    if (t.hour != last_hour || t.min != last_min) {
        last_hour = t.hour; last_min = t.min;
        char buf[6];
        snprintf(buf, sizeof(buf), "%02d:%02d", t.hour, t.min);
        lv_label_set_text(tf->currenttime, buf);
    }

    /* 待机页三标签（独立缓存，分钟/跨天/星期变化才更新） */
    waitmenu_apply_clock();
}
