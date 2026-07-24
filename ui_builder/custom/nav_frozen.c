#include "nav.h"

static void on_frozen_lasagna_click(lv_event_t *e);

static void on_frozen_lasagna_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_lasagna_menu();
}

void jump_to_frozen_cook(void)
{
    page_push(PAGE_FROZEN_COOK);
    lv_obj_clean(lv_scr_act());
    frozencookpage_create(&ui_manager);

    frozencookpage_t *fp = frozencookpage_get(&ui_manager);
    if (fp) {
        lv_obj_t *btns[] = {
            fp->Lasagna, fp->Strudel, fp->bread,
            fp->pizza3, fp->chip, fp->custom,
        };
        if (g_frozen_cook) lv_group_del(g_frozen_cook);
        g_frozen_cook = group_create_for_page(btns, 6);

        lv_obj_add_event_cb(fp->Lasagna, on_frozen_lasagna_click,
                            LV_EVENT_CLICKED, NULL);

        if (fp->Lasagna) lv_group_focus_obj(fp->Lasagna);
    }
    current_group = g_frozen_cook;

    lv_scr_load_anim(frozencookpage_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[frozen] jump: special_menu -> frozencookpage\n");
}

void frozen_rebuild(page_id_t child)
{
    frozencookpage_create(&ui_manager);
    frozencookpage_t *fp = frozencookpage_get(&ui_manager);
    if (fp) {
        lv_obj_t *btns[] = {
            fp->Lasagna, fp->Strudel, fp->bread,
            fp->pizza3, fp->chip, fp->custom,
        };
        if (g_frozen_cook) lv_group_del(g_frozen_cook);
        g_frozen_cook = group_create_for_page(btns, 6);

        lv_obj_add_event_cb(fp->Lasagna, on_frozen_lasagna_click,
                            LV_EVENT_CLICKED, NULL);

        if (child == PAGE_LASAGNA_MENU && fp->Lasagna)
            lv_group_focus_obj(fp->Lasagna);
    }
    current_group = g_frozen_cook;
    lv_scr_load_anim(frozencookpage_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[frozen] back to frozencookpage\n");
}
