/*
 * nav_clean.c - 清洁入口
 * 主菜单清洁入口分发：自清洁(水) / 高温清洁三档(节能/常规/强力)。
 */
#include "protocol.h"
#include "nav.h"

void on_clean_water_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_wc_set();
}

void on_clean_hot_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hotclean_menu();
}

void on_hc_save_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hcs_set();
}

void on_hc_middle_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hcm_set();
}

void on_hc_high_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hch_set();
}

void jump_to_hotclean_menu(void)
{
    page_push(PAGE_HOTCLEAN_MENU);
    lv_obj_clean(lv_scr_act());
    hotclean_menu_create(&ui_manager);

    hotclean_menu_t *hm = hotclean_menu_get(&ui_manager);
    if (hm) {
        lv_obj_t *btns[] = { hm->hotcleansave, hm->hotclean_middle, hm->hotclean_high };
        if (g_hotclean_menu) lv_group_del(g_hotclean_menu);
        g_hotclean_menu = group_create_for_page(btns, 3);

        lv_obj_add_event_cb(hm->hotcleansave, on_hc_save_click, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(hm->hotclean_middle, on_hc_middle_click, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(hm->hotclean_high, on_hc_high_click, LV_EVENT_CLICKED, NULL);

        if (hm->hotcleansave) lv_group_focus_obj(hm->hotcleansave);
    }
    current_group = g_hotclean_menu;

    lang_scr_load_anim(hotclean_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.cook_mode = MODE_HOT_CLEAN;
    printf("[clean] jump: clean_menu -> hotclean_menu\n");
}

void hotclean_rebuild(page_id_t child)
{
    hotclean_menu_create(&ui_manager);
    hotclean_menu_t *hm = hotclean_menu_get(&ui_manager);
    if (hm) {
        lv_obj_t *btns[] = { hm->hotcleansave, hm->hotclean_middle, hm->hotclean_high };
        if (g_hotclean_menu) lv_group_del(g_hotclean_menu);
        g_hotclean_menu = group_create_for_page(btns, 3);

        lv_obj_add_event_cb(hm->hotcleansave, on_hc_save_click, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(hm->hotclean_middle, on_hc_middle_click, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(hm->hotclean_high, on_hc_high_click, LV_EVENT_CLICKED, NULL);

        if (child >= PAGE_HOTCLEANSAVE_SET && child <= PAGE_HOTCLEANSAVE_COMPLETE && hm->hotcleansave)
            lv_group_focus_obj(hm->hotcleansave);
        else if (child >= PAGE_HOTCLEANMIDDLE_SET && child <= PAGE_HOTCLEANMIDDLE_COMPLETE && hm->hotclean_middle)
            lv_group_focus_obj(hm->hotclean_middle);
        else if (child >= PAGE_HOTCLEANHIGH_SET && child <= PAGE_HOTCLEANHIGH_COMPLETE && hm->hotclean_high)
            lv_group_focus_obj(hm->hotclean_high);
        else if (hm->hotcleansave)
            lv_group_focus_obj(hm->hotcleansave);
    }
    current_group = g_hotclean_menu;
    lang_scr_load_anim(hotclean_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[clean] back to hotclean_menu\n");
}

void jump_to_clean_menu(void)
{
    page_push(PAGE_CLEAN_MENU);
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

    lang_scr_load_anim(clean_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    printf("[clean] jump: waitmenu_24 -> clean_menu\n");
}

void clean_rebuild(page_id_t child)
{
    clean_menu_create(&ui_manager);
    clean_menu_t *cm = clean_menu_get(&ui_manager);
    if (cm) {
        lv_obj_t *btns[] = { cm->waterclean, cm->button_2 };
        if (g_clean_menu) lv_group_del(g_clean_menu);
        g_clean_menu = group_create_for_page(btns, 2);

        lv_obj_add_event_cb(cm->waterclean, on_clean_water_click, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cm->button_2, on_clean_hot_click, LV_EVENT_CLICKED, NULL);

        if (child >= PAGE_WATER_CLEAN_SET && child <= PAGE_WATER_CLEAN_COMPLETE && cm->waterclean)
            lv_group_focus_obj(cm->waterclean);
        else if (((child >= PAGE_HOTCLEANSAVE_SET && child <= PAGE_HOTCLEANHIGH_COMPLETE) ||
                  child == PAGE_HOTCLEAN_MENU) && cm->button_2)
            lv_group_focus_obj(cm->button_2);
        else if (cm->waterclean)
            lv_group_focus_obj(cm->waterclean);
    }
    current_group = g_clean_menu;
    lang_scr_load_anim(clean_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    printf("[clean] back to clean_menu\n");
}
