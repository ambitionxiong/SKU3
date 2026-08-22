/*
 * nav_heatcontain.c - 保温模式 完整流程
 *
 * 页面流程：菜单(menu) → 设置(set) → 烹饪(cooking) → 暂停(stop) → 确认退出(stop_back) → 完成(complete)
 *   cooking 页可点设置按钮进入 setting 页(运行时改参数)。
 * 模式标识：MODE_HEATCONTAIN，通讯/图标/描述均按此分发。
 * 支持：延迟预约(ondelay)、保温(contain)、预热(preheat)入口与聚焦移动。
 */
#include "protocol.h"
#include "nav.h"

/* setting 页进入时保存原始值，BACK 返回时恢复 */
int heatcontain_setting_saved_temp, heatcontain_setting_saved_hour, heatcontain_setting_saved_min;


static void on_heatcontain_menu_next_click(lv_event_t *e);
static void on_heatcontain_set_sure_click(lv_event_t *e);
static void on_heatcontain_cooking_stop_click(lv_event_t *e);
static void on_heatcontain_cooking_setting_click(lv_event_t *e);
static void on_heatcontain_setting_sure_click(lv_event_t *e);
static void on_heatcontain_stop_start_click(lv_event_t *e);
static void on_heatcontain_stop_back_sure_click(lv_event_t *e);
static void on_heatcontain_edit_focus(lv_event_t *e);
void update_heatcontain_dir_icon(heatcontain_setting_t *set);
static void heatcontain_set_status(lv_obj_t *label, int temp, int hour, int min);
static void heatcontain_preheat_toggle(lv_event_t *e);
static void heatcontain_delay_toggle(lv_event_t *e);
static void heatcontain_contain_toggle(lv_event_t *e);
void jump_to_heatcontain_cooking(void);
void jump_to_heatcontain_complete(void);

static void heatcontain_set_status(lv_obj_t *label, int temp, int hour, int min)
{
    if (hour == 0)
        lv_label_set_text_fmt(label, tr("| 保温 | %d℃ | %02d分钟"), temp, min);
    else
        lv_label_set_text_fmt(label, tr("| 保温 | %d℃ | %d小时%02d分钟"), temp, hour, min);
}

static void heatcontain_preheat_toggle(lv_event_t *e)
{
    heatcontain_set_t *set = heatcontain_set_get(&ui_manager);
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

static void heatcontain_delay_toggle(lv_event_t *e)
{
    heatcontain_set_t *set = heatcontain_set_get(&ui_manager);
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

static void heatcontain_contain_toggle(lv_event_t *e)
{
    heatcontain_set_t *set = heatcontain_set_get(&ui_manager);
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

void update_heatcontain_dir_icon(heatcontain_setting_t *set)
{
    if (!set) return;
    lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
    if (set_temp < 100)
        lv_obj_clear_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_clear_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
}

static void on_heatcontain_edit_focus(lv_event_t *e)
{
    on_edit_focus(e);
    heatcontain_setting_t *set = heatcontain_setting_get(&ui_manager);
    if (set && lv_event_get_target(e) == set->temp)
        update_heatcontain_dir_icon(set);
}

static void on_heatcontain_menu_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_heatcontain_set();
}

static void on_heatcontain_set_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr)) {
        if (delay_on)
            jump_to_delaycooking();
        else if (preheat_on)
            jump_to_preheat_cooking();
        else
            jump_to_heatcontain_cooking();
    }
}

static void on_heatcontain_cooking_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_heatcontain_stop();
}

static void on_heatcontain_cooking_setting_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_heatcontain_setting();
}

static void on_heatcontain_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        heatcontain_resume_cooking();
}


static void on_heatcontain_stop_back_sure_click(lv_event_t *e)
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
    set_temp = 70; set_temp_up = 70; set_temp_down = 70; set_hour = 0; set_min = 30;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    depth = 2;
    lv_obj_clean(lv_scr_act());
    major_menu_create(&ui_manager);
    groups_create();
    bind_events();
    current_group = g_major_menu;
    lang_scr_load_anim(major_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    printf("[heatcontain] stop_back sure -> major_menu\n");
}

// ==============================
// Jump 函数
// ==============================

void jump_to_heatcontain_menu(void)
{
    set_temp = 70; set_hour = 0; set_min = 30;
    page_push(PAGE_HEATCONTAIN_MENU);
    lv_obj_clean(lv_scr_act());
    heatcontain_menu_create(&ui_manager);

    heatcontain_menu_t *menu = heatcontain_menu_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp, menu->hour, menu->min,
            menu->next,
        };
        if (g_heatcontain_menu) lv_group_del(g_heatcontain_menu);
        g_heatcontain_menu = group_create_for_page(btns, 4);

        edit_clear();
        edit_register(menu->temp, menu->templine2, menu->templine3,
                      &set_temp, 50, 100, 5, "%d");
        edit_register(menu->hour, menu->hourline, NULL,
                      &set_hour, 0, 7, 1, "%02d");
        edit_register(menu->min, menu->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(menu->temp, on_heatcontain_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->hour, on_heatcontain_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->min, on_heatcontain_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_heatcontain_edit_focus,
                            LV_EVENT_FOCUSED, NULL);

        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_heatcontain_menu_next_click,
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
    current_group = g_heatcontain_menu;

    lang_scr_load_anim(heatcontain_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.cook_mode = MODE_HEATCONTAIN;
    printf("[heatcontain] jump: special_menu -> heatcontain_menu\n");
}

// menu → set
void jump_to_heatcontain_set(void)
{
    page_push(PAGE_HEATCONTAIN_SET);
    lv_obj_clean(lv_scr_act());
    heatcontain_set_create(&ui_manager);

    preheat_on = 0; delay_on = 0; contain_on = 0;

    heatcontain_set_t *set = heatcontain_set_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->sure,
            set->offpreheat, set->onpreheat,
            set->offdelay, set->ondelay,
            set->offcontain, set->oncontain,
        };
        if (g_heatcontain_set) lv_group_del(g_heatcontain_set);
        g_heatcontain_set = group_create_for_page(btns, 7);
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
        lv_obj_add_flag(set->label_599, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_600, 365, 269);
    } else {
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->label_599, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_600, 448, 269);
    }

        apply_toggle_state(set->offpreheat, set->onpreheat, preheat_on);
    if (set_hour == 0) {
        lv_obj_add_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->label_599, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_600, 365, 269);
    } else {
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->label_599, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_600, 448, 269);
    }
        apply_toggle_state(set->offdelay, set->ondelay, delay_on);
        mode_set_apply_delay_label(set->ondelay);
    if (set_hour == 0) {
        lv_obj_add_flag(set->label_599, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_600, 365, 269);
    } else {
        lv_obj_clear_flag(set->label_599, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_600, 448, 269);
    }
        apply_toggle_state(set->offcontain, set->oncontain, contain_on);

        lv_obj_add_event_cb(set->offpreheat, heatcontain_preheat_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->onpreheat, heatcontain_preheat_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offdelay, heatcontain_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->ondelay, heatcontain_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offcontain, heatcontain_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->oncontain, heatcontain_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->sure, on_heatcontain_set_sure_click,
                            LV_EVENT_CLICKED, NULL);
    }
    current_group = g_heatcontain_set;

    lang_scr_load_anim(heatcontain_set_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[heatcontain] jump: menu -> heatcontain_set\n");
}

// set → cooking
void jump_to_heatcontain_cooking(void)
{
    edit_clear();
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }

    g_keepwarm_active = 0;

    page_push(PAGE_HEATCONTAIN_COOKING);
    lv_obj_clean(lv_scr_act());
    heatcontain_cooking_create(&ui_manager);

    heatcontain_cooking_t *cook = heatcontain_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_heatcontain_cooking) lv_group_del(g_heatcontain_cooking);
        g_heatcontain_cooking = group_create_for_page(btns, 2);

        lv_obj_add_event_cb(cook->stop, on_heatcontain_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_heatcontain_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        heatcontain_set_status(cook->status, set_temp, set_hour, set_min);
        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", set_hour, set_min, 0);
    }

    cook_total_ms = (set_hour * 3600 + set_min * 60) * 1000;
    if (cook) {
        lv_bar_set_range(cook->bar_44, 0, 100);
        lv_bar_set_value(cook->bar_44, 3, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_44);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    set_temp_up = set_temp; set_temp_down = set_temp;
    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_heatcontain_cooking;

    lang_scr_load_anim(heatcontain_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = cook_total_ms;
    printf("[heatcontain] jump: set -> heatcontain_cooking\n");
}

// cooking → setting（不暂停 timer）
void jump_to_heatcontain_setting(void)
{
    heatcontain_setting_saved_temp = set_temp; heatcontain_setting_saved_hour = set_hour; heatcontain_setting_saved_min = set_min;
    page_push(PAGE_HEATCONTAIN_SETTING);
    lv_obj_clean(lv_scr_act());
    heatcontain_setting_create(&ui_manager);

    heatcontain_setting_t *set = heatcontain_setting_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = { set->temp, set->hour, set->min, set->sure };
        if (g_heatcontain_setting) lv_group_del(g_heatcontain_setting);
        g_heatcontain_setting = group_create_for_page(btns, 4);

        edit_clear();
        edit_register(set->temp, set->templine2, set->templine3,
                      &set_temp, 50, 100, 5, "%d");
        edit_register(set->hour, set->hourline, NULL,
                      &set_hour, 0, 7, 1, "%02d");
        edit_register(set->min, set->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(set->temp, on_heatcontain_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->hour, on_heatcontain_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->min, on_heatcontain_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_heatcontain_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_heatcontain_setting_sure_click,
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
        update_heatcontain_dir_icon(set);
    }
    current_group = g_heatcontain_setting;

    lang_scr_load_anim(heatcontain_setting_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        if (g_send.iface_status != IFACE_COMPLETE)
        g_send.iface_status = (cook_timer != NULL) ? IFACE_COOKING : IFACE_SETTING;
    printf("[heatcontain] jump: cooking -> heatcontain_setting\n");
}

// cooking → stop（暂停）
void jump_to_heatcontain_stop(void)
{
    edit_clear();
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

    cook_bar_saved = 3 + (int)((int64_t)cook_elapsed_saved * 97 / (cook_total_ms ? cook_total_ms : 1));
    if (cook_bar_saved > 100) cook_bar_saved = 100;

    page_push(PAGE_HEATCONTAIN_STOP);
    lv_obj_clean(lv_scr_act());
    heatcontain_stop_create(&ui_manager);

    heatcontain_stop_t *stop = heatcontain_stop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start, stop->little };
        if (g_heatcontain_stop) lv_group_del(g_heatcontain_stop);
        g_heatcontain_stop = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(stop->start, on_heatcontain_stop_start_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(stop->little, on_heatcontain_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(stop->timelabel, "%02d:%02d:%02d", h, m, s);
        heatcontain_set_status(stop->status, set_temp, set_hour, set_min);

        lv_bar_set_range(stop->bar_45, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_45, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_heatcontain_stop;

    lang_scr_load_anim(heatcontain_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = (cook_total_ms > (int)cook_elapsed_saved) ? cook_total_ms - (int)cook_elapsed_saved : 0;
    printf("[heatcontain] jump: cooking -> stop (pause)\n");
}

// stop → stop_back
void jump_to_heatcontain_stop_back(void)
{
    edit_clear();
    int cooking_bar_val = 0;
    if (cook_timer && depth > 0 && page_stack[depth - 1] == PAGE_HEATCONTAIN_COOKING) {
        heatcontain_cooking_t *cook = heatcontain_cooking_get(&ui_manager);
        if (cook && cook->bar_44) cooking_bar_val = lv_bar_get_value(cook->bar_44);
    }

    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_heatcontain_complete;
    page_push(PAGE_HEATCONTAIN_STOP_BACK);
    lv_obj_clean(lv_scr_act());
    heatcontain_stop_back_create(&ui_manager);

    heatcontain_stop_back_t *back = heatcontain_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_heatcontain_stop_back) lv_group_del(g_heatcontain_stop_back);
        g_heatcontain_stop_back = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(back->sure, on_heatcontain_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        heatcontain_set_status(back->status, set_temp, set_hour, set_min);
        int p = cooking_bar_val;
        if (p <= 0) {
            uint32_t elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
            p = stop_back_progress(elapsed, cook_total_ms);
        }
        if (p > 100) p = 100;
        lv_bar_set_range(back->bar_46, 0, 100);
                lv_bar_set_value(back->bar_46, p, LV_ANIM_OFF);
        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            if (g_keepwarm_active)
                lv_label_set_text(back->label_627, tr("保温中..."));
            else
                lv_label_set_text(back->label_627, tr("已完成"));
            lv_bar_set_value(back->bar_46, 100, LV_ANIM_OFF);
        }
        if (g_delay_cancel_to_stop_back) {
            g_delay_cancel_to_stop_back = 0;
            lv_label_set_text(back->label_627, tr("预约中..."));
            lv_label_set_text(back->label_629, g_delay_cancel_btn ? tr("回到上一页") : tr("回到主页"));
            lv_obj_add_flag(back->bar_46, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(back->image_278, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(back->little, LV_OBJ_FLAG_HIDDEN);
        }

        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(back->label_627, tr("保温中..."));
    }
    current_group = g_heatcontain_stop_back;

    lang_scr_load_anim(heatcontain_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[heatcontain] jump: stop/cooking -> stop_back\n");
}

// stop 恢复 cooking
void heatcontain_resume_cooking(void)
{
    edit_clear();
    g_on_stop_back = 0;
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    if (depth > 1) depth--;
    lv_obj_clean(lv_scr_act());
    heatcontain_cooking_create(&ui_manager);

    heatcontain_cooking_t *cook = heatcontain_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_heatcontain_cooking) lv_group_del(g_heatcontain_cooking);
        g_heatcontain_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop, on_heatcontain_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_heatcontain_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        heatcontain_set_status(cook->status, set_temp, set_hour, set_min);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", h, m, s);

        lv_bar_set_range(cook->bar_44, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(cook->bar_44, cook_bar_saved, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_44);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, cook_bar_saved, 100);
        lv_anim_set_time(&a, ((int)(cook_total_ms - (int)cook_elapsed_saved) < 0) ? 0 : (cook_total_ms - (int)cook_elapsed_saved));
        lv_anim_start(&a);
    }

    cook_start_time = lv_tick_get() - cook_elapsed_saved;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_heatcontain_cooking;

    lang_scr_load_anim(heatcontain_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        {
        int rem = cook_total_ms - (int)cook_elapsed_saved;
        if (rem < 0) rem = 0;
        g_send.iface_status = IFACE_COOKING;
        g_send.remaining_ms = rem;
    }
    printf("[heatcontain] resume: stop -> cooking\n");
}

// setting 确定 → 回到 cooking
static void on_heatcontain_setting_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    edit_clear();   /* 清除编辑注册表残留,防止悬空 label 指针(UAF) */

    cook_total_ms = (set_hour * 3600 + set_min * 60) * 1000;

    if (depth > 1) depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_HEATCONTAIN_STOP)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_HEATCONTAIN_COMPLETE)
        depth--;
    lv_obj_clean(lv_scr_act());
    heatcontain_cooking_create(&ui_manager);

    heatcontain_cooking_t *cook = heatcontain_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_heatcontain_cooking) lv_group_del(g_heatcontain_cooking);
        g_heatcontain_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop, on_heatcontain_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_heatcontain_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        heatcontain_set_status(cook->status, set_temp, set_hour, set_min);
        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", set_hour, set_min, 0);

        lv_bar_set_range(cook->bar_44, 0, 100);
        lv_bar_set_value(cook->bar_44, 3, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_44);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    set_temp_up = set_temp; set_temp_down = set_temp;
    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_heatcontain_cooking;

    lang_scr_load_anim(heatcontain_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.remaining_ms = cook_total_ms;
    printf("[heatcontain] setting sure -> cooking\n");
}

// cooking → complete
void jump_to_heatcontain_complete(void)
{
    edit_clear();
    if (depth > 0 && page_stack[depth - 1] == PAGE_HEATCONTAIN_STOP_BACK)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_HEATCONTAIN_STOP)
        depth--;
    page_push(PAGE_HEATCONTAIN_COMPLETE);
    lv_obj_clean(lv_scr_act());
    heatcontain_complete_create(&ui_manager);

    {
        heatcontain_complete_t *cook = heatcontain_complete_get(&ui_manager);
        if (cook) {
            lv_obj_t *btns[] = { cook->little };
            if (g_heatcontain_complete) lv_group_del(g_heatcontain_complete);
            g_heatcontain_complete = group_create_for_page(btns, 1);
            lv_obj_add_event_cb(cook->little, on_heatcontain_cooking_setting_click,
                                LV_EVENT_CLICKED, NULL);
            if (set_hour == 0)
                lv_label_set_text_fmt(cook->status, tr("| 保温 | %d℃ | %02d分钟"), set_temp, set_min);
            else
                lv_label_set_text_fmt(cook->status, tr("| 保温 | %d℃ | %d小时%02d分钟"), set_temp, set_hour, set_min);
            lv_bar_set_value(cook->bar_48, 100, LV_ANIM_OFF);
        }
    }
    current_group = g_heatcontain_complete;

    lang_scr_load_anim(heatcontain_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.iface_status = IFACE_COMPLETE;
    g_send.remaining_ms = 0;
    printf("[heatcontain] jump: cooking -> complete\n");
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

void heatcontain_rebuild_menu(page_id_t child)
{
    heatcontain_menu_create(&ui_manager);
    heatcontain_menu_t *menu = heatcontain_menu_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp, menu->hour, menu->min,
            menu->next,
        };
        if (g_heatcontain_menu) lv_group_del(g_heatcontain_menu);
        g_heatcontain_menu = group_create_for_page(btns, 4);

        edit_clear();
        edit_register(menu->temp, menu->templine2, menu->templine3,
                      &set_temp, 50, 100, 5, "%d");
        edit_register(menu->hour, menu->hourline, NULL,
                      &set_hour, 0, 7, 1, "%02d");
        edit_register(menu->min, menu->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(menu->temp, on_heatcontain_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->hour, on_heatcontain_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->min, on_heatcontain_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_heatcontain_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_heatcontain_menu_next_click,
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
    current_group = g_heatcontain_menu;
    lang_scr_load_anim(heatcontain_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[heatcontain] back to heatcontain_menu\n");
}

void heatcontain_rebuild_set(page_id_t child)
{
    heatcontain_set_create(&ui_manager);
    heatcontain_set_t *set = heatcontain_set_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->sure,
            set->offpreheat, set->onpreheat,
            set->offdelay, set->ondelay,
            set->offcontain, set->oncontain,
        };
        if (g_heatcontain_set) lv_group_del(g_heatcontain_set);
        g_heatcontain_set = group_create_for_page(btns, 7);
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
        lv_obj_add_flag(set->label_599, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_600, 365, 269);
    } else {
        lv_obj_clear_flag(set->label_599, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_600, 448, 269);
    }

        apply_toggle_state(set->offpreheat, set->onpreheat, preheat_on);
    if (set_hour == 0) {
        lv_obj_add_flag(set->label_599, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_600, 365, 269);
    } else {
        lv_obj_clear_flag(set->label_599, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_600, 448, 269);
    }
        apply_toggle_state(set->offdelay, set->ondelay, delay_on);
        mode_set_apply_delay_label(set->ondelay);
    if (set_hour == 0) {
        lv_obj_add_flag(set->label_599, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_600, 365, 269);
    } else {
        lv_obj_clear_flag(set->label_599, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_600, 448, 269);
    }
        apply_toggle_state(set->offcontain, set->oncontain, contain_on);

        lv_obj_add_event_cb(set->offpreheat, heatcontain_preheat_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->onpreheat, heatcontain_preheat_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offdelay, heatcontain_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->ondelay, heatcontain_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offcontain, heatcontain_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->oncontain, heatcontain_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->sure, on_heatcontain_set_sure_click,
                            LV_EVENT_CLICKED, NULL);

        if (child == PAGE_HEATCONTAIN_COOKING && set->sure)
            lv_group_focus_obj(set->sure);
    }
        if (child == PAGE_DELAYSET && set->offdelay)
            lv_group_focus_obj(delay_on ? set->ondelay : set->offdelay);

    current_group = g_heatcontain_set;
    lang_scr_load_anim(heatcontain_set_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[heatcontain] back to heatcontain_set\n");
}

void heatcontain_rebuild_cooking(page_id_t child)
{
    edit_clear();
    heatcontain_cooking_create(&ui_manager);
    heatcontain_cooking_t *cook = heatcontain_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_heatcontain_cooking) lv_group_del(g_heatcontain_cooking);
        g_heatcontain_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop, on_heatcontain_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_heatcontain_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        heatcontain_set_status(cook->status, set_temp, set_hour, set_min);

        if (child == PAGE_HEATCONTAIN_SETTING) {
            uint32_t elapsed = lv_tick_get() - cook_start_time;
            int elapsed_sec = (elapsed + 500) / 1000;
            int total_sec = cook_total_ms / 1000;
            int remaining_sec = total_sec - elapsed_sec;
            if (remaining_sec < 0) remaining_sec = 0;
            int h = remaining_sec / 3600;
            int m = (remaining_sec % 3600) / 60;
            int s = remaining_sec % 60;
            lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", h, m, s);
            lv_bar_set_range(cook->bar_44, 0, 100);
            int progress = stop_back_progress(elapsed, cook_total_ms);
            if (progress > 100) progress = 100;
            lv_bar_set_value(cook->bar_44, progress, LV_ANIM_OFF);
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, cook->bar_44);
            lv_anim_set_exec_cb(&a, anim_bar_set_value);
            lv_anim_set_values(&a, progress, 100);
            lv_anim_set_time(&a, ((int)(cook_total_ms - (int)elapsed) < 0) ? 0 : (cook_total_ms - (int)elapsed));
            lv_anim_start(&a);
        } else {
            lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", set_hour, set_min, 0);
            lv_bar_set_range(cook->bar_44, 0, 100);
            lv_bar_set_value(cook->bar_44, 3, LV_ANIM_OFF);
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, cook->bar_44);
            lv_anim_set_exec_cb(&a, anim_bar_set_value);
            lv_anim_set_values(&a, 3, 100);
            lv_anim_set_time(&a, cook_total_ms);
            lv_anim_start(&a);
            cook_start_time = lv_tick_get();
            if (cook_timer) lv_timer_del(cook_timer);
            cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
        }
    }
    current_group = g_heatcontain_cooking;
    lang_scr_load_anim(heatcontain_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[heatcontain] back to heatcontain_cooking\n");
}

void heatcontain_rebuild_setting(void)
{
    heatcontain_setting_create(&ui_manager);
    heatcontain_setting_t *set = heatcontain_setting_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = { set->temp, set->hour, set->min, set->sure };
        if (g_heatcontain_setting) lv_group_del(g_heatcontain_setting);
        g_heatcontain_setting = group_create_for_page(btns, 4);

        edit_clear();
        edit_register(set->temp, set->templine2, set->templine3,
                      &set_temp, 50, 100, 5, "%d");
        edit_register(set->hour, set->hourline, NULL,
                      &set_hour, 0, 7, 1, "%02d");
        edit_register(set->min, set->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(set->temp, on_heatcontain_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->hour, on_heatcontain_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->min, on_heatcontain_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_heatcontain_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_heatcontain_setting_sure_click,
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
        update_heatcontain_dir_icon(set);
    }
    current_group = g_heatcontain_setting;
    lang_scr_load_anim(heatcontain_setting_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[heatcontain] back to heatcontain_setting\n");
}

void heatcontain_rebuild_stop(void)
{
    edit_clear();
    g_on_stop_back = 0;
    heatcontain_stop_create(&ui_manager);
    heatcontain_stop_t *stop = heatcontain_stop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start, stop->little };
        if (g_heatcontain_stop) lv_group_del(g_heatcontain_stop);
        g_heatcontain_stop = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(stop->start, on_heatcontain_stop_start_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(stop->little, on_heatcontain_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(stop->timelabel, "%02d:%02d:%02d", h, m, s);
        heatcontain_set_status(stop->status, set_temp, set_hour, set_min);

        lv_bar_set_range(stop->bar_45, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_45, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_heatcontain_stop;
    lang_scr_load_anim(heatcontain_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[heatcontain] back to heatcontain_stop\n");
    g_send.iface_status = IFACE_PAUSE;
}

void heatcontain_rebuild_stop_back(void)
{
    edit_clear();
    int cooking_bar_val = 0;
    if (cook_timer && depth > 0 && page_stack[depth - 1] == PAGE_HEATCONTAIN_COOKING) {
        heatcontain_cooking_t *cook = heatcontain_cooking_get(&ui_manager);
        if (cook && cook->bar_44) cooking_bar_val = lv_bar_get_value(cook->bar_44);
    }
    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_heatcontain_complete;
    heatcontain_stop_back_create(&ui_manager);
    heatcontain_stop_back_t *back = heatcontain_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_heatcontain_stop_back) lv_group_del(g_heatcontain_stop_back);
        g_heatcontain_stop_back = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(back->sure, on_heatcontain_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        heatcontain_set_status(back->status, set_temp, set_hour, set_min);
        int p = cooking_bar_val;
        if (p <= 0) {
            uint32_t elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
            p = stop_back_progress(elapsed, cook_total_ms);
        }
        if (p > 100) p = 100;
        lv_bar_set_range(back->bar_46, 0, 100);
                lv_bar_set_value(back->bar_46, p, LV_ANIM_OFF);
        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            if (g_keepwarm_active)
                lv_label_set_text(back->label_627, tr("保温中..."));
            else
                lv_label_set_text(back->label_627, tr("已完成"));
            lv_bar_set_value(back->bar_46, 100, LV_ANIM_OFF);
        }
    }
    current_group = g_heatcontain_stop_back;
    lang_scr_load_anim(heatcontain_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[heatcontain] back to heatcontain_stop_back\n");
}

void heatcontain_rebuild_complete(void)
{
    edit_clear();
    heatcontain_complete_create(&ui_manager);
    current_group = g_heatcontain_complete;
    lang_scr_load_anim(heatcontain_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[heatcontain] back to heatcontain_complete\n");
}
void heatcontain_complete_rebind(lv_obj_t *btn)
{
    lv_obj_add_event_cb(btn, on_heatcontain_cooking_setting_click, LV_EVENT_CLICKED, NULL);
}
