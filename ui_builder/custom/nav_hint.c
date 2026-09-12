/*
 * nav_hint.c - 功能键无效提示弹窗 + 收藏保存结果提示
 * 烹饪中按功能键弹出'烤箱运行时不可用'提示(3秒自动关闭)，
 * 期间仅BACK有效，UAF安全处理。
 */

#include "nav.h"
#include "protocol.h"
#include "nav_internal.h"

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
    /* 清洁四模式烹饪页:右侧只有 timelabel(倒计时),无 temp/探针图标 */
    else if (current_group == g_wc_cooking) {
        waterclean_cooking_t *c = waterclean_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_hcs_cooking) {
        hotcleansave_cooking_t *c = hotcleansave_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_hcm_cooking) {
        hotcleanmiddle_cooking_t *c = hotcleanmiddle_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    } else if (current_group == g_hch_cooking) {
        hotcleanhigh_cooking_t *c = hotcleanhigh_cooking_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->timelabel;
    }
    /* 热解清洁冷却页:右侧提示图标(x902),无效弹窗时一并隐藏 */
    else if (current_group == g_hcs_cooling) {
        hotcleansave_cooling_t *c = hotcleansave_cooling_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->image_8;
    } else if (current_group == g_hcm_cooling) {
        hotcleanmiddle_cooling_t *c = hotcleanmiddle_cooling_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->image_11;
    } else if (current_group == g_hch_cooling) {
        hotcleanhigh_cooling_t *c = hotcleanhigh_cooling_get(&ui_manager); if (c) g_hint_objs[g_hint_n++] = c->image_22;
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
    topflagpage_lang_tune();   /* 英文排版:tip1 三行折行+高度自适应(每次显示刷新) */
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

/* 登记并隐藏单个右侧组件(去重:几何收集可能已含同一对象,重复登记会污染 was[] 恢复标记) */
static void favtip_collect_add(lv_obj_t *obj)
{
    if (!obj || g_favtip_n >= 8) return;
    if (!lv_obj_is_valid(obj)) return;
    for (int i = 0; i < g_favtip_n; i++)
        if (g_favtip_objs[i] == obj) return;
    g_favtip_was[g_favtip_n] = lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_event_cb(obj, favtip_del_cb);
    lv_obj_add_event_cb(obj, favtip_del_cb, LV_EVENT_DELETE, NULL);
    g_favtip_objs[g_favtip_n++] = obj;
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
    /* 多段/六感复用 somecook 页:英文 tune 把 text1/text2 挪到 x=725(<800),
     * 上面的几何收集漏掉 → 收藏弹窗与"高温防烫"文字叠显;按页面结构显式补收 */
    if (current_group == g_somecook_cooking || current_group == g_six_cooking) {
        somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
        if (sc) {
            favtip_collect_add(sc->text1);
            favtip_collect_add(sc->text2);
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
    nav_favtip_hide();   /* 重复触发:重置 2 秒计时 */
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->obj || !tf->tip3) return;
    lv_label_set_text(tf->tip3, tr("收藏成功"));   /* 每次显示时设文本,随语言切换 */
    nav_favtip_collect_hide();   /* 隐藏完成页右侧组件(防烫图标/文字),恢复在 hide */
    lv_obj_clear_flag(tf->tip3, LV_OBJ_FLAG_HIDDEN);
    g_favtip_timer = lv_timer_create(favtip_timer_cb, 2000, NULL);
    printf("[hint] fav tip show\n");
}

// 通用状态提示:右侧 tip3 显示任意文本 2 秒,无遮罩、不隐藏页面元素
// (设置覆盖层开关项的即时反馈等)
void nav_show_state_tip(const char *text)
{
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->obj || !tf->tip3) return;
    nav_favtip_hide();   /* 重置计时 */
    lv_label_set_text(tf->tip3, text);
    lv_obj_clear_flag(tf->tip3, LV_OBJ_FLAG_HIDDEN);
    g_favtip_timer = lv_timer_create(favtip_timer_cb, 2000, NULL);
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

/* ==============================
 * 童锁（topflag 顶层弹窗级,任意界面生效）
 * 素材/控件:9.4.2 已在 topflagpage.c 建好 image_1(lockifr 面板)
 *   image_2(childlock 图标) locktip1(已锁定) locktip2(长按旋钮3秒)
 *   locktip3(模式名+状态,五态动态),默认全隐藏。
 * 开启:设置页"童锁→开"(nav_screen_set.c SEL_WHERE_TS)→nav_childlock_set(1)。
 * 锁定中:process_key 守卫链最顶吞掉一切按键(nav_key.c)。
 * 解锁:长按旋钮(KEY_ENCODER_PRESS)3秒——真机走 nav_handle_key
 *   KEY_PRESSED 分支,模拟器(边缘喂数)走 sim_scan_cb 轮询
 *   nav_childlock_hold_poll,两路都调 nav_childlock_try_unlock(幂等)。
 * ============================== */
static int g_childlock_active = 0;

/* 锁层第三行刷新签名:界面状态/保温标志/模式名任一变化才重排(由 topflag_clock_cb 驱动) */
static int s_cl_sig_status = -1;
static int s_cl_sig_keepwarm = -1;
static const char *s_cl_sig_mode = NULL;

static void childlock_sig_update(void)
{
    s_cl_sig_status = g_send.iface_status;
    s_cl_sig_keepwarm = g_keepwarm_active;
    s_cl_sig_mode = mode_display_name();
}

/* locktip3 显示时 icon/tip1/tip2 用原位,隐藏时下移 41px:
   lockifr 面板 y 119..370(中心 244.5),icon 高 121 → 居中 y=184=143+41,tip 跟随 */
#define LOCK_DY_HIDE 41

static void childlock_apply_layout(topflagpage_t *tf)
{
    const char *word = NULL;
    if (g_keepwarm_active)                        word = "保温中";   /* 完成态内保温,优先于已完成 */
    else if (g_send.iface_status == IFACE_COOKING)      word = "烹饪中";
    else if (g_send.iface_status == IFACE_PAUSE)        word = "暂停中";
    else if (g_send.iface_status == IFACE_DELAY_RESERVE) word = "预约中";
    else if (g_send.iface_status == IFACE_COMPLETE)     word = "已完成";

    int dy = 0;
    if (word && tf->locktip3 && mode_display_name()) {
        /* 中文模式名+状态直连,英文中间补空格 */
        lv_label_set_text_fmt(tf->locktip3, is_english() ? "%s %s" : "%s%s",
                              mode_display_name(), tr(word));
        lv_obj_clear_flag(tf->locktip3, LV_OBJ_FLAG_HIDDEN);
    } else {
        if (tf->locktip3) lv_obj_add_flag(tf->locktip3, LV_OBJ_FLAG_HIDDEN);
        dy = LOCK_DY_HIDE;
    }
    if (tf->image_2)  lv_obj_set_pos(tf->image_2, 453, 143 + dy);
    if (is_english()) {
        /* 英文版式(9.8 设计稿+9.9 上机反馈):字号与中文一致(30/24/36)仅换
           Aktiv 字库;图标不动,标题/副标题左对齐排在图标右侧(x=600=图标
           右缘574+26,y 各上移 7px 使文字块与图标垂直居中);副标题高度随
           内容放开(原 36 高装不下两行,第二行被截断);第三行面板居中,
           宽 380=设计稿红框(图标左缘453→标题块右缘),超出则循环滚动 */
        if (tf->locktip1) {
            lv_obj_set_style_text_font(tf->locktip1, &c_aktivgroteskmedium_30, 0);
            lv_obj_set_style_text_align(tf->locktip1, LV_TEXT_ALIGN_LEFT, 0);
            lv_obj_set_pos(tf->locktip1, 600, 163 + dy);
        }
        if (tf->locktip2) {
            lv_obj_set_style_text_font(tf->locktip2, &c_aktivgroteskmedium_24, 0);
            lv_obj_set_style_text_align(tf->locktip2, LV_TEXT_ALIGN_LEFT, 0);
            lv_obj_set_pos(tf->locktip2, 600, 204 + dy);
            lv_obj_set_height(tf->locktip2, LV_SIZE_CONTENT);
        }
        if (tf->locktip3) {
            lv_obj_set_style_text_font(tf->locktip3, &c_aktivgroteskmedium_36, 0);
            lv_obj_set_style_text_align(tf->locktip3, LV_TEXT_ALIGN_CENTER, 0);
            lv_label_set_long_mode(tf->locktip3, LV_LABEL_LONG_SCROLL_CIRCULAR);
            lv_obj_set_pos(tf->locktip3, 450, 297);
            lv_obj_set_size(tf->locktip3, 380, LV_SIZE_CONTENT);
        }
    } else {
        /* 中文恢复生成页原版式(370x36 居中,珍珠体;含尺寸/换行模式回写,
           防英文会话残留) */
        if (tf->locktip1) {
            lv_obj_set_style_text_font(tf->locktip1, &c_taiwanpearl_regular_30, 0);
            lv_obj_set_style_text_align(tf->locktip1, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_pos(tf->locktip1, 458, 170 + dy);
        }
        if (tf->locktip2) {
            lv_obj_set_style_text_font(tf->locktip2, &c_taiwanpearl_regular_24, 0);
            lv_obj_set_style_text_align(tf->locktip2, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_pos(tf->locktip2, 528, 211 + dy);
            lv_obj_set_height(tf->locktip2, 36);
        }
        if (tf->locktip3) {
            lv_obj_set_style_text_font(tf->locktip3, &c_taiwanpearl_regular_36, 0);
            lv_obj_set_style_text_align(tf->locktip3, LV_TEXT_ALIGN_CENTER, 0);
            lv_label_set_long_mode(tf->locktip3, LV_LABEL_LONG_WRAP);
            lv_obj_set_pos(tf->locktip3, 455, 297);
            lv_obj_set_size(tf->locktip3, 370, 36);
        }
    }
}

// 开启(on=1)/关闭童锁层;on=0 且未锁定时无操作(幂等)
void nav_childlock_set(int on)
{
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->obj) return;

    if (on) {
        nav_favtip_hide();                    /* 收藏成功 toast 与锁层互斥 */
        if (nav_favask_active()) nav_favask_cancel();   /* 确认弹层让位给锁层 */
        if (tf->locktip1) lv_label_set_text(tf->locktip1, tr("已锁定"));
        if (tf->locktip2) lv_label_set_text(tf->locktip2, tr("长按旋钮3秒进行解锁"));
        childlock_apply_layout(tf);           /* 五态文案+布局(内部刷新 locktip3) */
        childlock_sig_update();               /* 记录开锁瞬间签名,后续靠 refresh 增量刷新 */
        if (tf->image_1) lv_obj_clear_flag(tf->image_1, LV_OBJ_FLAG_HIDDEN);
        if (tf->image_2) lv_obj_clear_flag(tf->image_2, LV_OBJ_FLAG_HIDDEN);
        if (tf->locktip1) lv_obj_clear_flag(tf->locktip1, LV_OBJ_FLAG_HIDDEN);
        if (tf->locktip2) lv_obj_clear_flag(tf->locktip2, LV_OBJ_FLAG_HIDDEN);
        if (tf->container_1) lv_obj_clear_flag(tf->container_1, LV_OBJ_FLAG_HIDDEN);   /* 原生 44% 黑遮罩,下层正常页面透出 */
        g_childlock_active = 1;               /* 先置位再刷显隐:待机页特判依赖 active */
        topflag_update_visibility();
        printf("[hint] childlock on\n");
    } else {
        if (!g_childlock_active) return;
        g_childlock_active = 0;
        if (tf->image_1)   lv_obj_add_flag(tf->image_1, LV_OBJ_FLAG_HIDDEN);
        if (tf->image_2)   lv_obj_add_flag(tf->image_2, LV_OBJ_FLAG_HIDDEN);
        if (tf->locktip1)  lv_obj_add_flag(tf->locktip1, LV_OBJ_FLAG_HIDDEN);
        if (tf->locktip2)  lv_obj_add_flag(tf->locktip2, LV_OBJ_FLAG_HIDDEN);
        if (tf->locktip3)  lv_obj_add_flag(tf->locktip3, LV_OBJ_FLAG_HIDDEN);
        if (tf->container_1) lv_obj_add_flag(tf->container_1, LV_OBJ_FLAG_HIDDEN);
        topflag_update_visibility();
        printf("[hint] childlock off\n");
    }
}

int nav_childlock_active(void)
{
    return g_childlock_active;
}

/* 下层状态变化时刷新锁层第三行(模式名+状态,五态)。
 * 童锁下按键全被吞,状态只经自动流转变化(倒计时归零→完成/保温、预约到点→烹饪),
 * 由常驻 500ms topflag_clock_cb 驱动,签名未变时零开销 */
void nav_childlock_refresh(void)
{
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!g_childlock_active || !tf || !tf->obj) return;
    if (s_cl_sig_status == g_send.iface_status &&
        s_cl_sig_keepwarm == g_keepwarm_active &&
        s_cl_sig_mode == mode_display_name()) return;
    childlock_sig_update();
    childlock_apply_layout(tf);
}

// 长按 3 秒解锁(真机 KEY_PRESSED 分支/模拟器 hold_poll 两路调用,幂等)
void nav_childlock_try_unlock(void)
{
    if (!g_childlock_active) return;
    SET_Data.Set_Lock = 0;    /* 长按解锁写回设置项:设置页回显跟随实际状态 */
    screen_set_ts_lb_sync();  /* 覆盖层存活则同步 TS_Lb(锁定中进不了设置页,通常空操作) */
    nav_childlock_set(0);
    uart_print();             /* 状态帧立即上报,与设置页改动路径一致 */
}
