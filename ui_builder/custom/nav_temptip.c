/*
 * nav_temptip.c - 腔温过热提示页
 * 进入需要冷却的功能（如发酵）时若腔温>50℃ 跳此页，5 秒自动返回；
 * BACK 可提前返回。逻辑与 nav_probetip.c 的探针提示页一致。
 */
#include "protocol.h"
#include "nav.h"

lv_group_t *g_temptip = NULL;
static lv_timer_t *temptip_dismiss_timer = NULL;
static uint8_t s_temptip_prev_iface = IFACE_SLEEP;   /* 进入前 iface,关闭时恢复 */

// 关闭提示:恢复进入前 iface + 回 prev(原页面)
void temptip_dismiss_now(void)
{
    temptip_cancel_auto_dismiss();
    if (depth > 0 && page_stack[depth - 1] == PAGE_TEMPTIP) {
        g_send.iface_status = s_temptip_prev_iface;
        page_pop();
    }
}

static void auto_dismiss_temptip(lv_timer_t *timer)
{
    (void)timer;
    temptip_dismiss_now();
}

void temptip_cancel_auto_dismiss(void)
{
    if (temptip_dismiss_timer) {
        lv_timer_del(temptip_dismiss_timer);
        temptip_dismiss_timer = NULL;
    }
}

void jump_to_temptip(void)
{
    s_temptip_prev_iface = g_send.iface_status;   /* 记录进入前状态,关闭时恢复 */
    page_push(PAGE_TEMPTIP);
    lv_obj_clean(lv_scr_act());
    temptip_create(&ui_manager);

    temptip_t *tip = temptip_get(&ui_manager);
    if (tip) {
        lv_obj_t *btns[] = { tip->button_1 };
        for (int k = 0; k < 1; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_temptip) lv_group_del(g_temptip);
        g_temptip = group_create_for_page(btns, 1);
        if (tip->button_1)
            lv_group_focus_obj(tip->button_1);
    }
    current_group = g_temptip;
    if (temptip_get(&ui_manager)->obj)
        lv_scr_load_anim(temptip_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
    /* 5 秒自动关闭；BACK 可提前返回 */
    temptip_cancel_auto_dismiss();
    temptip_dismiss_timer = lv_timer_create(auto_dismiss_temptip, 5000, NULL);
    printf("[temptip] jump: -> temptip\n");
}

void temptip_rebuild(page_id_t child)
{
    temptip_create(&ui_manager);
    temptip_t *tip = temptip_get(&ui_manager);
    if (tip) {
        lv_obj_t *btns[] = { tip->button_1 };
        for (int k = 0; k < 1; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_temptip) lv_group_del(g_temptip);
        g_temptip = group_create_for_page(btns, 1);
        if (tip->button_1)
            lv_group_focus_obj(tip->button_1);
    }
    current_group = g_temptip;
    if (temptip_get(&ui_manager)->obj)
        lv_scr_load_anim(temptip_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
    printf("[temptip] back to temptip\n");
}