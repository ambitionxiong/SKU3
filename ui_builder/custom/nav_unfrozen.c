/*
 * nav_unfrozen.c - 解冻模式 完整流程
 *
 * 页面流程：菜单(menu) → 设置(set) → 烹饪(cooking) → 暂停(stop) → 确认退出(stop_back) → 完成(complete)
 *   cooking 页可点设置按钮进入 setting 页(运行时改参数)。
 * 模式标识：MODE_UNFROZEN，通讯/图标/描述均按此分发。
 * 支持：延迟预约(ondelay)、保温(contain)、预热(preheat)入口与聚焦移动。
 */
#include "protocol.h"
#include "nav.h"

/* setting 页进入时保存原始值，BACK 返回时恢复 */
int unfrozen_setting_saved_temp, unfrozen_setting_saved_hour, unfrozen_setting_saved_min;


static void on_unfrozen_menu_next_click(lv_event_t *e);
static void on_unfrozen_set_sure_click(lv_event_t *e);
static void on_unfrozen_cooking_stop_click(lv_event_t *e);
static void on_unfrozen_cooking_setting_click(lv_event_t *e);
static void on_unfrozen_setting_sure_click(lv_event_t *e);
static void on_unfrozen_stop_start_click(lv_event_t *e);
static void on_unfrozen_stop_back_sure_click(lv_event_t *e);
static void on_unfrozen_edit_focus(lv_event_t *e);
void update_unfrozen_dir_icon(unfrozen_setting_t *set);
static void unfrozen_set_status(lv_obj_t *label, int temp, int hour, int min);
static void unfrozen_preheat_toggle(lv_event_t *e);
static void unfrozen_delay_toggle(lv_event_t *e);
static void unfrozen_contain_toggle(lv_event_t *e);
void jump_to_unfrozen_cooking(void);
void jump_to_unfrozen_complete(void);

static void unfrozen_set_status(lv_obj_t *label, int temp, int hour, int min)
{
    if (hour == 0)
        lv_label_set_text_fmt(label, tr("| 解冻 | %d℃ | %02d分钟"), temp, min);
    else
        lv_label_set_text_fmt(label, tr("| 解冻 | %d℃ | %d小时%02d分钟"), temp, hour, min);
}

static void unfrozen_preheat_toggle(lv_event_t *e)
{
    unfrozen_set_t *set = unfrozen_set_get(&ui_manager);
    if (!set) return;
    preheat_on = !preheat_on;
    if (preheat_on) {
        /* 互斥：开预热关延时 */
        delay_on = 0;
        apply_toggle_state(set->offdelay, set->ondelay, delay_on);
        mode_set_apply_delay_label(set->ondelay);
        lv_obj_add_flag(set->offpreheat, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->onpreheat, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->onpreheat);
    } else {
        lv_obj_add_flag(set->onpreheat, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->offpreheat, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->offpreheat);
    }
}

static void unfrozen_delay_toggle(lv_event_t *e)
{
    unfrozen_set_t *set = unfrozen_set_get(&ui_manager);
    if (!set) return;
    lv_obj_t *tgt = lv_event_get_target(e);
    if (tgt == set->offdelay) {
        jump_to_delayset();
        return;
    }
    /* ondelay：直接关闭 */
    delay_on = 0;
    lv_obj_add_flag(set->ondelay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(set->offdelay, LV_OBJ_FLAG_HIDDEN);
    lv_group_focus_obj(set->offdelay);
}

static void unfrozen_contain_toggle(lv_event_t *e)
{
    unfrozen_set_t *set = unfrozen_set_get(&ui_manager);
    if (!set) return;
    contain_on = !contain_on;
    if (contain_on) {
        lv_obj_add_flag(set->offcontain, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->oncontain, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->oncontain);
    } else {
        lv_obj_add_flag(set->oncontain, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->offcontain, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->offcontain);
    }
}

void update_unfrozen_dir_icon(unfrozen_setting_t *set)
{
    if (!set) return;
    lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
    if (set_temp < 100)
        lv_obj_clear_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_clear_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
}

static void on_unfrozen_edit_focus(lv_event_t *e)
{
    on_edit_focus(e);
    unfrozen_setting_t *set = unfrozen_setting_get(&ui_manager);
    if (set && lv_event_get_target(e) == set->temp)
        update_unfrozen_dir_icon(set);
}

static void on_unfrozen_menu_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_unfrozen_set();
}

static void on_unfrozen_set_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr)) {
        if (delay_on)
            jump_to_delaycooking();
        else if (preheat_on)
            jump_to_preheat_cooking();
        else
            jump_to_unfrozen_cooking();
    }
}

static void on_unfrozen_cooking_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_unfrozen_stop();
}

static void on_unfrozen_cooking_setting_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_unfrozen_setting();
}

static void on_unfrozen_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        unfrozen_resume_cooking();
}


static void on_unfrozen_stop_back_sure_click(lv_event_t *e)
{
    if (g_delay_cancel_btn) {
        delay_cancel_exit_to_set();
        return;
    }

    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    g_on_stop_back = 0;
    g_keepwarm_active = 0;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    set_temp = 40; set_temp_up = 40; set_temp_down = 40; set_hour = 0; set_min = 30;
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
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    printf("[unfrozen] stop_back sure -> major_menu\n");
}

// ==============================
// Jump 函数
// ==============================

void jump_to_unfrozen_menu(void)
{
    set_temp = 40; set_hour = 0; set_min = 30;
    page_push(PAGE_UNFROZEN_MENU);
    lv_obj_clean(lv_scr_act());
    unfrozen_menu_create(&ui_manager);

    unfrozen_menu_t *menu = unfrozen_menu_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp, menu->hour, menu->min,
            menu->next,
        };
        if (g_unfrozen_menu) lv_group_del(g_unfrozen_menu);
        g_unfrozen_menu = group_create_for_page(btns, 4);

        edit_clear();
        edit_register(menu->temp, menu->templine2, menu->templine3,
                      &set_temp, 30, 60, 5, "%d");
        edit_register(menu->hour, menu->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(menu->min, menu->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(menu->temp, on_unfrozen_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->hour, on_unfrozen_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->min, on_unfrozen_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_unfrozen_edit_focus,
                            LV_EVENT_FOCUSED, NULL);

        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_unfrozen_menu_next_click,
                                LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp);
        lv_label_set_text_fmt(menu->hour, "%02d", set_hour);
        lv_label_set_text_fmt(menu->min, "%02d", set_min);

        lv_obj_add_flag(menu->templine3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->templine2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(menu->templine2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(menu->templine3, LV_OBJ_FLAG_HIDDEN);

        validate_constraints();

        if (menu->next)
            lv_group_focus_obj(menu->next);
    }
    current_group = g_unfrozen_menu;

    lv_scr_load_anim(unfrozen_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.cook_mode = MODE_UNFROZEN;
    printf("[unfrozen] jump: special_menu -> unfrozen_menu\n");
}

// menu → set
void jump_to_unfrozen_set(void)
{
    page_push(PAGE_UNFROZEN_SET);
    lv_obj_clean(lv_scr_act());
    unfrozen_set_create(&ui_manager);

    preheat_on = 0; delay_on = 0; contain_on = 0;

    unfrozen_set_t *set = unfrozen_set_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->sure,
            set->offpreheat, set->onpreheat,
            set->offdelay, set->ondelay,
            set->offcontain, set->oncontain,
        };
        if (g_unfrozen_set) lv_group_del(g_unfrozen_set);
        g_unfrozen_set = group_create_for_page(btns, 7);
        clear_focus_states(btns, 7);
        lv_group_focus_obj(set->sure);

        lv_label_set_text_fmt(set->temp, "%d", set_temp);
        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_fmt(set->hour, "%02d", set_hour);
        lv_label_set_text_fmt(set->min, "%02d", set_min);
    if (set_hour == 0) {
        lv_obj_add_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->label_452, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_453, 365, 269);
    } else {
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->label_452, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_453, 448, 269);
    }

        apply_toggle_state(set->offpreheat, set->onpreheat, preheat_on);
    if (set_hour == 0) {
        lv_obj_add_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->label_452, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_453, 365, 269);
    } else {
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->label_452, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_453, 448, 269);
    }
        apply_toggle_state(set->offdelay, set->ondelay, delay_on);
        mode_set_apply_delay_label(set->ondelay);
    if (set_hour == 0) {
        lv_obj_add_flag(set->label_452, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_453, 365, 269);
    } else {
        lv_obj_clear_flag(set->label_452, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_453, 448, 269);
    }
        apply_toggle_state(set->offcontain, set->oncontain, contain_on);

        lv_obj_add_event_cb(set->offpreheat, unfrozen_preheat_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->onpreheat, unfrozen_preheat_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offdelay, unfrozen_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->ondelay, unfrozen_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offcontain, unfrozen_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->oncontain, unfrozen_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->sure, on_unfrozen_set_sure_click,
                            LV_EVENT_CLICKED, NULL);
    }
    current_group = g_unfrozen_set;

    lv_scr_load_anim(unfrozen_set_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[unfrozen] jump: menu -> unfrozen_set\n");
}

// set → cooking
void jump_to_unfrozen_cooking(void)
{
    edit_clear();
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }

    g_keepwarm_active = 0;

    page_push(PAGE_UNFROZEN_COOKING);
    lv_obj_clean(lv_scr_act());
    unfrozen_cooking_create(&ui_manager);

    unfrozen_cooking_t *cook = unfrozen_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_unfrozen_cooking) lv_group_del(g_unfrozen_cooking);
        g_unfrozen_cooking = group_create_for_page(btns, 2);

        lv_obj_add_event_cb(cook->stop, on_unfrozen_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_unfrozen_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        unfrozen_set_status(cook->status, set_temp, set_hour, set_min);
        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", set_hour, set_min, 0);
    }

    cook_total_ms = (set_hour * 3600 + set_min * 60) * 1000;
    if (cook) {
        lv_bar_set_range(cook->bar_32, 0, 100);
        lv_bar_set_value(cook->bar_32, 3, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_32);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    set_temp_up = set_temp; set_temp_down = set_temp;
    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_unfrozen_cooking;

    lv_scr_load_anim(unfrozen_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = cook_total_ms;
    printf("[unfrozen] jump: set -> unfrozen_cooking\n");
}

// cooking → setting（不暂停 timer）
void jump_to_unfrozen_setting(void)
{
    unfrozen_setting_saved_temp = set_temp; unfrozen_setting_saved_hour = set_hour; unfrozen_setting_saved_min = set_min;
    page_push(PAGE_UNFROZEN_SETTING);
    lv_obj_clean(lv_scr_act());
    unfrozen_setting_create(&ui_manager);

    unfrozen_setting_t *set = unfrozen_setting_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = { set->temp, set->hour, set->min, set->sure };
        if (g_unfrozen_setting) lv_group_del(g_unfrozen_setting);
        g_unfrozen_setting = group_create_for_page(btns, 4);

        edit_clear();
        edit_register(set->temp, set->templine2, set->templine3,
                      &set_temp, 30, 60, 5, "%d");
        edit_register(set->hour, set->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(set->min, set->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(set->temp, on_unfrozen_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->hour, on_unfrozen_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->min, on_unfrozen_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_unfrozen_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_unfrozen_setting_sure_click,
                            LV_EVENT_CLICKED, NULL);

        /* 从当前 timer 读剩余时间（支持 running 或 paused） */
        uint32_t elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
        int elapsed_sec = (elapsed + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        if (g_send.iface_status == IFACE_COMPLETE) { h = m = s = 0; }
        lv_label_set_text_fmt(set->timelabel, "%02d:%02d:%02d", h, m, s);
        lv_label_set_text_fmt(set->hour, "%02d", set_hour);
        lv_label_set_text_fmt(set->min, "%02d", set_min);
        lv_label_set_text_fmt(set->temp, "%d", set_temp);
        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_flag(set->templine3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->templine2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hourline, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->minline, LV_OBJ_FLAG_HIDDEN);

        lv_group_focus_obj(set->temp);
        update_unfrozen_dir_icon(set);
    }
    current_group = g_unfrozen_setting;

    lv_scr_load_anim(unfrozen_setting_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        if (g_send.iface_status != IFACE_COMPLETE)
        g_send.iface_status = (cook_timer != NULL) ? IFACE_COOKING : IFACE_SETTING;
    printf("[unfrozen] jump: cooking -> unfrozen_setting\n");
}

// cooking → stop（暂停）
void jump_to_unfrozen_stop(void)
{
    edit_clear();
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

    cook_bar_saved = 3 + (int)((int64_t)cook_elapsed_saved * 97 / (cook_total_ms ? cook_total_ms : 1));
    if (cook_bar_saved > 100) cook_bar_saved = 100;

    page_push(PAGE_UNFROZEN_STOP);
    lv_obj_clean(lv_scr_act());
    unfrozen_stop_create(&ui_manager);

    unfrozen_stop_t *stop = unfrozen_stop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start, stop->little };
        if (g_unfrozen_stop) lv_group_del(g_unfrozen_stop);
        g_unfrozen_stop = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(stop->start, on_unfrozen_stop_start_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(stop->little, on_unfrozen_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(stop->timelabel, "%02d:%02d:%02d", h, m, s);
        unfrozen_set_status(stop->status, set_temp, set_hour, set_min);

        lv_bar_set_range(stop->bar_33, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_33, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_unfrozen_stop;

    lv_scr_load_anim(unfrozen_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = (cook_total_ms > (int)cook_elapsed_saved) ? cook_total_ms - (int)cook_elapsed_saved : 0;
    printf("[unfrozen] jump: cooking -> stop (pause)\n");
}

// stop → stop_back
void jump_to_unfrozen_stop_back(void)
{
    edit_clear();
    
    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_unfrozen_complete;
    page_push(PAGE_UNFROZEN_STOP_BACK);
    lv_obj_clean(lv_scr_act());
    unfrozen_stop_back_create(&ui_manager);

    unfrozen_stop_back_t *back = unfrozen_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_unfrozen_stop_back) lv_group_del(g_unfrozen_stop_back);
        g_unfrozen_stop_back = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(back->sure, on_unfrozen_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        unfrozen_set_status(back->status, set_temp, set_hour, set_min);
        int cooking_bar_val = 0;
        if (cook_timer && depth > 0 && page_stack[depth - 1] == PAGE_UNFROZEN_COOKING) {
        unfrozen_cooking_t *cook = unfrozen_cooking_get(&ui_manager);
        if (cook && cook->bar_32) cooking_bar_val = lv_bar_get_value(cook->bar_32);
    }
        int p = cooking_bar_val;
        if (p <= 0) {
            uint32_t elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
            p = stop_back_progress(elapsed, cook_total_ms);
        }
        if (p > 100) p = 100;
        lv_bar_set_range(back->bar_34, 0, 100);
                lv_bar_set_value(back->bar_34, p, LV_ANIM_OFF);
        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            if (g_keepwarm_active)
                lv_label_set_text(back->label_480, tr("保温中..."));
            else
                lv_label_set_text(back->label_480, tr("已完成"));
            lv_bar_set_value(back->bar_34, 100, LV_ANIM_OFF);
        }
        if (g_delay_cancel_to_stop_back) {
            g_delay_cancel_to_stop_back = 0;
            lv_label_set_text(back->label_480, tr("预约中..."));
            lv_label_set_text(back->label_482, g_delay_cancel_btn ? "回到上一页" : "回到主页");
            lv_obj_add_flag(back->bar_34, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(back->image_215, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(back->little, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(back->label_480, tr("烹饪中..."));
    }
    current_group = g_unfrozen_stop_back;

    lv_scr_load_anim(unfrozen_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[unfrozen] jump: stop/cooking -> stop_back\n");
}

// stop 恢复 cooking
void unfrozen_resume_cooking(void)
{
    edit_clear();
    g_on_stop_back = 0;
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    if (depth > 1) depth--;
    lv_obj_clean(lv_scr_act());
    unfrozen_cooking_create(&ui_manager);

    unfrozen_cooking_t *cook = unfrozen_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_unfrozen_cooking) lv_group_del(g_unfrozen_cooking);
        g_unfrozen_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop, on_unfrozen_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_unfrozen_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        unfrozen_set_status(cook->status, set_temp, set_hour, set_min);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", h, m, s);

        lv_bar_set_range(cook->bar_32, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(cook->bar_32, cook_bar_saved, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_32);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, cook_bar_saved, 100);
        lv_anim_set_time(&a, ((int)(cook_total_ms - (int)cook_elapsed_saved) < 0) ? 0 : (cook_total_ms - (int)cook_elapsed_saved));
        lv_anim_start(&a);
    }

    cook_start_time = lv_tick_get() - cook_elapsed_saved;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_unfrozen_cooking;

    lv_scr_load_anim(unfrozen_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        {
        int rem = cook_total_ms - (int)cook_elapsed_saved;
        if (rem < 0) rem = 0;
        g_send.iface_status = IFACE_COOKING;
        g_send.remaining_ms = rem;
    }
    printf("[unfrozen] resume: stop -> cooking\n");
}

// setting 确定 → 回到 cooking
static void on_unfrozen_setting_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    edit_clear();   /* 清除编辑注册表残留,防止悬空 label 指针(UAF) */

    cook_total_ms = (set_hour * 3600 + set_min * 60) * 1000;

    if (depth > 1) depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_UNFROZEN_STOP)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_UNFROZEN_COMPLETE)
        depth--;
    lv_obj_clean(lv_scr_act());
    unfrozen_cooking_create(&ui_manager);

    unfrozen_cooking_t *cook = unfrozen_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_unfrozen_cooking) lv_group_del(g_unfrozen_cooking);
        g_unfrozen_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop, on_unfrozen_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_unfrozen_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        unfrozen_set_status(cook->status, set_temp, set_hour, set_min);
        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", set_hour, set_min, 0);

        lv_bar_set_range(cook->bar_32, 0, 100);
        lv_bar_set_value(cook->bar_32, 3, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_32);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    set_temp_up = set_temp; set_temp_down = set_temp;
    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_unfrozen_cooking;

    lv_scr_load_anim(unfrozen_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.remaining_ms = cook_total_ms;
    printf("[unfrozen] setting sure -> cooking\n");
}

// cooking → complete
void jump_to_unfrozen_complete(void)
{
    edit_clear();
    if (depth > 0 && page_stack[depth - 1] == PAGE_UNFROZEN_STOP_BACK)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_UNFROZEN_STOP)
        depth--;
    page_push(PAGE_UNFROZEN_COMPLETE);
    lv_obj_clean(lv_scr_act());
    unfrozen_complete_create(&ui_manager);

    {
        unfrozen_complete_t *cook = unfrozen_complete_get(&ui_manager);
        if (cook) {
            lv_obj_t *btns[] = { cook->little };
            if (g_unfrozen_complete) lv_group_del(g_unfrozen_complete);
            g_unfrozen_complete = group_create_for_page(btns, 1);
            lv_obj_add_event_cb(cook->little, on_unfrozen_cooking_setting_click,
                                LV_EVENT_CLICKED, NULL);
            if (set_hour == 0)
                lv_label_set_text_fmt(cook->status, tr("| 解冻 | %d℃ | %02d分钟"), set_temp, set_min);
            else
                lv_label_set_text_fmt(cook->status, tr("| 解冻 | %d℃ | %d小时%02d分钟"), set_temp, set_hour, set_min);
            lv_bar_set_value(cook->bar_35, 100, LV_ANIM_OFF);
        }
    }
    current_group = g_unfrozen_complete;

    lv_scr_load_anim(unfrozen_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.iface_status = IFACE_COMPLETE;
    g_send.remaining_ms = 0;
    printf("[unfrozen] jump: cooking -> complete\n");
    /* 自动保温：保温开关开启时 complete 页停留 1 分钟无操作 → 保温（15 分钟） */
    if (contain_on) {
        g_keepwarm_active = 0;
        g_keepwarm_sec = 0;
        if (cook_timer) lv_timer_del(cook_timer);
        cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
    }

}

// ==============================
// Rebuild 函数（供 page_pop 调用）
// ==============================

void unfrozen_rebuild_menu(page_id_t child)
{
    unfrozen_menu_create(&ui_manager);
    unfrozen_menu_t *menu = unfrozen_menu_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp, menu->hour, menu->min,
            menu->next,
        };
        if (g_unfrozen_menu) lv_group_del(g_unfrozen_menu);
        g_unfrozen_menu = group_create_for_page(btns, 4);

        edit_clear();
        edit_register(menu->temp, menu->templine2, menu->templine3,
                      &set_temp, 30, 60, 5, "%d");
        edit_register(menu->hour, menu->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(menu->min, menu->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(menu->temp, on_unfrozen_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->hour, on_unfrozen_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->min, on_unfrozen_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_unfrozen_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_unfrozen_menu_next_click,
                                LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp);
        lv_label_set_text_fmt(menu->hour, "%02d", set_hour);
        lv_label_set_text_fmt(menu->min, "%02d", set_min);

        lv_obj_add_flag(menu->templine3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->templine2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(menu->templine2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(menu->templine3, LV_OBJ_FLAG_HIDDEN);

        validate_constraints();
        if (menu->next) lv_group_focus_obj(menu->next);
    }
    current_group = g_unfrozen_menu;
    lv_scr_load_anim(unfrozen_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[unfrozen] back to unfrozen_menu\n");
}

void unfrozen_rebuild_set(page_id_t child)
{
    unfrozen_set_create(&ui_manager);
    unfrozen_set_t *set = unfrozen_set_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->sure,
            set->offpreheat, set->onpreheat,
            set->offdelay, set->ondelay,
            set->offcontain, set->oncontain,
        };
        if (g_unfrozen_set) lv_group_del(g_unfrozen_set);
        g_unfrozen_set = group_create_for_page(btns, 7);
        clear_focus_states(btns, 7);
        lv_group_focus_obj(set->sure);

        lv_label_set_text_fmt(set->temp, "%d", set_temp);
        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_fmt(set->hour, "%02d", set_hour);
        lv_label_set_text_fmt(set->min, "%02d", set_min);
    if (set_hour == 0) {
        lv_obj_add_flag(set->label_452, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_453, 365, 269);
    } else {
        lv_obj_clear_flag(set->label_452, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_453, 448, 269);
    }

        apply_toggle_state(set->offpreheat, set->onpreheat, preheat_on);
    if (set_hour == 0) {
        lv_obj_add_flag(set->label_452, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_453, 365, 269);
    } else {
        lv_obj_clear_flag(set->label_452, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_453, 448, 269);
    }
        apply_toggle_state(set->offdelay, set->ondelay, delay_on);
        mode_set_apply_delay_label(set->ondelay);
    if (set_hour == 0) {
        lv_obj_add_flag(set->label_452, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_453, 365, 269);
    } else {
        lv_obj_clear_flag(set->label_452, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_453, 448, 269);
    }
        apply_toggle_state(set->offcontain, set->oncontain, contain_on);

        lv_obj_add_event_cb(set->offpreheat, unfrozen_preheat_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->onpreheat, unfrozen_preheat_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offdelay, unfrozen_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->ondelay, unfrozen_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offcontain, unfrozen_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->oncontain, unfrozen_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->sure, on_unfrozen_set_sure_click,
                            LV_EVENT_CLICKED, NULL);

        if (child == PAGE_UNFROZEN_COOKING && set->sure)
            lv_group_focus_obj(set->sure);
    }
        if (child == PAGE_DELAYSET && set->offdelay)
            lv_group_focus_obj(delay_on ? set->ondelay : set->offdelay);

    current_group = g_unfrozen_set;
    lv_scr_load_anim(unfrozen_set_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[unfrozen] back to unfrozen_set\n");
}

void unfrozen_rebuild_cooking(page_id_t child)
{
    edit_clear();
    unfrozen_cooking_create(&ui_manager);
    unfrozen_cooking_t *cook = unfrozen_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_unfrozen_cooking) lv_group_del(g_unfrozen_cooking);
        g_unfrozen_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop, on_unfrozen_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_unfrozen_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        unfrozen_set_status(cook->status, set_temp, set_hour, set_min);

        if (child == PAGE_UNFROZEN_SETTING) {
            uint32_t elapsed = lv_tick_get() - cook_start_time;
            int elapsed_sec = (elapsed + 500) / 1000;
            int total_sec = cook_total_ms / 1000;
            int remaining_sec = total_sec - elapsed_sec;
            if (remaining_sec < 0) remaining_sec = 0;
            int h = remaining_sec / 3600;
            int m = (remaining_sec % 3600) / 60;
            int s = remaining_sec % 60;
            lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", h, m, s);
            lv_bar_set_range(cook->bar_32, 0, 100);
             int progress = stop_back_progress(elapsed, cook_total_ms);
            if (progress > 100) progress = 100;
            lv_bar_set_value(cook->bar_32, progress, LV_ANIM_OFF);
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, cook->bar_32);
            lv_anim_set_exec_cb(&a, anim_bar_set_value);
            lv_anim_set_values(&a, progress, 100);
            lv_anim_set_time(&a, ((int)(cook_total_ms - (int)elapsed) < 0) ? 0 : (cook_total_ms - (int)elapsed));
            lv_anim_start(&a);
        } else {
            lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", set_hour, set_min, 0);
            lv_bar_set_range(cook->bar_32, 0, 100);
            lv_bar_set_value(cook->bar_32, 3, LV_ANIM_OFF);
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, cook->bar_32);
            lv_anim_set_exec_cb(&a, anim_bar_set_value);
            lv_anim_set_values(&a, 3, 100);
            lv_anim_set_time(&a, cook_total_ms);
            lv_anim_start(&a);
            cook_start_time = lv_tick_get();
            if (cook_timer) lv_timer_del(cook_timer);
            cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
        }
    }
    current_group = g_unfrozen_cooking;
    lv_scr_load_anim(unfrozen_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[unfrozen] back to unfrozen_cooking\n");
}

void unfrozen_rebuild_setting(void)
{
    unfrozen_setting_create(&ui_manager);
    unfrozen_setting_t *set = unfrozen_setting_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = { set->temp, set->hour, set->min, set->sure };
        if (g_unfrozen_setting) lv_group_del(g_unfrozen_setting);
        g_unfrozen_setting = group_create_for_page(btns, 4);

        edit_clear();
        edit_register(set->temp, set->templine2, set->templine3,
                      &set_temp, 30, 60, 5, "%d");
        edit_register(set->hour, set->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(set->min, set->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(set->temp, on_unfrozen_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->hour, on_unfrozen_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->min, on_unfrozen_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_unfrozen_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_unfrozen_setting_sure_click,
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
        lv_label_set_text_fmt(set->timelabel, "%02d:%02d:%02d", h, m, s);
        lv_label_set_text_fmt(set->hour, "%02d", set_hour);
        lv_label_set_text_fmt(set->min, "%02d", set_min);
        lv_label_set_text_fmt(set->temp, "%d", set_temp);
        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_flag(set->templine3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->templine2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hourline, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->minline, LV_OBJ_FLAG_HIDDEN);

        lv_group_focus_obj(set->temp);
        update_unfrozen_dir_icon(set);
    }
    current_group = g_unfrozen_setting;
    lv_scr_load_anim(unfrozen_setting_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[unfrozen] back to unfrozen_setting\n");
}

void unfrozen_rebuild_stop(void)
{
    edit_clear();
    g_on_stop_back = 0;
    unfrozen_stop_create(&ui_manager);
    unfrozen_stop_t *stop = unfrozen_stop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start, stop->little };
        if (g_unfrozen_stop) lv_group_del(g_unfrozen_stop);
        g_unfrozen_stop = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(stop->start, on_unfrozen_stop_start_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(stop->little, on_unfrozen_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(stop->timelabel, "%02d:%02d:%02d", h, m, s);
        unfrozen_set_status(stop->status, set_temp, set_hour, set_min);

        lv_bar_set_range(stop->bar_33, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_33, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_unfrozen_stop;
    lv_scr_load_anim(unfrozen_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[unfrozen] back to unfrozen_stop\n");
    g_send.iface_status = IFACE_PAUSE;
}

void unfrozen_rebuild_stop_back(void)
{
    edit_clear();
    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_unfrozen_complete;
    unfrozen_stop_back_create(&ui_manager);
    unfrozen_stop_back_t *back = unfrozen_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_unfrozen_stop_back) lv_group_del(g_unfrozen_stop_back);
        g_unfrozen_stop_back = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(back->sure, on_unfrozen_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        unfrozen_set_status(back->status, set_temp, set_hour, set_min);
        int cooking_bar_val = 0;
        if (cook_timer && depth > 0 && page_stack[depth - 1] == PAGE_UNFROZEN_COOKING) {
        unfrozen_cooking_t *cook = unfrozen_cooking_get(&ui_manager);
        if (cook && cook->bar_32) cooking_bar_val = lv_bar_get_value(cook->bar_32);
    }
        int p = cooking_bar_val;
        if (p <= 0) {
            uint32_t elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
            p = stop_back_progress(elapsed, cook_total_ms);
        }
        if (p > 100) p = 100;
        lv_bar_set_range(back->bar_34, 0, 100);
                lv_bar_set_value(back->bar_34, p, LV_ANIM_OFF);
        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            if (g_keepwarm_active)
                lv_label_set_text(back->label_480, tr("保温中..."));
            else
                lv_label_set_text(back->label_480, tr("已完成"));
            lv_bar_set_value(back->bar_34, 100, LV_ANIM_OFF);
        }
    }
    current_group = g_unfrozen_stop_back;
    lv_scr_load_anim(unfrozen_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[unfrozen] back to unfrozen_stop_back\n");
}

void unfrozen_rebuild_complete(void)
{
    edit_clear();
    unfrozen_complete_create(&ui_manager);
    current_group = g_unfrozen_complete;
    lv_scr_load_anim(unfrozen_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[unfrozen] back to unfrozen_complete\n");
}
void unfrozen_complete_rebind(lv_obj_t *btn)
{
    lv_obj_add_event_cb(btn, on_unfrozen_cooking_setting_click, LV_EVENT_CLICKED, NULL);
}
