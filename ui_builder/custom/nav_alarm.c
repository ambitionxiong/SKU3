/*
 * nav_alarm.c - 警报页（PAGE_ALARM，独立屏幕）
 *
 * 收到报警协议（接收帧 BUF[12] 报警序号非 0，nav_system 500ms tick 边沿检测）
 * 立即触发：与关机同套清理停掉全部运行状态，落警报页；界面状态发 8（报警
 * 界面）+ 蜂鸣 7（报警音）。页面吞掉全部按键，仅 KEY1 长按开关机可用——
 * 长按在 nav_handle_key 状态机里处理，不经 process_key。文本按协议序号显示
 * "E-<n>:" + 售后提示（中/英双版）。警报一旦触发只能 KEY1 长按开关机解除，
 * BUF[12] 归 0 不解除；关机后同码不重弹，新码才重新触发。警报页豁免空闲策略。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"
#include "nav_internal.h"
#include "nav_idle.h"

static int s_alarm_code = 0;   /* 当前报警序号(0=无);触发后与 BUF[12] 同步跟踪 */

/* 全部停止:与关机(nav_power_off)同套运行状态清理,但不清屏幕落警报页 */
static void alarm_stop_all(void)
{
    probetip_cancel_auto_dismiss();   /* 陈旧的探针提示自动关闭定时器 */
    screen_set_reset();               /* 覆盖层若打开:清对象/组/焦点指针 */
    count_down_poweroff_reset();      /* 计时器后台/超时状态一并清 */
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
    six_cook_reset();
    set_temp = 180; set_temp_up = 180; set_temp_down = 180;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
}

void jump_to_alarm(int code)
{
    s_alarm_code = code;
    alarm_stop_all();
    depth = 0;                        /* 栈重置:警报页之上无任何历史页面 */
    page_push(PAGE_WAITMENU_24);      /* 哨兵栈底 */
    page_push(PAGE_ALARM);
    lv_obj_clean(lv_scr_act());

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xfcfcfc), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr, LVGL_IMAGE_PATH(used/bg1.jpg), LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 深灰圆角面板(与出厂/关于页同位 24,118,1232x337) */
    lv_obj_t *panel = lv_obj_create(scr);
    lv_obj_set_pos(panel, 24, 118);
    lv_obj_set_size(panel, 1232, 337);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x2a2a2a), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(panel, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 居中文本(48 号;设计图墨迹中心 y≈283,标签 y253;EN 全角冒号带空) */
    {
        char buf[128];
        if (is_english())
            snprintf(buf, sizeof(buf), "E-%d：%s", code, tr("请联系最近的售后服务点"));
        else
            snprintf(buf, sizeof(buf), "E-%d:%s", code, tr("请联系最近的售后服务点"));
        lv_obj_t *lb = lv_label_create(scr);
        lv_label_set_text(lb, buf);
        lv_obj_set_width(lb, 1280);
        lv_obj_set_style_text_font(lb, &c_taiwanpearl_regular_48, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(lb, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_pos(lb, 0, 253);
    }

    current_group = NULL;             /* 无焦点组:按键全由 process_key 顶层守卫吞掉 */
    g_send.buzzer_req = BUZZER_ALARM;      /* 蜂鸣 7:报警音 */
    g_send.iface_status = IFACE_ALARM;     /* 界面状态 8:报警界面 */
    uart_print();
#ifndef LV_USE_AIC_SIMULATOR
    backlight_set_level(100);         /* 若从 SLEEP(如待机温度异常 E-14)触发:恢复背光 */
#endif
    lang_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
    topflag_update_visibility();      /* 从 SLEEP 触发时 topflag 层此前被隐:重新放行 */
    nav_topflag_demo_sync();
    printf("[alarm] jump E-%d\n", code);
}

/* 常驻 tick(nav_system 500ms)调用:BUF[12] 边沿触发/跟踪。
   解除规则(2026-09-12 定稿):警报一旦触发只能长按开关机解除——BUF[12] 归 0
   不退页不解除;关机后电源板若仍发同码也不再重弹,新码才会重新触发 */
void nav_alarm_tick_check(void)
{
    int code = uart_data_receive[Receive_data_Power_ALL_Error];   /* BUF[12] 报警序号 */

    if (depth > 0 && page_stack[depth - 1] == PAGE_ALARM)
        return;                       /* 警报页存续:无视 BUF[12] 任何变化,仅长按开关机可离 */
    if (code && code != s_alarm_code)
        jump_to_alarm(code);          /* 边沿触发 */
    else if (!code)
        s_alarm_code = 0;             /* 无警报:同步跟踪 */
}
