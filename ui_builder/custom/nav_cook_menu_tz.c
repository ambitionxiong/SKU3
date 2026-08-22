/*
 * nav_cook_menu_tz.c - 探针下烹调菜单
 * 探针模式下的烹调模式列表，进入探针版各模式。
 */
#include "protocol.h"
#include "nav.h"

/* 探针下烹调菜单：只有 updown/hot/bottom 三个功能 */
static void on_cook_menu_tz_updown_click(lv_event_t *e);
static void on_cook_menu_tz_hot_click(lv_event_t *e);
static void on_cook_menu_tz_bottom_click(lv_event_t *e);

static void on_cook_menu_tz_updown_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_menu_probe();
}

static void on_cook_menu_tz_hot_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hot_bbq_menu_probe();
}

static void on_cook_menu_tz_bottom_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_bottom_bbq_menu_probe();
}

void jump_to_cook_menu_tz(void)
{
    page_push(PAGE_COOK_MENU_TZ);
    lv_obj_clean(lv_scr_act());
    cook_menu_tz_create(&ui_manager);

    cook_menu_tz_t *tz = cook_menu_tz_get(&ui_manager);
    if (tz) {
        lv_obj_t *btns[] = {
            tz->updown_button, tz->hot_bbq_button, tz->bottom_button,
        };
        if (g_cook_menu_tz) lv_group_del(g_cook_menu_tz);
        g_cook_menu_tz = group_create_for_page(btns, 3);

        if (tz->updown_button)
            lv_obj_add_event_cb(tz->updown_button, on_cook_menu_tz_updown_click,
                                LV_EVENT_CLICKED, NULL);
        if (tz->hot_bbq_button)
            lv_obj_add_event_cb(tz->hot_bbq_button, on_cook_menu_tz_hot_click,
                                LV_EVENT_CLICKED, NULL);
        if (tz->bottom_button)
            lv_obj_add_event_cb(tz->bottom_button, on_cook_menu_tz_bottom_click,
                                LV_EVENT_CLICKED, NULL);
    }
    current_group = g_cook_menu_tz;

    lang_scr_load_anim(cook_menu_tz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_SETTING;
    printf("[tz] jump: major_menu_tz -> cook_menu_tz\n");
}

void cook_menu_tz_rebuild(page_id_t child)
{
    cook_menu_tz_create(&ui_manager);
    cook_menu_tz_t *tz = cook_menu_tz_get(&ui_manager);
    if (tz) {
        lv_obj_t *btns[] = {
            tz->updown_button, tz->hot_bbq_button, tz->bottom_button,
        };
        if (g_cook_menu_tz) lv_group_del(g_cook_menu_tz);
        g_cook_menu_tz = group_create_for_page(btns, 3);

        if (tz->updown_button)
            lv_obj_add_event_cb(tz->updown_button, on_cook_menu_tz_updown_click,
                                LV_EVENT_CLICKED, NULL);
        if (tz->hot_bbq_button)
            lv_obj_add_event_cb(tz->hot_bbq_button, on_cook_menu_tz_hot_click,
                                LV_EVENT_CLICKED, NULL);
        if (tz->bottom_button)
            lv_obj_add_event_cb(tz->bottom_button, on_cook_menu_tz_bottom_click,
                                LV_EVENT_CLICKED, NULL);

        if (child == PAGE_UPDOWN_BBQ_MENU_PROBE && tz->updown_button)
            lv_group_focus_obj(tz->updown_button);
        else if (child == PAGE_HOT_BBQ_MENU_PROBE && tz->hot_bbq_button)
            lv_group_focus_obj(tz->hot_bbq_button);
        else if (child == PAGE_BOTTOM_BBQ_MENU_PROBE && tz->bottom_button)
            lv_group_focus_obj(tz->bottom_button);
        else if (tz->updown_button)
            lv_group_focus_obj(tz->updown_button);
    }
    current_group = g_cook_menu_tz;
    lang_scr_load_anim(cook_menu_tz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[tz] back to cook_menu_tz\n");
}
