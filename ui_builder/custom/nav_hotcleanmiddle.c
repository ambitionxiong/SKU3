#include "protocol.h"
#include "nav.h"

static void on_hcm_set_start_click(lv_event_t *e);
static void on_hcm_cooking_stop_click(lv_event_t *e);
static void on_hcm_stop_start_click(lv_event_t *e);
static void on_hcm_stop_back_sure_click(lv_event_t *e);
void jump_to_hcm_cooking(void);
void jump_to_hcm_complete(void);

static void on_hcm_set_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hcm_cooking();
}

static void on_hcm_cooking_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hcm_stop();
}

static void on_hcm_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        hcm_resume_cooking();
}

static void on_hcm_stop_back_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    set_hour = 0; set_min = 1;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    depth = 2;
    lv_obj_clean(lv_scr_act());
    clean_menu_create(&ui_manager);
    clean_menu_t *cm = clean_menu_get(&ui_manager);
    if (cm) {
        lv_obj_t *btns[] = { cm->waterclean, cm->button_2 };
        if (g_clean_menu) lv_group_del(g_clean_menu);
        g_clean_menu = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cm->waterclean, on_clean_water_click, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cm->button_2, on_clean_hot_click, LV_EVENT_CLICKED, NULL);
        if (cm->waterclean) lv_group_focus_obj(cm->waterclean);
    }
    current_group = g_clean_menu;
    lv_scr_load_anim(clean_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    printf("[hcs] stop_back sure -> clean_menu\n");
}

void jump_to_hcm_set(void)
{
    set_hour = 0; set_min = 1;
    page_push(PAGE_HOTCLEANMIDDLE_SET);
    lv_obj_clean(lv_scr_act());
    hotcleanmiddle_set_create(&ui_manager);

    hotcleanmiddle_set_t *set = hotcleanmiddle_set_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = { set->start };
        if (g_hcm_set) lv_group_del(g_hcm_set);
        g_hcm_set = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(set->start, on_hcm_set_start_click,
                            LV_EVENT_CLICKED, NULL);

        if (set->start) lv_group_focus_obj(set->start);
    }
    current_group = g_hcm_set;

    lv_scr_load_anim(hotcleanmiddle_set_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.cook_mode = MODE_HOT_CLEAN;
    g_send.iface_status = IFACE_SETTING;
    printf("[hcs] jump: hotclean_menu -> hcm_set\n");
}

void jump_to_hcm_cooking(void)
{
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }

    page_push(PAGE_HOTCLEANMIDDLE_COOKING);
    lv_obj_clean(lv_scr_act());
    hotcleanmiddle_cooking_create(&ui_manager);

    hotcleanmiddle_cooking_t *cook = hotcleanmiddle_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_hcm_cooking) lv_group_del(g_hcm_cooking);
        g_hcm_cooking = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(cook->stop, on_hcm_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", 0, 1, 0);
    }

    cook_total_ms = 60 * 1000;
    if (cook) {
        lv_bar_set_range(cook->bar_2, 0, 100);
        lv_bar_set_value(cook->bar_2, 3, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_2);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_hcm_cooking;

    lv_scr_load_anim(hotcleanmiddle_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    g_send.remaining_ms = cook_total_ms;
    printf("[hcs] jump: set -> hcm_cooking\n");
}

void jump_to_hcm_stop(void)
{
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

    {
        hotcleanmiddle_cooking_t *cook = hotcleanmiddle_cooking_get(&ui_manager);
        cook_bar_saved = cook ? lv_bar_get_value(cook->bar_2) : 0;
    }

    page_push(PAGE_HOTCLEANMIDDLE_STOP);
    lv_obj_clean(lv_scr_act());
    hotcleanmiddle_stop_create(&ui_manager);

    hotcleanmiddle_stop_t *stop = hotcleanmiddle_stop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start };
        if (g_hcm_stop) lv_group_del(g_hcm_stop);
        g_hcm_stop = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(stop->start, on_hcm_stop_start_click,
                            LV_EVENT_CLICKED, NULL);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(stop->timelabel, "%02d:%02d:%02d", h, m, s);

        lv_bar_set_range(stop->bar_3, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_3, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_hcm_stop;

    lv_scr_load_anim(hotcleanmiddle_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = (cook_total_ms > (int)cook_elapsed_saved) ? cook_total_ms - (int)cook_elapsed_saved : 0;
    printf("[hcs] jump: cooking -> stop (pause)\n");
}

void hcm_resume_cooking(void)
{
    
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
depth--;
    lv_obj_clean(lv_scr_act());
    hotcleanmiddle_cooking_create(&ui_manager);

    hotcleanmiddle_cooking_t *cook = hotcleanmiddle_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_hcm_cooking) lv_group_del(g_hcm_cooking);
        g_hcm_cooking = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(cook->stop, on_hcm_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", h, m, s);

        lv_bar_set_range(cook->bar_2, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(cook->bar_2, cook_bar_saved, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar_2);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, cook_bar_saved, 100);
        lv_anim_set_time(&a, cook_total_ms - (int)cook_elapsed_saved);
        lv_anim_start(&a);
    }

    cook_start_time = lv_tick_get() - cook_elapsed_saved;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_hcm_cooking;

    lv_scr_load_anim(hotcleanmiddle_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    {
        int rem = cook_total_ms - (int)cook_elapsed_saved;
        if (rem < 0) rem = 0;
        g_send.iface_status = IFACE_COOKING;
        g_send.remaining_ms = rem;
    }
    printf("[hcs] resume: stop -> cooking\n");
}

void jump_to_hcm_stop_back(void)
{
    page_push(PAGE_HOTCLEANMIDDLE_STOP_BACK);
    lv_obj_clean(lv_scr_act());
    hotcleanmiddle_stop_back_create(&ui_manager);

    hotcleanmiddle_stop_back_t *back = hotcleanmiddle_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_hcm_stop_back) lv_group_del(g_hcm_stop_back);
        g_hcm_stop_back = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(back->sure, on_hcm_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        lv_bar_set_range(back->bar_2, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(back->bar_2, cook_bar_saved, LV_ANIM_OFF);

        if (back->sure) lv_group_focus_obj(back->sure);
    }
    current_group = g_hcm_stop_back;

    lv_scr_load_anim(hotcleanmiddle_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hcs] jump: stop -> stop_back\n");
}

void jump_to_hcm_cooling(void)
{
    page_push(PAGE_HOTCLEANMIDDLE_COOLING);
    lv_obj_clean(lv_scr_act());
    hotcleanmiddle_cooling_create(&ui_manager);

    hotcleanmiddle_cooling_t *cool = hotcleanmiddle_cooling_get(&ui_manager);
    if (cool) {
        lv_bar_set_range(cool->bar_4, 0, 100);
        lv_bar_set_value(cool->bar_4, 100, LV_ANIM_OFF);
    }

    current_group = g_hcm_cooling;

#ifdef LV_USE_AIC_SIMULATOR
    g_sim_cavity_temp = 300;
    set_temp = g_sim_cavity_temp;
#else
    set_temp = get_cavity_temp();
#endif

    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    lv_scr_load_anim(hotcleanmiddle_cooling_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    printf("[hcs] jump: cooking -> cooling\n");
}

void jump_to_hcm_complete(void)
{
    page_push(PAGE_HOTCLEANMIDDLE_COMPLETE);
    lv_obj_clean(lv_scr_act());
    hotcleanmiddle_complete_create(&ui_manager);

    current_group = g_hcm_complete;

    lv_scr_load_anim(hotcleanmiddle_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COMPLETE;
    g_send.remaining_ms = 0;
    printf("[hcs] jump: cooling -> complete\n");
}

void hcm_rebuild_set(page_id_t child)
{
    hotcleanmiddle_set_create(&ui_manager);
    hotcleanmiddle_set_t *set = hotcleanmiddle_set_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = { set->start };
        if (g_hcm_set) lv_group_del(g_hcm_set);
        g_hcm_set = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(set->start, on_hcm_set_start_click,
                            LV_EVENT_CLICKED, NULL);

        if (set->start) lv_group_focus_obj(set->start);
    }
    current_group = g_hcm_set;
    lv_scr_load_anim(hotcleanmiddle_set_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hcs] back to hcm_set\n");
}

void hcm_rebuild_cooking(page_id_t child)
{
    hotcleanmiddle_cooking_create(&ui_manager);
    hotcleanmiddle_cooking_t *cook = hotcleanmiddle_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_hcm_cooking) lv_group_del(g_hcm_cooking);
        g_hcm_cooking = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(cook->stop, on_hcm_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        if (child == PAGE_HOTCLEANMIDDLE_COOLING || child == PAGE_HOTCLEANMIDDLE_STOP) {
            uint32_t elapsed = (child == PAGE_HOTCLEANMIDDLE_COOLING) ? cook_total_ms : (lv_tick_get() - cook_start_time);
            int elapsed_sec = (elapsed + 500) / 1000;
            int total_sec = cook_total_ms / 1000;
            int remaining_sec = total_sec - elapsed_sec;
            if (remaining_sec < 0) remaining_sec = 0;
            int h = remaining_sec / 3600;
            int m = (remaining_sec % 3600) / 60;
            int s = remaining_sec % 60;
            lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", h, m, s);
            lv_bar_set_range(cook->bar_2, 0, 100);
            int progress = (int)((int64_t)elapsed * 100 / cook_total_ms);
            if (progress > 100) progress = 100;
            lv_bar_set_value(cook->bar_2, progress, LV_ANIM_OFF);
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, cook->bar_2);
            lv_anim_set_exec_cb(&a, anim_bar_set_value);
            lv_anim_set_values(&a, progress, 100);
            lv_anim_set_time(&a, cook_total_ms - (int)elapsed);
            lv_anim_start(&a);
        } else {
            lv_label_set_text_fmt(cook->timelabel, "%02d:%02d:%02d", 0, 1, 0);
            lv_bar_set_range(cook->bar_2, 0, 100);
            lv_bar_set_value(cook->bar_2, 3, LV_ANIM_OFF);
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, cook->bar_2);
            lv_anim_set_exec_cb(&a, anim_bar_set_value);
            lv_anim_set_values(&a, 3, 100);
            lv_anim_set_time(&a, cook_total_ms);
            lv_anim_start(&a);
            cook_start_time = lv_tick_get();
            if (cook_timer) lv_timer_del(cook_timer);
            cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
        }
    }
    current_group = g_hcm_cooking;
    lv_scr_load_anim(hotcleanmiddle_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hcs] back to hcm_cooking\n");
}

void hcm_rebuild_stop(void)
{
    hotcleanmiddle_stop_create(&ui_manager);
    hotcleanmiddle_stop_t *stop = hotcleanmiddle_stop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start };
        if (g_hcm_stop) lv_group_del(g_hcm_stop);
        g_hcm_stop = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(stop->start, on_hcm_stop_start_click,
                            LV_EVENT_CLICKED, NULL);

        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(stop->timelabel, "%02d:%02d:%02d", h, m, s);

        lv_bar_set_range(stop->bar_3, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_3, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_hcm_stop;
    lv_scr_load_anim(hotcleanmiddle_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_PAUSE;
    printf("[hcs] back to hcm_stop\n");
}

void hcm_rebuild_stop_back(void)
{
    hotcleanmiddle_stop_back_create(&ui_manager);
    hotcleanmiddle_stop_back_t *back = hotcleanmiddle_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_hcm_stop_back) lv_group_del(g_hcm_stop_back);
        g_hcm_stop_back = group_create_for_page(btns, 1);

        lv_obj_add_event_cb(back->sure, on_hcm_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        lv_bar_set_range(back->bar_2, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(back->bar_2, cook_bar_saved, LV_ANIM_OFF);

        if (back->sure) lv_group_focus_obj(back->sure);
    }
    current_group = g_hcm_stop_back;
    lv_scr_load_anim(hotcleanmiddle_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hcs] back to hcm_stop_back\n");
}

void hcm_rebuild_cooling(void)
{
    hotcleanmiddle_cooling_create(&ui_manager);
    current_group = g_hcm_cooling;
    lv_scr_load_anim(hotcleanmiddle_cooling_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hcs] back to hcm_cooling\n");
}

void hcm_rebuild_complete(void)
{
    hotcleanmiddle_complete_create(&ui_manager);
    current_group = g_hcm_complete;
    lv_scr_load_anim(hotcleanmiddle_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hcs] back to hcm_complete\n");
}
