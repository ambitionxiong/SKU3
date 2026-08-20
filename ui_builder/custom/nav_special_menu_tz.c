/*
 * nav_special_menu_tz.c - 探针下特色菜单
 * 探针模式下的特色模式列表。
 */
#include "protocol.h"
#include "nav.h"

/* 探针下特色菜单：只有 slowcook 按钮 */
static void on_special_menu_tz_slowcook_click(lv_event_t *e);

static void on_special_menu_tz_slowcook_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_slowcook_menu_probe();
}

void jump_to_special_menu_tz(void)
{
    page_push(PAGE_SPECIAL_MENU_TZ);
    lv_obj_clean(lv_scr_act());
    special_menu_tz_create(&ui_manager);

    special_menu_tz_t *tz = special_menu_tz_get(&ui_manager);
    if (tz) {
        lv_obj_t *btns[] = { tz->slowcook_button };
        if (g_special_menu_tz) lv_group_del(g_special_menu_tz);
        g_special_menu_tz = group_create_for_page(btns, 1);

        if (tz->slowcook_button)
            lv_obj_add_event_cb(tz->slowcook_button, on_special_menu_tz_slowcook_click,
                                LV_EVENT_CLICKED, NULL);
    }
    current_group = g_special_menu_tz;

    lv_scr_load_anim(special_menu_tz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_SETTING;
    printf("[tz] jump: major_menu_tz -> special_menu_tz\n");
}

void special_menu_tz_rebuild(page_id_t child)
{
    special_menu_tz_create(&ui_manager);
    special_menu_tz_t *tz = special_menu_tz_get(&ui_manager);
    if (tz) {
        lv_obj_t *btns[] = { tz->slowcook_button };
        if (g_special_menu_tz) lv_group_del(g_special_menu_tz);
        g_special_menu_tz = group_create_for_page(btns, 1);

        if (tz->slowcook_button)
            lv_obj_add_event_cb(tz->slowcook_button, on_special_menu_tz_slowcook_click,
                                LV_EVENT_CLICKED, NULL);

        if (tz->slowcook_button)
            lv_group_focus_obj(tz->slowcook_button);
    }
    current_group = g_special_menu_tz;
    lv_scr_load_anim(special_menu_tz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[tz] back to special_menu_tz\n");
}
