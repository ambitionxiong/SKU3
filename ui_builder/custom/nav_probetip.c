#include "protocol.h"
#include "nav.h"

static lv_timer_t *probetip_dismiss_timer = NULL;

static void auto_dismiss_probetip(lv_timer_t *timer)
{
    lv_timer_del(timer);
    probetip_dismiss_timer = NULL;
    if (depth > 0 && page_stack[depth - 1] == PAGE_PROBETIP) {
        page_pop();
        g_send.iface_status = IFACE_SLEEP;
    }
}

void probetip_cancel_auto_dismiss(void)
{
    if (probetip_dismiss_timer) {
        lv_timer_del(probetip_dismiss_timer);
        probetip_dismiss_timer = NULL;
    }
}

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
    if (g_send.iface_status == IFACE_SLEEP) {
        probetip_cancel_auto_dismiss();
        probetip_dismiss_timer = lv_timer_create(auto_dismiss_probetip, 2000, NULL);
    }
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
