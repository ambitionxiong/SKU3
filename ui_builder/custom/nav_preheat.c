#include "nav.h"
#include "protocol.h"

static void on_preheat_menu_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_preheat_cooking();
}

static void on_preheat_cooking_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_preheat_stop();
}

static void on_preheat_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        preheat_resume_cooking();
}

static void on_preheat_complete_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    set_temp = 180;
    depth = 2;
    jump_to_cookmenu();
}

void jump_to_preheat_menu(void)
{
    g_send.cook_mode = MODE_PREHEAT;
    g_send.cook_flag = 1;

    page_push(PAGE_PREHEAT_MENU);
    lv_obj_clean(lv_scr_act());
    preheatmenu_create(&ui_manager);

    preheatmenu_t *menu = preheatmenu_get(&ui_manager);
    if (menu) {
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

        lv_group_focus_obj(menu->next);
    }
    current_group = g_preheat_menu;

    if (menu && menu->next)
        lv_obj_add_event_cb(menu->next, on_preheat_menu_next_click,
                            LV_EVENT_CLICKED, NULL);

    lv_scr_load_anim(preheatmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] cookmenu -> preheat_menu\n");
}

void jump_to_preheat_cooking(void)
{
    page_push(PAGE_PREHEAT_COOKING);
    lv_obj_clean(lv_scr_act());
    preheatcooking_create(&ui_manager);

#ifdef LV_USE_AIC_SIMULATOR
    g_sim_cavity_temp = 25;
#endif

    preheatcooking_t *cook = preheatcooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_preheat_cooking) lv_group_del(g_preheat_cooking);
        g_preheat_cooking = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cook->stop, on_preheat_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(cook->temp, "%d℃", set_temp);
        lv_bar_set_range(cook->bar_1, 0, 100);
        lv_bar_set_value(cook->bar_1, 0, LV_ANIM_OFF);
    }
    current_group = g_preheat_cooking;

    g_send.iface_status = IFACE_COOKING;

    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    lv_scr_load_anim(preheatcooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] menu -> preheat_cooking\n");
}

void jump_to_preheat_stop(void)
{
    page_push(PAGE_PREHEAT_STOP);
    lv_obj_clean(lv_scr_act());
    preheatstop_create(&ui_manager);

    preheatstop_t *stop = preheatstop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start };
        if (g_preheat_stop) lv_group_del(g_preheat_stop);
        g_preheat_stop = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(stop->start, on_preheat_stop_start_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(stop->temp, "%d℃", set_temp);
        lv_bar_set_range(stop->bar_2, 0, 100);
        uint16_t cavity = get_cavity_temp();
        int p = (cavity * 100) / (set_temp ? set_temp : 1);
        if (p > 100) p = 100;
        lv_bar_set_value(stop->bar_2, p, LV_ANIM_OFF);
    }
    current_group = g_preheat_stop;
    g_send.iface_status = IFACE_PAUSE;

    lv_scr_load_anim(preheatstop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] cooking -> preheat_stop\n");
}

void preheat_resume_cooking(void)
{
    depth--;
    lv_obj_clean(lv_scr_act());
    preheatcooking_create(&ui_manager);

    preheatcooking_t *cook = preheatcooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_preheat_cooking) lv_group_del(g_preheat_cooking);
        g_preheat_cooking = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cook->stop, on_preheat_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(cook->temp, "%d℃", set_temp);
        lv_bar_set_range(cook->bar_1, 0, 100);
        uint16_t cavity = get_cavity_temp();
        int p = (cavity * 100) / (set_temp ? set_temp : 1);
        if (p > 100) p = 100;
        lv_bar_set_value(cook->bar_1, p, LV_ANIM_OFF);
    }
    current_group = g_preheat_cooking;
    g_send.iface_status = IFACE_COOKING;

    lv_scr_load_anim(preheatcooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] resume: stop -> cooking\n");
}

void jump_to_preheat_complete(void)
{
    page_push(PAGE_PREHEAT_COMPLETE);
    lv_obj_clean(lv_scr_act());
    preheatcomplete_create(&ui_manager);

    {
        preheatcomplete_t *cook = preheatcomplete_get(&ui_manager);
        if (cook) {
            lv_obj_t *btns[] = { cook->sure };
            if (g_preheat_complete) lv_group_del(g_preheat_complete);
            g_preheat_complete = group_create_for_page(btns, 1);
            lv_obj_add_event_cb(cook->sure, on_preheat_complete_sure_click,
                                LV_EVENT_CLICKED, NULL);
        }
    }
    current_group = g_preheat_complete;
    g_send.iface_status = IFACE_COMPLETE;
    g_send.remaining_ms = 0;

    lv_scr_load_anim(preheatcomplete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] cooking -> preheat_complete\n");
}

void jump_to_preheat_stop_back(void)
{
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    depth = 2;
    lv_obj_clean(lv_scr_act());
    major_menu_create(&ui_manager);
    groups_create();
    bind_events();
    current_group = g_major_menu;
    lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] stop_back -> major_menu\n");
}
