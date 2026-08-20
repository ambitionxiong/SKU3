/*
 * nav_settings.c - 上下烧烤设置页(setting)
 *
 * 职责：
 *   jump_to_updown_bbq_setting：进入设置页(保存原始值，BACK 返回时恢复)
 *   on_setting_edit_focus：焦点跟随(线随焦点，dir/icon 按数值固定显示)
 *   on_setting_sure_click：确定 → 应用新参数(上下温/时长)回到 cooking
 *   update_setting_dir_icon：上下温数值变化时 dir/icon 即时更新
 *
 * 约束：上下温差 ≤20℃，由 adjust_value(nav_core.c) 执行。
 */

#include "nav.h"
#include "nav_internal.h"


// ==============================
// Cooking 设置页
// ==============================

// cooking little_button → 设置页（计时器继续运行，实时更新 time_label）
void jump_to_updown_bbq_setting(void)
{
    updown_setting_saved_temp_up = set_temp_up;
    updown_setting_saved_temp_down = set_temp_down;
    updown_setting_saved_hour = set_hour;
    updown_setting_saved_min = set_min;

    page_push(PAGE_UPDOWN_BBQ_SETTING);
    lv_obj_clean(lv_scr_act());
    updown_bbq_setting_create(&ui_manager);

    updown_bbq_setting_t *set = updown_bbq_setting_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->tempup_label, set->tempdown_label,
            set->hour_label, set->min_label,
            set->sure_button,
        };
        if (g_updown_bbq_setting) lv_group_del(g_updown_bbq_setting);
        g_updown_bbq_setting = group_create_for_page(btns, 5);

        edit_clear();
        edit_register(set->tempup_label, set->shortup_templine_img, set->longup_templine_img,
                      &set_temp_up, 30, 300, 5, "%d");
        edit_register(set->tempdown_label, set->shordown_templine_img, set->longdown_templine_img,
                      &set_temp_down, 30, 300, 5, "%d");
        edit_register(set->hour_label, set->hourline_img, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(set->min_label, set->minline_label, NULL,
                      &set_min, 0, 59, 1, "%02d");

        lv_obj_add_event_cb(set->tempup_label, on_setting_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->tempdown_label, on_setting_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->hour_label, on_setting_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->min_label, on_setting_edit_focus,
                            LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(set->sure_button, on_setting_edit_focus,
                            LV_EVENT_FOCUSED, NULL);

        lv_obj_add_event_cb(set->sure_button, on_setting_sure_click,
                            LV_EVENT_CLICKED, NULL);

        /* 同步显示（从 cooking 或 stop 进入） */
        uint32_t elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
        int elapsed_sec = (elapsed + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        if (g_send.iface_status == IFACE_COMPLETE) { h = m = s = 0; }
        lv_label_set_text_fmt(set->time_label, "%02d:%02d:%02d", h, m, s);
        lv_label_set_text_fmt(set->hour_label, "%02d", set_hour);
        lv_label_set_text_fmt(set->min_label, "%02d", set_min);
        lv_label_set_text_fmt(set->tempup_label, "%d", set_temp_up);
        lv_label_set_text_fmt(set->tempdown_label, "%d", set_temp_down);

        /* 初始隐藏所有指示线 + dir/icon */
        lv_obj_add_flag(set->shortup_templine_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->longup_templine_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->shordown_templine_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->longdown_templine_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->hourline_img, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->minline_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->dirup2_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->dirup3_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2_label1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon3_label1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->dirdown2_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->dirdown3_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon2_label2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->icon3_label2, LV_OBJ_FLAG_HIDDEN);

        lv_group_focus_obj(set->tempup_label);

        update_setting_dir_icon(set);
    }
    current_group = g_updown_bbq_setting;

    lv_scr_load_anim(updown_bbq_setting_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    /* complete 结束态（保温场景从 complete 进入设置页）保持 4，避免完成检测误触发 */
    if (g_send.iface_status != IFACE_COMPLETE)
        g_send.iface_status = (cook_timer != NULL) ? IFACE_COOKING : IFACE_SETTING;
    printf("[keepwarm] setting enter: iface=%d complete_keep=%d timer=%p\n",
           g_send.iface_status, (g_send.iface_status == IFACE_COMPLETE), (void *)cook_timer);
    printf("[nav] jump: cooking -> updown_bbq_setting\n");
}
// cooking little_button 点击
void on_cook_setting_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_setting();
}
// 设置页 dir/icon 即时更新
void update_setting_dir_icon(updown_bbq_setting_t *set)
{
    if (!set) return;
    lv_obj_add_flag(set->dirup2_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->dirup3_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->icon2_label1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->icon3_label1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->dirdown2_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->dirdown3_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->icon2_label2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->icon3_label2, LV_OBJ_FLAG_HIDDEN);

    if (set_temp_up < 100) {
        lv_obj_clear_flag(set->dirup2_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->icon2_label1, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(set->dirup3_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->icon3_label1, LV_OBJ_FLAG_HIDDEN);
    }
    if (set_temp_down < 100) {
        lv_obj_clear_flag(set->dirdown2_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->icon2_label2, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(set->dirdown3_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->icon3_label2, LV_OBJ_FLAG_HIDDEN);
    }
}
// 设置页 focus 回调（线随焦点，dir/icon 按数值固定显示）
void on_setting_edit_focus(lv_event_t *e)
{
    on_edit_focus(e);
    update_setting_dir_icon(updown_bbq_setting_get(&ui_manager));
}
// 设置页确定 → 应用新参数回到 cooking
void on_setting_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;

    /* 温差钳位 */
    if (set_temp_up - set_temp_down > 20) set_temp_up = set_temp_down + 20;
    else if (set_temp_down - set_temp_up > 20) set_temp_down = set_temp_up + 20;

    set_temp = set_temp_up;
    cook_total_ms = (set_hour * 3600 + set_min * 60) * 1000;

    if (depth > 1) depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_UPDOWN_BBQ_STOP)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_UPDOWN_BBQ_COMPLETE)
        depth--;
    lv_obj_clean(lv_scr_act());
    updown_bbq_cooking_create(&ui_manager);

    updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop_button, cook->little_button };
        if (g_updown_bbq_cooking) lv_group_del(g_updown_bbq_cooking);
        g_updown_bbq_cooking = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cook->stop_button, on_cook_stop_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cook->little_button, on_cook_setting_click,
                            LV_EVENT_CLICKED, NULL);

        set_status_label_min(cook->updown_label, set_temp_up, set_temp_down, set_hour, set_min);
        lv_label_set_text_fmt(cook->time_label, "%02d:%02d:%02d", set_hour, set_min, 0);

        lv_bar_set_range(cook->bar, 0, 100);
        lv_bar_set_value(cook->bar, 3, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_updown_bbq_cooking;

    lv_scr_load_anim(updown_bbq_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = (uint16_t)set_temp_up;
    g_send.set_temp_lower = (uint16_t)set_temp_down;
    g_send.remaining_ms = cook_total_ms;
    printf("[nav] setting sure -> updown_bbq_cooking\n");
}

