/*
 * nav_system.c - 系统级组件
 *
 * 职责：
 *   1. system_timer_cb：常驻 500ms 系统检测（探针插拔、门状态边沿、系统时间刷新）
 *   2. nav_init：初始化入口（建页面栈根页、常驻定时器、待机页）
 *   3. topflag 组件：顶栏时间/状态，随页面切换显隐、每 500ms 刷新
 *   4. waitmenu 时钟缓存：待机页时间刷新（避免重复读 RTC）
 *
 * 定时器约定：cook_timer 为烹饪专属(按需创建/删除)；
 *   system_timer_cb 常驻，仅做检测不驱动 UI 倒计时。
 */

#include "nav.h"
#include "nav_idle.h"
#include "nav_internal.h"

/* 文件内前向声明(定义在后方) */
void topflag_clock_cb(lv_timer_t *timer);
void topflag_update_visibility(void);


// ==============================
// 系统定时器（每500ms，常驻检测）
// ==============================

static void system_timer_cb(lv_timer_t *timer)
{
    static int probe_last = 0;
    static uint32_t probe_last_time = 0;
    int probe_now = is_probe_inserted();

    // 门状态边沿检测（预热完成等待放食材阶段：门开又关 → 重建 complete 显示 sure）
    static int door_last = -1;
    int door_now = is_door_open();
    if (door_now != door_last) {
        door_last = door_now;
        if (preheat_wait_door && !door_now) {
            preheat_wait_door = 0;
            if (depth > 0 && (page_stack[depth - 1] == PAGE_PREHEAT_COMPLETE ||
                              page_stack[depth - 1] == PAGE_PREHEAT_STOP_BACK))
                jump_to_preheat_complete();
        }
    }

    if (probe_now == probe_last)
        return;

    uint32_t now = lv_tick_get();
    if (now - probe_last_time < 1000) {
        probe_last = probe_now;
        return;
    }
    probe_last_time = now;
    probe_last = probe_now;

    if (depth > 0 && page_stack[depth - 1] == PAGE_PROBETIP) {
        probetip_t *tip = probetip_get(&ui_manager);
        if (tip && tip->button_1) {
            lv_obj_t *label = lv_obj_get_child(tip->button_1, 0);
            if (label) lv_label_set_text(label, probe_now ? tr("探针已插入") : tr("探针已拔出"));
        }
        return;
    }

    if (g_send.iface_status == IFACE_SLEEP) {
        jump_to_probetip(probe_now ? tr("探针已插入") : tr("探针已拔出"));
        return;
    }

    probetip_cancel_auto_dismiss();   /* 取消陈旧的探针提示自动关闭定时器,防止跨会话误触发 */
    screen_set_reset();               /* 覆盖层若打开:清理对象/组/焦点指针,防悬空 */
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    set_temp = 180; set_temp_up = 180; set_temp_down = 180;
    set_hour = 0; set_min = 30;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    cook_total_ms = 0; cook_is_color = 0;
    preheat_on = 0; delay_on = 0; contain_on = 0;
    g_on_stop_back = 0;
    g_complete_to_stop_back = 0;
    g_cooling_to_stop_back = 0;
    g_extra_color_to_stop_back = 0;
    g_stop_back_complete = NULL;
    g_delay_cancel_btn = 0;
    g_somecook_running = 0;
    g_somecook_run_idx = 0;
    six_cook_reset();   /* 六感运行:清理状态 */
    probe_target_temp = 80;
    g_send.cook_mode = MODE_NONE;
    g_send.cook_flag = 0;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;

    /* 探针状态复位落点=主菜单(返回链终点,不再落待机页),其上弹探针提示页 */
    nav_goto_major_menu();
    jump_to_probetip(probe_now ? tr("探针已插入") : tr("探针已拔出"));
}

// ==============================
// 初始化入口
// ==============================

void nav_init(void)
{
    waitmenu_24_t *wait = waitmenu_24_get(&ui_manager);
    if (!wait) {
        printf("[nav] waitmenu_24 struct is NULL, skip nav_init\n");
        return;
    }
    if (!wait->obj) {
        printf("[nav] waitmenu_24->obj is NULL, skip nav_init\n");
        return;
    }

    printf("[nav] init start\n");
    depth = 0;
    page_push(PAGE_WAITMENU_24);        // 根页 = waitmenu_24
    page_push(PAGE_MAJOR_MENU);         // 上电自动进入 major_menu
    lv_obj_clean(lv_scr_act());
    major_menu_create(&ui_manager);
    groups_create();
    bind_events();
    
    current_group = g_major_menu;
    lang_scr_load_anim(major_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    g_send.iface_status = IFACE_SETTING;
    lv_timer_create(system_timer_cb, 500, NULL);

    /* topflag 顶层状态页：生成代码已改挂 lv_layer_top，所有页面之上（wait 页面除外） */
    topflagpage_create(&ui_manager);
    {
        topflagpage_t *tf = topflagpage_get(&ui_manager);
        if (tf) {
            /* 状态图标先全隐:500ms tick 的 nav_topflag_demo_sync 按状态放行/排布 */
            if (tf->demo)   lv_obj_add_flag(tf->demo, LV_OBJ_FLAG_HIDDEN);
            if (tf->timer)  lv_obj_add_flag(tf->timer, LV_OBJ_FLAG_HIDDEN);
            if (tf->light)  lv_obj_add_flag(tf->light, LV_OBJ_FLAG_HIDDEN);
            if (tf->like)   lv_obj_add_flag(tf->like, LV_OBJ_FLAG_HIDDEN);
        }
    }
    topflag_update_visibility();
    lv_timer_create(topflag_clock_cb, 500, NULL);
    nav_idle_init();   /* 空闲策略:烹饪 1 分钟回页/非烹饪 5 分钟待机/待机 20 分钟关机 */
    printf("[nav] init done -> major_menu\n");
}

// topflag 顶层状态页显隐：除 wait 页面外都显示。
// 由 page_push / page_pop 驱动（事件驱动，无轮询延迟）
void topflag_update_visibility(void)
{
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->obj) return;
    int is_wait = (depth > 0 && page_stack[depth - 1] == PAGE_WAITMENU_24);
    /* 童锁锁定时强制显示:锁层在 topflag 内,待机页也要能看见锁定提示 */
    if (is_wait && !nav_childlock_active()) {
        /* 待机页默认整层隐藏(页面自带大时钟);演示模式例外:显示徽标(左上角),
           小时钟仍藏避免与待机大时钟重复 */
        if (SET_Data.Set_DemoMode) {
            if (tf->currenttime) lv_obj_add_flag(tf->currenttime, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(tf->obj, LV_OBJ_FLAG_HIDDEN);
            /* 放行瞬间即把徽标硬定左上角:wait 页徽标无条件 (24,24)。
               page_push 先于 wait 页面加载触发本函数,此刻 lv_scr_act() 仍是旧页,
               若不做硬写,徽标会以旧页留下的居中位置被放行,直到 wait 加载完
               lang_scr_load_anim 里的 sync 才归位——中间帧可见从中间"移"到左上 */
            if (tf->demo) lv_obj_set_pos(tf->demo, 24, 24);
        } else {
            if (tf->currenttime) lv_obj_clear_flag(tf->currenttime, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(tf->obj, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        if (tf->currenttime) lv_obj_clear_flag(tf->currenttime, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(tf->obj, LV_OBJ_FLAG_HIDDEN);
    }
}

/* 左上角是否有文字:设置覆盖层打开(自带"设置"标题)→有;
 * 普通页只扫活动屏直接子对象(只读不递归):左上角(x≤120,y≤60)有可见非空文字标签→有 */
static int nav_topleft_has_text(void)
{
    if (screen_set_overlay_open()) return 1;
    lv_obj_t *scr = lv_scr_act();
    if (!scr) return 0;
    uint32_t n = lv_obj_get_child_count(scr);
    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *c = lv_obj_get_child(scr, i);
        if (!c || lv_obj_has_flag(c, LV_OBJ_FLAG_HIDDEN)) continue;
        if (!lv_obj_check_type(c, &lv_label_class)) continue;
        if (lv_obj_get_x(c) > 120 || lv_obj_get_y(c) > 60) continue;
        const char *txt = lv_label_get_text(c);
        if (txt && txt[0]) return 1;
    }
    return 0;
}

/* 顶栏状态图标(demo/timer/light/like)统一显隐与排布:
 * 显隐:demo=Set_DemoMode;timer=设置计时器后台运行中(count_down_running);
 *      light=炉灯开启(接收 BUF[14] BIT0,20260618 协议);like=收藏成功后停留在完成页期间。
 * 排布:待机页仅演示徽标硬定左上角(原规则);普通页按 demo→timer→light→like 顺序
 *      链式排布,相邻图标边缘间隔 24px;左上角有文字→可见图标组整体水平居中(y=24);
 *      无文字→从左边距 24 起。like 例外:只出现在左上角——组居中时它不参与,固定 (24,24)。
 * 由 topflag_clock_cb 500ms 驱动,切语言/设置页切换后也立即调用 */
static int s_topflag_like = 0;        /* 收藏成功徽标:置位到离页期间显示 */
static int s_topflag_like_page = -1;  /* 置位时的页面栈顶(离页自动收回) */
static int s_demo_src_en = -1;   /* 徽标图当前语言(-1=未知,首次必设);避免 500ms tick 重复 set_src */
void nav_topflag_demo_sync(void)
{
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->obj || !tf->demo) return;

    /* like:离开置位页自动收回 */
    if (s_topflag_like && (depth <= 0 || page_stack[depth - 1] != s_topflag_like_page))
        s_topflag_like = 0;

    /* 英文模式换英文版徽标图(show_en 76x44 / 中文 show 61x38);
       必须先于下方取宽排布;语言切换经 lang_scr_load_anim 与 500ms tick 双路到达 */
    if (is_english() != s_demo_src_en) {
        lv_img_set_src(tf->demo, is_english() ? LVGL_IMAGE_PATH(show_en.png)
                                              : LVGL_IMAGE_PATH(show.png));
        s_demo_src_en = is_english();
    }

    int is_wait = (depth > 0 && page_stack[depth - 1] == PAGE_WAITMENU_24);
    int v_demo  = SET_Data.Set_DemoMode ? 1 : 0;
    int v_timer = count_down_running();
    int v_light = is_lamp_on();
    int v_like  = s_topflag_like;
    /* 待机页保持干净:仅演示徽标(整层显隐已由 topflag_update_visibility 处理) */
    if (is_wait) v_timer = v_light = v_like = 0;

    if (v_demo)  lv_obj_clear_flag(tf->demo, LV_OBJ_FLAG_HIDDEN);
    else         lv_obj_add_flag(tf->demo, LV_OBJ_FLAG_HIDDEN);
    if (v_timer) lv_obj_clear_flag(tf->timer, LV_OBJ_FLAG_HIDDEN);
    else         lv_obj_add_flag(tf->timer, LV_OBJ_FLAG_HIDDEN);
    if (v_light) lv_obj_clear_flag(tf->light, LV_OBJ_FLAG_HIDDEN);
    else         lv_obj_add_flag(tf->light, LV_OBJ_FLAG_HIDDEN);
    if (v_like)  lv_obj_clear_flag(tf->like, LV_OBJ_FLAG_HIDDEN);
    else         lv_obj_add_flag(tf->like, LV_OBJ_FLAG_HIDDEN);

    /* wait 页演示徽标无条件左上角(硬规则):不走扫描——覆盖层/尾部 sync 对 wait 的
       误判会把它定到居中再等 tick 纠正,造成可见移动 */
    if (is_wait) {
        if (v_demo) lv_obj_set_pos(tf->demo, 24, 24);
        return;
    }

    /* 链式排布:取各可见图宽,相邻边缘间隔 24px;左上角有文字→整组居中 */
    int centered = nav_topleft_has_text();
    lv_obj_t *chain[4];
    int w[4], n = 0, total = 0;
    if (v_demo) {
        chain[n] = tf->demo;
        lv_obj_update_layout(chain[n]);
        total += w[n] = (int)lv_obj_get_width(chain[n]);
        n++;
    }
    if (v_timer) {
        chain[n] = tf->timer;
        lv_obj_update_layout(chain[n]);
        total += w[n] = (int)lv_obj_get_width(chain[n]);
        n++;
    }
    if (v_light) {
        chain[n] = tf->light;
        lv_obj_update_layout(chain[n]);
        total += w[n] = (int)lv_obj_get_width(chain[n]);
        n++;
    }
    /* like 不进居中组;链式在左上角时按排序收尾 */
    int like_in_chain = (v_like && !centered);
    if (like_in_chain) {
        chain[n] = tf->like;
        lv_obj_update_layout(chain[n]);
        total += w[n] = (int)lv_obj_get_width(chain[n]);
        n++;
    }
    if (n) total += 24 * (n - 1);

    int x = centered ? (1280 - total) / 2 : 24;
    for (int i = 0; i < n; i++) {
        lv_obj_set_pos(chain[i], (lv_coord_t)x, 24);
        x += w[i] + 24;
    }
    if (v_like && !like_in_chain)
        lv_obj_set_pos(tf->like, 24, 24);
}

/* 收藏成功:like 徽标显示到当前完成页,离页自动收回(nav_favorites.c 两处成功路径调用) */
void nav_topflag_like_show(void)
{
    s_topflag_like = 1;
    s_topflag_like_page = (depth > 0) ? page_stack[depth - 1] : -1;
    nav_topflag_demo_sync();   /* 立即显形,不等 500ms tick */
}

// 待机页时钟缓存（waitmenu_apply_clock 使用）
static lv_obj_t *lw_obj = NULL;
static uint8_t lw_hour = 0xFF, lw_min = 0xFF;
static uint8_t lw_year = 0, lw_month = 0, lw_day = 0;
static int lw_wday = -1;
static lv_obj_t *lw_ampm = NULL;      /* 12 小时制上午/下午后缀(随待机页实例重建) */
static uint8_t lw_timetype = 0xFF;    /* 时制参与变更判定:设置页切换后立即重排 */
static uint8_t lw_lang_en = 0xFF;     /* 语言参与变更判定:切换后时间/星期/年月日全部重刷 */

// 显式重置缓存：waitmenu_24_create 后调用，强制刷新为真实时间。
// 不依赖指针相等判定——auto_del 下 malloc 地址复用会导致缓存不失效（显示默认假文本/陈旧星期）
void waitmenu_clock_cache_reset(void)
{
    lw_obj = NULL;
    lw_hour = 0xFF; lw_min = 0xFF;
    lw_year = 0; lw_month = 0; lw_day = 0; lw_wday = -1;
    lw_ampm = NULL; lw_timetype = 0xFF;
}

// 待机页 waitmenu_24 时间/星期/年月日 实时刷新：
// 页面重建（obj 指针变化或显式缓存重置）时清缓存强制刷新为真实时间；平时按数值变化更新。
// 有效性判断使用"页面栈顶 == 待机页"（业务状态，可靠），
// 不能用 obj == lv_scr_act 指针比较——离开待机页后 wait->obj 悬空，
// malloc 地址复用时可能误判通过导致 UAF 写入（模拟器卡死根因）。
void waitmenu_apply_clock(void)
{
    rtc_time_t t;
    if (rtc_get_time(&t) != 0) return;
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WAITMENU_24) return;
    waitmenu_24_t *wait = waitmenu_24_get(&ui_manager);
    if (!wait || !wait->obj) return;
    if (wait->obj != lw_obj) {
        lw_obj = wait->obj;
        lw_hour = 0xFF; lw_min = 0xFF;
        lw_year = 0; lw_month = 0; lw_day = 0; lw_wday = -1;
        lw_ampm = NULL;   /* 旧页后缀标签随页销毁,新实例重建 */
    }
    if (lw_lang_en != (is_english() ? 1 : 0)) {   /* 切语言:周/日缓存不感知语言,整体作废重刷 */
        lw_lang_en = is_english() ? 1 : 0;
        lw_hour = 0xFF; lw_min = 0xFF;
        lw_year = 0; lw_month = 0; lw_day = 0; lw_wday = -1;
    }
    if (t.hour != lw_hour || t.min != lw_min || lw_timetype != SET_Data.Set_TimeType) {
        lw_hour = t.hour; lw_min = t.min; lw_timetype = SET_Data.Set_TimeType;
        char buf[6];
        uint8_t disp_hour = t.hour;
        if (SET_Data.Set_TimeType == 1) {
            disp_hour = t.hour % 12;
            if (disp_hour == 0) disp_hour = 12;
        }
        snprintf(buf, sizeof(buf), "%02d:%02d", disp_hour, t.min);
        if (wait->time_label) lv_label_set_text(wait->time_label, buf);

        /* 12 小时制:时间右侧"上午/下午"后缀(48px)。
           中文:时间标签 438 宽居中盒,后缀 x=盒左+(盒宽+文本宽)/2;
           英文:tune 层把时间标签改 SIZE_CONTENT+屏幕居中,后缀 x=标签右缘。
           y 统一贴时间标签下缘(与数字底部对齐) */
        if (SET_Data.Set_TimeType == 1 && wait->time_label && wait->obj) {
            if (lw_ampm == NULL) {
                lw_ampm = lv_label_create(wait->obj);
                lv_obj_set_style_text_color(lw_ampm, lv_color_hex(0xffffff), 0);
            }
            const lv_font_t *big = is_english() ? &c_aktivgroteskmedium_125 : &c_taiwanpearl_regular_128;
            const lv_font_t *amf = is_english() ? &c_aktivgroteskmedium_48 : &c_taiwanpearl_regular_48;
            lv_obj_set_style_text_font(lw_ampm, amf, 0);
            lv_label_set_text(lw_ampm, tr(t.hour < 12 ? "上午" : "下午"));
            int txt_w = lv_txt_get_width(buf, (uint32_t)strlen(buf), big, 4);   /* 时间标签 letter_space=4 */
            const char *am_txt = lv_label_get_text(lw_ampm);
            int am_w = lv_txt_get_width(am_txt, (uint32_t)strlen(am_txt), amf, 0);
            int gap = 3;
            int dx = (am_w + gap) / 2;   /* 时间左移半后缀宽:"时间+后缀"整体居中(原时间单独居中) */
            lv_obj_set_style_translate_x(wait->time_label, -dx, 0);
            /* 时间文本右缘:中文=421 固定盒内居中;英文=tune 层标签屏幕居中。
               不读 lv_obj_get_x——translate 会算进坐标,分钟刷新时后缀会二次左移 */
            int text_right = is_english() ? (1280 - txt_w) / 2 + txt_w
                                          : 421 + (438 + txt_w) / 2;
            /* 后缀与时间数字基线对齐(数字无降部,基线即数字下缘)。
               先强制布局:进页瞬间标签坐标未算,get_y 会拿到 0 导致后缀飞到顶部 */
            lv_obj_update_layout(wait->time_label);
            int base_y = lv_obj_get_y(wait->time_label) + big->line_height - big->base_line;
            lv_obj_set_pos(lw_ampm, text_right + gap - dx, base_y - (amf->line_height - amf->base_line));
            lv_obj_clear_flag(lw_ampm, LV_OBJ_FLAG_HIDDEN);
        } else if (lw_ampm) {
            lv_obj_add_flag(lw_ampm, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_translate_x(wait->time_label, 0, 0);
        }
    }
    /* week_label = 星期 + 年月日 合并文本（生成默认："星期日, 2025年12月15日"） */
    if (t.wday != lw_wday || t.year != lw_year || t.month != lw_month || t.day != lw_day) {
        lw_wday = t.wday;
        lw_year = t.year; lw_month = t.month; lw_day = t.day;
        const char *wday_str = "";
        switch (t.wday) {
        case 0: wday_str = tr("星期日"); break;
        case 1: wday_str = tr("星期一"); break;
        case 2: wday_str = tr("星期二"); break;
        case 3: wday_str = tr("星期三"); break;
        case 4: wday_str = tr("星期四"); break;
        case 5: wday_str = tr("星期五"); break;
        case 6: wday_str = tr("星期六"); break;
        default: break;
        }
        char buf[40];
        if (is_english()) {
            /* 英文日期格式：Sunday, June 1st, 2025 */
            static const char *mon_en[] = {"January", "February", "March", "April",
                                           "May", "June", "July", "August",
                                           "September", "October", "November", "December"};
            int d = t.day, m = t.month;
            const char *suf = "th";
            if (d % 10 == 1 && d != 11) suf = "st";
            else if (d % 10 == 2 && d != 12) suf = "nd";
            else if (d % 10 == 3 && d != 13) suf = "rd";
            snprintf(buf, sizeof(buf), "%s, %s %d%s, %d",
                     wday_str,
                     (m >= 1 && m <= 12) ? mon_en[m - 1] : "",
                     d, suf, t.year);
        } else {
            snprintf(buf, sizeof(buf), tr("%s, %d年%d月%d日"),
                     wday_str, t.year, t.month, t.day);
        }
        if (wait->week_label) lv_label_set_text(wait->week_label, buf);
    }
}

// topflag 右上角时间刷新体(500ms tick 与强制刷新共用);
// 时/分/时制/语言任一变化才重建文本(12 小时制:小时转 12 制,标签左移给后缀让位)
static uint8_t tf_last_hour = 0xFF, tf_last_min = 0xFF;
static uint8_t tf_last_timetype = 0xFF, tf_last_lang_en = 0xFF;
static lv_obj_t *s_ct_ampm = NULL;   /* 上午/下午后缀(顶层常驻,只建一次) */
static void topflag_clock_update(void)
{
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->currenttime) return;
    rtc_time_t t;
    if (rtc_get_time(&t) != 0) return;

    uint8_t lang_en = is_english() ? 1 : 0;
    if (t.hour != tf_last_hour || t.min != tf_last_min ||
        tf_last_timetype != SET_Data.Set_TimeType || tf_last_lang_en != lang_en) {
        tf_last_hour = t.hour; tf_last_min = t.min;
        tf_last_timetype = SET_Data.Set_TimeType; tf_last_lang_en = lang_en;
        char buf[6];
        uint8_t disp_hour = t.hour;
        if (SET_Data.Set_TimeType == 1) {
            disp_hour = t.hour % 12;
            if (disp_hour == 0) disp_hour = 12;
        }
        snprintf(buf, sizeof(buf), "%02d:%02d", disp_hour, t.min);
        lv_label_set_text(tf->currenttime, buf);
        if (SET_Data.Set_TimeType == 1) {
            /* 后缀:24px 字体 transform 缩半≈12px(字库无 12px 档),pivot 左上角便于定位 */
            if (s_ct_ampm == NULL) {
                s_ct_ampm = lv_label_create(tf->obj);
                lv_obj_set_style_text_color(s_ct_ampm, lv_color_hex(0xffffff), 0);
                lv_obj_set_style_transform_pivot_x(s_ct_ampm, 0, 0);
                lv_obj_set_style_transform_pivot_y(s_ct_ampm, 0, 0);
                lv_obj_set_style_transform_scale_x(s_ct_ampm, 128, 0);
                lv_obj_set_style_transform_scale_y(s_ct_ampm, 128, 0);
            }
            const lv_font_t *amf = is_english() ? &c_aktivgroteskmedium_24 : &c_taiwanpearl_regular_24;
            lv_obj_set_style_text_font(s_ct_ampm, amf, 0);
            lv_label_set_text(s_ct_ampm, tr(t.hour < 12 ? "上午" : "下午"));
            /* 与时间数字基线对齐:currenttime 标签顶 y=25(taiwan24 基线 25+28-8=45);
               后缀缩放 50%,基线偏移随之减半 */
            int base_y = 25 + c_taiwanpearl_regular_24.line_height - c_taiwanpearl_regular_24.base_line;
            lv_obj_set_pos(s_ct_ampm, 1226, base_y - (amf->line_height - amf->base_line) / 2);
            lv_obj_clear_flag(s_ct_ampm, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_x(tf->currenttime, 1164);
        } else {
            lv_obj_set_x(tf->currenttime, 1196);
            if (s_ct_ampm) lv_obj_add_flag(s_ct_ampm, LV_OBJ_FLAG_HIDDEN);
        }
    }
    /* 后缀可见性与 currenttime 同步(待机页 currenttime 隐藏避免与大时钟重复);
       仅 12 小时制才允许显示,否则会把变更分支里刚隐藏的后缀又刷回来 */
    if (s_ct_ampm) {
        if (SET_Data.Set_TimeType == 1 && !lv_obj_has_flag(tf->currenttime, LV_OBJ_FLAG_HIDDEN))
            lv_obj_clear_flag(s_ct_ampm, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(s_ct_ampm, LV_OBJ_FLAG_HIDDEN);
    }
}

// topflag 时间走动：每 500ms 刷新 currenttime 为 "HH:MM"（分钟变化才更新）；
// 同时刷新待机页 waitmenu_24 的 时间/星期/年月日
void topflag_clock_cb(lv_timer_t *timer)
{
    (void)timer;
    nav_childlock_refresh();   /* 童锁激活时跟随下层状态刷新第三行(内部有签名守卫) */
    nav_topflag_demo_sync();   /* 演示模式徽标显隐/定位(纯读+属性写,2Hz 开销可忽略) */
    topflag_clock_update();

    /* 待机页三标签（独立缓存，分钟/跨天/星期变化才更新） */
    waitmenu_apply_clock();
}

// 切语言/切 12-24 时制后立即刷新右上角时钟(设置弹窗 YES、时制页 YES 处调用,不等 500ms tick)
void nav_topflag_clock_force(void)
{
    tf_last_hour = 0xFF; tf_last_min = 0xFF;
    tf_last_timetype = 0xFF; tf_last_lang_en = 0xFF;
    topflag_clock_update();
}

