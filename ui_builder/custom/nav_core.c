/*
 * nav_core.c - 导航核心：全局状态 + 基础工具
 *
 * 职责：
 *   1. 全局变量定义（页面栈、温度/时间设置值、各页面焦点组、运行状态标志）
 *   2. 可编辑字段注册表（edit_*：支持编码器加减数值的页面字段）
 *   3. 公共 UI 工具（时间/进度/开关/焦点态设置）
 *   4. 页面栈操作 page_push（跳转子页前入栈）
 *   5. 焦点组创建 groups_create（主菜单焦点组，开机/返回时重建）
 *
 * 页面栈约定：
 *   page_stack[0] = 根页(WAITMENU)，depth-1 = 当前页；
 *   跳转 = page_push + 重建 UI + 切 current_group；返回 = page_pop。
 */

#include "nav.h"
#include "nav_lang.h"
#include "nav_internal.h"

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

void uart_print(void)
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
/* 真机:必须保持空函数!帧的填充+发送由 custom.c 的 handle_uart_send 定时器
   (uart_send_fill + send_uart_param_array,100ms)统一完成;
   此处绝不能再调 uart_send_fill——那是"填充即消费 buzzer_req"的语义,
   提前消费会把按键音请求清零,导致实机无声(20260902 修过的坑) */
void uart_print(void)
{
}
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
lv_group_t *g_favorites;
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

int updown_menu_top_saved, updown_menu_low_saved;

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
/* updown_bbq setting 页进入时保存原始值，BACK 返回时恢复 */
int updown_setting_saved_temp_up, updown_setting_saved_temp_down;
int updown_setting_saved_hour, updown_setting_saved_min;
// ==============================
// 可编辑字段注册表
// ==============================

#define MAX_EDIT_FIELDS 8
edit_field_t edit_fields[MAX_EDIT_FIELDS];
int edit_count = 0;

/* 清空可编辑字段注册表（切页前调用，防止 find_edit_field 指针复用误判） */
void edit_clear(void)
{
    edit_count = 0;
}
/* 注册一个可编辑字段：label 为显示标签，ind_s/ind_l 为温度<100/≥100 的指示条，
   value 指向实际存储值，min/max/step 为循环范围，fmt 为显示格式（"%d"/"%02d"） */
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
/* 按 label 查找已注册的可编辑字段（编码器加减时定位字段）。
 * lv_obj_is_valid 校验:残条目(所属页已销毁,如功能键跳离编辑页未清注册)的
 * label 地址失效即跳过并就地置空——从根上杜绝"地址复用误命中→悬空 label
 * 调参"的 UAF 类问题,不依赖每条跳转路径都记得 edit_clear */
edit_field_t *find_edit_field(lv_obj_t *obj)
{
    for (int i = 0; i < edit_count; i++) {
        if (edit_fields[i].label == obj) {
            if (!lv_obj_is_valid(edit_fields[i].label)) {
                edit_fields[i].label = NULL;   /* 失效条目就地失效 */
                continue;
            }
            return &edit_fields[i];
        }
    }
    return NULL;
}
/* 编码器加减：循环调整数值、刷新标签与温度指示线、
   执行各设置页的温差约束(上下≤20℃)与 dir/icon 即时更新 */
void adjust_value(edit_field_t *f, int delta)
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
        min_field->min = 5;    // hour=0时，最少5分钟
        min_field->max = 59;
    } else if (set_hour == max_h) {
        min_field->min = 0;    // 最大小时时 minute 不可调
        min_field->max = 0;
    } else {
        min_field->min = 0;    // 0-59 正常循环
        min_field->max = 59;
    }

    /* 纠正 minute 值（如果当前值超出新范围） */
    if (set_hour == 0 && set_min < 5) {
        set_min = 5;   /* hour=0 时最少 5 分钟(与 min_field->min 一致,对齐 SDK) */
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
        lv_label_set_text_fmt(label, tr("| 上下烧烤 | %d℃ | %02d分钟"), temp, min);
    else
        lv_label_set_text_fmt(label, tr("| 上下烧烤 | %d℃ | %d小时%02d分钟"), temp, hour, min);
}
void set_status_label_min(lv_obj_t *label, int temp_up, int temp_down, int hour, int min)
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
void setup_set_temp_display(updown_bbq_set_t *set)
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

