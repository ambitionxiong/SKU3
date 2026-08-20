/*
 * nav_hint.c - 功能键无效提示弹窗
 * 烹饪中按功能键弹出'烤箱运行时不可用'提示(3秒自动关闭)，
 * 期间仅BACK有效，UAF安全处理。
 */
#include "nav.h"
#include "protocol.h"

/* ==============================
 * 烹饪中功能键无效提示
 * 提示时:隐藏当前 cooking 页右侧元素(timelabel/temp/探针图标),
 * 并在当前页面对象上动态创建遮罩 + tip1(随页面销毁自动清理,无残留重叠),
 * 3 秒后恢复;BACK 可提前结束(恢复页面,不执行返回)。
 * ============================== */

static lv_timer_t *g_hint_timer = NULL;
static lv_obj_t *g_hint_objs[2];
static int g_hint_n = 0;
static lv_group_t *g_hint_group = NULL;   /* 触发时页面组,恢复时校验元素有效性 */
static lv_obj_t *g_hint_scr = NULL;       /* 创建遮罩时的屏幕(恢复时对比防悬空) */
static lv_obj_t *g_hint_mask = NULL;      /* 动态遮罩 */
static lv_obj_t *g_hint_tip = NULL;       /* 动态 tip1 */

static void nav_hint_restore(void);

/* 对象销毁时同步置空指针(防悬空 UAF,LV_EVENT_DELETE 在销毁流程中同步发送) */
static void hint_del_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    if (obj == g_hint_mask) g_hint_mask = NULL;
    if (obj == g_hint_tip)  g_hint_tip = NULL;
    for (int i = 0; i < g_hint_n; i++)
        if (g_hint_objs[i] == obj) g_hint_objs[i] = NULL;
}

// 收集隐藏对象并挂 DELETE 回调
static void nav_hint_collect(void)
{
    g_hint_n = 0;
    /* 探针 cooking:temp + 右侧探针图标 */
    if (current_group == g_updown_bbq_cooking_probe) {
        updown_bbq_cooking_probe_t *c = updown_bbq_cooking_probe_get(&ui_manager);
        if (c) { g_hint_objs[g_hint_n++] = c->temp; g_hint_objs[g_hint_n++] = c->image_24; }
    } else if (current_group == g_hot_bbq_cooking_probe) {
        hot_bbq_cooking_probe_t *c = hot_bbq_cooking_probe_get(&ui_manager);
        if (c) { g_hint_objs[g_hint_n++] = c->temp; g_hint_objs[g_hint_n++] = c->image_9; }
    } else if (current_group == g_bottom_bbq_cooking_probe) {
        bottom_bbq_cooking_probe_t *c = bottom_bbq_cooking_probe_get(&ui_manager);
        if (c) { g_hint_objs[g_hint_n++] = c->temp; g_hint_objs[g_hint_n++] = c->image_25; }
    } else if (current_group == g_slowcook_cooking_probe) {
        slowcook_cooking_probe_t *c = slowcook_cooking_probe_get(&ui_manager);
        if (c) { g_hint_objs[g_hint_n++] = c->temp; g_hint_objs[g_hint_n++] = c->image_41; }
    }
    /* preheat cooking:temp */
    else if (current_group == g_preheat_cooking) {
        preheatcooking_t *c = preheatcooking_get(&ui_manager);
        if (c) g_hint_objs[g_hint_n++] = c->temp;
    }
    /* 普通 cooking:time_label / label_80 / timelabel */
    else if (current_group == g_updown_bbq_cooking) {
        updown_bbq_cooking_t *c = updown_bbq_cooking_get(&ui_manager);
        if (c) g_hint_objs[g_hint_n++] = c->time_label;
    } else if (current_group == g_color_cookoing) {
        color_cookoing_t *c = color_cookoing_get(&ui_manager);
        if (c) g_hint_objs[g_hint_n++] = c->time_label;
    } else if (current_group == g_somecook_cooking || current_group == g_six_cooking) {
        somecook_cooking_t *c = somecook_cooking_get(&ui_manager);
        if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_top_bbq_cooking) {
        top_bbq_cooking_t *c = top_bbq_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->label_80;
    } else if (current_group == g_bottom_bbq_cooking) {
        bottom_bbq_cooking_t *c = bottom_bbq_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_hot_bbq_cooking) {
        hot_bbq_cooking_t *c = hot_bbq_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_hotwind_bbq_cooking) {
        hotwind_bbq_cooking_t *c = hotwind_bbq_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_save_bbq_cooking) {
        save_bbq_cooking_t *c = save_bbq_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_central_bbq_cooking) {
        central_bbq_cooking_t *c = central_bbq_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_windchange_bbq_cooking) {
        windchange_bbq_cooking_t *c = windchange_bbq_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_air_cooking) {
        air_cooking_t *c = air_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_pizza_2_cooking) {
        pizza_2_cooking_t *c = pizza_2_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_slowcook_cooking) {
        slowcook_cooking_t *c = slowcook_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_unfrozen_cooking) {
        unfrozen_cooking_t *c = unfrozen_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_rising_cooking) {
        rising_cooking_t *c = rising_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_corn_cooking) {
        corn_cooking_t *c = corn_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_heatcontain_cooking) {
        heatcontain_cooking_t *c = heatcontain_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_cookie_cooking) {
        cookie_cooking_t *c = cookie_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_west_cooking) {
        west_cooking_t *c = west_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_pizza_cooking) {
        pizza_cooking_t *c = pizza_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_menu_cook_cooking) {
        menu_cooking_t *c = menu_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_lasagna_cooking) {
        lasagna_cooking_t *c = lasagna_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_strudel_cooking) {
        strudel_cooking_t *c = strudel_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_bread_cooking) {
        bread_cooking_t *c = bread_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_pizza3_cooking) {
        pizza3_cooking_t *c = pizza3_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_chip_cooking) {
        chip_cooking_t *c = chip_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_custom_cooking) {
        custom_cooking_t *c = custom_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    }
    for (int i = 0; i < g_hint_n; i++)
        if (g_hint_objs[i]) {
            /* 先移除再挂:元素跨多次提示复用,防 DELETE 回调事件列表累积 */
            lv_obj_remove_event_cb(g_hint_objs[i], hint_del_cb);
            lv_obj_add_event_cb(g_hint_objs[i], hint_del_cb, LV_EVENT_DELETE, NULL);
        }
}

// 恢复:元素恢复 + 重放页面状态 + 删除动态遮罩 + 删 timer
static void nav_hint_restore(void)
{
    if (g_hint_timer) { lv_timer_del(g_hint_timer); g_hint_timer = NULL; }

    for (int i = 0; i < g_hint_n; i++)   /* 元素有效(DELETE 回调已置空)才恢复 */
        if (g_hint_objs[i]) lv_obj_clear_flag(g_hint_objs[i], LV_OBJ_FLAG_HIDDEN);
    /* 重放页面状态:同页状态可能已变化(六感切段/多段 stopback 等),由状态函数重设显隐 */
    if (current_group == g_six_cooking)
        six_cook_refresh_display();
    else if (current_group == g_somecook_cooking)
        somecook_cooking_refresh();
    g_hint_n = 0;
    g_hint_group = NULL;

    /* 删除动态遮罩:非 NULL 即有效(页面切换销毁时 DELETE 回调已置空) */
    if (g_hint_mask) lv_obj_del(g_hint_mask);
    if (g_hint_tip)  lv_obj_del(g_hint_tip);
    g_hint_mask = NULL;
    g_hint_tip = NULL;
    g_hint_scr = NULL;
}

static void hint_timer_cb(lv_timer_t *t)
{
    (void)t;
    nav_hint_restore();
}

// 触发:烹饪中功能键无效时调用(无效音由调用处已发)
void nav_show_invalid_hint(void)
{
    if (g_send.iface_status != IFACE_COOKING) return;
    /* stopback/遮罩确认态:不弹提示(右侧无映射元素,且与页面自身遮罩重叠),仅保留无效音 */
    if (g_on_stop_back || six_cook_is_overlay() || somecook_cooking_is_stopback()) return;
    /* 已在提示:重置 3 秒计时 */
    nav_hint_restore();

    g_hint_group = current_group;
    nav_hint_collect();
    for (int i = 0; i < g_hint_n; i++)
        if (g_hint_objs[i]) lv_obj_add_flag(g_hint_objs[i], LV_OBJ_FLAG_HIDDEN);

    /* 动态创建遮罩 + tip1 到当前页面(随页面销毁自动清理) */
    g_hint_scr = lv_scr_act();
    if (g_hint_scr) {
        g_hint_mask = lv_obj_create(g_hint_scr);
        if (!g_hint_mask) {
            /* 遮罩创建失败:恢复已隐藏元素并放弃提示,避免后续空指针 */
            for (int i = 0; i < g_hint_n; i++)
                if (g_hint_objs[i]) lv_obj_clear_flag(g_hint_objs[i], LV_OBJ_FLAG_HIDDEN);
            g_hint_n = 0;
            g_hint_group = NULL;
            g_hint_scr = NULL;
            return;
        }
        lv_obj_remove_style_all(g_hint_mask);   /* 防默认主题白底 */
        lv_obj_set_pos(g_hint_mask, 0, 0);
        lv_obj_set_size(g_hint_mask, 1280, 480);
        lv_obj_set_scrollbar_mode(g_hint_mask, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_style_bg_color(g_hint_mask, lv_color_hex(0x060505), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(g_hint_mask, 113, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(g_hint_mask, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(g_hint_mask, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(g_hint_mask, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(g_hint_mask, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(g_hint_mask, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_left(g_hint_mask, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        g_hint_tip = lv_label_create(g_hint_scr);
        if (!g_hint_tip) {
            /* 提示标签创建失败:删除遮罩并恢复元素,避免后续空指针 */
            lv_obj_del(g_hint_mask);
            g_hint_mask = NULL;
            for (int i = 0; i < g_hint_n; i++)
                if (g_hint_objs[i]) lv_obj_clear_flag(g_hint_objs[i], LV_OBJ_FLAG_HIDDEN);
            g_hint_n = 0;
            g_hint_group = NULL;
            g_hint_scr = NULL;
            return;
        }
        lv_label_set_text(g_hint_tip, "烤箱运行时不可用。");
        lv_label_set_long_mode(g_hint_tip, LV_LABEL_LONG_WRAP);
        lv_obj_set_pos(g_hint_tip, 885, 161);
        lv_obj_set_size(g_hint_tip, 275, 36);
        lv_obj_set_style_text_font(g_hint_tip, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(g_hint_tip, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(g_hint_tip, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_event_cb(g_hint_mask, hint_del_cb, LV_EVENT_DELETE, NULL);
        lv_obj_add_event_cb(g_hint_tip, hint_del_cb, LV_EVENT_DELETE, NULL);
    }
    g_hint_timer = lv_timer_create(hint_timer_cb, 3000, NULL);
    printf("[hint] show\n");
}

// BACK 提前结束提示(恢复页面,不执行返回)
void nav_hint_cancel(void)
{
    nav_hint_restore();
    printf("[hint] cancel\n");
}

int nav_hint_active(void)
{
    return g_hint_timer != NULL;
}
