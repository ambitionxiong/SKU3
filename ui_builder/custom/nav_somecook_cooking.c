/*
 * nav_somecook_cooking.c - 多段烹饪运行页
 * 单页五态(烹饪/暂停/确认退出/完成/编辑)，按步骤顺序运行。
 */
#include "nav.h"
#include "protocol.h"

/* ==============================
 * 多段运行（somecook_cooking 单页五态）
 * ============================== */

lv_group_t *g_somecook_cooking;   /* somecook_cooking 运行页焦点组 */
uint8_t g_somecook_running = 0;   /* 多段运行中 */
int g_somecook_run_idx = 0;       /* 当前运行段 0-2 */

enum { SMC_COOKING, SMC_STOP, SMC_STOPBACK, SMC_COMPLETE };
static uint8_t g_somecook_state = SMC_COOKING;
static uint8_t g_somecook_from = SMC_COOKING;   /* stopback 进入源 */

static void somecook_cooking_exit(void);
static void somecook_cooking_set_state(int state, int from);
void somecook_cooking_update_timer(somecook_cooking_t *sc);
static void somecook_cooking_apply_info(somecook_cooking_t *sc);

// 所有已设置步骤总时间（进度条分母）
static int64_t somecook_total_time_ms(void)
{
    int64_t t = 0;
    for (int k = 0; k < 3; k++)
        if (g_steps[k].set)
            t += ((int64_t)g_steps[k].hour * 3600 + g_steps[k].min * 60) * 1000;
    return t;
}

// 当前段之前已设置步骤的完整时间（总进度已走部分）
static int64_t somecook_prev_steps_ms(void)
{
    int64_t t = 0;
    for (int k = 0; k < g_somecook_run_idx && k < 3; k++)
        if (g_steps[k].set)
            t += ((int64_t)g_steps[k].hour * 3600 + g_steps[k].min * 60) * 1000;
    return t;
}

// 总倒计时 = 当前段剩余 + 后续已设置步骤时间总和
static int64_t somecook_total_remaining_ms(void)
{
    int64_t cur;
    if (g_somecook_state == SMC_STOP)
        cur = (int64_t)cook_total_ms - cook_elapsed_saved;
    else
        cur = (int64_t)cook_total_ms - (lv_tick_get() - cook_start_time);
    if (cur < 0) cur = 0;
    for (int k = g_somecook_run_idx + 1; k < 3; k++)
        if (g_steps[k].set)
            cur += ((int64_t)g_steps[k].hour * 3600 + g_steps[k].min * 60) * 1000;
    return cur;
}

// 刷新倒计时（总）与总进度条 + 发送剩余时间
void somecook_cooking_update_timer(somecook_cooking_t *sc)
{
    if (!sc) return;
    int64_t rem = somecook_total_remaining_ms();
    int total_sec = (int)((rem + 500) / 1000);
    int h = total_sec / 3600, m = (total_sec % 3600) / 60, s = total_sec % 60;
    lv_label_set_text_fmt(sc->timelabel, "%02d:%02d:%02d", h, m, s);

    int64_t total_ms = somecook_total_time_ms();
    uint32_t cur_elapsed = (g_somecook_state == SMC_STOP) ? (uint32_t)cook_elapsed_saved
                                                          : (lv_tick_get() - cook_start_time);
    int64_t done = somecook_prev_steps_ms() + cur_elapsed;
    int p = 3 + (int)(done * 97 / (total_ms ? total_ms : 1));
    if (p > 100) p = 100;
    if (p < 3) p = 3;
    lv_bar_set_range(sc->bar_1, 0, 100);
    lv_bar_set_value(sc->bar_1, p, LV_ANIM_OFF);

    g_send.remaining_ms = (int32_t)(rem > INT32_MAX ? INT32_MAX : rem);
}

// activestatus / label_12 按小时规则显示
static void somecook_cooking_apply_info(somecook_cooking_t *sc)
{
    if (!sc || g_somecook_run_idx < 0 || g_somecook_run_idx > 2) return;
    static const char *cn[] = { "一", "二", "三" };
    int i = g_somecook_run_idx;
    if (set_hour > 0) {
        lv_obj_clear_flag(sc->activestatus, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_fmt(sc->activestatus, "步骤%s：%s", cn[i], mode_display_name());
        lv_label_set_text_fmt(sc->label_12, "|                         | %d℃ | %d小时%02d分钟",
                              set_temp, set_hour, set_min);
    } else {
        lv_obj_add_flag(sc->activestatus, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_fmt(sc->label_12, "| 步骤%s：%s | %d℃ | %02d分钟",
                              cn[i], mode_display_name(), set_temp, set_min);
    }
}

// 状态切换：显隐 + 文字 + iface
static void somecook_cooking_set_state(int state, int from)
{
    somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
    if (!sc) return;
    g_somecook_state = state;
    g_somecook_from = from;

    lv_obj_clear_flag(sc->icon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->cookstatus, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->label_12, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->image_10, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->bar_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->timelabel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->stop, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sc->container_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sc->text1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sc->text2, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *bl = lv_obj_get_child(sc->stop, 0);
    switch (state) {
    case SMC_COOKING:
        lv_label_set_text(sc->cookstatus, "烹饪中...");
        if (bl) lv_label_set_text(bl, "暂 停");
        break;
    case SMC_STOP:
        lv_label_set_text(sc->cookstatus, "暂停中...");
        if (bl) lv_label_set_text(bl, "开 始");
        break;
    case SMC_STOPBACK:
        lv_obj_add_flag(sc->timelabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->container_1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->text1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->text2, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(sc->text1, "是否结束当前任务");
        lv_label_set_text(sc->text2, "回到主页");
        if (bl) lv_label_set_text(bl, "确 定");
        if (from == SMC_COMPLETE)      lv_label_set_text(sc->cookstatus, "已完成");
        else if (from == SMC_STOP)     lv_label_set_text(sc->cookstatus, "暂停中...");
        else                           lv_label_set_text(sc->cookstatus, "烹饪中...");
        break;
    case SMC_COMPLETE:
        lv_obj_add_flag(sc->stop, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(sc->timelabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->text1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->text2, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(sc->text1, "高温防烫");
        lv_label_set_text(sc->text2, "请缓慢打开门体！");
        lv_label_set_text(sc->cookstatus, "已完成");
        lv_bar_set_range(sc->bar_1, 0, 100);
        lv_bar_set_value(sc->bar_1, 100, LV_ANIM_OFF);
        break;
    default: break;
    }

    if (state == SMC_STOP)       g_send.iface_status = IFACE_PAUSE;
    else if (state == SMC_COMPLETE) g_send.iface_status = IFACE_COMPLETE;
}

// 提示恢复后重放当前状态显隐(供 nav_hint.c 调用)
void somecook_cooking_refresh(void)
{
    /* 地址复用场景下 current_group 可能误判,先校验页面对象有效再重放 */
    somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
    if (sc && sc->obj && lv_obj_is_valid(sc->obj))
        somecook_cooking_set_state(g_somecook_state, g_somecook_from);
}

// 多段是否处于 stopback 遮罩确认态(供 nav_hint.c 判断不弹提示)
int somecook_cooking_is_stopback(void)
{
    return g_somecook_state == SMC_STOPBACK;
}

// 取消/停止确认 → 回主菜单（清状态）
static void somecook_cooking_exit(void)
{
    g_somecook_running = 0;
    g_somecook_run_idx = 0;
    g_somecook_state = SMC_COOKING;
    g_somecook_from = SMC_COOKING;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    g_on_stop_back = 0;
    g_stop_back_complete = NULL;
    cook_elapsed_saved = 0; cook_bar_saved = 0; cook_total_ms = 0;
    memset(g_steps, 0, sizeof(g_steps));   /* 清已设置步骤信息 */
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0; g_send.set_temp_lower = 0; g_send.remaining_ms = -1;
    depth = 2;
    lv_obj_clean(lv_scr_act());
    major_menu_create(&ui_manager);
    groups_create();
    bind_events();
    current_group = g_major_menu;
    lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[somecook] cooking exit -> major_menu\n");
}

// 运行页 stop 按钮："暂 停" / "开 始" / "确 定"
static void on_somecook_stop_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    if (g_somecook_state == SMC_COOKING) {
        cook_elapsed_saved = lv_tick_get() - cook_start_time;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        somecook_cooking_set_state(SMC_STOP, SMC_COOKING);
        somecook_cooking_update_timer(somecook_cooking_get(&ui_manager));
    } else if (g_somecook_state == SMC_STOP) {
        cook_start_time = lv_tick_get() - cook_elapsed_saved;
        if (cook_timer) lv_timer_del(cook_timer);
        cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
        g_send.iface_status = IFACE_COOKING;
        somecook_cooking_set_state(SMC_COOKING, SMC_STOP);
    } else if (g_somecook_state == SMC_STOPBACK) {
        somecook_cooking_exit();
    }
}

// 开门自动暂停（多段运行留在本页,手动"开 始"恢复）
void somecook_cooking_auto_pause(void)
{
    if (g_somecook_state != SMC_COOKING) return;
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    somecook_cooking_set_state(SMC_STOP, SMC_COOKING);
    somecook_cooking_update_timer(somecook_cooking_get(&ui_manager));
    printf("[somecook] door open -> auto pause\n");
}

// 进入当前段 cooking
static void somecook_cooking_start(void)
{
    /* 空洞步骤:从 run_idx 向后找第一个已设置步骤再启动 */
    int i = g_somecook_run_idx;
    while (i < 3 && !g_steps[i].set) i++;
    if (i >= 3) {
        g_somecook_running = 0;
        return;
    }
    g_somecook_run_idx = i;
    edit_clear();
    g_on_stop_back = 0;
    g_stop_back_complete = NULL;

    set_temp = g_steps[i].temp;
    set_hour = g_steps[i].hour;
    set_min  = g_steps[i].min;
    g_send.cook_mode = g_steps[i].mode;

    page_push(PAGE_SOMECOOK_COOKING);
    lv_obj_clean(lv_scr_act());
    somecook_cooking_create(&ui_manager);

    somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
    if (sc) {
        lv_obj_t *btns[] = { sc->stop };
        if (g_somecook_cooking) lv_group_del(g_somecook_cooking);
        g_somecook_cooking = group_create_for_page(btns, 1);
        clear_focus_states(btns, 1);
        lv_group_focus_obj(sc->stop);
        lv_obj_add_event_cb(sc->stop, on_somecook_stop_click,
                            LV_EVENT_CLICKED, NULL);

        mode_apply_icon(sc->icon);
        /* 解冻段 icon 位置与其他模式不同（对齐普通解冻 149） */
        if (g_send.cook_mode == MODE_UNFROZEN)
            lv_obj_set_pos(sc->icon, 149, 161);
        else
            lv_obj_set_pos(sc->icon, 115, 161);
        somecook_cooking_apply_info(sc);
        somecook_cooking_set_state(SMC_COOKING, SMC_COOKING);
    }
    current_group = g_somecook_cooking;

    cook_start_time = lv_tick_get();
    cook_total_ms = ((int64_t)set_hour * 3600 + set_min * 60) * 1000;
    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.set_temp_lower = (g_send.cook_mode == MODE_UPDOWN_BBQ) ? set_temp : 0;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
    somecook_cooking_update_timer(somecook_cooking_get(&ui_manager));

    lv_scr_load_anim(somecook_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[somecook] cooking start: step %d mode=%d\n", i, g_send.cook_mode);
}

// 当前段完成：切下一段 / 全部完成 → complete 态
void somecook_cooking_next_step(void)
{
    /* 空洞步骤:跳到下一个已设置步骤,没有则全部完成 */
    g_somecook_run_idx++;
    while (g_somecook_run_idx < 3 && !g_steps[g_somecook_run_idx].set)
        g_somecook_run_idx++;
    if (g_somecook_run_idx < 3) {
        if (depth > 0 && page_stack[depth - 1] == PAGE_SOMECOOK_COOKING)
            depth--;
        somecook_cooking_start();
    } else {
        /* 回退到最后一段索引,完成页按该段模式/温度/时间显示(空洞跳过) */
        g_somecook_run_idx = 2;
        while (g_somecook_run_idx > 0 && !g_steps[g_somecook_run_idx].set)
            g_somecook_run_idx--;
        g_somecook_running = 0;
        if (depth > 0 && page_stack[depth - 1] == PAGE_SOMECOOK_COOKING)
            depth--;
        page_push(PAGE_SOMECOOK_COOKING);
        lv_obj_clean(lv_scr_act());
        somecook_cooking_create(&ui_manager);
        somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
        if (sc) {
            lv_obj_t *btns[] = { sc->stop };
            if (g_somecook_cooking) lv_group_del(g_somecook_cooking);
            g_somecook_cooking = group_create_for_page(btns, 1);
            clear_focus_states(btns, 1);
            lv_group_focus_obj(sc->stop);
            lv_obj_add_event_cb(sc->stop, on_somecook_stop_click,
                                LV_EVENT_CLICKED, NULL);
            mode_apply_icon(sc->icon);
            /* 解冻段 icon 位置与其他模式不同（对齐普通解冻 149） */
            if (g_send.cook_mode == MODE_UNFROZEN)
                lv_obj_set_pos(sc->icon, 149, 161);
            else
                lv_obj_set_pos(sc->icon, 115, 161);
            somecook_cooking_apply_info(sc);
        }
        current_group = g_somecook_cooking;
        g_send.iface_status = IFACE_COMPLETE;
        somecook_cooking_set_state(SMC_COMPLETE, SMC_COMPLETE);
        lv_scr_load_anim(somecook_cooking_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[somecook] all steps done -> complete\n");
    }
}

// 运行页 BACK：cooking/stop → stopback；stopback → 回源；complete → stopback
void somecook_cooking_handle_back(void)
{
    if (g_somecook_state == SMC_COOKING)
        somecook_cooking_set_state(SMC_STOPBACK, SMC_COOKING);
    else if (g_somecook_state == SMC_STOP)
        somecook_cooking_set_state(SMC_STOPBACK, SMC_STOP);
    else if (g_somecook_state == SMC_STOPBACK)
        somecook_cooking_set_state(g_somecook_from, g_somecook_from);
    else if (g_somecook_state == SMC_COMPLETE)
        somecook_cooking_set_state(SMC_STOPBACK, SMC_COMPLETE);
}

// somecook 确定 → 开始多段运行
void on_somecook_sure_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    int nset = 0;
    for (int i = 0; i < 3; i++) if (g_steps[i].set) nset++;
    if (nset < 2) return;
    g_somecook_running = 1;
    g_somecook_run_idx = 0;
    if (depth > 0 && page_stack[depth - 1] == PAGE_SOMECOOK)
        depth--;
    somecook_cooking_start();
}
