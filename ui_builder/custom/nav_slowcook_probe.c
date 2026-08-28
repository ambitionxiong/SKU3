/*
 * nav_slowcook_probe.c - 慢煮探针版
 * 探针模式下慢煮全流程，进度按探针目标温度驱动。
 */
#include "protocol.h"
#include "nav.h"

static inline int probe_progress(int probe_val) {
    int range = probe_target_temp - cook_start_probe;
    if (range <= 0) return 3;
    int p = 3 + (int)((int64_t)(probe_val - cook_start_probe) * 97 / range);
    if (p > 100) p = 100;
    if (p < 3) p = 3;
    return p;
}

static void on_slowcook_probe_next_click(lv_event_t *e);
static void on_slowcook_probe_set_sure_click(lv_event_t *e);
static void on_slowcook_probe_cooking_stop_click(lv_event_t *e);
static void on_slowcook_probe_stop_start_click(lv_event_t *e);
static void on_slowcook_probe_stop_back_sure_click(lv_event_t *e);
static void on_slowcook_probe_edit_focus(lv_event_t *e);
static void on_slowcook_probe_complete_click(lv_event_t *e);
static void on_slowcook_probe_delay_toggle(lv_event_t *e);

static void on_slowcook_probe_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_slowcook_set_probe();
}

static void on_slowcook_probe_set_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr)) {
        if (delay_on)
            jump_to_delaycooking();
        else
            jump_to_slowcook_cooking_probe();
    }
}

static void on_slowcook_probe_cooking_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_slowcook_stop_probe();
}

static void on_slowcook_probe_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        slowcook_probe_resume_cooking();
}

static void on_slowcook_probe_stop_back_sure_click(lv_event_t *e)
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
    probe_target_temp = 80; set_temp = 80; set_hour = 0; set_min = 30;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    if (is_probe_inserted()) {
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        jump_to_major_menu_tz();
    } else {
        depth = 2;
        page_stack[1] = PAGE_MAJOR_MENU;   /* 探针拔出后回普通主菜单，修正栈残留 */
        lv_obj_clean(lv_scr_act());
        major_menu_create(&ui_manager);
        groups_create();
        bind_events();
        current_group = g_major_menu;
        lang_scr_load_anim(major_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
    }
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    g_send.cook_flag = 0;
    g_send.probe_temp = 0;
    printf("[slowcook_probe] stop_back sure -> major_menu\n");
}

static void on_slowcook_probe_edit_focus(lv_event_t *e)
{
    on_edit_focus(e);
}

static void on_slowcook_probe_delay_toggle(lv_event_t *e)
{
    slowcook_set_probe_t *set = slowcook_set_probe_get(&ui_manager);
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

static void on_slowcook_probe_complete_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        page_pop();
}

// ==============================
// Jump 函数
// ==============================

void jump_to_slowcook_menu_probe(void)
{
    set_temp = 80;
    set_hour = 0; set_min = 30; probe_target_temp = 80;
    page_push(PAGE_SLOWCOOK_MENU_PROBE);
    lv_obj_clean(lv_scr_act());
    slowcook_menu_probe_create(&ui_manager);

    slowcook_menu_probe_t *menu = slowcook_menu_probe_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp,
            menu->probetemp,
            menu->next,
        };
        if (g_slowcook_menu_probe) lv_group_del(g_slowcook_menu_probe);
        g_slowcook_menu_probe = group_create_for_page(btns, 3);

        edit_clear();
        edit_register(menu->temp, menu->templine2, menu->templine3,
                      &set_temp, 70, 120, 5, "%d");
        edit_register(menu->probetemp, menu->probetempline, NULL,
                      &probe_target_temp, 30, 99, 1, "%d");

        lv_obj_add_event_cb(menu->temp, on_slowcook_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->probetemp, on_slowcook_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_slowcook_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);

        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_slowcook_probe_next_click,
                                LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp);
        lv_label_set_text_fmt(menu->probetemp, "%d", probe_target_temp);

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
    current_group = g_slowcook_menu_probe;

    lang_scr_load_anim(slowcook_menu_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.cook_mode = MODE_SLOWCOOK;
    printf("[slowcook_probe] jump: menu_probe\n");
}

void jump_to_slowcook_set_probe(void)
{
    edit_clear();
    delay_on = 0;
    page_push(PAGE_SLOWCOOK_SET_PROBE);
    lv_obj_clean(lv_scr_act());
    slowcook_set_probe_create(&ui_manager);

    slowcook_set_probe_t *set = slowcook_set_probe_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = { set->sure, set->offdelay, set->ondelay };
        if (g_slowcook_set_probe) lv_group_del(g_slowcook_set_probe);
        g_slowcook_set_probe = group_create_for_page(btns, 3);
        clear_focus_states(btns, 3);

        lv_label_set_text_fmt(set->temp, "%d", set_temp);
        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_fmt(set->probetemp, "%d", probe_target_temp);

        apply_toggle_state(set->offdelay, set->ondelay, delay_on);
        mode_set_apply_delay_label(set->ondelay);

        lv_obj_add_event_cb(set->sure, on_slowcook_probe_set_sure_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offdelay, on_slowcook_probe_delay_toggle,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->ondelay, on_slowcook_probe_delay_toggle,
                            LV_EVENT_CLICKED, NULL);

        lv_group_focus_obj(set->sure);
    }
    current_group = g_slowcook_set_probe;

    lang_scr_load_anim(slowcook_set_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[slowcook_probe] jump: set_probe\n");
}

void jump_to_slowcook_cooking_probe(void)
{
    fav_snapshot_save();   /* 收藏:进入 cooking 时快照初始参数 */
    edit_clear();
    g_on_stop_back = 0;
    g_keepwarm_active = 0;
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }

    slowcook_set_probe_t *set = slowcook_set_probe_get(&ui_manager);
    if (set) {
        const char *txt = lv_label_get_text(set->probetemp);
        probe_target_temp = txt ? atoi(txt) : 80;
    }

    page_push(PAGE_SLOWCOOK_COOKING_PROBE);
    lv_obj_clean(lv_scr_act());
    slowcook_cooking_probe_create(&ui_manager);

    slowcook_cooking_probe_t *cook = slowcook_cooking_probe_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_slowcook_cooking_probe) lv_group_del(g_slowcook_cooking_probe);
        g_slowcook_cooking_probe = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(cook->stop, on_slowcook_probe_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        cook_start_probe = get_probe_temp();

        lv_label_set_text_fmt(cook->status, tr("| 慢煮 | %d℃ | %d℃"), set_temp, probe_target_temp);
        {int tp = get_probe_temp(); if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(cook->temp, tr("%d℃"), tp);}

        lv_bar_set_range(cook->bar_9, 0, 100);
        lv_bar_set_value(cook->bar_9, 3, LV_ANIM_OFF);
    }

    g_send.cook_flag = 2;
    cook_start_time = lv_tick_get();
    cook_total_ms = 3600000;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_slowcook_cooking_probe;

    lang_scr_load_anim(slowcook_cooking_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = 0;
    g_send.probe_temp = probe_target_temp;
    printf("[slowcook_probe] jump: set_probe -> cooking_probe\n");
}

void jump_to_slowcook_stop_probe(void)
{
    edit_clear();
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

    int probe_temp = get_probe_temp();
    cook_bar_saved = probe_progress(probe_temp);
    if (cook_bar_saved > 100) cook_bar_saved = 100;

    page_push(PAGE_SLOWCOOK_STOP_PROBE);
    lv_obj_clean(lv_scr_act());
    slowcook_stop_probe_create(&ui_manager);

    slowcook_stop_probe_t *stop = slowcook_stop_probe_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start };
        if (g_slowcook_stop_probe) lv_group_del(g_slowcook_stop_probe);
        g_slowcook_stop_probe = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(stop->start, on_slowcook_probe_stop_start_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(stop->label_101, tr("| 慢煮 | %d℃ | %d℃"), set_temp, probe_target_temp);
        {int tp = probe_temp; if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(stop->temp, tr("%d℃"), tp);}

        lv_bar_set_range(stop->bar_10, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_10, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_slowcook_stop_probe;

    lang_scr_load_anim(slowcook_stop_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = 0;
    printf("[slowcook_probe] jump: cooking_probe -> stop_probe\n");
}

void jump_to_slowcook_stop_back_probe(void)
{
    edit_clear();
    int probe_temp = get_probe_temp();
    int bar_val = probe_progress(probe_temp);
    if (bar_val > 100) bar_val = 100;

    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_slowcook_complete_probe;
    page_push(PAGE_SLOWCOOK_STOP_BACK_PROBE);
    lv_obj_clean(lv_scr_act());
    slowcook_stop_back_probe_create(&ui_manager);

    slowcook_stop_back_probe_t *back = slowcook_stop_back_probe_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_slowcook_stop_back_probe) lv_group_del(g_slowcook_stop_back_probe);
        g_slowcook_stop_back_probe = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(back->sure, on_slowcook_probe_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(back->status, tr("| 慢煮 | %d℃ | %d℃"), set_temp, probe_target_temp);

        lv_bar_set_range(back->bar_11, 0, 100);
        lv_bar_set_value(back->bar_11, bar_val, LV_ANIM_OFF);
        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(back->label_105, tr("烹饪中..."));

        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            lv_label_set_text(back->label_105, tr("已完成"));
            lv_bar_set_value(back->bar_11, 100, LV_ANIM_OFF);
        }
        if (g_delay_cancel_to_stop_back) {
            g_delay_cancel_to_stop_back = 0;
            lv_label_set_text(back->label_105, tr("预约中..."));
            lv_label_set_text(back->label_107, g_delay_cancel_btn ? tr("回到上一页") : tr("回到主页"));
            lv_obj_add_flag(back->bar_11, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(back->image_45, LV_OBJ_FLAG_HIDDEN);
        }

    }
    current_group = g_slowcook_stop_back_probe;

    lang_scr_load_anim(slowcook_stop_back_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[slowcook_probe] jump: stop_back_probe\n");
}

void jump_to_slowcook_complete_probe(void)
{
    edit_clear();
    if (depth > 0 && page_stack[depth - 1] == PAGE_SLOWCOOK_STOP_BACK_PROBE)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_SLOWCOOK_STOP_PROBE)
        depth--;
    page_push(PAGE_SLOWCOOK_COMPLETE_PROBE);
    lv_obj_clean(lv_scr_act());
    slowcook_complete_probe_create(&ui_manager);

    {
        slowcook_complete_probe_t *complete = slowcook_complete_probe_get(&ui_manager);
        if (complete) {
            lv_obj_t *btns[] = { complete->image_48 };
            if (g_slowcook_complete_probe) lv_group_del(g_slowcook_complete_probe);
            g_slowcook_complete_probe = group_create_for_page(btns, 1);
            lv_obj_add_event_cb(complete->image_48, on_slowcook_probe_complete_click,
                                LV_EVENT_CLICKED, NULL);
            lv_label_set_text_fmt(complete->status, tr("| 慢煮 | %d℃ | %d℃"), set_temp, probe_target_temp);
            lv_bar_set_value(complete->bar_12, 100, LV_ANIM_OFF);
        }
    }
    current_group = g_slowcook_complete_probe;

    lang_scr_load_anim(slowcook_complete_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COMPLETE;
    g_send.cook_flag = 0;
    g_send.remaining_ms = 0;
    printf("[slowcook_probe] jump: complete_probe\n");
}

void slowcook_probe_resume_cooking(void)
{
    edit_clear();
    g_on_stop_back = 0;
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    if (depth > 1) depth--;
    lv_obj_clean(lv_scr_act());
    slowcook_cooking_probe_create(&ui_manager);

    slowcook_cooking_probe_t *cook = slowcook_cooking_probe_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_slowcook_cooking_probe) lv_group_del(g_slowcook_cooking_probe);
        g_slowcook_cooking_probe = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cook->stop, on_slowcook_probe_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(cook->status, tr("| 慢煮 | %d℃ | %d℃"), set_temp, probe_target_temp);
        {int tp = get_probe_temp(); if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(cook->temp, tr("%d℃"), tp);}

        lv_bar_set_range(cook->bar_9, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(cook->bar_9, cook_bar_saved, LV_ANIM_OFF);
    }

    cook_start_time = lv_tick_get() - cook_elapsed_saved;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_slowcook_cooking_probe;

    lang_scr_load_anim(slowcook_cooking_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = 0;
    printf("[slowcook_probe] resume: stop_probe -> cooking_probe\n");
}

// ==============================
// Rebuild 函数（供 page_pop 调用）
// ==============================

void slowcook_probe_rebuild_menu(page_id_t child)
{
    slowcook_menu_probe_create(&ui_manager);
    slowcook_menu_probe_t *menu = slowcook_menu_probe_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp,
            menu->probetemp,
            menu->next,
        };
        if (g_slowcook_menu_probe) lv_group_del(g_slowcook_menu_probe);
        g_slowcook_menu_probe = group_create_for_page(btns, 3);

        edit_clear();
        edit_register(menu->temp, menu->templine2, menu->templine3,
                      &set_temp, 70, 120, 5, "%d");
        edit_register(menu->probetemp, menu->probetempline, NULL,
                      &probe_target_temp, 30, 99, 1, "%d");

        lv_obj_add_event_cb(menu->temp, on_slowcook_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->probetemp, on_slowcook_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_slowcook_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_slowcook_probe_next_click,
                                LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp);
        lv_label_set_text_fmt(menu->probetemp, "%d", probe_target_temp);

        lv_obj_add_flag(menu->templine3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->templine2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(menu->templine2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(menu->templine3, LV_OBJ_FLAG_HIDDEN);

        validate_constraints();
        if (menu->next) lv_group_focus_obj(menu->next);
    }
    current_group = g_slowcook_menu_probe;
    lang_scr_load_anim(slowcook_menu_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[slowcook_probe] back to slowcook_menu_probe\n");
}

void slowcook_probe_rebuild_set(page_id_t child)
{
    edit_clear();
    slowcook_set_probe_create(&ui_manager);
    slowcook_set_probe_t *set = slowcook_set_probe_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = { set->sure, set->offdelay, set->ondelay };
        if (g_slowcook_set_probe) lv_group_del(g_slowcook_set_probe);
        g_slowcook_set_probe = group_create_for_page(btns, 3);
        clear_focus_states(btns, 3);

        lv_label_set_text_fmt(set->temp, "%d", set_temp);
        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_fmt(set->probetemp, "%d", probe_target_temp);

        apply_toggle_state(set->offdelay, set->ondelay, delay_on);
        mode_set_apply_delay_label(set->ondelay);

        lv_obj_add_event_cb(set->sure, on_slowcook_probe_set_sure_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->offdelay, on_slowcook_probe_delay_toggle,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->ondelay, on_slowcook_probe_delay_toggle,
                            LV_EVENT_CLICKED, NULL);
        if (child == PAGE_DELAYSET && set->offdelay)
            lv_group_focus_obj(delay_on ? set->ondelay : set->offdelay);

        lv_group_focus_obj(set->sure);
    }
    current_group = g_slowcook_set_probe;
    lang_scr_load_anim(slowcook_set_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[slowcook_probe] back to slowcook_set_probe\n");
}

void slowcook_probe_rebuild_cooking(page_id_t child)
{
    edit_clear();
    g_on_stop_back = 0;
    slowcook_cooking_probe_create(&ui_manager);
    slowcook_cooking_probe_t *cook = slowcook_cooking_probe_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_slowcook_cooking_probe) lv_group_del(g_slowcook_cooking_probe);
        g_slowcook_cooking_probe = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cook->stop, on_slowcook_probe_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(cook->status, tr("| 慢煮 | %d℃ | %d℃"), set_temp, probe_target_temp);
        {int tp = get_probe_temp(); if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(cook->temp, tr("%d℃"), tp);}

        lv_bar_set_range(cook->bar_9, 0, 100);
        int bar_val = probe_progress(get_probe_temp());
        if (bar_val > 100) bar_val = 100;
        lv_bar_set_value(cook->bar_9, bar_val, LV_ANIM_OFF);
    }
    current_group = g_slowcook_cooking_probe;
    lang_scr_load_anim(slowcook_cooking_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[slowcook_probe] back to slowcook_cooking_probe\n");
}

void slowcook_probe_rebuild_stop(void)
{
    edit_clear();
    g_on_stop_back = 0;
    slowcook_stop_probe_create(&ui_manager);
    slowcook_stop_probe_t *stop = slowcook_stop_probe_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start };
        if (g_slowcook_stop_probe) lv_group_del(g_slowcook_stop_probe);
        g_slowcook_stop_probe = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(stop->start, on_slowcook_probe_stop_start_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(stop->label_101, tr("| 慢煮 | %d℃ | %d℃"), set_temp, probe_target_temp);
        {int tp = get_probe_temp(); if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(stop->temp, tr("%d℃"), tp);}

        lv_bar_set_range(stop->bar_10, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_10, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_slowcook_stop_probe;
    lang_scr_load_anim(slowcook_stop_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[slowcook_probe] back to slowcook_stop_probe\n");
    g_send.iface_status = IFACE_PAUSE;
}

void slowcook_probe_rebuild_stop_back(void)
{
    edit_clear();
    int probe_temp = get_probe_temp();
    int bar_val = probe_progress(probe_temp);
    if (bar_val > 100) bar_val = 100;

    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_slowcook_complete_probe;
    slowcook_stop_back_probe_create(&ui_manager);
    slowcook_stop_back_probe_t *back = slowcook_stop_back_probe_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_slowcook_stop_back_probe) lv_group_del(g_slowcook_stop_back_probe);
        g_slowcook_stop_back_probe = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(back->sure, on_slowcook_probe_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        lv_bar_set_range(back->bar_11, 0, 100);
        lv_bar_set_value(back->bar_11, bar_val, LV_ANIM_OFF);

        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(back->label_105, tr("烹饪中..."));

        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            lv_label_set_text(back->label_105, tr("已完成"));
            lv_bar_set_value(back->bar_11, 100, LV_ANIM_OFF);
        }
    }
    current_group = g_slowcook_stop_back_probe;
    lang_scr_load_anim(slowcook_stop_back_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[slowcook_probe] back to slowcook_stop_back_probe\n");
}

void slowcook_probe_rebuild_complete(void)
{
    edit_clear();
    slowcook_complete_probe_create(&ui_manager);
    {
        slowcook_complete_probe_t *complete = slowcook_complete_probe_get(&ui_manager);
        if (complete) {
            lv_obj_t *btns[] = { complete->image_48 };
            if (g_slowcook_complete_probe) lv_group_del(g_slowcook_complete_probe);
            g_slowcook_complete_probe = group_create_for_page(btns, 1);
            lv_obj_add_event_cb(complete->image_48, on_slowcook_probe_complete_click,
                                LV_EVENT_CLICKED, NULL);
        }
    }
    current_group = g_slowcook_complete_probe;
    lang_scr_load_anim(slowcook_complete_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[slowcook_probe] back to slowcook_complete_probe\n");
}

void slowcook_probe_complete_rebind(lv_obj_t *btn)
{
    lv_obj_add_event_cb(btn, on_slowcook_probe_complete_click, LV_EVENT_CLICKED, NULL);
}
