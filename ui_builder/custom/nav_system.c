#include "nav.h"
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
            if (label) lv_label_set_text(label, probe_now ? "探针已插入" : "探针已拔出");
        }
        return;
    }

    if (g_send.iface_status == IFACE_SLEEP) {
        jump_to_probetip(probe_now ? "探针已插入" : "探针已拔出");
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
    g_send.iface_status = IFACE_STANDBY;
    g_send.cook_mode = MODE_NONE;
    g_send.cook_flag = 0;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;

    depth = 0;
    page_push(PAGE_WAITMENU_24);
    lv_obj_clean(lv_scr_act());
    waitmenu_24_create(&ui_manager);
    waitmenu_clock_cache_reset();   /* 强制刷新为真实时间 */
    current_group = NULL;
    lv_scr_load(waitmenu_24_get(&ui_manager)->obj);
    waitmenu_apply_clock();   /* 立即刷新为真实时间 */
    jump_to_probetip(probe_now ? "探针已插入" : "探针已拔出");
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
    major_menu_apply_lang();   /* i18n */
    current_group = g_major_menu;
    lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    g_send.iface_status = IFACE_SETTING;
    lv_timer_create(system_timer_cb, 500, NULL);

    /* topflag 顶层状态页：生成代码已改挂 lv_layer_top，所有页面之上（wait 页面除外） */
    topflagpage_create(&ui_manager);
    {
        topflagpage_t *tf = topflagpage_get(&ui_manager);
        if (tf) {
            /* 暂时只显示 currenttime */
            if (tf->demo)   lv_obj_add_flag(tf->demo, LV_OBJ_FLAG_HIDDEN);
            if (tf->timer)  lv_obj_add_flag(tf->timer, LV_OBJ_FLAG_HIDDEN);
            if (tf->light)  lv_obj_add_flag(tf->light, LV_OBJ_FLAG_HIDDEN);
        }
    }
    topflag_update_visibility();
    lv_timer_create(topflag_clock_cb, 500, NULL);
    printf("[nav] init done -> major_menu\n");
}

// topflag 顶层状态页显隐：除 wait 页面外都显示。
// 由 page_push / page_pop 驱动（事件驱动，无轮询延迟）
void topflag_update_visibility(void)
{
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->obj) return;
    int is_wait = (depth > 0 && page_stack[depth - 1] == PAGE_WAITMENU_24);
    if (is_wait) lv_obj_add_flag(tf->obj, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_clear_flag(tf->obj, LV_OBJ_FLAG_HIDDEN);
}

// 待机页时钟缓存（waitmenu_apply_clock 使用）
static lv_obj_t *lw_obj = NULL;
static uint8_t lw_hour = 0xFF, lw_min = 0xFF;
static uint8_t lw_year = 0, lw_month = 0, lw_day = 0;
static int lw_wday = -1;

// 显式重置缓存：waitmenu_24_create 后调用，强制刷新为真实时间。
// 不依赖指针相等判定——auto_del 下 malloc 地址复用会导致缓存不失效（显示默认假文本/陈旧星期）
void waitmenu_clock_cache_reset(void)
{
    lw_obj = NULL;
    lw_hour = 0xFF; lw_min = 0xFF;
    lw_year = 0; lw_month = 0; lw_day = 0; lw_wday = -1;
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
    }
    if (t.hour != lw_hour || t.min != lw_min) {
        lw_hour = t.hour; lw_min = t.min;
        char buf[6];
        snprintf(buf, sizeof(buf), "%02d:%02d", t.hour, t.min);
        if (wait->time_label) lv_label_set_text(wait->time_label, buf);
    }
    /* week_label = 星期 + 年月日 合并文本（生成默认："星期日, 2025年12月15日"） */
    if (t.wday != lw_wday || t.year != lw_year || t.month != lw_month || t.day != lw_day) {
        lw_wday = t.wday;
        lw_year = t.year; lw_month = t.month; lw_day = t.day;
        static const char *week_cn[] = {"星期日", "星期一", "星期二",
                                        "星期三", "星期四", "星期五", "星期六"};
        char buf[32];
        snprintf(buf, sizeof(buf), "%s, %d年%d月%d日",
                 (t.wday >= 0 && t.wday <= 6) ? week_cn[t.wday] : "",
                 t.year, t.month, t.day);
        if (wait->week_label) lv_label_set_text(wait->week_label, buf);
    }
}

// topflag 时间走动：每 500ms 刷新 currenttime 为 "HH:MM"（分钟变化才更新）；
// 同时刷新待机页 waitmenu_24 的 时间/星期/年月日
void topflag_clock_cb(lv_timer_t *timer)
{
    topflagpage_t *tf = topflagpage_get(&ui_manager);
    if (!tf || !tf->currenttime) return;
    rtc_time_t t;
    if (rtc_get_time(&t) != 0) return;

    /* topflag 右上角时间 */
    static uint8_t last_hour = 0xFF, last_min = 0xFF;
    if (t.hour != last_hour || t.min != last_min) {
        last_hour = t.hour; last_min = t.min;
        char buf[6];
        snprintf(buf, sizeof(buf), "%02d:%02d", t.hour, t.min);
        lv_label_set_text(tf->currenttime, buf);
    }

    /* 待机页三标签（独立缓存，分钟/跨天/星期变化才更新） */
    waitmenu_apply_clock();
}

