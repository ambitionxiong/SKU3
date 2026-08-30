/*
 * nav_hint.c - 功能键无效提示弹窗 + 收藏保存结果提示
 * 烹饪中按功能键弹出'烤箱运行时不可用'提示(3秒自动关闭)，
 * 期间仅BACK有效，UAF安全处理。
 */

#include "nav.h"
#include "protocol.h"

/* ==============================
 * 烹饪中功能键无效提示
 * 提示时:隐藏当前 cooking 页右侧元素(timelabel/temp/探针图标),
 * 遮罩与文字用 topflagpage 的 container_1 + tip1(常驻 lv_layer_top,
 * 与收藏结果提示同款;时钟/状态图标在遮罩下层随层级自然显示),
 * 3 秒后恢复;BACK 可提前结束(恢复页面,不执行返回)。
 * ============================== */

static lv_timer_t *g_hint_timer = NULL;
static lv_obj_t *g_hint_objs[2];
static int g_hint_n = 0;
static lv_group_t *g_hint_group = NULL;   /* 触发时页面组,恢复时校验元素有效性 */

static void nav_hint_restore(void);

/* 对象销毁时同步置空指针(防悬空 UAF,LV_EVENT_DELETE 在销毁流程中同步发送) */
static void hint_del_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
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

// 恢复:元素恢复 + 重放页面状态 + 收起 topflag 遮罩/文字 + 删 timer
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

    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (tf && tf->obj) {
        if (tf->tip1)        lv_obj_add_flag(tf->tip1, LV_OBJ_FLAG_HIDDEN);
        if (tf->container_1) lv_obj_add_flag(tf->container_1, LV_OBJ_FLAG_HIDDEN);
    }
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
    /* SETTING 页(烹饪中小按钮进入,仍处烹饪态):仅无效音,不弹窗 */
    if (depth > 0 && is_cook_setting_page(page_stack[depth - 1])) return;
    /* stopback/遮罩确认态:不弹提示(右侧无映射元素,且与页面自身遮罩重叠),仅保留无效音 */
    if (g_on_stop_back || six_cook_is_overlay() || somecook_cooking_is_stopback()) return;
    /* 已在提示:重置 3 秒计时 */
    nav_hint_restore();

    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->obj || !tf->tip1 || !tf->container_1) return;

    g_hint_group = current_group;
    nav_hint_collect();
    for (int i = 0; i < g_hint_n; i++)
        if (g_hint_objs[i]) lv_obj_add_flag(g_hint_objs[i], LV_OBJ_FLAG_HIDDEN);

    /* 显示 topflag 遮罩 + tip1(文本每次设置,随语言切换) */
    lv_label_set_text(tf->tip1, tr("烤箱运行时不可用。"));
    lv_obj_clear_flag(tf->container_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(tf->tip1, LV_OBJ_FLAG_HIDDEN);
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

/* ==============================
 * 收藏保存结果提示（topflagpage 顶层 tip3,2 秒自动消失）
 * 三种情况之一:正常收藏成功(已收藏/收藏夹已满后续接入)。
 * 不显示遮罩(container_1 保持隐藏),仅右侧 tip3 文字;完成页右侧组件
 * 隐藏/恢复见 collect_hide。期间功能键忽略、BACK 提前关闭(nav_key.c 守卫)。
 * ============================== */
static lv_timer_t *g_favtip_timer = NULL;
static lv_obj_t *g_favtip_objs[8];   /* 提示期间隐藏的完成页右侧组件 */
static int g_favtip_n = 0;
static int g_favtip_was[8];          /* 隐藏前是否本就隐藏(恢复时只还原我们藏掉的) */

/* 对象销毁时同步置空指针(防悬空,同 hint_del_cb) */
static void favtip_del_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    for (int i = 0; i < g_favtip_n; i++)
        if (g_favtip_objs[i] == obj) g_favtip_objs[i] = NULL;
}

/* 收集并隐藏当前完成页右侧组件(防烫图标/文字等)。
 * 完成页 30+ 张、右侧组件字段名各异(image_3/6/26/70、text1/2...),
 * 按几何位置通用识别:屏幕直属子对象 x>=800 即右侧列(同 nav_hint_collect 思路) */
static void nav_favtip_collect_hide(void)
{
    g_favtip_n = 0;
    lv_obj_t *scr = lv_scr_act();
    if (!scr) return;
    uint32_t cnt = lv_obj_get_child_cnt(scr);
    for (uint32_t i = 0; i < cnt && g_favtip_n < 8; i++) {
        lv_obj_t *ch = lv_obj_get_child(scr, i);
        if (!ch) continue;
        if (lv_obj_get_x(ch) >= 800) {
            g_favtip_was[g_favtip_n] = lv_obj_has_flag(ch, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(ch, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_event_cb(ch, favtip_del_cb);
            lv_obj_add_event_cb(ch, favtip_del_cb, LV_EVENT_DELETE, NULL);
            g_favtip_objs[g_favtip_n++] = ch;
        }
    }
}

/* 恢复被隐藏的完成页右侧组件(只还原我们藏掉的,本就隐藏的保持原状) */
static void nav_favtip_restore_elems(void)
{
    for (int i = 0; i < g_favtip_n; i++)
        if (g_favtip_objs[i] && !g_favtip_was[i])
            lv_obj_clear_flag(g_favtip_objs[i], LV_OBJ_FLAG_HIDDEN);
    g_favtip_n = 0;
}

static void nav_favtip_hide(void)
{
    if (g_favtip_timer) { lv_timer_del(g_favtip_timer); g_favtip_timer = NULL; }
    nav_favtip_restore_elems();
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->obj) return;
    if (tf->tip3)        lv_obj_add_flag(tf->tip3, LV_OBJ_FLAG_HIDDEN);
    if (tf->container_1) lv_obj_add_flag(tf->container_1, LV_OBJ_FLAG_HIDDEN);   /* 防御:保持隐藏 */
}

static void favtip_timer_cb(lv_timer_t *t)
{
    (void)t;
    nav_favtip_hide();
}

// 触发:正常收藏成功时调用(favorites_save_current)
// 收藏成功不显示遮罩(container_1 保持隐藏),仅右侧 tip3 文字,2 秒自动消失
void nav_show_fav_tip(void)
{
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->obj || !tf->tip3) return;
    nav_favtip_hide();   /* 重复触发:重置 2 秒计时 */
    lv_label_set_text(tf->tip3, tr("收藏成功"));   /* 每次显示时设文本,随语言切换 */
    nav_favtip_collect_hide();   /* 隐藏完成页右侧组件(防烫图标/文字),恢复在 hide */
    lv_obj_clear_flag(tf->tip3, LV_OBJ_FLAG_HIDDEN);
    g_favtip_timer = lv_timer_create(favtip_timer_cb, 2000, NULL);
    printf("[hint] fav tip show\n");
}

// BACK 提前关闭收藏提示
void nav_favtip_cancel(void)
{
    nav_favtip_hide();
    printf("[hint] fav tip cancel\n");
}

int nav_favtip_active(void)
{
    return g_favtip_timer != NULL;
}

/* ==============================
 * 收藏确认弹层（topflag 顶层遮罩+tip1+tip2+sure,不自动消失）
 * 两种模式:
 *   FAV_ASK_DUPLICATE:tip1="该烹调已有，" tip2="需要覆盖原有烹调吗？"
 *                     确认→覆盖保存(nav_favorites.c nav_favask_confirm)
 *   FAV_ASK_FULL:    tip1="收藏夹已满，" tip2="请删除不太喜欢的烹调！"
 *                     确认→进收藏夹删除界面(整卡删除,Del_Fav_create_flag)
 * BACK→关闭回完成页;其余键忽略(nav_key.c 模态守卫)。均无自动返回。
 * ============================== */
static int g_favask_active = 0;
static int g_favask_mode = 0;    /* 0 关闭 1 重复收藏确认 2 收藏夹已满 */

#define FAV_ASK_DUPLICATE 1
#define FAV_ASK_FULL      2

// 显示收藏确认弹层(mode:1 重复收藏 2 收藏夹已满;favorites_save_current 调用)
static void fav_ask_show(int mode)
{
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->obj || !tf->tip1 || !tf->tip2 || !tf->sure) return;
    nav_favtip_hide();   /* 关掉可能存在的成功提示,重置状态 */
    if (mode == FAV_ASK_FULL) {
        lv_label_set_text(tf->tip1, tr("收藏夹已满，"));
        lv_label_set_text(tf->tip2, tr("请删除不太喜欢的烹调！"));
    } else {
        lv_label_set_text(tf->tip1, tr("该烹调已有，"));
        lv_label_set_text(tf->tip2, tr("需要覆盖原有烹调吗？"));
    }
    if (tf->sure) {
        lv_obj_t *lbl = lv_obj_get_child(tf->sure, 0);   /* 按钮文字随语言切换 */
        if (lbl) lv_label_set_text(lbl, tr("确 定"));
        lv_obj_add_state(tf->sure, LV_STATE_FOCUSED);    /* 常亮聚焦底图,提示可按 */
    }
    lv_obj_clear_flag(tf->tip1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(tf->tip2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(tf->sure, LV_OBJ_FLAG_HIDDEN);
    if (tf->container_1) lv_obj_clear_flag(tf->container_1, LV_OBJ_FLAG_HIDDEN);   /* 遮罩 */
    nav_favtip_collect_hide();   /* tip1 与完成页右侧防烫组件同位,同样藏掉,关闭时恢复 */
    g_favask_active = 1;
    g_favask_mode = mode;
    printf("[hint] fav ask show mode=%d\n", mode);
}

// 重复收藏:覆盖保存确认(favorites_save_current 检测到已收藏时调用)
void nav_show_fav_ask(void)
{
    fav_ask_show(FAV_ASK_DUPLICATE);
}

// 收藏夹已满:确认后进删除界面(favorites_save_current 检测满时调用)
void nav_show_fav_full(void)
{
    fav_ask_show(FAV_ASK_FULL);
}

// 关闭确认弹层(确认保存后与 BACK 取消共用;完成页在弹层之下,关闭即回到完成页)
void nav_favask_cancel(void)
{
    if (!g_favask_active) return;
    g_favask_active = 0;
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->obj) return;
    if (tf->tip1) lv_obj_add_flag(tf->tip1, LV_OBJ_FLAG_HIDDEN);
    if (tf->tip2) lv_obj_add_flag(tf->tip2, LV_OBJ_FLAG_HIDDEN);
    if (tf->sure) {
        lv_obj_add_flag(tf->sure, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_state(tf->sure, LV_STATE_FOCUSED);
    }
    if (tf->container_1) lv_obj_add_flag(tf->container_1, LV_OBJ_FLAG_HIDDEN);
    nav_favtip_restore_elems();
    printf("[hint] fav ask cancel\n");
}

int nav_favask_active(void)
{
    return g_favask_active;
}

int nav_favask_get_mode(void)
{
    return g_favask_mode;
}
