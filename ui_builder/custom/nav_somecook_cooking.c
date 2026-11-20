/*
 * nav_somecook_cooking.c - 多段烹饪运行页
 * 单页五态(烹饪/暂停/确认退出/完成/编辑)，按步骤顺序运行。
 */
#include "nav.h"
#include "protocol.h"
#include "nav_internal.h"   /* uart_print(完成音随帧下发) */

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
static void somecook_cooking_build_page(somecook_cooking_t *sc);
static void on_somecook_setting_click(lv_event_t *e);
static void on_somecook_setting_focus(lv_event_t *e);
void on_somecook_setting_sure_click(lv_event_t *e);

/* ===== 运行中"设置"覆盖层（2026-11-19 用户要求）=====
 * 其它模式烹饪页都有中间那枚"设置"小按钮，多段页生成层没有，这里运行时补；
 * 点开复用 hot_bbq_setting 的"单温 + 烹饪时间 + 确定"覆盖页（上下烧烤也只一份温度）。
 * 覆盖层打开期间本段到点(2026-11-19 用户定):后台直接切"暂停"并把本段记为已走满,
 * 不动覆盖页/关机弹窗;之后不论主动返回还是 1 分钟无操作自动返回,落到的是
 * "本段计时结束"的暂停态(暂停中 + 00:00:00 + 开始),不自动切下一段。 */
lv_group_t *g_somecook_setting = NULL;   /* 覆盖层焦点组 */
static lv_obj_t *s_little_btn = NULL;    /* 运行页"设置"小按钮(生成层没有,运行时建) */
static int s_little_in_group = 0;        /* 小按钮当前是否在焦点组里(返回确认遮罩时摘出) */
static int s_stage_done_pause = 0;       /* 覆盖层期间本段到点:返回后进"本段到点"暂停态 */

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

    /* 每秒重放一次信息行:EN 排版微调(名窗 226)晚于进页执行,apply_info 只在进页/
       切阶段跑会错过锚定,名窗一整段贴线;文本未变不重设,代价两三次字符串比较 */
    somecook_cooking_apply_info(sc);
}

// activestatus / label_12 显示:合并单行实测超宽才切动态填空(语言/模式/时长通吃,替代原"小时>0"启发)
static void somecook_cooking_apply_info(somecook_cooking_t *sc)
{
    if (!sc || g_somecook_run_idx < 0 || g_somecook_run_idx > 2) return;
    static const char *cn[] = { "一", "二", "三" };
    int i = g_somecook_run_idx;
    char merged[128];
    if (set_hour > 0)
        snprintf(merged, sizeof(merged), tr("| 步骤%s：%s | %d℃ | %d小时%d分钟"),
                 tr(cn[i]), mode_display_name(), set_temp, set_hour, set_min);
    else
        snprintf(merged, sizeof(merged), tr("| 步骤%s：%s | %d℃ | %d分钟"),
                 tr(cn[i]), mode_display_name(), set_temp, set_min);
    nav_strip_wholehour_tail(merged);   /* 整小时只显示小时段(2026-10-21) */
    char step[64];
    snprintf(step, sizeof(step), tr("步骤%s：%s"), tr(cn[i]), mode_display_name());
    nav_status_bar_dynamic(sc->label_12, sc->activestatus, step, merged, 0);   /* 多段维持原 490 口径(2026-10-21 拆分) */
}

// 状态切换：显隐 + 文字 + iface
static void somecook_cooking_set_state(int state, int from)
{
    somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
    if (!sc) return;
    int was_stopback = (g_somecook_state == SMC_STOPBACK);   /* 改写前捕获 */
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
        lv_label_set_text(sc->cookstatus, tr("烹饪中..."));
        if (bl) lv_label_set_text(bl, tr("暂 停"));
        break;
    case SMC_STOP:
        lv_label_set_text(sc->cookstatus, is_english() ? "Pause..." : tr("暂停中..."));   /* EN 对齐生成 STOP 页 cls==1 */
        if (bl) lv_label_set_text(bl, is_english() ? "Resume" : tr("开 始"));
        break;
    case SMC_STOPBACK:
        lv_obj_add_flag(sc->timelabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->container_1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->text1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->text2, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(sc->text1, tr("是否结束当前任务"));
        lv_label_set_text(sc->text2, tr("回到主页？"));
        if (bl) lv_label_set_text(bl, tr("确 定"));
        if (from == SMC_COMPLETE)      lv_label_set_text(sc->cookstatus, tr("已完成"));
        /* EN 暂停源进遮罩保持 Pause...(生成 stopback cls==2 口径,同 SMC_STOP 态写法;
         * 词典 "暂停中..."→"Paused..." 是旧型,自定义页不走 bskin 遍历改写,须在此直写) */
        else if (from == SMC_STOP)     lv_label_set_text(sc->cookstatus, is_english() ? "Pause..." : tr("暂停中..."));
        else                           lv_label_set_text(sc->cookstatus, tr("烹饪中..."));
        break;
    case SMC_COMPLETE:
        lv_obj_add_flag(sc->stop, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(sc->timelabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->text1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->text2, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(sc->text1, tr("高温防烫"));
        lv_label_set_text(sc->text2, tr("请缓慢打开门体！"));
        lv_label_set_text(sc->cookstatus, tr("已完成"));
        lv_bar_set_range(sc->bar_1, 0, 100);
        lv_bar_set_value(sc->bar_1, 100, LV_ANIM_OFF);
        break;
    default: break;
    }

    /* 设置小按钮(2026-11-19):
     *  - 运行/暂停:可点、在焦点组里;
     *  - 返回确认(同页遮罩):仍然显示,但压在遮罩层下面(变暗)且不参与焦点、不可点
     *    (用户:返回确认页显示按钮在遮罩下面,但是不能选中操作);
     *  - 完成态:收起(避免"确定"把已完成的最后一段再跑一遍)。 */
    if (s_little_btn && lv_obj_is_valid(s_little_btn)) {
        if (state == SMC_STOPBACK) {
            lv_obj_clear_flag(s_little_btn, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(s_little_btn, LV_OBJ_FLAG_CLICKABLE);
            if (s_little_in_group) {
                lv_group_remove_obj(s_little_btn);
                s_little_in_group = 0;
            }
            if (sc->stop) lv_group_focus_obj(sc->stop);   /* 遮罩里只有"确定"可操作 */
        } else if (state == SMC_COMPLETE) {
            lv_obj_add_flag(s_little_btn, LV_OBJ_FLAG_HIDDEN);
            if (s_little_in_group) {
                lv_group_remove_obj(s_little_btn);
                s_little_in_group = 0;
            }
        } else {
            lv_obj_clear_flag(s_little_btn, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(s_little_btn, LV_OBJ_FLAG_CLICKABLE);
            if (!s_little_in_group && g_somecook_cooking) {
                lv_group_add_obj(g_somecook_cooking, s_little_btn);
                s_little_in_group = 1;
            }
        }
    }

    /* EN 取消遮罩换装/还原:进遮罩换新文案+Stop Cooking 底图,退出遮罩还原构建
     * 基线(仅英文执行,须在上方文案写入之后;只在跳变时切换——常驻还原会把底图
     * 提前改成 pause.png,构建末尾 nav_btnskin_refit 按 stopbk 匹配失效 → 首帧方钮) */
    if (state == SMC_STOPBACK)
        nav_stopback_overlay_en(sc, 1);
    else if (was_stopback)
        nav_stopback_overlay_en(sc, 0);

    if (state == SMC_STOP)       g_send.iface_status = IFACE_PAUSE;
    else if (state == SMC_COMPLETE) g_send.iface_status = IFACE_COMPLETE;
}

// 提示恢复后重放当前状态显隐(供 nav_hint.c 调用)
void somecook_cooking_refresh(void)
{
    /* 地址复用场景下 current_group 可能误判,先校验页面对象有效再重放 */
    if (depth > 0 && page_stack[depth - 1] != PAGE_SOMECOOK_COOKING) return;   /* 不在本页:不碰本页控件 */
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
    s_little_btn = NULL;   /* 运行页即将换掉,别留悬空指针 */
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
    page_stack[depth - 1] = PAGE_MAJOR_MENU;   /* 收藏启动时栈[1]非主菜单:显式修正,防tune误派发/BACK错乱 */
    lv_obj_clean(lv_scr_act());
    major_menu_create(&ui_manager);
    groups_create();
    bind_events();
    current_group = g_major_menu;
    lang_scr_load_anim(major_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[somecook_cooking] cooking exit -> major_menu\n");
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
    nav_topflag_popups_yield();   /* 门开转暂停态让位:set_state 会回显被弹窗藏掉的元素(2026-10-30) */
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    somecook_cooking_set_state(SMC_STOP, SMC_COOKING);
    somecook_cooking_update_timer(somecook_cooking_get(&ui_manager));
    printf("[somecook_cooking] door open -> auto pause\n");
}

/* 运行页补"设置"小按钮：位置/底图/焦点高亮与其它模式烹饪页同一套(little.png/little100.png,
 * 50x43 @ (609,170))。生成层没有该控件，所以每次重建页面后都补一次。 */
static void somecook_cooking_add_little(somecook_cooking_t *sc)
{
    s_little_btn = NULL;
    if (!sc || !sc->obj) return;
    lv_obj_t *btn = lv_btn_create(sc->obj);
    if (!btn) return;
    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "");
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(btn, 609, 170);
    lv_obj_set_size(btn, 50, 43);
    lv_obj_set_style_bg_opa(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(btn, LVGL_IMAGE_PATH(little.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(btn, &c_montserratmedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(btn, LVGL_IMAGE_PATH(little100.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn, on_somecook_setting_click, LV_EVENT_CLICKED, NULL);
    /* 图层次序:插到遮罩容器(container_1)之前,让"返回确认"的整屏遮罩盖住它变暗;
     * 遮罩隐藏时不影响正常显示(用户:返回确认页显示按钮在遮罩下面)。 */
    if (sc->container_1 && lv_obj_is_valid(sc->container_1))
        lv_obj_move_to_index(btn, lv_obj_get_index(sc->container_1));
    s_little_btn = btn;
}

/* 运行页控件就绪后的统一收尾：焦点组(暂停 + 设置小按钮)、事件、图标、状态行、五态显隐。
 * start / 切下一段 / 从覆盖层返回 三条路径共用（原先两处各自手写一遍）。 */
static void somecook_cooking_build_page(somecook_cooking_t *sc)
{
    if (!sc) return;
    somecook_cooking_add_little(sc);

    lv_obj_t *btns[2];
    int n = 0;
    if (sc->stop) btns[n++] = sc->stop;
    if (s_little_btn) btns[n++] = s_little_btn;
    if (g_somecook_cooking) { lv_group_del(g_somecook_cooking); g_somecook_cooking = NULL; }
    g_somecook_cooking = group_create_for_page(btns, n);
    s_little_in_group = (s_little_btn != NULL) ? 1 : 0;
    clear_focus_states(btns, n);
    if (sc->stop) {
        lv_group_focus_obj(sc->stop);
        lv_obj_add_event_cb(sc->stop, on_somecook_stop_click,
                            LV_EVENT_CLICKED, NULL);
    }

    mode_apply_icon(sc->icon);
    /* 解冻段 icon 位置与其他模式不同（对齐普通解冻 149）——多段维持原几何
       (2026-10-21 用户定稿:图标贴左仅第六感烹饪页,多段不跟) */
    if (g_send.cook_mode == MODE_UNFROZEN)
        lv_obj_set_pos(sc->icon, 149, 161);
    else
        lv_obj_set_pos(sc->icon, 115, 161);
    somecook_cooking_apply_info(sc);
    somecook_cooking_set_state(g_somecook_state, g_somecook_from);
    current_group = g_somecook_cooking;
}

/* 小按钮 → 打开设置覆盖层 */
static void on_somecook_setting_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_somecook_setting();
}

// 进入当前段 cooking
void somecook_cooking_start(void)
{
    /* 覆盖层"确定"路径:上一刻可能已因本段到点切成暂停,起跑必须显式回运行态 */
    g_somecook_state = SMC_COOKING;
    g_somecook_from = SMC_COOKING;
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

    /* 收藏:进入 cooking 时快照初始参数——全项目 24 个烹饪入口均有,唯多段漏,
     * s_fav_saved_this_run 残留上次会话的 1,完成页按 FAV 直接走"取消收藏"分支
     * (2026-10-15 用户实测:先收藏过其它烹饪,再跑多段收藏直接显示 Removed) */
    fav_snapshot_save();

    page_push(PAGE_SOMECOOK_COOKING);
    lv_obj_clean(lv_scr_act());
    somecook_cooking_create(&ui_manager);

    somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
    somecook_cooking_build_page(sc);

    cook_start_time = lv_tick_get();
    cook_total_ms = ((int64_t)set_hour * 3600 + set_min * 60) * 1000;
    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.set_temp_lower = (g_send.cook_mode == MODE_UPDOWN_BBQ) ? set_temp : 0;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
    somecook_cooking_update_timer(somecook_cooking_get(&ui_manager));

    lang_scr_load_anim(somecook_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[somecook_cooking] cooking start: step %d mode=%d\n", i, g_send.cook_mode);
}

// 当前段完成：切下一段 / 全部完成 → complete 态
void somecook_cooking_next_step(void)
{
    /* 切段重建页让位:弹窗悬浮 layer_top,重建后其文字压新页元素,先收(2026-10-30) */
    nav_topflag_popups_yield();
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
        somecook_cooking_build_page(sc);
        g_send.iface_status = IFACE_COMPLETE;
        /* 全部段做完才发完成音(2026-11-04 用户定:段间切换保持安静);
           nav_cooktimer 的两处切段调用点已不再发,统一由这里收尾 */
        g_send.buzzer_req = BUZZER_COOK_DONE;
        somecook_cooking_set_state(SMC_COMPLETE, SMC_COMPLETE);
        somecook_steps_mark_consumed();   /* 步骤已消费:标记待清(KEY_FAV 仍可存完整步骤) */
        lang_scr_load_anim(somecook_cooking_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        uart_print();   /* 完成音随帧立即下发(模拟器走此处,实机由 100ms 帧定时器消费同一请求) */
        printf("[somecook_cooking] all steps done -> complete\n");
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

/* ==============================
 * 多段运行中"设置"覆盖层（2026-11-19 用户）
 * ------------------------------
 * 需求（用户截图+批注）：
 *   ① 运行页中间补上和其它模式一样的"设置"小按钮；
 *   ② 点开可改当前段的温度/时间，上下烧烤也只有一份温度（复用 hot_bbq_setting
 *      的"单温 + 烹饪时间 + 确定"版式）；
 *   ③ 在此界面期间本段烹饪时间结束，不切下一段；按"确定"则本段按新温度/时间重跑；
 *      不操作 1 分钟自动回运行页并按原规则进入下一段。
 * ============================== */

int somecook_setting_is_open(void)
{
    return depth > 0 && page_stack[depth - 1] == PAGE_SOMECOOK_SETTING;
}

int somecook_setting_stage_was_expired(void)
{
    return s_stage_done_pause;
}
/* 覆盖页右侧倒计时=多段总剩余(用户 2026-11-19:打开设置温度时间时右边显示总时间就行;
 * 本段到达 00:00:00 的判据仍在 cooking_timer_cb 里用本段 cook_total_ms) */
void somecook_setting_update_countdown(void)
{
    hot_bbq_setting_t *set = hot_bbq_setting_get(&ui_manager);
    if (!set || !set->timelabel) return;
    int s = (int)((somecook_total_remaining_ms() + 500) / 1000);
    if (s < 0) s = 0;
    lv_label_set_text_fmt(set->timelabel, "%02d:%02d:%02d", s / 3600, (s % 3600) / 60, s % 60);
}

/* 覆盖层期间本段到点(2026-11-19 用户定):后台直接切暂停,本段记为"已走满"。
 * 这里不碰覆盖页控件(页面对象是覆盖层的),只改运行态;返回后由 rebuild 按暂停态重建。 */
void somecook_setting_stage_expired(void)
{
    s_stage_done_pause = 1;              /* 返回时落到"本段计时结束"的暂停态 */
    cook_elapsed_saved = cook_total_ms;  /* 本段剩余 = 0(暂停态倒计时显示 00:00:00) */
    g_somecook_state = SMC_STOP;         /* 后台状态:暂停 */
    g_somecook_from = SMC_COOKING;
    g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = 0;
    somecook_setting_update_countdown();
    printf("[somecook_cooking] setting: stage time ended -> background pause\n");
}

/* 从"设置"覆盖页进入的关机确认(用户 2026-11-20):本段到点只做后台暂停 —— 与设置页到点
 * 同口径(本段记满/状态暂停/iface=PAUSE),确认框原样保留、不切下一段。
 * 页面就地切暂停态而不是重建:确认框的 collector 收的是本页右栏对象(倒计时/开始),
 * 重建会让它指向已删对象;就地切完重新收一次右栏,取消确认后看到的就是"本段到点"
 * 暂停页(倒计时=后续段总时长,按原规则冻结)。到点后按"开始"仍走原路:起点按已走满
 * 恢复,下一秒切下一段。 */
void somecook_powerask_stage_expired(void)
{
    s_stage_done_pause = 1;
    cook_elapsed_saved = cook_total_ms;
    g_somecook_state = SMC_STOP;
    g_somecook_from = SMC_COOKING;
    g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = 0;

    somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
    if (sc && sc->obj && lv_obj_is_valid(sc->obj)) {
        somecook_cooking_set_state(SMC_STOP, SMC_COOKING);
        somecook_cooking_update_timer(sc);
        nav_favtip_collect_hide();   /* set_state 回显了被弹窗藏掉的右栏(倒计时/开始):重新收栏 */
    }
    printf("[somecook_cooking] power ask (from setting): stage time ended -> background pause\n");
}

/* 覆盖层焦点：下划线随焦点，℃ 单位按位数显隐（与各模式设置页同款） */
static void on_somecook_setting_focus(lv_event_t *e)
{
    on_edit_focus(e);
    update_hot_bbq_dir_icon(hot_bbq_setting_get(&ui_manager));
}

/* 运行页小按钮 → 设置覆盖层（计时器继续跑，与其它模式一致） */
void jump_to_somecook_setting(void)
{
    if (g_somecook_state != SMC_COOKING && g_somecook_state != SMC_STOP) return;   /* 完成/退出确认不设 */
    if (g_somecook_run_idx < 0 || g_somecook_run_idx > 2) return;
    if (!g_steps[g_somecook_run_idx].set) return;

    s_stage_done_pause = 0;
    page_push(PAGE_SOMECOOK_SETTING);
    lv_obj_clean(lv_scr_act());
    hot_bbq_setting_create(&ui_manager);

    hot_bbq_setting_t *set = hot_bbq_setting_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = { set->temp, set->hour, set->min, set->sure };
        if (g_somecook_setting) { lv_group_del(g_somecook_setting); g_somecook_setting = NULL; }
        g_somecook_setting = group_create_for_page(btns, 4);

        edit_clear();
        /* 温度/小时范围跟当前段的模式走(用户 2026-11-20:不同模式的温度/时间限制不同)。
         * 与步骤设置页共用 stepset 范围表;模式下限可能高于当前值(如节能热风 125℃ 起),
         * 先夹一下再注册,免得初始值超出可调范围 */
        {
            int tmin = 30, tmax = 300, hmax = 4;
            stepset_mode_range(g_steps[g_somecook_run_idx].mode, &tmin, &tmax, &hmax);
            if (set_temp < tmin) set_temp = tmin;
            if (set_temp > tmax) set_temp = tmax;
            if (set_hour > hmax) set_hour = hmax;
            edit_register_temp(set->temp, set->templine2, set->templine3,
                               &set_temp, tmin, tmax, 5, "%d");
            edit_register(set->hour, set->hourline, NULL, &set_hour, 0, hmax, 1, "%02d");
        }
        edit_register(set->min, set->minline, NULL, &set_min, 0, 59, 1, "%02d");
        nav_blink_extra(set->temp, set->icon2);
        nav_blink_extra(set->temp, set->icon3);
        nav_blink_extra(set->hour, set->label_193);
        nav_blink_extra(set->min, set->label_194);

        lv_obj_add_event_cb(set->temp, on_somecook_setting_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->hour, on_somecook_setting_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->min,  on_somecook_setting_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_somecook_setting_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure, on_somecook_setting_sure_click, LV_EVENT_CLICKED, NULL);

        /* 右侧倒计时=多段总剩余(用户 2026-11-19 定:打开设置温度时间时显示总时间) */
        somecook_setting_update_countdown();
        lv_label_set_text_fmt(set->temp, "%d", temp_disp_c(set_temp));
        lv_label_set_text_fmt(set->hour, "%02d", set_hour);
        lv_label_set_text_fmt(set->min, "%02d", set_min);

        /* 单位/下划线初始全隐，聚焦时点亮 */
        lv_obj_add_flag(set->icon2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->templine2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->templine3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hourline, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->minline, LV_OBJ_FLAG_HIDDEN);

        lv_group_focus_obj(set->temp);
        update_hot_bbq_dir_icon(set);
    }
    current_group = g_somecook_setting;

    lang_scr_load_anim(hot_bbq_setting_get(&ui_manager)->obj,
                       LV_SCR_LOAD_ANIM_NONE, 0, 0,
                       ui_manager.auto_del);

    /* 右侧倒计时与运行页同位(用户 2026-11-20:两页位置要一致)。
     * 运行页 EN 在 somecook_cooking_common 里把 timelabel 挪到 (902,162)、尺寸=内容;
     * 覆盖页复用 hot_bbq_setting 的 timelabel,EN tune 给的是 (907,157)+257x60,差 (5,-5)。
     * 中/繁两页都用生成值 (907,157)+235x60,本来一致,不动 */
    if (is_english()) {
        hot_bbq_setting_t *tset = hot_bbq_setting_get(&ui_manager);
        if (tset && tset->timelabel) {
            lv_obj_set_pos(tset->timelabel, 902, 162);
            lv_obj_set_size(tset->timelabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        }
    }
    printf("[somecook_cooking] jump: cooking -> setting (step %d)\n", g_somecook_run_idx);
}

/* 设置页"确定"：把新温度/时间写回当前段，退出覆盖层并按新参数重跑本段 */
void on_somecook_setting_sure_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    edit_clear();
    int i = g_somecook_run_idx;
    if (i < 0 || i > 2 || !g_steps[i].set) return;

    g_steps[i].temp = set_temp;
    g_steps[i].hour = set_hour;
    g_steps[i].min  = set_min;
    s_stage_done_pause = 0;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

    /* 退出覆盖层 + 旧运行页，再按新参数重建（与切段同一路径：先裁栈再 start） */
    if (depth > 0 && page_stack[depth - 1] == PAGE_SOMECOOK_SETTING) depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_SOMECOOK_COOKING)  depth--;
    somecook_cooking_start();
    printf("[somecook_cooking] setting sure: step %d -> temp=%d %d:%02d, restart\n",
           i, set_temp, set_hour, set_min);
}

/* 从覆盖层/子页返回运行页（nav_pop 的 PAGE_SOMECOOK_COOKING 分支调用）：
 * 未确定的编辑丢弃（按 g_steps 原值重建）；覆盖层期间本段到点的话，后台已在
 * somecook_setting_stage_expired 里切成暂停，这里按暂停态重建即可（用户 2026-11-19：
 * 主动返回/自动返回都落到"本段计时结束"的暂停页，不再自动切下一段）。 */
void somecook_cooking_rebuild(page_id_t child)
{
    int was_done = s_stage_done_pause;
    s_stage_done_pause = 0;

    int i = g_somecook_run_idx;
    if (i >= 0 && i < 3 && g_steps[i].set) {
        set_temp = g_steps[i].temp;
        set_hour = g_steps[i].hour;
        set_min  = g_steps[i].min;
        g_send.cook_mode = g_steps[i].mode;
    }
    lv_obj_clean(lv_scr_act());
    somecook_cooking_create(&ui_manager);
    somecook_cooking_build_page(somecook_cooking_get(&ui_manager));
    somecook_cooking_update_timer(somecook_cooking_get(&ui_manager));

    lang_scr_load_anim(somecook_cooking_get(&ui_manager)->obj,
                       LV_SCR_LOAD_ANIM_NONE, 0, 0,
                       ui_manager.auto_del);
    g_send.iface_status = (g_somecook_state == SMC_STOP) ? IFACE_PAUSE : IFACE_COOKING;
    printf("[somecook_cooking] back from setting -> %s (step %d)\n",
           was_done ? "paused(stage ended)" : "cooking", i);
}

