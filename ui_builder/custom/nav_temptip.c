/*
 * nav_temptip.c - 腔温过热提示页
 * 进入需要冷却的功能（如发酵）时若腔温>50℃ 跳此页，5 秒自动返回；
 * BACK 可提前返回。逻辑与 nav_probetip.c 的探针提示页一致。
 * 2026-11-23 用户:门槛统一收进 nav_temptip_block_if_hot()，除单独发酵模式的
 * 入口(特殊功能菜单)外，多段第一步发酵 / 六感选发酵后开始 / 收藏的发酵模式 /
 * 收藏的多段第一步发酵 四处启动点共用，5 秒后各自回到原页面。
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
        /* 自愈兜底:超时层抢屏中本页又被关闭(遗留的 5 秒定时器到点),
           pop 重建下层会清掉超时层屏幕——同步清其状态,防 s_overtime 残留吞键(2026-10-21) */
        if (count_down_overtime_active())
            count_down_overtime_reset();
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
        lang_scr_load_anim(temptip_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
    /* 5 秒自动关闭；BACK 可提前返回 */
    temptip_cancel_auto_dismiss();
    temptip_dismiss_timer = lv_timer_create(auto_dismiss_temptip, 5000, NULL);
    printf("[temptip] jump: -> temptip\n");
}

/* 发酵类功能启动前的腔温门槛(腔温单位 ℃):高于此值视为"腔体未冷却" */
#define TEMPTIP_MAX_CAVITY_TEMP 50

/*
 * 腔温门槛:腔温 > 50℃ 就抢"腔体过热"提示页并返回 true,调用方必须直接 return
 * (不启动烹饪、不改任何参数);提示页 5 秒后自动 page_pop 回调用方所在页面
 * (nav_pop 已支持 PAGE_SOMECOOK/PAGE_DESCRIPTIONMENU/PAGE_FAVORITES 重建)。
 * 调用点(2026-11-23 用户):
 *   ① nav_events.c on_rising_click        特殊功能菜单 → 发酵(原有)
 *   ② nav_somecook_cooking.c on_somecook_sure_click   多段"确定",第一步是发酵
 *   ③ nav_six_desc.c on_description_start_click       六感选了发酵后按"开始"
 *   ④ nav_favorites.c favorites_start_selected        收藏的发酵模式 / 多段第一步发酵
 */
bool nav_temptip_block_if_hot(void)
{
    if (get_cavity_temp() <= TEMPTIP_MAX_CAVITY_TEMP)
        return false;
    printf("[temptip] cavity %d > %d, block start\n",
           (int)get_cavity_temp(), TEMPTIP_MAX_CAVITY_TEMP);
    jump_to_temptip();
    return true;
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
        lang_scr_load_anim(temptip_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
    printf("[temptip] back to temptip\n");
}