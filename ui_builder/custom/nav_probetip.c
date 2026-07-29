#include "protocol.h"
#include "nav.h"

void jump_to_probetip(const char *text)
{
    page_push(PAGE_PROBETIP);
    lv_obj_clean(lv_scr_act());
    probetip_create(&ui_manager);
    probetip_t *tip = probetip_get(&ui_manager);
    if (tip) {
        lv_obj_t *btns[] = { tip->button_1 };
        if (g_probetip) lv_group_del(g_probetip);
        g_probetip = group_create_for_page(btns, 1);
        lv_obj_t *label = lv_obj_get_child(tip->button_1, 0);
        if (label) lv_label_set_text(label, text);
        lv_group_focus_obj(tip->button_1);
    }
    current_group = g_probetip;
    lv_scr_load_anim(probetip_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0, 0);
    printf("[probetip] jump: -> probetip (%s)\n", text);
}

void probetip_rebuild(page_id_t child)
{
    probetip_create(&ui_manager);
    probetip_t *tip = probetip_get(&ui_manager);
    if (tip) {
        lv_obj_t *btns[] = { tip->button_1 };
        if (g_probetip) lv_group_del(g_probetip);
        g_probetip = group_create_for_page(btns, 1);
        lv_group_focus_obj(tip->button_1);
    }
    current_group = g_probetip;
    lv_scr_load_anim(probetip_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0, 0);
    printf("[probetip] back to probetip\n");
}
