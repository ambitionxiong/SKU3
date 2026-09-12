/*
 * nav_idle.c - 空闲策略调度（无操作自动回页/待机/关机）
 *
 * 职责：
 *   1. nav_idle_touch：用户活动打点（nav_handle_key 每个实际按键调用，
 *      含被童锁/SLEEP 吞掉的键——有按键动作就算活动）
 *   2. nav_idle_init：创建常驻 1s 调度定时器（nav_init 调用，一次创建永不重建）
 *   3. 三规则（nav_idle_timer_cb）：
 *      烹饪会话 60s 无操作 → 逐层退出设置覆盖层/子页回烹饪层；
 *      非烹饪 300s 无操作 → 进待机页；
 *      待机页 1200s 无操作 → 关机。
 *   4. nav_goto_major_menu：返回链终点统一落主菜单（栈底 WAITMENU_24 仅作哨兵）。
 *
 * 模态：童锁不豁免（锁层挂 lv_layer_top，下层照常切换，解锁即正确页面）；
 *   唯一豁免是计时器超时抢屏层（该层自带 300s 自动退出闭环）。
 * 演示模式不豁免（按需求，规则统一）。
 * 稳定性：定时器一次创建常驻，回调零内存分配、不持有 lv_obj_t*，
 *   只读页面栈/状态变量并调用导航原语——无孤儿定时器/泄露风险。
 */

#include "nav_idle.h"
#include "nav_internal.h"

#define NAV_IDLE_TO_COOKING_S   100    /* 烹饪会话无操作回烹饪层 */
#define NAV_IDLE_TO_STANDBY_S   100   /* 非烹饪无操作进待机 */
#define NAV_IDLE_TO_POWEROFF_S  100  /* 待机页无操作关机 */

static uint32_t g_last_activity_ms = 0;

void nav_idle_touch(void)
{
    g_last_activity_ms = lv_tick_get();
}

/* 烹饪会话进行中(运行/暂停/预约/完成+保温未结束)。
   与 screen_set 的运行态口径一致；完成态以保温流程收尾为准 */
static int cook_session_active(void)
{
    if (g_send.iface_status == IFACE_COOKING ||
        g_send.iface_status == IFACE_PAUSE ||
        g_send.iface_status == IFACE_DELAY_RESERVE)
        return 1;
    if (g_send.iface_status == IFACE_COMPLETE && (cook_timer != NULL || g_keepwarm_active))
        return 1;
    return 0;
}

/* 烹饪会话中可出现在烹饪层之上的页面：设置覆盖层/其子页/各模式 SETTING 小按钮页。
   空闲回页只处理这些；遇到其它页面即视为已在运行/暂停/完成/预约层 */
static int is_cook_above_page(page_id_t p)
{
    if (p == PAGE_SCREEN_SET || p == PAGE_LOUDNESS || p == PAGE_SET_VAL ||
        p == PAGE_SET_COUNT || p == PAGE_SET_SYSTIME)
        return 1;
    return is_cook_setting_page(p);
}

/* 栈重置为 [WAITMENU_24, MAJOR_MENU(_TZ)] 并重建主菜单。
   WAITMENU_24 仅作栈底哨兵(开机/功能键入口同构)，永不作为显示页 */
void nav_goto_major_menu(void)
{
    depth = 0;
    page_push(PAGE_WAITMENU_24);
    lv_obj_clean(lv_scr_act());
    if (is_probe_inserted()) {
        jump_to_major_menu_tz();          /* 自带重建/焦点组/IFACE_SETTING */
    } else {
        page_push(PAGE_MAJOR_MENU);
        major_menu_create(&ui_manager);
        groups_create();
        bind_events();
        current_group = g_major_menu;
        lang_scr_load_anim(major_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        g_send.iface_status = IFACE_SETTING;
    }
    topflag_update_visibility();
}

/* 进待机页：waitmenu + IFACE_STANDBY（空闲 5 分钟等共用入口） */
void nav_enter_standby(void)
{
    probetip_cancel_auto_dismiss();       /* 取消陈旧的探针提示自动关闭定时器 */
    screen_set_reset();                   /* 覆盖层若开着:清理对象/组/焦点指针 */
    depth = 0;
    page_push(PAGE_WAITMENU_24);
    lv_obj_clean(lv_scr_act());
    waitmenu_24_create(&ui_manager);
    waitmenu_clock_cache_reset();         /* 强制刷新为真实时间 */
    current_group = NULL;
    lang_scr_load_anim(waitmenu_24_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    waitmenu_apply_clock();               /* 立即刷新为真实时间 */
    g_send.iface_status = IFACE_STANDBY;
    g_last_activity_ms = lv_tick_get();   /* 20 分钟关机从进待机起重新计 */
    printf("[idle] no-op %ds -> waitmenu standby\n", NAV_IDLE_TO_STANDBY_S);
}

/* 常驻 1s 调度。每拍最多走一步，逐步收敛到目标层，每步与用户按键同路 */
static void nav_idle_timer_cb(lv_timer_t *t)
{
    uint32_t idle_s;
    page_id_t top;

    (void)t;
    if (depth <= 0) return;
    /* 计时器超时抢屏层:唯一模态豁免 */
    if (count_down_overtime_active()) return;

    idle_s = (uint32_t)((lv_tick_get() - g_last_activity_ms) / 1000);
    top = page_stack[depth - 1];

    /* 规则3:待机页无操作 20 分钟关机 */
    if (top == PAGE_WAITMENU_24 && g_send.iface_status == IFACE_STANDBY &&
        idle_s >= NAV_IDLE_TO_POWEROFF_S) {
        printf("[idle] standby no-op %ds -> power off\n", NAV_IDLE_TO_POWEROFF_S);
        nav_power_off();
        return;
    }

    /* 规则2:非烹饪无操作 5 分钟进待机 */
    if (top != PAGE_WAITMENU_24 && g_send.iface_status != IFACE_SLEEP &&
        !cook_session_active() && idle_s >= NAV_IDLE_TO_STANDBY_S) {
        nav_enter_standby();
        return;
    }

    /* 规则1:烹饪会话无操作 1 分钟逐层退出覆盖层/子页回烹饪层 */
    if (cook_session_active() && idle_s >= NAV_IDLE_TO_COOKING_S &&
        is_cook_above_page(top)) {
        switch (top) {
        case PAGE_SCREEN_SET:
            screen_set_back();            /* 运行态分支:还原下层焦点组并补刷倒计时 */
            break;
        case PAGE_SET_COUNT:
            count_down_back_action();     /* 与 BACK 同路:回设置层,后台计时继续 */
            break;
        case PAGE_LOUDNESS:
            return_Loudness_action();
            break;
        case PAGE_SET_VAL:
            set_val_return_action();
            break;
        case PAGE_SET_SYSTIME:
            systime_back_action();
            break;
        default:
            page_pop();                   /* 各模式 SETTING 小按钮页:弹回烹饪层 */
            break;
        }
    }
}

void nav_idle_init(void)
{
    g_last_activity_ms = lv_tick_get();
    lv_timer_create(nav_idle_timer_cb, 1000, NULL);
}
