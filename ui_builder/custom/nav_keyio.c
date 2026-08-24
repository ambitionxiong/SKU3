/*
 * nav_keyio.c - 按键输入状态机 + 长按
 *
 * 职责：
 *   1. nav_handle_key：按键状态机（KEY_IDLE/KEY_PRESSED），
 *      首按触发 process_key，编码器长按按 ENC_REPEAT_MS 重复触发，
 *      松开回空闲。由底层按键回调(硬件/模拟器)逐次调用。
 *   2. nav_key1_long_press：KEY1 长按(2s)开关机——
 *      关机:清理全部运行状态→SLEEP 待机暗屏；开机:回主菜单(探针则探针主菜单)。
 *   3. nav_key1_hold_check：供外部周期查询 KEY1 是否已长按 2s。
 *
 * 状态变量(key_state/active_key/active_key_time)定义在 nav_key.c。
 */

#include "nav.h"
#include "nav_internal.h"

/* KEY1 长按(2s)：开关机。开机=回主菜单，关机=清状态+进入 SLEEP 待机暗屏。
   执行前清理所有残留(定时器/标志/覆盖层)，避免跨会话悬空。 */
void nav_key1_long_press(void)
{
    probetip_cancel_auto_dismiss();   /* 取消陈旧的探针提示自动关闭定时器,防止跨会话误触发 */
    screen_set_reset();               /* 覆盖层若打开:清理对象/组/焦点指针,防悬空 */
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    g_on_stop_back = 0;
    g_complete_to_stop_back = 0;
    g_cooling_to_stop_back = 0;
    g_extra_color_to_stop_back = 0;
    g_keepwarm_active = 0;
    g_keepwarm_sec = 0;
    cook_is_color = 0;
    g_stop_back_complete = NULL;
    g_delay_cancel_btn = 0;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    delay_on = 0; preheat_on = 0; contain_on = 0;
    delay_hour = 0; delay_min = 0;
    g_delay_target = -1;
    g_somecook_running = 0;
    g_somecook_run_idx = 0;
    six_cook_reset();   /* 六感运行:清理状态(定时器已由上面 cook_timer 删除覆盖) */
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;

    if (g_send.iface_status != IFACE_SLEEP) {
        g_send.buzzer_req = BUZZER_POWER_OFF;
        g_send.iface_status = IFACE_SLEEP;
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        lv_obj_clean(lv_scr_act());
        waitmenu_24_create(&ui_manager);
        waitmenu_clock_cache_reset();   /* 强制刷新为真实时间 */
        current_group = NULL;
        lv_scr_load(waitmenu_24_get(&ui_manager)->obj);
        waitmenu_apply_clock();   /* 立即刷新为真实时间 */
#ifndef LV_USE_AIC_SIMULATOR
        backlight_set_level(10);
#endif
        printf("[KEY] KEY1 long press -> SLEEP (dim waitmenu)\n");
    } else {
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        if (is_probe_inserted()) {
            jump_to_major_menu_tz();
        } else {
            page_push(PAGE_MAJOR_MENU);
            lv_obj_clean(lv_scr_act());
            major_menu_create(&ui_manager);
            groups_create();
            bind_events();
            current_group = g_major_menu;
            lang_scr_load_anim(major_menu_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, 0);
        }
        g_send.buzzer_req = BUZZER_POWER_ON;
        g_send.iface_status = IFACE_SETTING;
#ifndef LV_USE_AIC_SIMULATOR
        backlight_set_level(100);
#endif
        printf("[KEY] KEY1 long press -> WAKE (major_menu)\n");
    }
#ifdef LV_USE_AIC_SIMULATOR
    uart_print();
#endif
}
/* 供外部周期调用：KEY1 按住已持续 2s 则触发长按并返回 1（用于长按防抖） */
uint8_t nav_key1_hold_check(void)
{
    if (active_key == KEY1 && key_state == KEY_PRESSED) {
        uint32_t interval = lv_tick_get() - active_key_time;
        if (interval >= 2000) {
            active_key_time = lv_tick_get();
            nav_key1_long_press();
            return 1;
        }
    }
    return 0;
}
/* 按键状态机：KEY_IDLE 首按→记键值+调 process_key；
   KEY_PRESSED 按住→编码器按 50ms 重复、KEY1 按 2s 长按；松开回 KEY_IDLE。 */
void nav_handle_key(uint8_t key)
{
    uint32_t now = lv_tick_get();

    switch (key_state) {
    case KEY_IDLE:
        if (key != 0) {
#ifdef LV_USE_AIC_SIMULATOR
            static const char *kn[] = {
                [1]="KEY1", [3]="MENU", [5]="COLOR", [21]="BACK",
                [31]="CW", [41]="CCW", [51]="PRESS"
            };
            printf("[KEY] %s (%d)\n", key<=51&&kn[key]?kn[key]:"?", key);
#endif
            active_key = key;
            active_key_time = now;
            key_state = KEY_PRESSED;
            process_key(key);
        }
        break;

    case KEY_PRESSED:
        if (key == 0) {
            // 松开 → 回到空闲
#ifdef LV_USE_AIC_SIMULATOR
            printf("[KEY] release\n");
#endif
            key_state = KEY_IDLE;
            active_key = 0;
        } else if (key == active_key) {
            // 同键按住
            uint32_t interval = now - active_key_time;
            int is_encoder = (key == KEY_ENCODER_CW || key == KEY_ENCODER_CCW);

            if (is_encoder && interval >= ENC_REPEAT_MS) {
                active_key_time = now;
                process_key(key);
            }
            if (active_key == KEY1 && interval >= 2000) {
                active_key_time = now;
                nav_key1_long_press();
            }
            // 触控键按住不重复（只有 KEY_IDLE 后的第一次触发）
        } else {
            // 键值变化（如编码器方向切换）
            active_key = key;
            active_key_time = now;
            process_key(key);
        }
        break;
    }
}

