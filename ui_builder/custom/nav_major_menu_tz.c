#include "protocol.h"
#include "nav.h"

static void on_major_menu_tz_cook_click(lv_event_t *e);
static void on_major_menu_tz_special_click(lv_event_t *e);

static void on_major_menu_tz_cook_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_cook_menu_tz();
}

static void on_major_menu_tz_special_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_special_menu_tz();
}

void jump_to_major_menu_tz(void)
{
    page_push(PAGE_MAJOR_MENU_TZ);
    lv_obj_clean(lv_scr_act());
    major_menu_tz_create(&ui_manager);

    major_menu_tz_t *tz = major_menu_tz_get(&ui_manager);
    if (tz) {
        lv_obj_t *btns[] = { tz->cook_button, tz->special_button };
        if (g_major_menu_tz) lv_group_del(g_major_menu_tz);
        g_major_menu_tz = group_create_for_page(btns, 2);

        if (tz->cook_button)
            lv_obj_add_event_cb(tz->cook_button, on_major_menu_tz_cook_click,
                                LV_EVENT_CLICKED, NULL);
        if (tz->special_button)
            lv_obj_add_event_cb(tz->special_button, on_major_menu_tz_special_click,
                                LV_EVENT_CLICKED, NULL);
    }
    current_group = g_major_menu_tz;

    lv_scr_load_anim(major_menu_tz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_SETTING;
    printf("[tz] jump: wait -> major_menu_tz\n");
}

void major_menu_tz_rebuild(page_id_t child)
{
    major_menu_tz_create(&ui_manager);
    major_menu_tz_t *tz = major_menu_tz_get(&ui_manager);
    if (tz) {
        lv_obj_t *btns[] = { tz->cook_button, tz->special_button };
        if (g_major_menu_tz) lv_group_del(g_major_menu_tz);
        g_major_menu_tz = group_create_for_page(btns, 2);

        if (tz->cook_button)
            lv_obj_add_event_cb(tz->cook_button, on_major_menu_tz_cook_click,
                                LV_EVENT_CLICKED, NULL);
        if (tz->special_button)
            lv_obj_add_event_cb(tz->special_button, on_major_menu_tz_special_click,
                                LV_EVENT_CLICKED, NULL);

        if (child == PAGE_COOK_MENU_TZ && tz->cook_button)
            lv_group_focus_obj(tz->cook_button);
        else if (child == PAGE_SPECIAL_MENU_TZ && tz->special_button)
            lv_group_focus_obj(tz->special_button);
    }
    current_group = g_major_menu_tz;
    lv_scr_load_anim(major_menu_tz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[tz] back to major_menu_tz\n");
}
