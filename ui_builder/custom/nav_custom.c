#include "protocol.h"
#include "nav.h"

int custom_setting_saved_hour, custom_setting_saved_min;

static void on_custom_menu_next_click(lv_event_t *e);
static void on_custom_set_sure_click(lv_event_t *e);
static void on_custom_cooking_stop_click(lv_event_t *e);
static void on_custom_cooking_setting_click(lv_event_t *e);
static void on_custom_setting_sure_click(lv_event_t *e);
static void on_custom_stop_start_click(lv_event_t *e);
static void on_custom_stop_back_sure_click(lv_event_t *e);
static void on_custom_edit_focus(lv_event_t *e);
static void custom_set_status(lv_obj_t *label, int temp, int hour, int min);
static void custom_delay_toggle(lv_event_t *e);
static void custom_contain_toggle(lv_event_t *e);
void jump_to_custom_cooking(void);
void jump_to_custom_complete(void);

static void custom_set_status(lv_obj_t *label, int temp, int hour, int min)
{
    if (hour == 0)
        lv_label_set_text_fmt(label, "| 自定义 | %d℃ | %02d分钟", temp, min);
    else
        lv_label_set_text_fmt(label, "| 自定义 | %d℃ | %d小时%02d分钟", temp, hour, min);
}

static void custom_delay_toggle(lv_event_t *e)
{
    custom_set_t *set = custom_set_get(&ui_manager);
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

static void custom_contain_toggle(lv_event_t *e)
{
    custom_set_t *set = custom_set_get(&ui_manager);
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

static void on_custom_edit_focus(lv_event_t *e)
{
    on_edit_focus(e);
}

static void on_custom_menu_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_custom_set();
}

static void on_custom_set_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr)) {
        if (delay_on)
            jump_to_delaycooking();
        else
            jump_to_custom_cooking();
    }
}

static void on_custom_cooking_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_custom_stop();
}

static void on_custom_cooking_setting_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_custom_setting();
}

static void on_custom_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        custom_resume_cooking();
}

static void on_custom_stop_back_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    g_on_stop_back = 0;
    g_keepwarm_active = 0;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    set_temp = 200; set_hour = 0; set_min = 30;
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
    printf("[lasagna] stop_back sure -> major_menu\n");
}

void jump_to_custom_menu(void)
{
    set_hour = 0; set_min = 30; set_temp = 200;
    page_push(PAGE_CUSTOM_MENU);
    lv_obj_clean(lv_scr_act());
    custom_menu_create(&ui_manager);

    custom_menu_t *menu = custom_menu_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->hour, menu->min,
            menu->next,
        };
        if (g_custom_menu) lv_group_del(g_custom_menu);
        g_custom_menu = group_create_for_page(btns, 3);

        edit_clear();
        edit_register(menu->hour, menu->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(menu->min, menu->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(menu->hour, on_custom_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->min, on_custom_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_custom_edit_focus,
                            LV_EVENT_FOCUSED, NULL);

        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_custom_menu_next_click,
                                LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(menu->hour, "%02d", set_hour);
        lv_label_set_text_fmt(menu->min, "%02d", set_min);

        validate_constraints();

        if (menu->next)
            lv_group_focus_obj(menu->next);
    }
    current_group = g_custom_menu;

    lv_scr_load_anim(custom_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.cook_mode = MODE_FROZEN_BAKE;
    printf("[lasagna] jump: frozencook -> custom_menu\n");
}

void jump_to_custom_set(void)
{
    page_push(PAGE_CUSTOM_SET);

    preheat_on = 0; delay_on = 0; contain_on = 0;
    lv_obj_clean(lv_scr_act());
    custom_set_create(&ui_manager);

    custom_set_t *set = custom_set_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->sure,
            set->offdelay, set->ondelay,
            set->offcontain, set->oncontain,
        };
        if (g_custom_set) lv_group_del(g_custom_set);
        g_custom_set = group_create_for_page(btns, 5);
        clear_focus_states(btns, 5);
        lv_group_focus_obj(set->sure);

        lv_label_set_text_fmt(set->temp, "%d", set_temp);
        lv_label_set_text_fmt(set->hour, "%02d", set_hour);
        lv_label_set_text_fmt(set->min, "%02d", set_min);

        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);

        apply_toggle_state(set->offdelay, set->ondelay, delay_on);
        mode_set_apply_delay_label(set->ondelay);
    if (set_hour == 0) {
        lv_obj_add_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->label_859, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_860, 365, 269);
    } else {
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->label_859, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_860, 448, 269);
    }

        apply_toggle_state(set->offcontain, set->oncontain, contain_on);
    if (set_hour == 0) {
        lv_obj_add_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->label_859, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_860, 365, 269);
    } else {
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->label_859, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_860, 448, 269);
    }


        lv_obj_add_event_cb(set->offdelay, custom_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->ondelay, custom_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offcontain, custom_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->oncontain, custom_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->sure, on_custom_set_sure_click,
                            LV_EVENT_CLICKED, NULL);
    }
    current_group = g_custom_set;

    lv_scr_load_anim(custom_set_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[lasagna] jump: menu -> custom_set\n");
}

void jump_to_custom_cooking(void)
{
    edit_clear();
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }

    g_keepwarm_active = 0;

    page_push(PAGE_CUSTOM_COOKING);
    lv_obj_clean(lv_scr_act());
    custom_cooking_create(&ui_manager);

    custom_cooking_t *cook = custom_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_custom_cooking) lv_group_del(g_custom_cooking);
        g_custom_cooking = group_create_for_page(btns, 2);

        lv_obj_add_event_cb(cook->stop, on_custom_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_custom_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        custom_set_status(cook->status, set_temp, set_hour, set_min);
        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", set_hour, set_min, 0);
    }

    cook_total_ms = (set_hour * 3600 + set_min * 60) * 1000;
    if (cook) {
        lv_bar_set_range(cook->bar_68, 0, 100);
        lv_bar_set_value(cook->bar_68, 3, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_68);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    set_temp_up = set_temp; set_temp_down = set_temp;
    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_custom_cooking;

    lv_scr_load_anim(custom_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = cook_total_ms;
    printf("[lasagna] jump: set -> custom_cooking\n");
}

void jump_to_custom_setting(void)
{
    custom_setting_saved_hour = set_hour;
    custom_setting_saved_min = set_min;

    page_push(PAGE_CUSTOM_SETTING);
    lv_obj_clean(lv_scr_act());
    custom_setting_create(&ui_manager);

    custom_setting_t *set = custom_setting_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = { set->hour, set->min, set->sure };
        if (g_custom_setting) lv_group_del(g_custom_setting);
        g_custom_setting = group_create_for_page(btns, 3);

        edit_clear();
        edit_register(set->hour, set->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(set->min, set->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(set->hour, on_custom_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->min, on_custom_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_custom_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_custom_setting_sure_click,
                            LV_EVENT_CLICKED, NULL);

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

        lv_obj_add_flag(set->hourline, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->minline, LV_OBJ_FLAG_HIDDEN);

        lv_group_focus_obj(set->hour);
    }
    current_group = g_custom_setting;

    lv_scr_load_anim(custom_setting_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    if (g_send.iface_status != IFACE_COMPLETE)
        g_send.iface_status = (cook_timer != NULL) ? IFACE_COOKING : IFACE_SETTING;
    printf("[lasagna] jump: cooking -> custom_setting\n");
}

void jump_to_custom_stop(void)
{
    edit_clear();
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

    cook_bar_saved = 3 + (int)((int64_t)cook_elapsed_saved * 97 / (cook_total_ms ? cook_total_ms : 1));
    if (cook_bar_saved > 100) cook_bar_saved = 100;

    page_push(PAGE_CUSTOM_STOP);
    lv_obj_clean(lv_scr_act());
    custom_stop_create(&ui_manager);

    custom_stop_t *stop = custom_stop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start, stop->little };
        if (g_custom_stop) lv_group_del(g_custom_stop);
        g_custom_stop = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(stop->start, on_custom_stop_start_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(stop->little, on_custom_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(stop->timelabel, "%02d:%02d:%02d", h, m, s);
        custom_set_status(stop->status, set_temp, set_hour, set_min);

        lv_bar_set_range(stop->bar_69, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_69, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_custom_stop;

    lv_scr_load_anim(custom_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = (cook_total_ms > (int)cook_elapsed_saved) ? cook_total_ms - (int)cook_elapsed_saved : 0;
    printf("[lasagna] jump: cooking -> stop (pause)\n");
}

void jump_to_custom_stop_back(void)
{
    edit_clear();
    int cooking_bar_val = 0;
    if (cook_timer && depth > 0 && page_stack[depth - 1] == PAGE_CUSTOM_COOKING) {
        custom_cooking_t *cook = custom_cooking_get(&ui_manager);
        if (cook && cook->bar_68) cooking_bar_val = lv_bar_get_value(cook->bar_68);
    }
    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_custom_complete;
    page_push(PAGE_CUSTOM_STOP_BACK);
    lv_obj_clean(lv_scr_act());
    custom_stop_back_create(&ui_manager);

    custom_stop_back_t *back = custom_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_custom_stop_back) lv_group_del(g_custom_stop_back);
        g_custom_stop_back = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(back->sure, on_custom_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        custom_set_status(back->status, set_temp, set_hour, set_min);
        int p = cooking_bar_val;
        if (p <= 0) {
            uint32_t elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
            p = stop_back_progress(elapsed, cook_total_ms);
        }
        if (p > 100) p = 100;
        lv_bar_set_range(back->bar_70, 0, 100);
        lv_bar_set_value(back->bar_70, p, LV_ANIM_OFF);
        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(back->label_882, "烹饪中...");

        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            if (g_keepwarm_active)
                lv_label_set_text(back->label_882, "保温中...");
            else
                lv_label_set_text(back->label_882, "已完成");
            lv_bar_set_value(back->bar_70, 100, LV_ANIM_OFF);
        }
        if (g_delay_cancel_to_stop_back) {
            g_delay_cancel_to_stop_back = 0;
            lv_label_set_text(back->label_882, "预约中...");
            lv_obj_add_flag(back->bar_70, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(back->image_400, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(back->little, LV_OBJ_FLAG_HIDDEN);
        }

    }
    current_group = g_custom_stop_back;

    lv_scr_load_anim(custom_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[lasagna] stop/cooking -> stop_back\n");
}

void custom_resume_cooking(void)
{
    edit_clear();
    g_on_stop_back = 0;
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    if (depth > 1) depth--;
    lv_obj_clean(lv_scr_act());
    custom_cooking_create(&ui_manager);

    custom_cooking_t *cook = custom_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_custom_cooking) lv_group_del(g_custom_cooking);
        g_custom_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop, on_custom_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_custom_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        custom_set_status(cook->status, set_temp, set_hour, set_min);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", h, m, s);

        lv_bar_set_range(cook->bar_68, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(cook->bar_68, cook_bar_saved, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_68);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, cook_bar_saved, 100);
        lv_anim_set_time(&a, ((int)(cook_total_ms - (int)cook_elapsed_saved) < 0) ? 0 : (cook_total_ms - (int)cook_elapsed_saved));
        lv_anim_start(&a);
    }

    cook_start_time = lv_tick_get() - cook_elapsed_saved;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_custom_cooking;

    lv_scr_load_anim(custom_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    {
        int rem = cook_total_ms - (int)cook_elapsed_saved;
        if (rem < 0) rem = 0;
        g_send.iface_status = IFACE_COOKING;
        g_send.remaining_ms = rem;
    }
    printf("[lasagna] resume: stop -> cooking\n");
}

static void on_custom_setting_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;

    cook_total_ms = (set_hour * 3600 + set_min * 60) * 1000;

    if (depth > 1) depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_CUSTOM_STOP)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_CUSTOM_COMPLETE)
        depth--;
    lv_obj_clean(lv_scr_act());
    custom_cooking_create(&ui_manager);

    custom_cooking_t *cook = custom_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_custom_cooking) lv_group_del(g_custom_cooking);
        g_custom_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop, on_custom_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_custom_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        custom_set_status(cook->status, set_temp, set_hour, set_min);
        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", set_hour, set_min, 0);

        lv_bar_set_range(cook->bar_68, 0, 100);
        lv_bar_set_value(cook->bar_68, 3, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_68);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    set_temp_up = set_temp; set_temp_down = set_temp;
    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_custom_cooking;

    lv_scr_load_anim(custom_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.remaining_ms = cook_total_ms;
    printf("[lasagna] setting sure -> cooking\n");
}

void jump_to_custom_complete(void)
{
    edit_clear();
    if (depth > 0 && page_stack[depth - 1] == PAGE_CUSTOM_STOP_BACK)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_CUSTOM_STOP)
        depth--;
    page_push(PAGE_CUSTOM_COMPLETE);
    lv_obj_clean(lv_scr_act());
    custom_complete_create(&ui_manager);

    {
        custom_complete_t *cook = custom_complete_get(&ui_manager);
        if (cook) {
            lv_obj_t *btns[] = { cook->little };
            if (g_custom_complete) lv_group_del(g_custom_complete);
            g_custom_complete = group_create_for_page(btns, 1);
            lv_obj_add_event_cb(cook->little, on_custom_cooking_setting_click,
                                LV_EVENT_CLICKED, NULL);
            custom_set_status(cook->status, set_temp, set_hour, set_min);
            lv_bar_set_value(cook->bar_71, 100, LV_ANIM_OFF);
        }
    }
    current_group = g_custom_complete;

    lv_scr_load_anim(custom_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COMPLETE;
    g_send.remaining_ms = 0;
    printf("[lasagna] jump: cooking -> complete\n");
    /* 自动保温：保温开关开启时 complete 页停留 1 分钟无操作 → 保温（15 分钟） */
    if (contain_on) {
        g_keepwarm_active = 0;
        g_keepwarm_sec = 0;
        if (cook_timer) lv_timer_del(cook_timer);
        cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
    }

}

void custom_rebuild_menu(page_id_t child)
{
    custom_menu_create(&ui_manager);
    custom_menu_t *menu = custom_menu_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->hour, menu->min,
            menu->next,
        };
        if (g_custom_menu) lv_group_del(g_custom_menu);
        g_custom_menu = group_create_for_page(btns, 3);

        edit_clear();
        edit_register(menu->hour, menu->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(menu->min, menu->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(menu->hour, on_custom_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->min, on_custom_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_custom_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_custom_menu_next_click,
                                LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(menu->hour, "%02d", set_hour);
        lv_label_set_text_fmt(menu->min, "%02d", set_min);

        validate_constraints();
        if (menu->next) lv_group_focus_obj(menu->next);
    }
    current_group = g_custom_menu;
    lv_scr_load_anim(custom_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[lasagna] back to custom_menu\n");
}

void custom_rebuild_set(page_id_t child)
{
    custom_set_create(&ui_manager);
    custom_set_t *set = custom_set_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->sure,
            set->offdelay, set->ondelay,
            set->offcontain, set->oncontain,
        };
        if (g_custom_set) lv_group_del(g_custom_set);
        g_custom_set = group_create_for_page(btns, 5);
        clear_focus_states(btns, 5);
        lv_group_focus_obj(set->sure);

        lv_label_set_text_fmt(set->temp, "%d", set_temp);
        lv_label_set_text_fmt(set->hour, "%02d", set_hour);
        lv_label_set_text_fmt(set->min, "%02d", set_min);

        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);

        apply_toggle_state(set->offdelay, set->ondelay, delay_on);
        mode_set_apply_delay_label(set->ondelay);
    if (set_hour == 0) {
        lv_obj_add_flag(set->label_859, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_860, 365, 269);
    } else {
        lv_obj_clear_flag(set->label_859, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_860, 448, 269);
    }

        apply_toggle_state(set->offcontain, set->oncontain, contain_on);
    if (set_hour == 0) {
        lv_obj_add_flag(set->label_859, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 312, 254);
        lv_obj_set_pos(set->label_860, 365, 269);
    } else {
        lv_obj_clear_flag(set->label_859, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->hour, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(set->min, 395, 254);
        lv_obj_set_pos(set->label_860, 448, 269);
    }


        lv_obj_add_event_cb(set->offdelay, custom_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->ondelay, custom_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offcontain, custom_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->oncontain, custom_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->sure, on_custom_set_sure_click,
                            LV_EVENT_CLICKED, NULL);

        if (child == PAGE_CUSTOM_COOKING && set->sure)
            lv_group_focus_obj(set->sure);
    }
        if (child == PAGE_DELAYSET && set->offdelay)
            lv_group_focus_obj(delay_on ? set->ondelay : set->offdelay);

    current_group = g_custom_set;
    lv_scr_load_anim(custom_set_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[lasagna] back to custom_set\n");
}

void custom_rebuild_cooking(page_id_t child)
{
    edit_clear();
    custom_cooking_create(&ui_manager);
    custom_cooking_t *cook = custom_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop, cook->little };
        if (g_custom_cooking) lv_group_del(g_custom_cooking);
        g_custom_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop, on_custom_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little, on_custom_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        custom_set_status(cook->status, set_temp, set_hour, set_min);

        if (child == PAGE_CUSTOM_SETTING) {
            uint32_t elapsed = lv_tick_get() - cook_start_time;
            int elapsed_sec = (elapsed + 500) / 1000;
            int total_sec = cook_total_ms / 1000;
            int remaining_sec = total_sec - elapsed_sec;
            if (remaining_sec < 0) remaining_sec = 0;
            int h = remaining_sec / 3600;
            int m = (remaining_sec % 3600) / 60;
            int s = remaining_sec % 60;
            lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", h, m, s);
            lv_bar_set_range(cook->bar_68, 0, 100);
            int progress = stop_back_progress(elapsed, cook_total_ms);
            if (progress > 100) progress = 100;
            lv_bar_set_value(cook->bar_68, progress, LV_ANIM_OFF);
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, cook->bar_68);
            lv_anim_set_exec_cb(&a, anim_bar_set_value);
            lv_anim_set_values(&a, progress, 100);
            lv_anim_set_time(&a, ((int)(cook_total_ms - (int)elapsed) < 0) ? 0 : (cook_total_ms - (int)elapsed));
            lv_anim_start(&a);
        } else {
            lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", set_hour, set_min, 0);
            lv_bar_set_range(cook->bar_68, 0, 100);
            lv_bar_set_value(cook->bar_68, 3, LV_ANIM_OFF);
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, cook->bar_68);
            lv_anim_set_exec_cb(&a, anim_bar_set_value);
            lv_anim_set_values(&a, 3, 100);
            lv_anim_set_time(&a, cook_total_ms);
            lv_anim_start(&a);
            cook_start_time = lv_tick_get();
            if (cook_timer) lv_timer_del(cook_timer);
            cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
        }
    }
    current_group = g_custom_cooking;
    lv_scr_load_anim(custom_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[lasagna] back to custom_cooking\n");
}

void custom_rebuild_setting(void)
{
    custom_setting_create(&ui_manager);
    custom_setting_t *set = custom_setting_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = { set->hour, set->min, set->sure };
        if (g_custom_setting) lv_group_del(g_custom_setting);
        g_custom_setting = group_create_for_page(btns, 3);

        edit_clear();
        edit_register(set->hour, set->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(set->min, set->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(set->hour, on_custom_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->min, on_custom_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_custom_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_custom_setting_sure_click,
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

        lv_obj_add_flag(set->hourline, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->minline, LV_OBJ_FLAG_HIDDEN);

        lv_group_focus_obj(set->hour);
    }
    current_group = g_custom_setting;
    lv_scr_load_anim(custom_setting_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[lasagna] back to custom_setting\n");
}

void custom_rebuild_stop(void)
{
    edit_clear();
    g_on_stop_back = 0;
    custom_stop_create(&ui_manager);
    custom_stop_t *stop = custom_stop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start, stop->little };
        if (g_custom_stop) lv_group_del(g_custom_stop);
        g_custom_stop = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(stop->start, on_custom_stop_start_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(stop->little, on_custom_cooking_setting_click,
                            LV_EVENT_CLICKED, NULL);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(stop->timelabel, "%02d:%02d:%02d", h, m, s);
        custom_set_status(stop->status, set_temp, set_hour, set_min);

        lv_bar_set_range(stop->bar_69, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_69, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_custom_stop;
    lv_scr_load_anim(custom_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[lasagna] back to custom_stop\n");
    g_send.iface_status = IFACE_PAUSE;
}

void custom_rebuild_stop_back(void)
{
    edit_clear();
    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_custom_complete;
    custom_stop_back_create(&ui_manager);
    custom_stop_back_t *back = custom_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_custom_stop_back) lv_group_del(g_custom_stop_back);
        g_custom_stop_back = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(back->sure, on_custom_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            if (g_keepwarm_active)
                lv_label_set_text(back->label_882, "保温中...");
            else
                lv_label_set_text(back->label_882, "已完成");
            lv_bar_set_value(back->bar_70, 100, LV_ANIM_OFF);
        }

        custom_set_status(back->status, set_temp, set_hour, set_min);
        lv_bar_set_range(back->bar_70, 0, 100);
        uint32_t elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
        int p = stop_back_progress(elapsed, cook_total_ms);
        if (p > 100) p = 100;
        lv_bar_set_range(back->bar_70, 0, 100);
        lv_bar_set_value(back->bar_70, p, LV_ANIM_OFF);
    }
    current_group = g_custom_stop_back;
    lv_scr_load_anim(custom_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[lasagna] back to custom_stop_back\n");
}

void custom_rebuild_complete(void)
{
    edit_clear();
    custom_complete_create(&ui_manager);
    current_group = g_custom_complete;
    lv_scr_load_anim(custom_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[lasagna] back to custom_complete\n");
}

void custom_complete_rebind(lv_obj_t *btn)
{
    lv_obj_add_event_cb(btn, on_custom_cooking_setting_click, LV_EVENT_CLICKED, NULL);
}
