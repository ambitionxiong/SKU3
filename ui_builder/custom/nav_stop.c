/*
 * nav_stop.c - 暂停/恢复 + 额外上色暂停/恢复
 *
 * 职责：
 *   1. 各模式烹饪页 → 暂停页(stop) → 确认退出页(stop_back) 的跳转与恢复
 *   2. stop_resume_cooking：暂停后恢复计时(不经过 page_pop，直接重建 cooking)
 *   3. 额外上色(color) 的 stop/stop_back/resume 对称流程
 *   4. stop_back 确认退出后的清理与目标跳转(含延时取消、六感取消分支)
 *
 * 状态标志：g_on_stop_back / g_complete_to_stop_back / g_delay_cancel_* 控制
 *   stop_back 页的显示文案与确定后跳转目标。
 */

#include "nav.h"
#include "nav_internal.h"

/* 文件内前向声明(定义在后方) */
void stop_resume_cooking(void);
void color_resume_cooking(void);
void jump_to_color_stop(void);
static void six_delay_exit_to_description(void);
static void six_delay_exit_to_sixmenu(void);


// ==============================
// 暂停 / 恢复
// ==============================

// cooking → stop（暂停）
void jump_to_updown_bbq_stop(void)
{
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

    /* 保存 cooking bar 的实际值（3→100，与 cooking 页动画一致） */
    cook_bar_saved = 3 + (int)((int64_t)cook_elapsed_saved * 97 / (cook_total_ms ? cook_total_ms : 1));
    if (cook_bar_saved > 100) cook_bar_saved = 100;

    page_push(PAGE_UPDOWN_BBQ_STOP);
    lv_obj_clean(lv_scr_act());
    updown_bbq_stop_create(&ui_manager);

    updown_bbq_stop_t *stop = updown_bbq_stop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start_button, stop->little_button };
        if (g_updown_bbq_stop) lv_group_del(g_updown_bbq_stop);
        g_updown_bbq_stop = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(stop->start_button, on_stop_start_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(stop->little_button, on_cook_setting_click,
                            LV_EVENT_CLICKED, NULL);

        /* 同步 time_label / statu_label / bar_1 */
        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(stop->time_label, "%02d:%02d:%02d", h, m, s);

        set_status_label_min(stop->statu_label, set_temp_up, set_temp_down, set_hour, set_min);

        lv_bar_set_range(stop->bar_1, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(stop->bar_1, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_updown_bbq_stop;

    lang_scr_load_anim(updown_bbq_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = (cook_total_ms > (int)cook_elapsed_saved)
                          ? cook_total_ms - (int)cook_elapsed_saved : 0;
    printf("[nav] jump: cooking -> updown_bbq_stop (pause)\n");
}
// stop/cooking → stop_back（确认退出）
void jump_to_updown_bbq_stop_back(void)
{
    int cooking_bar_val = 0;
    if (cook_timer && depth > 0 && page_stack[depth - 1] == PAGE_UPDOWN_BBQ_COOKING) {
        updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
        if (cook && cook->bar) cooking_bar_val = lv_bar_get_value(cook->bar);
    }

    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_updown_bbq_complete;

    page_push(PAGE_UPDOWN_BBQ_STOP_BACK);
    lv_obj_clean(lv_scr_act());
    updown_bbq_stop_back_create(&ui_manager);

    updown_bbq_stop_back_t *back = updown_bbq_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure_button };
        if (g_updown_bbq_stop_back) lv_group_del(g_updown_bbq_stop_back);
        g_updown_bbq_stop_back = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(back->sure_button, on_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        /* 同步 statu_label / bar_2（从耗时计算进度） */
        set_status_label_min(back->statu_label, set_temp_up, set_temp_down, set_hour, set_min);

        int p = cooking_bar_val;
        if (p <= 0) {
            uint32_t elapsed = cook_elapsed_saved;
            p = stop_back_progress(elapsed, cook_total_ms);
        }
        if (p > 100) p = 100;
        lv_bar_set_range(back->bar_2, 0, 100);
        lv_bar_set_value(back->bar_2, p, LV_ANIM_OFF);

        if (g_complete_to_stop_back) {
            printf("[keepwarm] stop_back enter (from complete): keepwarm=%d iface=%d\n",
                   g_keepwarm_active, g_send.iface_status);
            g_complete_to_stop_back = 0;
            if (g_keepwarm_active)
                lv_label_set_text(back->label_8, tr("保温中..."));
            else
                lv_label_set_text(back->label_8, tr("已完成"));
            lv_bar_set_value(back->bar_2, 100, LV_ANIM_OFF);
        }

        if (g_delay_cancel_to_stop_back) {
            g_delay_cancel_to_stop_back = 0;
            lv_label_set_text(back->label_8, tr("预约中..."));
            if (g_delay_source_page == PAGE_DESCRIPTIONMENU) {
                /* 六感:status/图标/位置与六感运行页一致 */
                if (six_chick_is_probe())
                    lv_label_set_text_fmt(back->statu_label, "| %s | %s |",
                                          six_chick_name(), six_chick_degree_text());   /* 探针菜:菜名+烤色程度 */
                else if (six_chick_is_degree_time()) {
                    int d = toastcolor_degree_value();
                    if (d < 1 || d > 3) d = 2;
                    const char *dt = (d == 1) ? "浅色" : (d == 3) ? "深色" : "中等色";
                    lv_label_set_text_fmt(back->statu_label, tr("| %s | %s | %d分钟 |"),
                                          six_chick_name(), dt, six_chick_degree_min(d));   /* 烤羊肉串:菜名+程度+时间 */
                } else if (six_chick_is_kind()) {
                    int w = toastcolor_weight_value();
                    if (w < 0) w = 800;
                    lv_label_set_text_fmt(back->statu_label, tr("| %s | %dg | %d分钟 |"),
                                          six_chick_name(), w, six_chick_cook_min(w));   /* 烤鸡翅类:菜名+克重+时间 */
                } else
                    lv_label_set_text_fmt(back->statu_label, tr("| %s | %d分钟"), six_bread_name(), six_bread_cook_min());
                lv_img_set_src(back->image_7, LVGL_IMAGE_PATH(sixicon.png));
                lv_obj_set_pos(back->image_7, 163, 161);
            }
            lv_label_set_text(back->label_11, g_delay_cancel_btn ? "回到上一页" : "回到主页");
            lv_obj_add_flag(back->bar_2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(back->image_6, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(back->littal_button, LV_OBJ_FLAG_HIDDEN);
            printf("[keepwarm] stop_back enter (delay cancel): iface=%d active=%d\n",
                   g_send.iface_status, g_keepwarm_active);
        }

        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(back->label_8, tr("烹饪中..."));
        printf("[keepwarm] stop_back enter: keepwarm=%d iface=%d label=%s\n",
               g_keepwarm_active, g_send.iface_status,
               lv_label_get_text(back->label_8) ? lv_label_get_text(back->label_8) : "?");
    }
    current_group = g_updown_bbq_stop_back;

    lang_scr_load_anim(updown_bbq_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] jump: stop/cooking -> updown_bbq_stop_back\n");
}
// cooking 暂停按钮点击
void on_cook_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_stop();
}
// stop 开始按钮点击 → 恢复计时
void on_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        stop_resume_cooking();
}
// stop 小按钮点击 → 确认退出页
void on_stop_littal_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_stop_back();
}
// stop_back 小按钮点击 → 返回暂停页
void on_stop_back_littal_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        page_pop();
}
// stop 恢复 cooking（不经过 page_pop，直接重建）
void stop_resume_cooking(void)
{
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    depth--;  /* pop STOP 栈顶 */
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

        /* 恢复状态标签 */
        set_status_label_min(cook->updown_label, set_temp_up, set_temp_down, set_hour, set_min);

        /* 恢复时间显示 */
        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(cook->time_label, "%02d:%02d:%02d", h, m, s);

        /* 进度条从保存的实际值 → 100 */
        lv_bar_set_range(cook->bar, 0, 100);
        lv_bar_set_value(cook->bar, cook_bar_saved, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, cook_bar_saved, 100);
        lv_anim_set_time(&a, ((int)(cook_total_ms - (int)cook_elapsed_saved) < 0) ? 0 : (cook_total_ms - (int)cook_elapsed_saved));
        lv_anim_start(&a);
    }

    /* 恢复定时器 */
    cook_start_time = lv_tick_get() - cook_elapsed_saved;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_updown_bbq_cooking;

    lang_scr_load_anim(updown_bbq_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    {
        int rem = cook_total_ms - (int)cook_elapsed_saved;
        if (rem < 0) rem = 0;
        g_send.iface_status = IFACE_COOKING;
        g_send.remaining_ms = rem;
    }
    printf("[nav] resume: stop -> updown_bbq_cooking\n");
}
// stop_back 确定 → 退出到 major_menu
// delay 取消确认（stop_back 确定）→ 取消预约并回来源 set 页（温度/时间等设置保持）
void delay_cancel_exit_to_set(void)
{
    /* 栈守卫:仅当栈结构为 [.., DELAYCOOKING, STOP_BACK]（delay 取消场景）才执行;
       标志残留误调时只清标志,不动页面栈,防止破坏栈 */
    if (depth < 3 || page_stack[depth - 2] != PAGE_DELAYCOOKING) {
        g_delay_cancel_btn = 0;
        g_delay_cancel_to_stop_back = 0;
        return;
    }
    g_delay_cancel_to_stop_back = 0;
    g_delay_cancel_btn = 0;
    delay_on = 0;
    page_pop();   /* pop STOP_BACK（内部 rebuild_delaycooking 会重建 cook_timer/iface） */
    page_pop();   /* pop DELAYCOOKING → 重建 set 屏 */
    /* 清理必须放在两次 pop 之后，否则会被 rebuild_delaycooking 抵消 */
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    cook_total_ms = 0;
    cook_start_time = 0;
    cook_elapsed_saved = 0;
    cook_bar_saved = 0;
    g_on_stop_back = 0;
    g_complete_to_stop_back = 0;
    g_stop_back_complete = NULL;
    g_delay_target = -1;
    g_send.iface_status = IFACE_SETTING;
    g_send.remaining_ms = -1;
    g_send.cook_flag = 0;
}
// 六感 delay 取消确认:取消按钮路径 → 回 descriptionmenu
static void six_delay_exit_to_description(void)
{
    g_delay_cancel_btn = 0;
    g_delay_cancel_to_stop_back = 0;
    delay_on = 0;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    cook_total_ms = 0;
    cook_start_time = 0;
    cook_elapsed_saved = 0;
    cook_bar_saved = 0;
    g_on_stop_back = 0;
    g_complete_to_stop_back = 0;
    g_stop_back_complete = NULL;
    g_delay_target = -1;
    g_send.iface_status = IFACE_SETTING;
    g_send.remaining_ms = -1;
    g_send.cook_flag = 0;
    g_delay_source_page = PAGE_WAITMENU_24;   /* 防残留误走六感分支 */
    /* 栈守卫:仅当 [.., DELAYCOOKING, STOP_BACK] 结构才执行 */
    if (depth < 3 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_STOP_BACK ||
        page_stack[depth - 2] != PAGE_DELAYCOOKING)
        return;
    /* 弹 STOP_BACK + DELAYCOOKING + DELAYSET,不走 page_pop(会重建中间页并抵消清理) */
    depth -= 3;
    topflag_update_visibility();
    descriptionmenu_rebuild(PAGE_DESCRIPTIONMENU);
    printf("[six_cook] delay cancel -> descriptionmenu\n");
}
// 六感 delay 取消确认:BACK 路径 → 回 sixmenu
static void six_delay_exit_to_sixmenu(void)
{
    g_delay_cancel_btn = 0;
    g_delay_cancel_to_stop_back = 0;
    delay_on = 0;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    cook_total_ms = 0;
    cook_start_time = 0;
    cook_elapsed_saved = 0;
    cook_bar_saved = 0;
    g_on_stop_back = 0;
    g_stop_back_complete = NULL;
    g_delay_target = -1;
    g_send.iface_status = IFACE_SETTING;
    g_send.remaining_ms = -1;
    g_send.cook_flag = 0;
    g_delay_source_page = PAGE_WAITMENU_24;   /* 防残留误走六感分支 */
    depth = 0;
    page_push(PAGE_WAITMENU_24);
    jump_to_sixmenu();
    printf("[six_cook] delay cancel -> sixmenu\n");
}
void on_stop_back_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    if (g_delay_source_page == PAGE_DESCRIPTIONMENU &&
        depth >= 2 && page_stack[depth - 2] == PAGE_DELAYCOOKING) {
        /* 六感 delay 取消确认(栈守卫,防来源标志残留误判) */
        if (g_delay_cancel_btn)
            six_delay_exit_to_description();
        else
            six_delay_exit_to_sixmenu();
        return;
    }
    if (g_delay_cancel_btn) {
        delay_cancel_exit_to_set();
        return;
    }
    g_on_stop_back = 0;
    g_keepwarm_active = 0;

    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    cook_elapsed_saved = 0; cook_bar_saved = 0;

    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;

    depth = 2;  /* 保留 WAITMENU_24 + MAJOR_MENU */
    lv_obj_clean(lv_scr_act());
    major_menu_create(&ui_manager);
    groups_create();
    bind_events();
    major_menu_apply_lang();   /* i18n */
    current_group = g_major_menu;
    lang_scr_load_anim(major_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] stop_back sure -> major_menu\n");
}

// ==============================
// 额外上色暂停 / 恢复
// ==============================

// color_cookoing → color_stop（暂停）
void jump_to_color_stop(void)
{
    g_on_stop_back = 0;
    cook_elapsed_saved = lv_tick_get() - cook_start_time;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

    {
        color_cookoing_t *cc = color_cookoing_get(&ui_manager);
        cook_bar_saved = cc ? lv_bar_get_value(cc->bar) : 0;
    }

    page_push(PAGE_COLOR_STOP);
    lv_obj_clean(lv_scr_act());
    color_stop_create(&ui_manager);

    color_stop_t *cs = color_stop_get(&ui_manager);
    if (cs) {
        lv_obj_t *btns[] = { cs->button_6 };
        if (g_color_stop) lv_group_del(g_color_stop);
        g_color_stop = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cs->button_6, on_color_stop_start_click,
                            LV_EVENT_CLICKED, NULL);

        /* 同步 time_label / label_13 / bar_3 */
        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(cs->time_label, "%02d:%02d:%02d", h, m, s);
        lv_label_set_text_fmt(cs->label_13, tr("| 额外上色 | %d℃ | 5分钟"), set_temp);
        lv_bar_set_range(cs->bar_3, 0, 100);
        if (cook_bar_saved > 100) cook_bar_saved = 100;
        lv_bar_set_value(cs->bar_3, cook_bar_saved, LV_ANIM_OFF);
    }
    current_group = g_color_stop;

    lang_scr_load_anim(color_stop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = (cook_total_ms > (int)cook_elapsed_saved) ? cook_total_ms - (int)cook_elapsed_saved : 0;
    printf("[nav] jump: color_cookoing -> color_stop (pause)\n");
}
// color_stop → color_stop_back（确认退出）
void jump_to_color_stop_back(void)
{
    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_color_complete;
    page_push(PAGE_COLOR_STOP_BACK);
    lv_obj_clean(lv_scr_act());
    color_stop_back_create(&ui_manager);

    color_stop_back_t *csb = color_stop_back_get(&ui_manager);
    if (csb) {
        lv_obj_t *btns[] = { csb->button_7 };
        if (g_color_stop_back) lv_group_del(g_color_stop_back);
        g_color_stop_back = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(csb->button_7, on_color_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(csb->label_17, tr("| 额外上色 | %d℃ | 5分钟"), set_temp);
        uint32_t _elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
        int _p = stop_back_progress(_elapsed, cook_total_ms);
        if (_p > 100) _p = 100;
        lv_bar_set_range(csb->bar_4, 0, 100);
        lv_bar_set_value(csb->bar_4, _p, LV_ANIM_OFF);

        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(csb->label_19, tr("烹饪中..."));

        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            lv_label_set_text(csb->label_19, tr("已完成"));
            lv_bar_set_value(csb->bar_4, 100, LV_ANIM_OFF);
        }

        if (g_extra_color_to_stop_back) {
            g_extra_color_to_stop_back = 0;
            lv_label_set_text(csb->label_17, tr("| 5分钟 |"));
            lv_label_set_text(csb->label_19, tr("额外上色"));
            lv_bar_set_value(csb->bar_4, 0, LV_ANIM_OFF);
        }
    }
    current_group = g_color_stop_back;

    lang_scr_load_anim(color_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] jump: stop/cooking -> color_stop_back\n");
}
// color_cookoing 暂停按钮点击
void on_color_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_color_stop();
}
// color_stop 开始按钮点击 → 恢复计时
void on_color_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        color_resume_cooking();
}
// color_stop 恢复 cooking（不经过 page_pop，直接重建）
void color_resume_cooking(void)
{
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    g_on_stop_back = 0;
    depth--;  /* pop COLOR_STOP 栈顶 */
    lv_obj_clean(lv_scr_act());
    color_cookoing_create(&ui_manager);

    color_cookoing_t *cc = color_cookoing_get(&ui_manager);
    if (cc) {
        lv_obj_t *btns[] = { cc->stop_button };
        if (g_color_cookoing) lv_group_del(g_color_cookoing);
        g_color_cookoing = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cc->stop_button, on_color_stop_click,
                            LV_EVENT_CLICKED, NULL);

        /* 恢复时间显示 */
        int elapsed_sec = (cook_elapsed_saved + 500) / 1000;
        int total_sec = cook_total_ms / 1000;
        int remaining_sec = total_sec - elapsed_sec;
        if (remaining_sec < 0) remaining_sec = 0;
        int h = remaining_sec / 3600;
        int m = (remaining_sec % 3600) / 60;
        int s = remaining_sec % 60;
        lv_label_set_text_fmt(cc->time_label, "%02d:%02d:%02d", h, m, s);
        lv_label_set_text_fmt(cc->status_label, tr("| 额外上色 | %d℃ | 5分钟"), set_temp);

        /* 进度条从保存的值 → 100 */
        lv_bar_set_range(cc->bar, 0, 100);
        lv_bar_set_value(cc->bar, cook_bar_saved, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cc->bar);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, cook_bar_saved, 100);
        lv_anim_set_time(&a, ((int)(cook_total_ms - (int)cook_elapsed_saved) < 0) ? 0 : (cook_total_ms - (int)cook_elapsed_saved));
        lv_anim_start(&a);
    }

    /* 恢复定时器 */
    cook_is_color = 1;
    cook_start_time = lv_tick_get() - cook_elapsed_saved;
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    current_group = g_color_cookoing;

    lang_scr_load_anim(color_cookoing_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COOKING;
    g_send.remaining_ms = (cook_total_ms > (int)cook_elapsed_saved) ? cook_total_ms - (int)cook_elapsed_saved : 0;
    printf("[nav] resume: color_stop -> color_cookoing\n");
}
// color_stop_back 确定 → 退出到 major_menu
void on_color_stop_back_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    color_exit_to_home();
    printf("[nav] color_stop_back sure -> home\n");
}

