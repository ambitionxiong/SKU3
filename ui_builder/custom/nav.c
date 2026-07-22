#include "nav.h"
#include "protocol.h"

// === 页面栈 ===
#define MAX_STACK 16
page_id_t page_stack[MAX_STACK];  // 栈数组，stack[0]=根页，stack[depth-1]=当前页
int depth = 0;                     // 栈高度，depth=1 时只有根页

int set_temp_up = 180;
int set_temp_down = 180;
int cook_bar_saved = 0;

#ifdef LV_USE_AIC_SIMULATOR

static void uart_print(void)
{
    uart_send_fill();
    printf("[UART]");
    for (int i = 0; i < 24; i++) printf(" %02X", uart_data_send[i]);
    static const char *st[] = {"stdby","set","cook","pause","done","sleep"};
    static const char *md[] = {"none","","","updown","top","bottom","","","hot","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","color"};
    printf("\n[UART] decoded: st=%s(%d) mode=%s(%d) t=%d t_lo=%d time=%02d:%02d:%02d buz=%d\n",
           st[g_send.iface_status<=5?g_send.iface_status:0], g_send.iface_status,
           md[g_send.cook_mode<=38?g_send.cook_mode:0], g_send.cook_mode,
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
lv_group_t *g_special_menu;
lv_group_t *g_updown_bbq_menu;
lv_group_t *g_updown_bbq_set;
lv_group_t *g_updown_bbq_cooking;
lv_group_t *g_updown_bbq_complete;

lv_group_t *g_updown_bbq_menu_top;
lv_group_t *g_updown_bbq_menu_low;
lv_group_t *g_preheat_menu;

lv_group_t *g_preheat_cooking;

lv_group_t *g_preheat_stop;

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
static void on_updown_next_click(lv_event_t *e);
void on_edit_focus(lv_event_t *e);
void validate_constraints(void);
static void on_preheat_toggle(lv_event_t *e);
static void on_delay_toggle(lv_event_t *e);
static void on_contain_toggle(lv_event_t *e);
static void on_sure_click(lv_event_t *e);
void cooking_timer_cb(lv_timer_t *timer);
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

    validate_constraints();

    /* 设置页：上下温差 ≤ 20°C，超限则回弹 */
    if (current_group == g_updown_bbq_setting) {
        int diff = set_temp_up - set_temp_down;
        if (diff < 0) diff = -diff;
        updown_bbq_setting_t *set = updown_bbq_setting_get(&ui_manager);
        if (diff > 20) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            *f->value = old_val;
            lv_label_set_text_fmt(f->label, f->fmt, old_val);
            if (f->ind_short && f->ind_long) {
                lv_obj_add_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
                if (old_val < 100)
                    lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
            }
        }
        update_setting_dir_icon(set);
    }
    /* 上层温度页：上下温差 ≤ 20°C，超限则回弹 */
    if (current_group == g_updown_bbq_menu_top) {
        int diff = set_temp_up - set_temp_down;
        if (diff < 0) diff = -diff;
        if (diff > 20) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            *f->value = old_val;
            lv_label_set_text_fmt(f->label, f->fmt, old_val);
            if (f->ind_short && f->ind_long) {
                lv_obj_add_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
                if (old_val < 100)
                    lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
            }
            lv_obj_invalidate(lv_scr_act());
        }
    }
    /* 下层温度页：上下温差 ≤ 20°C，超限则回弹 */
    if (current_group == g_updown_bbq_menu_low) {
        int diff = set_temp_up - set_temp_down;
        if (diff < 0) diff = -diff;
        if (diff > 20) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            *f->value = old_val;
            lv_label_set_text_fmt(f->label, f->fmt, old_val);
            if (f->ind_short && f->ind_long) {
                lv_obj_add_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
                if (old_val < 100)
                    lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
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

    /* 根据 hour 动态调整 minute 的循环范围 */
    if (set_hour == 0) {
        min_field->min = 0;    // 0-59 正常循环
    } else if (set_hour == 4) {
        min_field->min = 0;    // hour=4 时 minute 不可调
        min_field->max = 0;
    } else {
        min_field->min = 0;    // 0-59 正常循环
        min_field->max = 59;
    }

    /* 纠正 minute 值（如果当前值超出新范围） */
    if (set_hour == 4 && set_min != 0) {
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
    }
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
    page_id_t child = page_stack[depth];
    page_id_t prev = page_stack[depth - 1];
    printf("[nav] page pop: depth=%d, back to id=%d (from id=%d)\n", depth, prev, child);

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
            }
            current_group = g_special_menu;
        }
        lv_scr_load_anim(special_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to special_menu\n");
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
                    int progress = (int)((int64_t)elapsed * 100 / cook_total_ms);
                    if (progress > 100) progress = 100;
                    lv_bar_set_value(cook->bar, progress, LV_ANIM_OFF);
                    lv_anim_t a;
                    lv_anim_init(&a);
                    lv_anim_set_var(&a, cook->bar);
                    lv_anim_set_exec_cb(&a, anim_bar_set_value);
                    lv_anim_set_values(&a, progress, 100);
                    lv_anim_set_time(&a, cook_total_ms - (int)elapsed);
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
        } else {
            goto rebuild_updown_bbq_set;
        }
        break;

    case PAGE_UPDOWN_BBQ_STOP:
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
        updown_bbq_stop_back_create(&ui_manager);
        {
            updown_bbq_stop_back_t *back = updown_bbq_stop_back_get(&ui_manager);
            if (back) {
                lv_obj_t *btns[] = { back->sure_button, back->littal_button };
                if (g_updown_bbq_stop_back) lv_group_del(g_updown_bbq_stop_back);
                g_updown_bbq_stop_back = group_create_for_page(btns, 2);
                lv_obj_add_event_cb(back->sure_button, on_stop_back_sure_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(back->littal_button, on_stop_back_littal_click,
                                    LV_EVENT_CLICKED, NULL);

                /* 同步显示（与暂停页一致） */
                set_status_label_min(back->statu_label, set_temp_up, set_temp_down, set_hour, set_min);

                lv_bar_set_range(back->bar_2, 0, 100);
                if (cook_bar_saved > 100) cook_bar_saved = 100;
                lv_bar_set_value(back->bar_2, cook_bar_saved, LV_ANIM_OFF);
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
                set_hour = h;
                set_min = m;
                lv_label_set_text_fmt(set->time_label, "%02d:%02d:%02d", h, m, s);
                lv_label_set_text_fmt(set->hour_label, "%02d", h);
                lv_label_set_text_fmt(set->min_label, "%02d", m);
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
    case PAGE_EXTRA_COLOR:
    case PAGE_COLOR_COOKING:
    case PAGE_COLOR_COOKING_COMPLETE:
        goto pop_to_major_menu;

    case PAGE_UPDOWN_BBQ_MENU_TOP:
        goto rebuild_updown_bbq_set;
    case PAGE_UPDOWN_BBQ_MENU_LOW:
        goto rebuild_updown_bbq_set;
    case PAGE_UPDOWN_BBQ_SET:
    rebuild_updown_bbq_set:
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

                /* 时间显示 */
                lv_label_set_text_fmt(set->hour_label, "%02d", set_hour);
                lv_label_set_text_fmt(set->min_label, "%02d", set_min);

                /* 恢复 toggle 状态（跟随变量） */
                apply_toggle_state(set->preheat_button, set->preheat_on_button, preheat_on);
                apply_toggle_state(set->delay_button, set->delay_on_button, delay_on);
                apply_toggle_state(set->contain_button, set->contain_on_button, contain_on);

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
            }
            current_group = g_updown_bbq_set;
        }
        lv_scr_load_anim(updown_bbq_set_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to updown_bbq_set\n");
        break;

    case PAGE_COLOR_STOP:
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
                lv_label_set_text(cs->label_13, "| 额外上色 | 5分钟");
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

                lv_label_set_text(csb->label_17, "| 额外上色 | 5分钟");
                lv_bar_set_range(csb->bar_4, 0, 100);
                if (cook_bar_saved > 100) cook_bar_saved = 100;
                lv_bar_set_value(csb->bar_4, cook_bar_saved, LV_ANIM_OFF);
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
        if (child == PAGE_TOP_BBQ_SETTING) {
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
        if (child == PAGE_BOTTOM_BBQ_SETTING) {
            set_temp = bottom_bbq_setting_saved_temp;
            set_hour = bottom_bbq_setting_saved_hour;
            set_min = bottom_bbq_setting_saved_min;
        }
        bottom_bbq_rebuild_cooking(child);
        if (child == PAGE_BOTTOM_BBQ_SETTING) {
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        }
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
        if (child == PAGE_HOT_BBQ_SETTING) {
            set_temp = hot_bbq_setting_saved_temp;
            set_hour = hot_bbq_setting_saved_hour;
            set_min = hot_bbq_setting_saved_min;
        }
        hot_bbq_rebuild_cooking(child);
        if (child == PAGE_HOT_BBQ_SETTING) {
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        }
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
        if (child == PAGE_HOTWIND_BBQ_SETTING) {
            set_temp = hotwind_bbq_setting_saved_temp;
            set_hour = hotwind_bbq_setting_saved_hour;
            set_min = hotwind_bbq_setting_saved_min;
        }
        hotwind_bbq_rebuild_cooking(child);
        if (child == PAGE_HOTWIND_BBQ_SETTING) {
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        }
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
        if (child == PAGE_SAVE_BBQ_SETTING) {
            set_temp = save_bbq_setting_saved_temp;
            set_hour = save_bbq_setting_saved_hour;
            set_min = save_bbq_setting_saved_min;
        }
        save_bbq_rebuild_cooking(child);
        if (child == PAGE_SAVE_BBQ_SETTING) {
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        }
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
        if (child == PAGE_CENTRAL_BBQ_SETTING) {
            set_temp = central_bbq_setting_saved_temp;
            set_hour = central_bbq_setting_saved_hour;
            set_min = central_bbq_setting_saved_min;
        }
        central_bbq_rebuild_cooking(child);
        if (child == PAGE_CENTRAL_BBQ_SETTING) {
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        }
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
        if (child == PAGE_WINDCHANGE_BBQ_SETTING) {
            set_temp = windchange_bbq_setting_saved_temp;
            set_hour = windchange_bbq_setting_saved_hour;
            set_min = windchange_bbq_setting_saved_min;
        }
        windchange_bbq_rebuild_cooking(child);
        if (child == PAGE_WINDCHANGE_BBQ_SETTING) {
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        }
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
        if (child == PAGE_COOKIE_SETTING) {
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
        if (child == PAGE_WEST_SETTING) {
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
        if (child == PAGE_PIZZA_SETTING) {
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
        if (child == PAGE_MENU_COOK_SETTING) {
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
        goto pop_to_major_menu;

    case PAGE_PREHEAT_COOKING:
    case PAGE_PREHEAT_STOP:
    case PAGE_PREHEAT_COMPLETE:
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

        depth = 2;  /* 保留 WAITMENU_24 + MAJOR_MENU */
        lv_obj_clean(lv_scr_act());
        major_menu_create(&ui_manager);
        groups_create();
        bind_events();
        current_group = g_major_menu;
        lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] pop to major_menu\n");
        break;

    case PAGE_WAITMENU_24:
        waitmenu_24_create(&ui_manager);
        current_group = NULL;
        lv_scr_load_anim(waitmenu_24_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        g_send.iface_status = IFACE_STANDBY;
        printf("[nav] back to waitmenu_24\n");
        break;

    default:
        printf("[nav] unknown page to restore: %d\n", prev);
        break;
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
        lv_obj_add_event_cb(cook->hot_wind_button, on_hotwind_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->save_button, on_save_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->central_button, on_central_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->windchange_buttonn, on_windchange_click,
                            LV_EVENT_CLICKED, NULL);
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
    }

    current_group = g_special_menu;

    lv_scr_load_anim(special_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    printf("[nav] jump: major_menu -> special_menu\n");
}

// cookmenu → updown_bbq_menu
static void jump_to_updown_bbq_menu(void)
{
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

        /* 按钮状态重置（跟随变量） */
        apply_toggle_state(set->preheat_button, set->preheat_on_button, preheat_on);
        apply_toggle_state(set->delay_button, set->delay_on_button, delay_on);
        apply_toggle_state(set->contain_button, set->contain_on_button, contain_on);

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
static void jump_to_updown_bbq_cooking(void)
{
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
    }

    current_group = g_updown_bbq_complete;

    lv_scr_load_anim(updown_bbq_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    g_send.iface_status = IFACE_COMPLETE;
    g_send.remaining_ms = 0;
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
    if (!screen_is_loading(act_scr))
        page_pop();
}

static void on_updown_low_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        page_pop();
}

static void jump_to_updown_bbq_menu_top(void)
{
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
        if (set_temp_up < 100)
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);

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
        if (set_temp_down < 100)
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);

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

// extra_color → color_cookoing（固定 5 分钟倒计时）
static void jump_to_color_cookoing(void)
{
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
        lv_label_set_text_fmt(cc->time_label, "%02d:%02d:%02d", 0, 5, 0);
        lv_bar_set_range(cc->bar, 0, 100);
        lv_bar_set_value(cc->bar, 3, LV_ANIM_OFF);
    }

    /* 启动进度条动画 */
    cook_total_ms = 5 * 60 * 1000;
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

    current_group = g_color_cookoing;

    lv_scr_load_anim(color_cookoing_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    printf("[nav] jump: extra_color -> color_cookoing\n");
}

// color_cookoing → colorcooking_complete
static void jump_to_color_complete(void)
{
    page_push(PAGE_COLOR_COOKING_COMPLETE);
    lv_obj_clean(lv_scr_act());
    colorcooking_complete_create(&ui_manager);

    current_group = NULL;

    lv_scr_load_anim(colorcooking_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

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

static void process_key(uint8_t key)
{
    uart_data_receive[Receive_data_Touch_Key] = 0;

    switch (key) {
    case KEY1:              // 1: 开关机键，短按无操作
        break;
    case KEY_MENU:          // 3: 从 waitmenu_24 进入主菜单
        g_send.buzzer_req = BUZZER_KEY_VALID;
        if (depth == 1 && page_stack[0] == PAGE_WAITMENU_24) {
            page_push(PAGE_MAJOR_MENU);
            lv_obj_clean(lv_scr_act());
            major_menu_create(&ui_manager);
            groups_create();
            bind_events();
            current_group = g_major_menu;
            lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0,
                             ui_manager.auto_del);
            g_send.iface_status = IFACE_SETTING;
            g_send.cook_mode = MODE_NONE;
            g_send.set_temp = 0;
            g_send.set_temp_lower = 0;
            g_send.remaining_ms = -1;
            printf("[nav] jump: waitmenu_24 -> major_menu\n");
        }
        uart_print();
        break;
    case KEY_EXTRA_COLOR:   // 5: 进入额外上色
        g_send.buzzer_req = BUZZER_KEY_VALID;
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
    case KEY_BACK:          // 21: 返回
        g_send.buzzer_req = BUZZER_KEY_VALID;
        {
            page_id_t cur = page_stack[depth - 1];
            if (cur == PAGE_UPDOWN_BBQ_COOKING)
                jump_to_updown_bbq_stop();
            else if (cur == PAGE_UPDOWN_BBQ_STOP)
                jump_to_updown_bbq_stop_back();
            else if (cur == PAGE_COLOR_COOKING)
                jump_to_color_stop();
            else if (cur == PAGE_COLOR_STOP)
                jump_to_color_stop_back();
            else if (cur == PAGE_TOP_BBQ_COOKING)
                jump_to_top_bbq_stop();
            else if (cur == PAGE_TOP_BBQ_STOP)
                jump_to_top_bbq_stop_back();
            else if (cur == PAGE_BOTTOM_BBQ_COOKING)
                jump_to_bottom_bbq_stop();
            else if (cur == PAGE_BOTTOM_BBQ_STOP)
                jump_to_bottom_bbq_stop_back();
            else if (cur == PAGE_HOT_BBQ_COOKING)
                jump_to_hot_bbq_stop();
            else if (cur == PAGE_HOT_BBQ_STOP)
                jump_to_hot_bbq_stop_back();
            else if (cur == PAGE_HOTWIND_BBQ_COOKING)
                jump_to_hotwind_bbq_stop();
            else if (cur == PAGE_HOTWIND_BBQ_STOP)
                jump_to_hotwind_bbq_stop_back();
            else if (cur == PAGE_SAVE_BBQ_COOKING)
                jump_to_save_bbq_stop();
            else if (cur == PAGE_SAVE_BBQ_STOP)
                jump_to_save_bbq_stop_back();
            else if (cur == PAGE_CENTRAL_BBQ_COOKING)
                jump_to_central_bbq_stop();
            else if (cur == PAGE_CENTRAL_BBQ_STOP)
                jump_to_central_bbq_stop_back();
            else if (cur == PAGE_WINDCHANGE_BBQ_COOKING)
                jump_to_windchange_bbq_stop();
            else if (cur == PAGE_WINDCHANGE_BBQ_STOP)
                jump_to_windchange_bbq_stop_back();
            else if (cur == PAGE_PREHEAT_COOKING)
                jump_to_preheat_stop();
            else if (cur == PAGE_PREHEAT_STOP)
                jump_to_preheat_stop_back();
            else if (cur == PAGE_COOKIE_COOKING)
                jump_to_cookie_stop();
            else if (cur == PAGE_COOKIE_STOP)
                jump_to_cookie_stop_back();
            else if (cur == PAGE_WEST_COOKING)
                jump_to_west_stop();
            else if (cur == PAGE_WEST_STOP)
                jump_to_west_stop_back();
            else if (cur == PAGE_PIZZA_COOKING)
                jump_to_pizza_stop();
            else if (cur == PAGE_PIZZA_STOP)
                jump_to_pizza_stop_back();
            else if (cur == PAGE_MENU_COOK_COOKING)
                jump_to_menu_stop();
            else if (cur == PAGE_MENU_COOK_STOP)
                jump_to_menu_stop_back();
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
#endif
        if (!current_group) break;
        lv_obj_t *focused = lv_group_get_focused(current_group);
        edit_field_t *ef = find_edit_field(focused);
        if (ef) {
            g_send.buzzer_req = BUZZER_ENCODER;
            adjust_value(ef, +1);
            printf("[nav] adjust +: %d\n", *ef->value);
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
#endif
        if (!current_group) break;
        g_send.buzzer_req = BUZZER_ENCODER;
        lv_obj_t *focused = lv_group_get_focused(current_group);
        edit_field_t *ef = find_edit_field(focused);
        if (ef) {
            g_send.buzzer_req = BUZZER_ENCODER;
            adjust_value(ef, -1);
            printf("[nav] adjust -: %d\n", *ef->value);
        } else if (current_group == g_updown_bbq_menu) {
            updown_bbq_menu_t *bbq = updown_bbq_menu_get(&ui_manager);
            if (bbq && focused == bbq->next_button) {
                lv_group_focus_obj(bbq->tempnum_label);
                printf("[nav] focus wrap to tempnum\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[nav] focus prev\n");
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
        } else if (current_group == g_west_menu) {
            west_menu_t *menu = west_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[west] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[west] focus prev\n");
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
        } else if (current_group == g_menu_cook_menu) {
            menu_menu_t *menu = menu_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[menu] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[menu] focus prev\n");
            }
        } else {
            lv_group_focus_prev(current_group);
            printf("[nav] focus prev\n");
        }
        uart_print();
        break;
    }
    case KEY_ENCODER_PRESS: { // 51: 确认 / 跳到下一焦点
        g_send.buzzer_req = BUZZER_KEY_VALID;
        if (!current_group) break;
        lv_obj_t *focused = lv_group_get_focused(current_group);
        edit_field_t *ef = find_edit_field(focused);
        if (ef) {
            lv_group_focus_next(current_group);
            printf("[nav] press -> next focus\n");
        } else {
            lv_obj_send_event(focused, LV_EVENT_CLICKED, NULL);
            printf("[nav] press -> click\n");
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
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;

    if (g_send.iface_status != IFACE_SLEEP) {
        g_send.buzzer_req = BUZZER_POWER_OFF;
        g_send.iface_status = IFACE_SLEEP;
        lv_obj_clean(lv_scr_act());
        lv_obj_t *scr = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
        lv_scr_load(scr);
#ifndef LV_USE_AIC_SIMULATOR
        backlight_set_level(0);
#endif
        printf("[KEY] KEY1 long press -> SLEEP\n");
    } else {
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        lv_obj_clean(lv_scr_act());
        waitmenu_24_create(&ui_manager);
        current_group = NULL;
        lv_scr_load_anim(waitmenu_24_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        g_send.buzzer_req = BUZZER_POWER_ON;
        g_send.iface_status = IFACE_STANDBY;
#ifndef LV_USE_AIC_SIMULATOR
        backlight_set_level(100);
#endif
        printf("[KEY] KEY1 long press -> WAKE (waitmenu)\n");
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
        lv_obj_add_flag(set->preheat_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->preheat_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->preheat_on_button);
    } else {
        lv_obj_add_flag(set->preheat_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->preheat_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->preheat_button);
    }
}

static void on_delay_toggle(lv_event_t *e)
{
    updown_bbq_set_t *set = updown_bbq_set_get(&ui_manager);
    if (!set) return;
    delay_on = !delay_on;
    if (delay_on) {
        lv_obj_add_flag(set->delay_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->delay_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->delay_on_button);
    } else {
        lv_obj_add_flag(set->delay_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->delay_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->delay_button);
    }
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
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_cooking();
}

static void on_color_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_color_cookoing();
}

void cooking_timer_cb(lv_timer_t *timer)
{
    if (current_group == g_preheat_cooking) {
        preheatcooking_t *cook = preheatcooking_get(&ui_manager);
        if (cook) {
            uint16_t cavity = get_cavity_temp();
            int p = (cavity * 100) / (set_temp ? set_temp : 1);
            if (p > 100) p = 100;
            lv_bar_set_value(cook->bar_1, p, LV_ANIM_OFF);
            if (p >= 100 && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                jump_to_preheat_complete();
            }
        }
        return;
    }
    if (current_group == g_preheat_stop) {
        preheatstop_t *stop = preheatstop_get(&ui_manager);
        if (stop) {
            uint16_t cavity = get_cavity_temp();
            int p = (cavity * 100) / (set_temp ? set_temp : 1);
            if (p > 100) p = 100;
            lv_bar_set_value(stop->bar_2, p, LV_ANIM_OFF);
        }
        return;
    }

    lv_obj_t *time_label = NULL;

    if (current_group == g_updown_bbq_setting) {
        updown_bbq_setting_t *set = updown_bbq_setting_get(&ui_manager);
        if (set) time_label = set->time_label;
    } else if (current_group == g_top_bbq_setting) {
        top_bbq_setting_t *set = top_bbq_setting_get(&ui_manager);
        if (set) time_label = set->time_label;
    } else if (cook_is_color) {
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
        updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->time_label;
    }

    if (!time_label) return;

    uint32_t elapsed = lv_tick_get() - cook_start_time;
    if (elapsed >= (uint32_t)cook_total_ms) {
        lv_timer_del(cook_timer);
        cook_timer = NULL;
        g_send.buzzer_req = BUZZER_COOK_DONE;
        if (current_group == g_updown_bbq_setting) {
            depth--;
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
        if (cook_is_color) {
            cook_is_color = 0;
            jump_to_color_complete();
        } else {
            jump_to_updown_bbq_complete();
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

    /* 保存 cooking bar 的实际值（动画从 3→100，与 time-based 有偏移） */
    {
        updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
        cook_bar_saved = cook ? lv_bar_get_value(cook->bar) : 0;
    }

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

// stop → stop_back（确认退出）
static void jump_to_updown_bbq_stop_back(void)
{
    page_push(PAGE_UPDOWN_BBQ_STOP_BACK);
    lv_obj_clean(lv_scr_act());
    updown_bbq_stop_back_create(&ui_manager);

    updown_bbq_stop_back_t *back = updown_bbq_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure_button, back->littal_button };
        if (g_updown_bbq_stop_back) lv_group_del(g_updown_bbq_stop_back);
        g_updown_bbq_stop_back = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(back->sure_button, on_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(back->littal_button, on_stop_back_littal_click,
                            LV_EVENT_CLICKED, NULL);

        /* 同步 statu_label / bar_2 */
        set_status_label_min(back->statu_label, set_temp_up, set_temp_down, set_hour, set_min);

        lv_bar_set_range(back->bar_2, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(back->bar_2, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_updown_bbq_stop_back;

    lv_scr_load_anim(updown_bbq_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] jump: stop -> updown_bbq_stop_back\n");
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
    page_pop();
}

// stop 恢复 cooking（不经过 page_pop，直接重建）
static void stop_resume_cooking(void)
{
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
        lv_anim_set_time(&a, cook_total_ms - (int)cook_elapsed_saved);
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
static void on_stop_back_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;

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
        lv_label_set_text(cs->label_13, "| 额外上色 | 5分钟");
        lv_bar_set_range(cs->bar_3, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(cs->bar_3, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_color_stop;

    lv_scr_load_anim(color_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] jump: color_cookoing -> color_stop (pause)\n");
}

// color_stop → color_stop_back（确认退出）
static void jump_to_color_stop_back(void)
{
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

        lv_label_set_text(csb->label_17, "| 额外上色 | 5分钟");
        lv_bar_set_range(csb->bar_4, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(csb->bar_4, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_color_stop_back;

    lv_scr_load_anim(color_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] jump: color_stop -> color_stop_back\n");
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

        /* 进度条从保存的值 → 100 */
        lv_bar_set_range(cc->bar, 0, 100);
        lv_bar_set_value(cc->bar, cook_bar_saved, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cc->bar);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, cook_bar_saved, 100);
        lv_anim_set_time(&a, cook_total_ms - (int)cook_elapsed_saved);
        lv_anim_start(&a);
    }

    /* 恢复定时器 */
    cook_start_time = lv_tick_get() - cook_elapsed_saved;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_color_cookoing;

    lv_scr_load_anim(color_cookoing_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] resume: color_stop -> color_cookoing\n");
}

// color_stop_back 确定 → 退出到 major_menu
static void on_color_stop_back_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;

    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    cook_is_color = 0;
    cook_elapsed_saved = 0; cook_bar_saved = 0;

    depth = 2;
    lv_obj_clean(lv_scr_act());
    major_menu_create(&ui_manager);
    groups_create();
    bind_events();
    current_group = g_major_menu;
    lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] color_stop_back sure -> major_menu\n");
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
        set_hour = h;
        set_min = m;
        lv_label_set_text_fmt(set->time_label, "%02d:%02d:%02d", h, m, s);
        lv_label_set_text_fmt(set->hour_label, "%02d", h);
        lv_label_set_text_fmt(set->min_label, "%02d", m);
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
    g_send.iface_status = (cook_timer != NULL) ? IFACE_COOKING : IFACE_SETTING;
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

    depth--;
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
    printf("[nav] init done -> major_menu\n");
}
