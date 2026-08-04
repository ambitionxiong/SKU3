#include "protocol.h"
#include "nav.h"

static int probe_menu_top_saved;
static int probe_menu_low_saved;
int cook_start_probe = 0;

static inline int probe_progress(int probe_val) {
    int range = probe_target_temp - cook_start_probe;
    if (range <= 0) return 3;
    int p = 3 + (int)((int64_t)(probe_val - cook_start_probe) * 97 / range);
    if (p > 100) p = 100;
    if (p < 3) p = 3;
    return p;
}

static void on_updown_probe_next_click(lv_event_t *e);
static void on_updown_probe_set_sure_click(lv_event_t *e);
static void on_updown_probe_cooking_stop_click(lv_event_t *e);
static void on_updown_probe_cooking_setting_click(lv_event_t *e);
static void on_updown_probe_stop_start_click(lv_event_t *e);
static void on_updown_probe_stop_back_sure_click(lv_event_t *e);
static void on_updown_probe_uptemp_click(lv_event_t *e);
static void on_updown_probe_downtemp_click(lv_event_t *e);
static void on_updown_probe_top_next_click(lv_event_t *e);
static void on_updown_probe_low_next_click(lv_event_t *e);
static void on_updown_probe_edit_focus(lv_event_t *e);
static void on_updown_probe_complete_click(lv_event_t *e);

static void on_updown_probe_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_set_probe();
}

static void on_updown_probe_set_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_cooking_probe();
}

static void on_updown_probe_cooking_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_stop_probe();
}

static void on_updown_probe_cooking_setting_click(lv_event_t *e)
{
}

static void on_updown_probe_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        updown_bbq_probe_resume_cooking();
}

static void on_updown_probe_stop_back_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    g_on_stop_back = 0;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    probe_target_temp = 80; set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
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
    g_send.probe_temp = 0;
    printf("[updown_bbq_probe] stop_back sure -> major_menu\n");
}

static void on_updown_probe_uptemp_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_menu_top_probe();
}

static void on_updown_probe_downtemp_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_menu_low_probe();
}

static void on_updown_probe_top_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr)) {
        probe_menu_top_saved = set_temp_up;
        page_pop();
    }
}

static void on_updown_probe_low_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr)) {
        probe_menu_low_saved = set_temp_down;
        page_pop();
    }
}

static void on_updown_probe_edit_focus(lv_event_t *e)
{
    on_edit_focus(e);
}

static void on_updown_probe_complete_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        page_pop();
}

// ==============================
// Jump 函数
// ==============================

void jump_to_updown_bbq_menu_probe(void)
{
    set_temp = 180; set_temp_up = 180; set_temp_down = 180;
    set_hour = 0; set_min = 30; probe_target_temp = 80;
    page_push(PAGE_UPDOWN_BBQ_MENU_PROBE);
    lv_obj_clean(lv_scr_act());
    updown_bbq_menu_probe_create(&ui_manager);

    updown_bbq_menu_probe_t *menu = updown_bbq_menu_probe_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp,
            menu->probetemp,
            menu->next,
        };
        if (g_updown_bbq_menu_probe) lv_group_del(g_updown_bbq_menu_probe);
        g_updown_bbq_menu_probe = group_create_for_page(btns, 3);

        edit_clear();
        edit_register(menu->temp, menu->templine2, menu->templine3,
                      &set_temp, 30, 300, 5, "%d");
        edit_register(menu->probetemp, menu->probeline2, NULL,
                      &probe_target_temp, 30, 99, 1, "%d");

        lv_obj_add_event_cb(menu->temp, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->probetemp, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);

        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_updown_probe_next_click,
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
    current_group = g_updown_bbq_menu_probe;

    lv_scr_load_anim(updown_bbq_menu_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.cook_mode = MODE_UPDOWN_BBQ;
    printf("[updown_bbq_probe] jump: menu_probe\n");
}

void jump_to_updown_bbq_menu_top_probe(void)
{
    probe_menu_top_saved = set_temp_up;
    page_push(PAGE_UPDOWN_BBQ_MENU_TOP_PROBE);
    lv_obj_clean(lv_scr_act());
    updown_bbq_menu_top_probe_create(&ui_manager);

    updown_bbq_menu_top_probe_t *menu = updown_bbq_menu_top_probe_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp,
            menu->next,
        };
        if (g_updown_bbq_menu_top_probe) lv_group_del(g_updown_bbq_menu_top_probe);
        g_updown_bbq_menu_top_probe = group_create_for_page(btns, 2);

        edit_clear();
        edit_register(menu->temp, menu->line2, menu->line3,
                      &set_temp_up, 30, 300, 5, "%d");

        lv_obj_add_event_cb(menu->temp, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);

        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_updown_probe_top_next_click,
                                LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp_up);

        lv_obj_add_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp_up < 100) {
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        }

        if (menu->next)
            lv_group_focus_obj(menu->next);
    }
    current_group = g_updown_bbq_menu_top_probe;

    lv_scr_load_anim(updown_bbq_menu_top_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[updown_bbq_probe] jump: menu_top_probe\n");
}

void jump_to_updown_bbq_menu_low_probe(void)
{
    probe_menu_low_saved = set_temp_down;
    page_push(PAGE_UPDOWN_BBQ_MENU_LOW_PROBE);
    lv_obj_clean(lv_scr_act());
    updown_bbq_menu_low_probe_create(&ui_manager);

    updown_bbq_menu_low_probe_t *menu = updown_bbq_menu_low_probe_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp,
            menu->next,
        };
        if (g_updown_bbq_menu_low_probe) lv_group_del(g_updown_bbq_menu_low_probe);
        g_updown_bbq_menu_low_probe = group_create_for_page(btns, 2);

        edit_clear();
        edit_register(menu->temp, menu->line2, menu->line3,
                      &set_temp_down, 30, 300, 5, "%d");

        lv_obj_add_event_cb(menu->temp, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);

        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_updown_probe_low_next_click,
                                LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp_down);

        lv_obj_add_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp_down < 100) {
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        }

        if (menu->next)
            lv_group_focus_obj(menu->next);
    }
    current_group = g_updown_bbq_menu_low_probe;

    lv_scr_load_anim(updown_bbq_menu_low_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[updown_bbq_probe] jump: menu_low_probe\n");
}

void jump_to_updown_bbq_set_probe(void)
{
    page_push(PAGE_UPDOWN_BBQ_SET_PROBE);
    lv_obj_clean(lv_scr_act());
    updown_bbq_set_probe_create(&ui_manager);

    set_temp_up = set_temp; set_temp_down = set_temp;

    updown_bbq_set_probe_t *set = updown_bbq_set_probe_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->uptemp, set->downtemp,
            set->sure,
        };
        if (g_updown_bbq_set_probe) lv_group_del(g_updown_bbq_set_probe);
        g_updown_bbq_set_probe = group_create_for_page(btns, 3);
        clear_focus_states(btns, 3);
        lv_group_focus_obj(set->sure);

        lv_obj_add_flag(set->up2num, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up2dir, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up2icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down2num, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down2dir, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down2icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up3tempnum, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up3dir, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up3icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down3num, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down3dir, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down3icon, LV_OBJ_FLAG_HIDDEN);

        if (set_temp_up < 100) {
            lv_label_set_text_fmt(set->up2num, "%d", set_temp_up);
            lv_obj_clear_flag(set->up2num, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up2dir, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up2icon, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_label_set_text_fmt(set->up3tempnum, "%d", set_temp_up);
            lv_obj_clear_flag(set->up3tempnum, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up3dir, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up3icon, LV_OBJ_FLAG_HIDDEN);
        }
        if (set_temp_down < 100) {
            lv_label_set_text_fmt(set->down2num, "%d", set_temp_down);
            lv_obj_clear_flag(set->down2num, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down2dir, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down2icon, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_label_set_text_fmt(set->down3num, "%d", set_temp_down);
            lv_obj_clear_flag(set->down3num, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down3dir, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down3icon, LV_OBJ_FLAG_HIDDEN);
        }
        lv_label_set_text_fmt(set->probetemp, "%d", probe_target_temp);

        lv_obj_add_event_cb(set->uptemp, on_updown_probe_uptemp_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->downtemp, on_updown_probe_downtemp_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->sure, on_updown_probe_set_sure_click,
                            LV_EVENT_CLICKED, NULL);
    }
    current_group = g_updown_bbq_set_probe;

    lv_scr_load_anim(updown_bbq_set_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[updown_bbq_probe] jump: set_probe\n");
}

void jump_to_updown_bbq_cooking_probe(void)
{
    edit_clear();
    g_on_stop_back = 0;
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }

    updown_bbq_set_probe_t *set = updown_bbq_set_probe_get(&ui_manager);
    if (set) {
        const char *txt = lv_label_get_text(set->probetemp);
        probe_target_temp = txt ? atoi(txt) : 80;
    }

    page_push(PAGE_UPDOWN_BBQ_COOKING_PROBE);
    lv_obj_clean(lv_scr_act());
    updown_bbq_cooking_probe_create(&ui_manager);

    updown_bbq_cooking_probe_t *cook = updown_bbq_cooking_probe_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_updown_bbq_cooking_probe) lv_group_del(g_updown_bbq_cooking_probe);
        g_updown_bbq_cooking_probe = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(cook->stop, on_updown_probe_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        cook_start_probe = get_probe_temp();

        int min_temp = set_temp_up < set_temp_down ? set_temp_up : set_temp_down;
        lv_label_set_text_fmt(cook->status, "| 上下烧烤 | %d℃ | %d℃", min_temp, probe_target_temp);
        {int tp = get_probe_temp(); if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(cook->temp, "%d℃", tp);}

        lv_bar_set_range(cook->bar_1, 0, 100);
        lv_bar_set_value(cook->bar_1, 3, LV_ANIM_OFF);
    }

    g_send.cook_flag = 2;
    cook_start_time = lv_tick_get();
    cook_total_ms = 3600000;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_updown_bbq_cooking_probe;

    lv_scr_load_anim(updown_bbq_cooking_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp_up;
    g_send.set_temp_lower = set_temp_down;
    g_send.remaining_ms = 0;
    g_send.probe_temp = probe_target_temp;
    printf("[updown_bbq_probe] jump: set_probe -> cooking_probe\n");
}

void jump_to_updown_bbq_stop_probe(void)
{
    edit_clear();
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

    int probe_temp = get_probe_temp();
    cook_bar_saved = probe_progress(probe_temp);
    if (cook_bar_saved > 100) cook_bar_saved = 100;

    page_push(PAGE_UPDOWN_BBQ_STOP_PROBE);
    lv_obj_clean(lv_scr_act());
    updown_bbq_stop_probe_create(&ui_manager);

    updown_bbq_stop_probe_t *stop = updown_bbq_stop_probe_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start };
        if (g_updown_bbq_stop_probe) lv_group_del(g_updown_bbq_stop_probe);
        g_updown_bbq_stop_probe = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(stop->start, on_updown_probe_stop_start_click,
                            LV_EVENT_CLICKED, NULL);

        int min_stop_temp = set_temp_up < set_temp_down ? set_temp_up : set_temp_down;
        lv_label_set_text_fmt(stop->status, "| 上下烧烤 | %d℃ | %d℃", min_stop_temp, probe_target_temp);
        {int tp = probe_temp; if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(stop->temp, "%d℃", tp);}

        lv_bar_set_range(stop->bar_2, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_2, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_updown_bbq_stop_probe;

    lv_scr_load_anim(updown_bbq_stop_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = 0;
    printf("[updown_bbq_probe] jump: cooking_probe -> stop_probe\n");
}

void jump_to_updown_bbq_stop_back_probe(void)
{
    edit_clear();
    int probe_temp = get_probe_temp();
    int bar_val = probe_progress(probe_temp);
    if (bar_val > 100) bar_val = 100;

    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_updown_bbq_complete_probe;
    page_push(PAGE_UPDOWN_BBQ_STOP_BACK_PROBE);
    lv_obj_clean(lv_scr_act());
    updown_bbq_stop_back_probe_create(&ui_manager);

    updown_bbq_stop_back_probe_t *back = updown_bbq_stop_back_probe_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_updown_bbq_stop_back_probe) lv_group_del(g_updown_bbq_stop_back_probe);
        g_updown_bbq_stop_back_probe = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(back->sure, on_updown_probe_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        int min_back = set_temp_up < set_temp_down ? set_temp_up : set_temp_down;
        lv_label_set_text_fmt(back->status, "| 上下烧烤 | %d℃ | %d℃", min_back, probe_target_temp);

        lv_bar_set_range(back->bar_3, 0, 100);
        lv_bar_set_value(back->bar_3, bar_val, LV_ANIM_OFF);
        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(back->label_71, "烹饪中...");

        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            lv_label_set_text(back->label_71, "已完成");
            lv_bar_set_value(back->bar_3, 100, LV_ANIM_OFF);
        }
    }
    current_group = g_updown_bbq_stop_back_probe;

    lv_scr_load_anim(updown_bbq_stop_back_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[updown_bbq_probe] jump: stop_back_probe\n");
}

void jump_to_updown_bbq_complete_probe(void)
{
    edit_clear();
    if (depth > 0 && page_stack[depth - 1] == PAGE_UPDOWN_BBQ_STOP_BACK_PROBE)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_UPDOWN_BBQ_STOP_PROBE)
        depth--;
    page_push(PAGE_UPDOWN_BBQ_COMPLETE_PROBE);
    lv_obj_clean(lv_scr_act());
    updown_bbq_complete_probe_create(&ui_manager);

    {
        updown_bbq_complete_probe_t *complete = updown_bbq_complete_probe_get(&ui_manager);
        if (complete) {
            lv_obj_t *btns[] = { complete->image_31 };
            if (g_updown_bbq_complete_probe) lv_group_del(g_updown_bbq_complete_probe);
            g_updown_bbq_complete_probe = group_create_for_page(btns, 1);
            lv_obj_add_event_cb(complete->image_31, on_updown_probe_complete_click,
                                LV_EVENT_CLICKED, NULL);
            int min_back = set_temp_up < set_temp_down ? set_temp_up : set_temp_down;
            lv_label_set_text_fmt(complete->label_74, "| 上下烧烤 | %d℃ | %d℃", min_back, probe_target_temp);
            lv_bar_set_value(complete->bar_4, 100, LV_ANIM_OFF);
        }
    }
    current_group = g_updown_bbq_complete_probe;

    lv_scr_load_anim(updown_bbq_complete_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COMPLETE;
    g_send.cook_flag = 0;
    g_send.remaining_ms = 0;
    printf("[updown_bbq_probe] jump: complete_probe\n");
}

void updown_bbq_probe_resume_cooking(void)
{
    edit_clear();
    g_on_stop_back = 0;
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    if (depth > 1) depth--;
    lv_obj_clean(lv_scr_act());
    updown_bbq_cooking_probe_create(&ui_manager);

    updown_bbq_cooking_probe_t *cook = updown_bbq_cooking_probe_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_updown_bbq_cooking_probe) lv_group_del(g_updown_bbq_cooking_probe);
        g_updown_bbq_cooking_probe = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cook->stop, on_updown_probe_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        int min_temp = set_temp_up < set_temp_down ? set_temp_up : set_temp_down;
        lv_label_set_text_fmt(cook->status, "| 上下烧烤 | %d℃ | %d℃", min_temp, probe_target_temp);
        {int tp = get_probe_temp(); if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(cook->temp, "%d℃", tp);}

        lv_bar_set_range(cook->bar_1, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(cook->bar_1, cook_bar_saved, LV_ANIM_OFF);
    }

    cook_start_time = lv_tick_get() - cook_elapsed_saved;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_updown_bbq_cooking_probe;

    lv_scr_load_anim(updown_bbq_cooking_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp_up;
    g_send.set_temp_lower = set_temp_down;
    g_send.remaining_ms = 0;
    printf("[updown_bbq_probe] resume: stop_probe -> cooking_probe\n");
}

// ==============================
// Rebuild 函数（供 page_pop 调用）
// ==============================

void updown_bbq_probe_rebuild_menu(page_id_t child)
{
    updown_bbq_menu_probe_create(&ui_manager);
    updown_bbq_menu_probe_t *menu = updown_bbq_menu_probe_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp,
            menu->probetemp,
            menu->next,
        };
        if (g_updown_bbq_menu_probe) lv_group_del(g_updown_bbq_menu_probe);
        g_updown_bbq_menu_probe = group_create_for_page(btns, 3);

        edit_clear();
        edit_register(menu->temp, menu->templine2, menu->templine3,
                      &set_temp, 30, 300, 5, "%d");
        edit_register(menu->probetemp, menu->probeline2, NULL,
                      &probe_target_temp, 30, 99, 1, "%d");

        lv_obj_add_event_cb(menu->temp, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->probetemp, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_updown_probe_next_click,
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
    current_group = g_updown_bbq_menu_probe;
    lv_scr_load_anim(updown_bbq_menu_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[updown_bbq_probe] back to updown_bbq_menu_probe\n");
}

void updown_bbq_probe_rebuild_menu_top(page_id_t child)
{
    updown_bbq_menu_top_probe_create(&ui_manager);
    updown_bbq_menu_top_probe_t *menu = updown_bbq_menu_top_probe_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp,
            menu->next,
        };
        if (g_updown_bbq_menu_top_probe) lv_group_del(g_updown_bbq_menu_top_probe);
        g_updown_bbq_menu_top_probe = group_create_for_page(btns, 2);

        edit_clear();
        edit_register(menu->temp, menu->line2, menu->line3,
                      &set_temp_up, 30, 300, 5, "%d");

        lv_obj_add_event_cb(menu->temp, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_updown_probe_top_next_click,
                                LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp_up);

        lv_obj_add_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp_up < 100) {
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        }

        if (menu->next) lv_group_focus_obj(menu->next);
    }
    current_group = g_updown_bbq_menu_top_probe;
    lv_scr_load_anim(updown_bbq_menu_top_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[updown_bbq_probe] back to updown_bbq_menu_top_probe\n");
}

void updown_bbq_probe_rebuild_menu_low(page_id_t child)
{
    updown_bbq_menu_low_probe_create(&ui_manager);
    updown_bbq_menu_low_probe_t *menu = updown_bbq_menu_low_probe_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = {
            menu->temp,
            menu->next,
        };
        if (g_updown_bbq_menu_low_probe) lv_group_del(g_updown_bbq_menu_low_probe);
        g_updown_bbq_menu_low_probe = group_create_for_page(btns, 2);

        edit_clear();
        edit_register(menu->temp, menu->line2, menu->line3,
                      &set_temp_down, 30, 300, 5, "%d");

        lv_obj_add_event_cb(menu->temp, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_updown_probe_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        if (menu->next)
            lv_obj_add_event_cb(menu->next, on_updown_probe_low_next_click,
                                LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp_down);

        lv_obj_add_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp_down < 100) {
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        }

        if (menu->next) lv_group_focus_obj(menu->next);
    }
    current_group = g_updown_bbq_menu_low_probe;
    lv_scr_load_anim(updown_bbq_menu_low_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[updown_bbq_probe] back to updown_bbq_menu_low_probe\n");
}

void updown_bbq_probe_rebuild_set(page_id_t child)
{
    if (child == PAGE_UPDOWN_BBQ_MENU_TOP_PROBE)
        set_temp_up = probe_menu_top_saved;
    if (child == PAGE_UPDOWN_BBQ_MENU_LOW_PROBE)
        set_temp_down = probe_menu_low_saved;

    updown_bbq_set_probe_create(&ui_manager);
    updown_bbq_set_probe_t *set = updown_bbq_set_probe_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->uptemp, set->downtemp,
            set->sure,
        };
        if (g_updown_bbq_set_probe) lv_group_del(g_updown_bbq_set_probe);
        g_updown_bbq_set_probe = group_create_for_page(btns, 3);
        clear_focus_states(btns, 3);
        lv_group_focus_obj(set->sure);

        lv_obj_add_flag(set->up2num, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up2dir, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up2icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down2num, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down2dir, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down2icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up3tempnum, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up3dir, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up3icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down3num, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down3dir, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down3icon, LV_OBJ_FLAG_HIDDEN);

        if (set_temp_up < 100) {
            lv_label_set_text_fmt(set->up2num, "%d", set_temp_up);
            lv_obj_clear_flag(set->up2num, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up2dir, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up2icon, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_label_set_text_fmt(set->up3tempnum, "%d", set_temp_up);
            lv_obj_clear_flag(set->up3tempnum, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up3dir, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up3icon, LV_OBJ_FLAG_HIDDEN);
        }
        if (set_temp_down < 100) {
            lv_label_set_text_fmt(set->down2num, "%d", set_temp_down);
            lv_obj_clear_flag(set->down2num, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down2dir, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down2icon, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_label_set_text_fmt(set->down3num, "%d", set_temp_down);
            lv_obj_clear_flag(set->down3num, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down3dir, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down3icon, LV_OBJ_FLAG_HIDDEN);
        }
        lv_label_set_text_fmt(set->probetemp, "%d", probe_target_temp);

        lv_obj_add_event_cb(set->uptemp, on_updown_probe_uptemp_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->downtemp, on_updown_probe_downtemp_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->sure, on_updown_probe_set_sure_click,
                            LV_EVENT_CLICKED, NULL);

        if (child == PAGE_UPDOWN_BBQ_MENU_TOP_PROBE && set->uptemp)
            lv_group_focus_obj(set->uptemp);
        else if (child == PAGE_UPDOWN_BBQ_MENU_LOW_PROBE && set->downtemp)
            lv_group_focus_obj(set->downtemp);
        else if (child == PAGE_UPDOWN_BBQ_COOKING_PROBE && set->sure)
            lv_group_focus_obj(set->sure);
    }
    current_group = g_updown_bbq_set_probe;
    lv_scr_load_anim(updown_bbq_set_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[updown_bbq_probe] back to updown_bbq_set_probe\n");
}

void updown_bbq_probe_rebuild_cooking(page_id_t child)
{
    edit_clear();
    g_on_stop_back = 0;
    updown_bbq_cooking_probe_create(&ui_manager);
    updown_bbq_cooking_probe_t *cook = updown_bbq_cooking_probe_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_updown_bbq_cooking_probe) lv_group_del(g_updown_bbq_cooking_probe);
        g_updown_bbq_cooking_probe = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cook->stop, on_updown_probe_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        int min_temp = set_temp_up < set_temp_down ? set_temp_up : set_temp_down;
        lv_label_set_text_fmt(cook->status, "| 上下烧烤 | %d℃ | %d℃", min_temp, probe_target_temp);
        {int tp = get_probe_temp(); if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(cook->temp, "%d℃", tp);}

        lv_bar_set_range(cook->bar_1, 0, 100);
        int bar_val = probe_progress(get_probe_temp());
        if (bar_val > 100) bar_val = 100;
        lv_bar_set_value(cook->bar_1, bar_val, LV_ANIM_OFF);
    }
    current_group = g_updown_bbq_cooking_probe;
    lv_scr_load_anim(updown_bbq_cooking_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[updown_bbq_probe] back to updown_bbq_cooking_probe\n");
}

void updown_bbq_probe_rebuild_setting(void)
{
    updown_bbq_set_probe_create(&ui_manager);
    updown_bbq_set_probe_t *set = updown_bbq_set_probe_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->uptemp, set->downtemp,
            set->sure,
        };
        if (g_updown_bbq_set_probe) lv_group_del(g_updown_bbq_set_probe);
        g_updown_bbq_set_probe = group_create_for_page(btns, 3);
        clear_focus_states(btns, 3);
        lv_group_focus_obj(set->sure);

        lv_obj_add_flag(set->up2num, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up2dir, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up2icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down2num, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down2dir, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down2icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up3tempnum, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up3dir, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up3icon, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down3num, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down3dir, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down3icon, LV_OBJ_FLAG_HIDDEN);

        if (set_temp_up < 100) {
            lv_label_set_text_fmt(set->up2num, "%d", set_temp_up);
            lv_obj_clear_flag(set->up2num, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up2dir, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up2icon, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_label_set_text_fmt(set->up3tempnum, "%d", set_temp_up);
            lv_obj_clear_flag(set->up3tempnum, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up3dir, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up3icon, LV_OBJ_FLAG_HIDDEN);
        }
        if (set_temp_down < 100) {
            lv_label_set_text_fmt(set->down2num, "%d", set_temp_down);
            lv_obj_clear_flag(set->down2num, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down2dir, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down2icon, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_label_set_text_fmt(set->down3num, "%d", set_temp_down);
            lv_obj_clear_flag(set->down3num, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down3dir, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down3icon, LV_OBJ_FLAG_HIDDEN);
        }
        lv_label_set_text_fmt(set->probetemp, "%d", probe_target_temp);

        lv_obj_add_event_cb(set->uptemp, on_updown_probe_uptemp_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->downtemp, on_updown_probe_downtemp_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->sure, on_updown_probe_set_sure_click,
                            LV_EVENT_CLICKED, NULL);
    }
    current_group = g_updown_bbq_set_probe;
    lv_scr_load_anim(updown_bbq_set_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[updown_bbq_probe] back to updown_bbq_set_probe\n");
}

void updown_bbq_probe_rebuild_stop(void)
{
    edit_clear();
    g_on_stop_back = 0;
    updown_bbq_stop_probe_create(&ui_manager);
    updown_bbq_stop_probe_t *stop = updown_bbq_stop_probe_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start };
        if (g_updown_bbq_stop_probe) lv_group_del(g_updown_bbq_stop_probe);
        g_updown_bbq_stop_probe = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(stop->start, on_updown_probe_stop_start_click,
                            LV_EVENT_CLICKED, NULL);

        int min_rstop = set_temp_up < set_temp_down ? set_temp_up : set_temp_down;
        lv_label_set_text_fmt(stop->status, "| 上下烧烤 | %d℃ | %d℃", min_rstop, probe_target_temp);
        {int tp = get_probe_temp(); if (tp > probe_target_temp) tp = probe_target_temp; lv_label_set_text_fmt(stop->temp, "%d℃", tp);}

        lv_bar_set_range(stop->bar_2, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_2, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_updown_bbq_stop_probe;
    lv_scr_load_anim(updown_bbq_stop_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[updown_bbq_probe] back to updown_bbq_stop_probe\n");
    g_send.iface_status = IFACE_PAUSE;
}

void updown_bbq_probe_rebuild_stop_back(void)
{
    edit_clear();
    int probe_temp = get_probe_temp();
    int bar_val = probe_progress(probe_temp);
    if (bar_val > 100) bar_val = 100;

    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_updown_bbq_complete_probe;
    updown_bbq_stop_back_probe_create(&ui_manager);
    updown_bbq_stop_back_probe_t *back = updown_bbq_stop_back_probe_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_updown_bbq_stop_back_probe) lv_group_del(g_updown_bbq_stop_back_probe);
        g_updown_bbq_stop_back_probe = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(back->sure, on_updown_probe_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        int min_back = set_temp_up < set_temp_down ? set_temp_up : set_temp_down;
        lv_label_set_text_fmt(back->status, "| 上下烧烤 | %d℃ | %d℃", min_back, probe_target_temp);

        lv_bar_set_range(back->bar_3, 0, 100);
        lv_bar_set_value(back->bar_3, bar_val, LV_ANIM_OFF);

        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(back->label_71, "烹饪中...");

        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            lv_label_set_text(back->label_71, "已完成");
            lv_bar_set_value(back->bar_3, 100, LV_ANIM_OFF);
        }
    }
    current_group = g_updown_bbq_stop_back_probe;
    lv_scr_load_anim(updown_bbq_stop_back_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[updown_bbq_probe] back to updown_bbq_stop_back_probe\n");
}

void updown_bbq_probe_rebuild_complete(void)
{
    edit_clear();
    updown_bbq_complete_probe_create(&ui_manager);
    {
        updown_bbq_complete_probe_t *complete = updown_bbq_complete_probe_get(&ui_manager);
        if (complete) {
            lv_obj_t *btns[] = { complete->image_31 };
            if (g_updown_bbq_complete_probe) lv_group_del(g_updown_bbq_complete_probe);
            g_updown_bbq_complete_probe = group_create_for_page(btns, 1);
            lv_obj_add_event_cb(complete->image_31, on_updown_probe_complete_click,
                                LV_EVENT_CLICKED, NULL);
        }
    }
    current_group = g_updown_bbq_complete_probe;
    lv_scr_load_anim(updown_bbq_complete_probe_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[updown_bbq_probe] back to updown_bbq_complete_probe\n");
}

void updown_bbq_probe_complete_rebind(lv_obj_t *btn)
{
    lv_obj_add_event_cb(btn, on_updown_probe_complete_click, LV_EVENT_CLICKED, NULL);
}
