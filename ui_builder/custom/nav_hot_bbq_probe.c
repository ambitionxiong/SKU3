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

static void on_hot_bbq_probe_menu_next_click(lv_event_t *e);
static void on_hot_bbq_probe_set_sure_click(lv_event_t *e);
static void on_hot_bbq_probe_cooking_stop_click(lv_event_t *e);
static void on_hot_bbq_probe_stop_start_click(lv_event_t *e);
static void on_hot_bbq_probe_stop_back_sure_click(lv_event_t *e);
static void on_hot_bbq_probe_edit_focus(lv_event_t *e);
static void on_hot_bbq_probe_complete_click(lv_event_t *e);

static void on_hot_bbq_probe_menu_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hot_bbq_set_probe();
}

static void on_hot_bbq_probe_set_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hot_bbq_cooking_probe();
}

static void on_hot_bbq_probe_cooking_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hot_bbq_stop_probe();
}

static void on_hot_bbq_probe_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        hot_bbq_probe_resume_cooking();
}

static void on_hot_bbq_probe_stop_back_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    g_on_stop_back = 0;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    probe_target_temp = 80; set_temp = 180; set_hour = 0; set_min = 30;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    if (is_probe_inserted()) {
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        jump_to_major_menu_tz();
    } else {
        depth = 2;
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
    g_send.cook_flag = 0;
    printf("[hot_bbq_probe] stop_back sure -> major_menu\n");
}

static void on_hot_bbq_probe_edit_focus(lv_event_t *e)
{
    on_edit_focus(e);
}

static void on_hot_bbq_probe_complete_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        page_pop();
}

// ==============================
// Jump 函数
// ==============================

void jump_to_hot_bbq_menu_probe(void)
{
    set_temp = 180; set_hour = 0; set_min = 30; probe_target_temp = 80;
    page_push(PAGE_HOT_BBQ_MENU_PROBE);
    lv_obj_clean(lv_scr_act());
    hot_bbq_menu_probe_create(&ui_manager);

    hot_bbq_menu_probe_t *menu = hot_bbq_menu_probe_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp,
            menu->probetemp,
            menu->next,
        };
        if (g_hot_bbq_menu_probe) lv_group_del(g_hot_bbq_menu_probe);
        g_hot_bbq_menu_probe = group_create_for_page(btns, 3);

        edit_clear();
        edit_register(menu->temp, menu->templine2, menu->templine3,
                      &set_temp, 30, 300, 5, "%d");
        edit_register(menu->probetemp, menu->probetempline, NULL,
                      &probe_target_temp, 30, 99, 1, "%d");

        lv_obj_add_event_cb(menu->temp, on_hot_bbq_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->probetemp, on_hot_bbq_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_hot_bbq_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);

        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_hot_bbq_probe_menu_next_click,
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
    current_group = g_hot_bbq_menu_probe;

    lv_scr_load_anim(hot_bbq_menu_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.cook_mode = MODE_HOT_BBQ;
    printf("[hot_bbq_probe] jump: menu_probe\n");
}

void jump_to_hot_bbq_set_probe(void)
{
    edit_clear();
    page_push(PAGE_HOT_BBQ_SET_PROBE);
    lv_obj_clean(lv_scr_act());
    hot_bbq_set_probe_create(&ui_manager);

    hot_bbq_set_probe_t *set = hot_bbq_set_probe_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->temp,
            set->probetemp,
            set->sure,
        };
        if (g_hot_bbq_set_probe) lv_group_del(g_hot_bbq_set_probe);
        g_hot_bbq_set_probe = group_create_for_page(btns, 3);
        clear_focus_states(btns, 3);
        lv_group_focus_obj(set->sure);

        lv_label_set_text_fmt(set->temp, "%d", set_temp);
        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_fmt(set->probetemp, "%d", probe_target_temp);

        lv_obj_add_event_cb(set->temp, on_hot_bbq_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->probetemp, on_hot_bbq_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_hot_bbq_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);

        lv_obj_add_event_cb(set->sure, on_hot_bbq_probe_set_sure_click,
                            LV_EVENT_CLICKED, NULL);
    }
    current_group = g_hot_bbq_set_probe;

    lv_scr_load_anim(hot_bbq_set_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hot_bbq_probe] jump: set_probe\n");
}

void jump_to_hot_bbq_cooking_probe(void)
{
    edit_clear();
    g_on_stop_back = 0;
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }

    page_push(PAGE_HOT_BBQ_COOKING_PROBE);
    lv_obj_clean(lv_scr_act());
    hot_bbq_cooking_probe_create(&ui_manager);

    hot_bbq_cooking_probe_t *cook = hot_bbq_cooking_probe_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_hot_bbq_cooking_probe) lv_group_del(g_hot_bbq_cooking_probe);
        g_hot_bbq_cooking_probe = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(cook->stop, on_hot_bbq_probe_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        cook_start_probe = get_probe_temp();

        lv_label_set_text_fmt(cook->status, "| 热风烧烤 | %d℃ | %d℃", set_temp, probe_target_temp);
        {int tp = get_probe_temp(); if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(cook->temp, "%d℃", tp);}

        lv_bar_set_range(cook->bar_1, 0, 100);
        lv_bar_set_value(cook->bar_1, 3, LV_ANIM_OFF);
    }

    g_send.cook_flag = 2;
    cook_start_time = lv_tick_get();
    cook_total_ms = 3600000;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_hot_bbq_cooking_probe;

    lv_scr_load_anim(hot_bbq_cooking_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = cook_total_ms;
    printf("[hot_bbq_probe] jump: set_probe -> cooking_probe\n");
}

void jump_to_hot_bbq_stop_probe(void)
{
    edit_clear();
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

    int probe_temp = get_probe_temp();
    cook_bar_saved = probe_progress(probe_temp);
    if (cook_bar_saved > 100) cook_bar_saved = 100;

    page_push(PAGE_HOT_BBQ_STOP_PROBE);
    lv_obj_clean(lv_scr_act());
    hot_bbq_stop_probe_create(&ui_manager);

    hot_bbq_stop_probe_t *stop = hot_bbq_stop_probe_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start };
        if (g_hot_bbq_stop_probe) lv_group_del(g_hot_bbq_stop_probe);
        g_hot_bbq_stop_probe = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(stop->start, on_hot_bbq_probe_stop_start_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(stop->status, "| 热风烧烤 | %d℃ | %d℃", set_temp, probe_target_temp);
        {int tp = probe_temp; if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(stop->temp, "%d℃", tp);}

        lv_bar_set_range(stop->bar_2, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_2, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_hot_bbq_stop_probe;

    lv_scr_load_anim(hot_bbq_stop_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = (cook_total_ms > (int)cook_elapsed_saved) ? cook_total_ms - (int)cook_elapsed_saved : 0;
    printf("[hot_bbq_probe] jump: cooking_probe -> stop_probe\n");
}

void jump_to_hot_bbq_stop_back_probe(void)
{
    edit_clear();
    int probe_temp = get_probe_temp();
    int bar_val = probe_progress(probe_temp);
    if (bar_val > 100) bar_val = 100;

    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_hot_bbq_complete_probe;
    page_push(PAGE_HOT_BBQ_STOP_BACK_PROBE);
    lv_obj_clean(lv_scr_act());
    hot_bbq_stop_back_probe_create(&ui_manager);

    hot_bbq_stop_back_probe_t *back = hot_bbq_stop_back_probe_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_hot_bbq_stop_back_probe) lv_group_del(g_hot_bbq_stop_back_probe);
        g_hot_bbq_stop_back_probe = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(back->sure, on_hot_bbq_probe_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(back->status, "| 热风烧烤 | %d℃ | %d℃", set_temp, probe_target_temp);

        lv_bar_set_range(back->bar_3, 0, 100);
        lv_bar_set_value(back->bar_3, bar_val, LV_ANIM_OFF);
        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(back->label_39, "烹饪中...");

        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            lv_label_set_text(back->label_39, "已完成");
            lv_bar_set_value(back->bar_3, 100, LV_ANIM_OFF);
        }
    }
    current_group = g_hot_bbq_stop_back_probe;

    lv_scr_load_anim(hot_bbq_stop_back_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hot_bbq_probe] jump: stop_back_probe\n");
}

void jump_to_hot_bbq_complete_probe(void)
{
    edit_clear();
    if (depth > 0 && page_stack[depth - 1] == PAGE_HOT_BBQ_STOP_BACK_PROBE)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_HOT_BBQ_STOP_PROBE)
        depth--;
    page_push(PAGE_HOT_BBQ_COMPLETE_PROBE);
    lv_obj_clean(lv_scr_act());
    hot_bbq_complete_probe_create(&ui_manager);

    {
        hot_bbq_complete_probe_t *complete = hot_bbq_complete_probe_get(&ui_manager);
        if (complete) {
            lv_obj_t *btns[] = { complete->image_15 };
            if (g_hot_bbq_complete_probe) lv_group_del(g_hot_bbq_complete_probe);
            g_hot_bbq_complete_probe = group_create_for_page(btns, 1);
            lv_obj_add_event_cb(complete->image_15, on_hot_bbq_probe_complete_click,
                                LV_EVENT_CLICKED, NULL);
            lv_label_set_text_fmt(complete->status, "| 热风烧烤 | %d℃ | %d℃", set_temp, probe_target_temp);
            lv_bar_set_value(complete->bar_4, 100, LV_ANIM_OFF);
        }
    }
    current_group = g_hot_bbq_complete_probe;

    lv_scr_load_anim(hot_bbq_complete_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COMPLETE;
    g_send.cook_flag = 0;
    g_send.remaining_ms = 0;
    printf("[hot_bbq_probe] jump: complete_probe\n");
}

void hot_bbq_probe_resume_cooking(void)
{
    edit_clear();
    g_on_stop_back = 0;
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    if (depth > 1) depth--;
    lv_obj_clean(lv_scr_act());
    hot_bbq_cooking_probe_create(&ui_manager);

    hot_bbq_cooking_probe_t *cook = hot_bbq_cooking_probe_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_hot_bbq_cooking_probe) lv_group_del(g_hot_bbq_cooking_probe);
        g_hot_bbq_cooking_probe = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cook->stop, on_hot_bbq_probe_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(cook->status, "| 热风烧烤 | %d℃ | %d℃", set_temp, probe_target_temp);
        {int tp = get_probe_temp(); if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(cook->temp, "%d℃", tp);}

        lv_bar_set_range(cook->bar_1, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(cook->bar_1, cook_bar_saved, LV_ANIM_OFF);
    }

    cook_start_time = lv_tick_get() - cook_elapsed_saved;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_hot_bbq_cooking_probe;

    lv_scr_load_anim(hot_bbq_cooking_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = (cook_total_ms > (int)cook_elapsed_saved) ? cook_total_ms - (int)cook_elapsed_saved : 0;
    printf("[hot_bbq_probe] resume: stop_probe -> cooking_probe\n");
}

// ==============================
// Rebuild 函数（供 page_pop 调用）
// ==============================

void hot_bbq_probe_rebuild_menu(page_id_t child)
{
    hot_bbq_menu_probe_create(&ui_manager);
    hot_bbq_menu_probe_t *menu = hot_bbq_menu_probe_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp,
            menu->probetemp,
            menu->next,
        };
        if (g_hot_bbq_menu_probe) lv_group_del(g_hot_bbq_menu_probe);
        g_hot_bbq_menu_probe = group_create_for_page(btns, 3);

        edit_clear();
        edit_register(menu->temp, menu->templine2, menu->templine3,
                      &set_temp, 30, 300, 5, "%d");
        edit_register(menu->probetemp, menu->probetempline, NULL,
                      &probe_target_temp, 30, 99, 1, "%d");

        lv_obj_add_event_cb(menu->temp, on_hot_bbq_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->probetemp, on_hot_bbq_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_hot_bbq_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_hot_bbq_probe_menu_next_click,
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
    current_group = g_hot_bbq_menu_probe;
    lv_scr_load_anim(hot_bbq_menu_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hot_bbq_probe] back to hot_bbq_menu_probe\n");
}

void hot_bbq_probe_rebuild_set(page_id_t child)
{
    edit_clear();
    hot_bbq_set_probe_create(&ui_manager);
    hot_bbq_set_probe_t *set = hot_bbq_set_probe_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->temp,
            set->probetemp,
            set->sure,
        };
        if (g_hot_bbq_set_probe) lv_group_del(g_hot_bbq_set_probe);
        g_hot_bbq_set_probe = group_create_for_page(btns, 3);
        clear_focus_states(btns, 3);
        lv_group_focus_obj(set->sure);

        lv_label_set_text_fmt(set->temp, "%d", set_temp);
        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_fmt(set->probetemp, "%d", probe_target_temp);

        lv_obj_add_event_cb(set->temp, on_hot_bbq_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->probetemp, on_hot_bbq_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_hot_bbq_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);

        lv_obj_add_event_cb(set->sure, on_hot_bbq_probe_set_sure_click,
                            LV_EVENT_CLICKED, NULL);
    }
    current_group = g_hot_bbq_set_probe;
    lv_scr_load_anim(hot_bbq_set_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hot_bbq_probe] back to hot_bbq_set_probe\n");
}

void hot_bbq_probe_rebuild_cooking(page_id_t child)
{
    edit_clear();
    g_on_stop_back = 0;
    hot_bbq_cooking_probe_create(&ui_manager);
    hot_bbq_cooking_probe_t *cook = hot_bbq_cooking_probe_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_hot_bbq_cooking_probe) lv_group_del(g_hot_bbq_cooking_probe);
        g_hot_bbq_cooking_probe = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cook->stop, on_hot_bbq_probe_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(cook->status, "| 热风烧烤 | %d℃ | %d℃", set_temp, probe_target_temp);
        {int tp = get_probe_temp(); if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(cook->temp, "%d℃", tp);}

        lv_bar_set_range(cook->bar_1, 0, 100);
        int bar_val = probe_progress(get_probe_temp());
        if (bar_val > 100) bar_val = 100;
        lv_bar_set_value(cook->bar_1, bar_val, LV_ANIM_OFF);
    }
    current_group = g_hot_bbq_cooking_probe;
    lv_scr_load_anim(hot_bbq_cooking_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hot_bbq_probe] back to hot_bbq_cooking_probe\n");
}

void hot_bbq_probe_rebuild_stop(void)
{
    edit_clear();
    g_on_stop_back = 0;
    hot_bbq_stop_probe_create(&ui_manager);
    hot_bbq_stop_probe_t *stop = hot_bbq_stop_probe_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start };
        if (g_hot_bbq_stop_probe) lv_group_del(g_hot_bbq_stop_probe);
        g_hot_bbq_stop_probe = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(stop->start, on_hot_bbq_probe_stop_start_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(stop->status, "| 热风烧烤 | %d℃ | %d℃", set_temp, probe_target_temp);
        {int tp = get_probe_temp(); if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(stop->temp, "%d℃", tp);}

        lv_bar_set_range(stop->bar_2, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_2, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_hot_bbq_stop_probe;
    lv_scr_load_anim(hot_bbq_stop_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hot_bbq_probe] back to hot_bbq_stop_probe\n");
    g_send.iface_status = IFACE_PAUSE;
}

void hot_bbq_probe_rebuild_stop_back(void)
{
    edit_clear();
    int probe_temp = get_probe_temp();
    int bar_val = probe_progress(probe_temp);
    if (bar_val > 100) bar_val = 100;

    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_hot_bbq_complete_probe;
    hot_bbq_stop_back_probe_create(&ui_manager);
    hot_bbq_stop_back_probe_t *back = hot_bbq_stop_back_probe_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_hot_bbq_stop_back_probe) lv_group_del(g_hot_bbq_stop_back_probe);
        g_hot_bbq_stop_back_probe = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(back->sure, on_hot_bbq_probe_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        lv_bar_set_range(back->bar_3, 0, 100);
        lv_bar_set_value(back->bar_3, bar_val, LV_ANIM_OFF);

        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(back->label_39, "烹饪中...");

        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            lv_label_set_text(back->label_39, "已完成");
            lv_bar_set_value(back->bar_3, 100, LV_ANIM_OFF);
        }
    }
    current_group = g_hot_bbq_stop_back_probe;
    lv_scr_load_anim(hot_bbq_stop_back_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hot_bbq_probe] back to hot_bbq_stop_back_probe\n");
}

void hot_bbq_probe_rebuild_complete(void)
{
    edit_clear();
    hot_bbq_complete_probe_create(&ui_manager);
    {
        hot_bbq_complete_probe_t *complete = hot_bbq_complete_probe_get(&ui_manager);
        if (complete) {
            lv_obj_t *btns[] = { complete->image_15 };
            if (g_hot_bbq_complete_probe) lv_group_del(g_hot_bbq_complete_probe);
            g_hot_bbq_complete_probe = group_create_for_page(btns, 1);
            lv_obj_add_event_cb(complete->image_15, on_hot_bbq_probe_complete_click,
                                LV_EVENT_CLICKED, NULL);
        }
    }
    current_group = g_hot_bbq_complete_probe;
    lv_scr_load_anim(hot_bbq_complete_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hot_bbq_probe] back to hot_bbq_complete_probe\n");
}

void hot_bbq_probe_complete_rebind(lv_obj_t *btn)
{
    lv_obj_add_event_cb(btn, on_hot_bbq_probe_complete_click, LV_EVENT_CLICKED, NULL);
}
