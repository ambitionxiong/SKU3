#include "protocol.h"
#include "protocol.h"
#include "nav.h"

/* setting 页进入时保存原始值，BACK 返回时恢复 */
int west_setting_saved_temp, west_setting_saved_hour, west_setting_saved_min;


static void on_west_menu_next_click(lv_event_t *e);
static void on_west_set_sure_click(lv_event_t *e);
static void on_west_cooking_stop_click(lv_event_t *e);
static void on_west_cooking_setting_click(lv_event_t *e);
static void on_west_setting_sure_click(lv_event_t *e);
static void on_west_stop_start_click(lv_event_t *e);
static void on_west_stop_back_littal_click(lv_event_t *e);
static void on_west_stop_back_sure_click(lv_event_t *e);
static void on_west_edit_focus(lv_event_t *e);
void update_west_dir_icon(west_setting_t *set);
static void west_set_status(lv_obj_t *label, int temp, int hour, int min);
static void west_preheat_toggle(lv_event_t *e);
static void west_delay_toggle(lv_event_t *e);
static void west_contain_toggle(lv_event_t *e);
void jump_to_west_cooking(void);
void jump_to_west_complete(void);

static void west_set_status(lv_obj_t *label, int temp, int hour, int min)
{
    if (hour == 0)
        lv_label_set_text_fmt(label, "| 顶部烧烤 | %d℃ | %02d分钟", temp, min);
    else
        lv_label_set_text_fmt(label, "| 顶部烧烤 | %d℃ | %d小时%02d分钟", temp, hour, min);
}

static void west_preheat_toggle(lv_event_t *e)
{
    west_set_t *set = west_set_get(&ui_manager);
    if (!set) return;
    preheat_on = !preheat_on;
    if (preheat_on) {
        lv_obj_add_flag(set->offcontain, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->oncontain, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->oncontain);
    } else {
        lv_obj_add_flag(set->oncontain, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->offcontain, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->offcontain);
    }
}

static void west_delay_toggle(lv_event_t *e)
{
    west_set_t *set = west_set_get(&ui_manager);
    if (!set) return;
    delay_on = !delay_on;
    if (delay_on) {
        lv_obj_add_flag(set->offcontain, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->oncontain, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->oncontain);
    } else {
        lv_obj_add_flag(set->oncontain, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->offcontain, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->offcontain);
    }
}

static void west_contain_toggle(lv_event_t *e)
{
    west_set_t *set = west_set_get(&ui_manager);
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

void update_west_dir_icon(west_setting_t *set)
{
    if (!set) return;
    lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
    if (set_temp < 100)
        lv_obj_clear_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_clear_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
}

static void on_west_edit_focus(lv_event_t *e)
{
    on_edit_focus(e);
    west_setting_t *set = west_setting_get(&ui_manager);
    if (set && lv_event_get_target(e) == set->temp)
        update_west_dir_icon(set);
}

static void on_west_menu_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_west_set();
}

static void on_west_set_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_west_cooking();
}

static void on_west_cooking_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_west_stop();
}

static void on_west_cooking_setting_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_west_setting();
}

static void on_west_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        west_resume_cooking();
}


static void on_west_stop_back_littal_click(lv_event_t *e)
{
    page_pop();
}

static void on_west_stop_back_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
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
    g_send.cook_mode = MODE_COOK4;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    printf("[west] stop_back sure -> major_menu\n");
}

// ==============================
// Jump 函数
// ==============================

void jump_to_west_menu(void)
{
    page_push(PAGE_WEST_MENU);
    lv_obj_clean(lv_scr_act());
    west_menu_create(&ui_manager);

    west_menu_t *menu = west_menu_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp, menu->hour, menu->min,
            menu->next,
        };
        if (g_west_menu) lv_group_del(g_west_menu);
        g_west_menu = group_create_for_page(btns, 4);

        edit_clear();
        edit_register(menu->temp, menu->templine2, menu->templine3,
                      &set_temp, 30, 300, 5, "%d");
        edit_register(menu->hour, menu->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(menu->min, menu->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(menu->temp, on_west_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->hour, on_west_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->min, on_west_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_west_edit_focus,
                            LV_EVENT_FOCUSED, NULL);

        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_west_menu_next_click,
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
    current_group = g_west_menu;

    lv_scr_load_anim(west_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.cook_mode = MODE_COOK4;
    printf("[west] jump: cookmenu -> west_menu\n");
}

// menu → set
void jump_to_west_set(void)
{
    page_push(PAGE_WEST_SET);
    lv_obj_clean(lv_scr_act());
    west_set_create(&ui_manager);

    preheat_on = 0; delay_on = 0; contain_on = 0;

    west_set_t *set = west_set_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->sure,
            set->offdelay, set->ondelay,
            set->offcontain, set->oncontain,
        };
        if (g_west_set) lv_group_del(g_west_set);
        g_west_set = group_create_for_page(btns, 5);
        clear_focus_states(btns, 5);
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

        apply_toggle_state(set->offdelay, set->ondelay, delay_on);
        apply_toggle_state(set->offcontain, set->oncontain, contain_on);

        lv_obj_add_event_cb(set->offdelay, west_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->ondelay, west_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offcontain, west_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->oncontain, west_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->sure, on_west_set_sure_click,
                            LV_EVENT_CLICKED, NULL);
    }
    current_group = g_west_set;

    lv_scr_load_anim(west_set_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[west] jump: menu -> west_set\n");
}

// set → cooking
void jump_to_west_cooking(void)
{
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }

    page_push(PAGE_WEST_COOKING);
    lv_obj_clean(lv_scr_act());
    west_cooking_create(&ui_manager);

    west_cooking_t *cook = west_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_west_cooking) lv_group_del(g_west_cooking);
        g_west_cooking = group_create_for_page(btns, 2);

        lv_obj_add_event_cb(cook->stop, on_west_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_west_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        west_set_status(cook->status, set_temp, set_hour, set_min);
        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", set_hour, set_min, 0);
    }

    cook_total_ms = (set_hour * 3600 + set_min * 60) * 1000;
    if (cook) {
        lv_bar_set_range(cook->bar_8, 0, 100);
        lv_bar_set_value(cook->bar_8, 3, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_8);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    set_temp_up = set_temp; set_temp_down = set_temp;
    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_west_cooking;

    lv_scr_load_anim(west_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = cook_total_ms;
    printf("[west] jump: set -> west_cooking\n");
}

// cooking → setting（不暂停 timer）
void jump_to_west_setting(void)
{
    west_setting_saved_temp = set_temp; west_setting_saved_hour = set_hour; west_setting_saved_min = set_min;
    page_push(PAGE_WEST_SETTING);
    lv_obj_clean(lv_scr_act());
    west_setting_create(&ui_manager);

    west_setting_t *set = west_setting_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = { set->temp, set->hour, set->min, set->sure };
        if (g_west_setting) lv_group_del(g_west_setting);
        g_west_setting = group_create_for_page(btns, 4);

        edit_clear();
        edit_register(set->temp, set->templine2, set->templine3,
                      &set_temp, 30, 300, 5, "%d");
        edit_register(set->hour, set->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(set->min, set->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(set->temp, on_west_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->hour, on_west_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->min, on_west_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_west_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_west_setting_sure_click,
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
        set_hour = h; set_min = m;
        lv_label_set_text_fmt(set->timelabel, "%02d:%02d:%02d", h, m, s);
        lv_label_set_text_fmt(set->hour, "%02d", h);
        lv_label_set_text_fmt(set->min, "%02d", m);
        lv_label_set_text_fmt(set->temp, "%d", set_temp);

        lv_obj_add_flag(set->templine3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->templine2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hourline, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->minline, LV_OBJ_FLAG_HIDDEN);

        lv_group_focus_obj(set->temp);
        update_west_dir_icon(set);
    }
    current_group = g_west_setting;

    lv_scr_load_anim(west_setting_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.iface_status = (cook_timer != NULL) ? IFACE_COOKING : IFACE_SETTING;
    printf("[west] jump: cooking -> west_setting\n");
}

// cooking → stop（暂停）
void jump_to_west_stop(void)
{
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

    {
        west_cooking_t *cook = west_cooking_get(&ui_manager);
        cook_bar_saved = cook ? lv_bar_get_value(cook->bar_8) : 0;
    }

    page_push(PAGE_WEST_STOP);
    lv_obj_clean(lv_scr_act());
    west_stop_create(&ui_manager);

    west_stop_t *stop = west_stop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start, stop->little };
        if (g_west_stop) lv_group_del(g_west_stop);
        g_west_stop = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(stop->start, on_west_stop_start_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(stop->little, on_west_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(stop->timelabel, "%02d:%02d:%02d", h, m, s);
        west_set_status(stop->status, set_temp, set_hour, set_min);

        lv_bar_set_range(stop->bar_9, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_9, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_west_stop;

    lv_scr_load_anim(west_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = (cook_total_ms > (int)cook_elapsed_saved) ? cook_total_ms - (int)cook_elapsed_saved : 0;
    printf("[west] jump: cooking -> stop (pause)\n");
}

// stop → stop_back
void jump_to_west_stop_back(void)
{
    page_push(PAGE_WEST_STOP_BACK);
    lv_obj_clean(lv_scr_act());
    west_stop_back_create(&ui_manager);

    west_stop_back_t *back = west_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure, back->little };
        if (g_west_stop_back) lv_group_del(g_west_stop_back);
        g_west_stop_back = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(back->sure, on_west_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(back->little, on_west_stop_back_littal_click,
                            LV_EVENT_CLICKED, NULL);

        west_set_status(back->status, set_temp, set_hour, set_min);
        lv_bar_set_range(back->bar_10, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(back->bar_10, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_west_stop_back;

    lv_scr_load_anim(west_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[west] jump: stop -> stop_back\n");
}

// stop 恢复 cooking
void west_resume_cooking(void)
{
    
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
depth--;
    lv_obj_clean(lv_scr_act());
    west_cooking_create(&ui_manager);

    west_cooking_t *cook = west_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_west_cooking) lv_group_del(g_west_cooking);
        g_west_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop, on_west_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_west_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        west_set_status(cook->status, set_temp, set_hour, set_min);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", h, m, s);

        lv_bar_set_range(cook->bar_8, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(cook->bar_8, cook_bar_saved, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_8);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, cook_bar_saved, 100);
        lv_anim_set_time(&a, cook_total_ms - (int)cook_elapsed_saved);
        lv_anim_start(&a);
    }

    cook_start_time = lv_tick_get() - cook_elapsed_saved;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_west_cooking;

    lv_scr_load_anim(west_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        {
        int rem = cook_total_ms - (int)cook_elapsed_saved;
        if (rem < 0) rem = 0;
        g_send.iface_status = IFACE_COOKING;
        g_send.remaining_ms = rem;
    }
    printf("[west] resume: stop -> cooking\n");
}

// setting 确定 → 回到 cooking
static void on_west_setting_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;

    cook_total_ms = (set_hour * 3600 + set_min * 60) * 1000;

    depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_WEST_STOP)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_WEST_COMPLETE)
        depth--;
    lv_obj_clean(lv_scr_act());
    west_cooking_create(&ui_manager);

    west_cooking_t *cook = west_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_west_cooking) lv_group_del(g_west_cooking);
        g_west_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop, on_west_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_west_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        west_set_status(cook->status, set_temp, set_hour, set_min);
        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", set_hour, set_min, 0);

        lv_bar_set_range(cook->bar_8, 0, 100);
        lv_bar_set_value(cook->bar_8, 3, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_8);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    set_temp_up = set_temp; set_temp_down = set_temp;
    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_west_cooking;

    lv_scr_load_anim(west_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.remaining_ms = cook_total_ms;
    printf("[west] setting sure -> cooking\n");
}

// cooking → complete
void jump_to_west_complete(void)
{
    page_push(PAGE_WEST_COMPLETE);
    lv_obj_clean(lv_scr_act());
    west_complete_create(&ui_manager);

    {
        west_complete_t *cook = west_complete_get(&ui_manager);
        if (cook) {
            lv_obj_t *btns[] = { cook->little };
            if (g_west_complete) lv_group_del(g_west_complete);
            g_west_complete = group_create_for_page(btns, 1);
            lv_obj_add_event_cb(cook->little, on_west_cooking_setting_click,
                                LV_EVENT_CLICKED, NULL);
        }
    }
    current_group = g_west_complete;

    lv_scr_load_anim(west_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
        g_send.iface_status = IFACE_COMPLETE;
    g_send.remaining_ms = 0;
        g_send.iface_status = IFACE_COMPLETE;
    g_send.remaining_ms = 0;
    printf("[west] jump: cooking -> complete\n");
}

// ==============================
// Rebuild 函数（供 page_pop 调用）
// ==============================

void west_rebuild_menu(page_id_t child)
{
    west_menu_create(&ui_manager);
    west_menu_t *menu = west_menu_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp, menu->hour, menu->min,
            menu->next,
        };
        if (g_west_menu) lv_group_del(g_west_menu);
        g_west_menu = group_create_for_page(btns, 4);

        edit_clear();
        edit_register(menu->temp, menu->templine2, menu->templine3,
                      &set_temp, 30, 300, 5, "%d");
        edit_register(menu->hour, menu->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(menu->min, menu->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(menu->temp, on_west_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->hour, on_west_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->min, on_west_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_west_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_west_menu_next_click,
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
    current_group = g_west_menu;
    lv_scr_load_anim(west_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[west] back to west_menu\n");
}

void west_rebuild_set(page_id_t child)
{
    west_set_create(&ui_manager);
    west_set_t *set = west_set_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->sure,
            set->offdelay, set->ondelay,
            set->offcontain, set->oncontain,
        };
        if (g_west_set) lv_group_del(g_west_set);
        g_west_set = group_create_for_page(btns, 5);
        clear_focus_states(btns, 5);
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

        apply_toggle_state(set->offdelay, set->ondelay, delay_on);
        apply_toggle_state(set->offcontain, set->oncontain, contain_on);

        lv_obj_add_event_cb(set->offdelay, west_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->ondelay, west_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offcontain, west_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->oncontain, west_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->sure, on_west_set_sure_click,
                            LV_EVENT_CLICKED, NULL);

        if (child == PAGE_WEST_COOKING && set->sure)
            lv_group_focus_obj(set->sure);
    }
    current_group = g_west_set;
    lv_scr_load_anim(west_set_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[west] back to west_set\n");
}

void west_rebuild_cooking(page_id_t child)
{
    west_cooking_create(&ui_manager);
    west_cooking_t *cook = west_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_west_cooking) lv_group_del(g_west_cooking);
        g_west_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop, on_west_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_west_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        west_set_status(cook->status, set_temp, set_hour, set_min);

        if (child == PAGE_WEST_SETTING) {
            uint32_t elapsed = lv_tick_get() - cook_start_time;
            int elapsed_sec = (elapsed + 500) / 1000;
            int total_sec = cook_total_ms / 1000;
            int remaining_sec = total_sec - elapsed_sec;
            if (remaining_sec < 0) remaining_sec = 0;
            int h = remaining_sec / 3600;
            int m = (remaining_sec % 3600) / 60;
            int s = remaining_sec % 60;
            lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", h, m, s);
            lv_bar_set_range(cook->bar_8, 0, 100);
            int progress = (int)((int64_t)elapsed * 100 / cook_total_ms);
            if (progress > 100) progress = 100;
            lv_bar_set_value(cook->bar_8, progress, LV_ANIM_OFF);
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, cook->bar_8);
            lv_anim_set_exec_cb(&a, anim_bar_set_value);
            lv_anim_set_values(&a, progress, 100);
            lv_anim_set_time(&a, cook_total_ms - (int)elapsed);
            lv_anim_start(&a);
        } else {
            lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", set_hour, set_min, 0);
            lv_bar_set_range(cook->bar_8, 0, 100);
            lv_bar_set_value(cook->bar_8, 3, LV_ANIM_OFF);
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, cook->bar_8);
            lv_anim_set_exec_cb(&a, anim_bar_set_value);
            lv_anim_set_values(&a, 3, 100);
            lv_anim_set_time(&a, cook_total_ms);
            lv_anim_start(&a);
            cook_start_time = lv_tick_get();
            if (cook_timer) lv_timer_del(cook_timer);
            cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
        }
    }
    current_group = g_west_cooking;
    lv_scr_load_anim(west_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[west] back to west_cooking\n");
}

void west_rebuild_setting(void)
{
    west_setting_create(&ui_manager);
    west_setting_t *set = west_setting_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = { set->temp, set->hour, set->min, set->sure };
        if (g_west_setting) lv_group_del(g_west_setting);
        g_west_setting = group_create_for_page(btns, 4);

        edit_clear();
        edit_register(set->temp, set->templine2, set->templine3,
                      &set_temp, 30, 300, 5, "%d");
        edit_register(set->hour, set->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(set->min, set->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(set->temp, on_west_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->hour, on_west_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->min, on_west_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_west_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_west_setting_sure_click,
                            LV_EVENT_CLICKED, NULL);

        uint32_t elapsed = lv_tick_get() - cook_start_time;
        int elapsed_sec = (elapsed + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        set_hour = h; set_min = m;
        lv_label_set_text_fmt(set->timelabel, "%02d:%02d:%02d", h, m, s);
        lv_label_set_text_fmt(set->hour, "%02d", h);
        lv_label_set_text_fmt(set->min, "%02d", m);
        lv_label_set_text_fmt(set->temp, "%d", set_temp);

        lv_obj_add_flag(set->templine3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->templine2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hourline, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->minline, LV_OBJ_FLAG_HIDDEN);

        lv_group_focus_obj(set->temp);
        update_west_dir_icon(set);
    }
    current_group = g_west_setting;
    lv_scr_load_anim(west_setting_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[west] back to west_setting\n");
}

void west_rebuild_stop(void)
{
    west_stop_create(&ui_manager);
    west_stop_t *stop = west_stop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start, stop->little };
        if (g_west_stop) lv_group_del(g_west_stop);
        g_west_stop = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(stop->start, on_west_stop_start_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(stop->little, on_west_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(stop->timelabel, "%02d:%02d:%02d", h, m, s);
        west_set_status(stop->status, set_temp, set_hour, set_min);

        lv_bar_set_range(stop->bar_9, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_9, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_west_stop;
    lv_scr_load_anim(west_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[west] back to west_stop\n");
    g_send.iface_status = IFACE_PAUSE;
}

void west_rebuild_stop_back(void)
{
    west_stop_back_create(&ui_manager);
    west_stop_back_t *back = west_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure, back->little };
        if (g_west_stop_back) lv_group_del(g_west_stop_back);
        g_west_stop_back = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(back->sure, on_west_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(back->little, on_west_stop_back_littal_click,
                            LV_EVENT_CLICKED, NULL);

        west_set_status(back->status, set_temp, set_hour, set_min);
        lv_bar_set_range(back->bar_10, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(back->bar_10, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_west_stop_back;
    lv_scr_load_anim(west_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[west] back to west_stop_back\n");
}

void west_rebuild_complete(void)
{
    west_complete_create(&ui_manager);
    current_group = g_west_complete;
    lv_scr_load_anim(west_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[west] back to west_complete\n");
}
