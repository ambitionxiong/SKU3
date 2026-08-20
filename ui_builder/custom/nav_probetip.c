/*
 * nav_probetip.c - 探针提示页
 * 插拔探针提示(3秒自动关闭)；探针下不支持的按键跳此页提示拔出。
 */
#include "protocol.h"
#include "nav.h"

static lv_timer_t *probetip_dismiss_timer = NULL;
static uint8_t s_probetip_prev_iface = IFACE_SLEEP;   /* 进入提示前 iface,关闭时恢复 */

// 关闭提示:恢复进入前 iface + 回 prev(待机页/原页面)
void probetip_dismiss_now(void)
{
    probetip_cancel_auto_dismiss();
    if (depth > 0 && page_stack[depth - 1] == PAGE_PROBETIP) {
        g_send.iface_status = s_probetip_prev_iface;
        page_pop();
    }
}

static void auto_dismiss_probetip(lv_timer_t *timer)
{
    (void)timer;
    probetip_dismiss_now();
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
    s_probetip_prev_iface = g_send.iface_status;   /* 记录进入前状态,关闭时恢复 */
    if (depth > 0 && page_stack[depth - 1] == PAGE_PROBETIP) {
        /* 已在探针提示页:更新文字并重置 3 秒计时(每次插拔变化重新计时) */
        probetip_cancel_auto_dismiss();
        probetip_dismiss_timer = lv_timer_create(auto_dismiss_probetip, 3000, NULL);
        probetip_t *tip = probetip_get(&ui_manager);
        lv_obj_t *label = (tip && tip->button_1) ? lv_obj_get_child(tip->button_1, 0) : NULL;
        if (label) lv_label_set_text(label, text);
        return;
    }
    page_push(PAGE_PROBETIP);
    lv_obj_clean(lv_scr_act());
    probetip_create(&ui_manager);
    probetip_t *tip = probetip_get(&ui_manager);
    if (tip) {
        lv_obj_t *btns[] = { tip->button_1 };
        for (int k = 0; k < 1; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_probetip) lv_group_del(g_probetip);
        g_probetip = group_create_for_page(btns, 1);
        if (tip->button_1) {
            lv_obj_t *label = lv_obj_get_child(tip->button_1, 0);
            if (label) lv_label_set_text(label, text);
            lv_group_focus_obj(tip->button_1);
        }
    }
    current_group = g_probetip;
    if (probetip_get(&ui_manager)->obj)
        lv_scr_load_anim(probetip_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
    /* 所有场景统一 3 秒自动关闭 */
    probetip_cancel_auto_dismiss();
    probetip_dismiss_timer = lv_timer_create(auto_dismiss_probetip, 3000, NULL);
    printf("[probetip] jump: -> probetip (%s)\n", text);
}

void probetip_rebuild(page_id_t child)
{
    probetip_create(&ui_manager);
    probetip_t *tip = probetip_get(&ui_manager);
    if (tip) {
        lv_obj_t *btns[] = { tip->button_1 };
        for (int k = 0; k < 1; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_probetip) lv_group_del(g_probetip);
        g_probetip = group_create_for_page(btns, 1);
        if (tip->button_1)
            lv_group_focus_obj(tip->button_1);
    }
    current_group = g_probetip;
    if (probetip_get(&ui_manager)->obj)
        lv_scr_load_anim(probetip_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
    printf("[probetip] back to probetip\n");
}
