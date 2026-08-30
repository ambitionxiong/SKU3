/*
 * nav_pop.c - 页面栈弹出/返回重建
 *
 * 职责：
 *   核心函数 page_pop（收到 KEY_BACK 时调用）：回到上一页。
 *   内部是一个巨型 switch：按"要返回的父页 prev"重建对应页面 UI + 焦点组，
 *   并按"从哪个子页 child 回来"恢复焦点/特殊状态。
 *
 * 页面栈模型：
 *   page_push(id) 入栈 → page_pop() 出栈并重建 prev。
 *   覆盖层(设置页/探针提示/无效提示)不通过此栈管理，单独处理。
 *
 * 安全点：
 *   - depth<=1 根页保护
 *   - 返回前 edit_clear() 防悬空 label 指针(UAF)
 *   - 各 stop_back/六感/预约/多段页面的状态标志按 child 清理，防残留错分支
 */

#include "nav.h"
#include "nav_lang.h"
#include "nav_internal.h"


// 收到 KEY21 时调用，回到上一页
// 步骤：
//   ① 根页保护（depth≤1 说明只有根页，不能 pop）
//   ② depth-- 丢弃栈顶（"回到上一页"）
//   ③ child = 刚丢弃的那一页（即"从哪个页面回来的"）
//   ④ prev = 栈顶之下的那页（即"要回到哪个页面"）
//   ⑤ lv_obj_clean 清当前屏
//   ⑥ 根据 prev 重建对应页面的 UI + group
//   ⑦ 根据 child 恢复焦点到进入子页前的位置
//   ⑧ lv_scr_load_anim 显示重建后的页面
void page_pop(void)
{
    /* ① 根页保护 */
    if (depth <= 1) {
        printf("[nav] at root, cannot pop\n");
        return;
    }

    /* ②~④ 算出 child(子页) 和 prev(父页) */
    depth--;
    edit_clear();   /* 离开当前页清编辑注册表,防止悬空 label 指针残留(UAF) */
    page_id_t child = page_stack[depth];
    page_id_t prev = page_stack[depth - 1];
    printf("[nav] page pop: depth=%d, back to id=%d (from id=%d)\n", depth, prev, child);

    /* 页面栈变化后立即刷新 topflag 显隐（等值栈顶已是目标页） */
    topflag_update_visibility();

    /* 从 stop_back 确认页退出时清除暂停确认状态（防止残留导致 cooking 定时器走错分支） */
    if (g_on_stop_back) {
        switch (child) {
        case PAGE_UPDOWN_BBQ_STOP_BACK:
        case PAGE_TOP_BBQ_STOP_BACK:
        case PAGE_BOTTOM_BBQ_STOP_BACK:
        case PAGE_HOT_BBQ_STOP_BACK:
        case PAGE_HOTWIND_BBQ_STOP_BACK:
        case PAGE_SAVE_BBQ_STOP_BACK:
        case PAGE_CENTRAL_BBQ_STOP_BACK:
        case PAGE_WINDCHANGE_BBQ_STOP_BACK:
        case PAGE_AIR_STOP_BACK:
        case PAGE_PIZZA_2_STOP_BACK:
        case PAGE_SLOWCOOK_STOP_BACK:
        case PAGE_UNFROZEN_STOP_BACK:
        case PAGE_RISING_STOP_BACK:
        case PAGE_CORN_STOP_BACK:
        case PAGE_HEATCONTAIN_STOP_BACK:
        case PAGE_UPDOWN_BBQ_STOP_BACK_PROBE:
        case PAGE_HOT_BBQ_STOP_BACK_PROBE:
        case PAGE_BOTTOM_BBQ_STOP_BACK_PROBE:
        case PAGE_SLOWCOOK_STOP_BACK_PROBE:
        case PAGE_COOKIE_STOP_BACK:
        case PAGE_WEST_STOP_BACK:
        case PAGE_PIZZA_STOP_BACK:
        case PAGE_MENU_COOK_STOP_BACK:
        case PAGE_LASAGNA_STOP_BACK:
        case PAGE_STRUDEL_STOP_BACK:
        case PAGE_BREAD_STOP_BACK:
        case PAGE_PIZZA3_STOP_BACK:
        case PAGE_CHIP_STOP_BACK:
        case PAGE_CUSTOM_STOP_BACK:
        case PAGE_PREHEAT_STOP_BACK:
        case PAGE_WATER_CLEAN_STOP_BACK:
        case PAGE_HOTCLEANSAVE_STOP_BACK:
        case PAGE_HOTCLEANMIDDLE_STOP_BACK:
        case PAGE_HOTCLEANHIGH_STOP_BACK:
        case PAGE_COLOR_STOP_BACK:
            g_on_stop_back = 0;
            g_delay_cancel_btn = 0;
            break;
        default:
            break;
        }
    }

    /* ⑤ 清当前屏 */
    lv_obj_clean(lv_scr_act());

    /* ⑥ 重建父页 UI + group */
    switch (prev) {
    case PAGE_MAJOR_MENU:
        major_menu_create(&ui_manager);
        groups_create();
        bind_events();   // 新按钮需要重新绑定点击事件
        current_group = g_major_menu;

        /* ⑦ 根据 child 恢复焦点 */
        {
            major_menu_t *major = major_menu_get(&ui_manager);
            if (major) {
                if (child == PAGE_COOKMENU && major->cook_button)
                    lv_group_focus_obj(major->cook_button);
                else if (child == PAGE_SPECIAL_MENU && major->special_button)
                    lv_group_focus_obj(major->special_button);
                else if (child == PAGE_COOK4_MENU && major->cook4_button)
                    lv_group_focus_obj(major->cook4_button);
            }
        }
        lang_scr_load_anim(major_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to major_menu\n");
        break;

    case PAGE_COOKMENU:
        cookmenu_create(&ui_manager);
        {
            cookmenu_t *cook = cookmenu_get(&ui_manager);
            if (cook) {
                lv_obj_t *btns[] = {
                    cook->up_down_button, cook->top_bbq_button, cook->hot_bbq_button,
                    cook->hot_wind_button, cook->save_button, cook->bottom_button,
                    cook->central_button, cook->windchange_buttonn, cook->preheater_button,
                };
                /* 销毁旧 group + 创建新 group（旧按钮已被 lv_obj_clean 销毁） */
                if (g_cookmenu) lv_group_del(g_cookmenu);
                g_cookmenu = group_create_for_page(btns, sizeof(btns) / sizeof(btns[0]));
            }
            current_group = g_cookmenu;

            /* 根据 child 恢复焦点 */
            if (child == PAGE_UPDOWN_BBQ_MENU && cook->up_down_button)
                lv_group_focus_obj(cook->up_down_button);
            else if (child == PAGE_TOP_BBQ_MENU && cook->top_bbq_button)
                lv_group_focus_obj(cook->top_bbq_button);
            else if (child == PAGE_BOTTOM_BBQ_MENU && cook->bottom_button)
                lv_group_focus_obj(cook->bottom_button);
            else if (child == PAGE_HOT_BBQ_MENU && cook->hot_bbq_button)
                lv_group_focus_obj(cook->hot_bbq_button);
            else if (child == PAGE_HOTWIND_BBQ_MENU && cook->hot_wind_button)
                lv_group_focus_obj(cook->hot_wind_button);
            else if (child == PAGE_SAVE_BBQ_MENU && cook->save_button)
                lv_group_focus_obj(cook->save_button);
            else if (child == PAGE_CENTRAL_BBQ_MENU && cook->central_button)
                lv_group_focus_obj(cook->central_button);
            else if (child == PAGE_WINDCHANGE_BBQ_MENU && cook->windchange_buttonn)
                lv_group_focus_obj(cook->windchange_buttonn);
            else if (child == PAGE_PREHEAT_MENU && cook->preheater_button)
                lv_group_focus_obj(cook->preheater_button);

            /* 新按钮需要重新绑定事件 */
            if (cook && cook->up_down_button)
                lv_obj_add_event_cb(cook->up_down_button, on_cook_updown_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->top_bbq_button)
                lv_obj_add_event_cb(cook->top_bbq_button, on_top_bbq_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->bottom_button)
                lv_obj_add_event_cb(cook->bottom_button, on_bottom_bbq_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->hot_bbq_button)
                lv_obj_add_event_cb(cook->hot_bbq_button, on_hot_bbq_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->hot_wind_button)
                lv_obj_add_event_cb(cook->hot_wind_button, on_hotwind_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->save_button)
                lv_obj_add_event_cb(cook->save_button, on_save_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->central_button)
                lv_obj_add_event_cb(cook->central_button, on_central_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->windchange_buttonn)
                lv_obj_add_event_cb(cook->windchange_buttonn, on_windchange_click,
                                    LV_EVENT_CLICKED, NULL);
            if (cook && cook->preheater_button)
                lv_obj_add_event_cb(cook->preheater_button, on_preheat_click,
                                    LV_EVENT_CLICKED, NULL);

            /* 退出 updown_bbq 流程后清空临时值 */
            set_temp = 180;
            set_hour = 0;
            set_min = 30;
            g_send.cook_mode = MODE_NONE;
        }
        lang_scr_load_anim(cookmenu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to cookmenu\n");
        break;

    case PAGE_SPECIAL_MENU:
        special_menu_create(&ui_manager);
        {
            special_menu_t *sp = special_menu_get(&ui_manager);
            if (sp) {
                lv_obj_t *btns[] = {
                    sp->air_button, sp->piza_button, sp->frozen_cook_button,
                    sp->slow_cook_button, sp->unfrozen_button, sp->rising_button,
                    sp->corn_button, sp->heat_contain_button, sp->some_cook_button,
                };
                if (g_special_menu) lv_group_del(g_special_menu);
                g_special_menu = group_create_for_page(btns, sizeof(btns) / sizeof(btns[0]));

                lv_obj_add_event_cb(sp->air_button, on_air_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->piza_button, on_pizza_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->slow_cook_button, on_slowcook_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->unfrozen_button, on_unfrozen_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->rising_button, on_rising_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->corn_button, on_corn_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->heat_contain_button, on_heatcontain_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->frozen_cook_button, on_frozen_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(sp->some_cook_button, on_some_cook_click,
                                    LV_EVENT_CLICKED, NULL);

                if (child == PAGE_AIR_MENU && sp->air_button)
                    lv_group_focus_obj(sp->air_button);
                else if (child == PAGE_PIZZA_2_MENU && sp->piza_button)
                    lv_group_focus_obj(sp->piza_button);
                else if (child == PAGE_SLOWCOOK_MENU && sp->slow_cook_button)
                    lv_group_focus_obj(sp->slow_cook_button);
                else if (child == PAGE_UNFROZEN_MENU && sp->unfrozen_button)
                    lv_group_focus_obj(sp->unfrozen_button);
                else if (child == PAGE_RISING_MENU && sp->rising_button)
                    lv_group_focus_obj(sp->rising_button);
                else if (child == PAGE_CORN_MENU && sp->corn_button)
                    lv_group_focus_obj(sp->corn_button);
                else if (child == PAGE_HEATCONTAIN_MENU && sp->heat_contain_button)
                    lv_group_focus_obj(sp->heat_contain_button);
                else if (child == PAGE_FROZEN_COOK && sp->frozen_cook_button)
                    lv_group_focus_obj(sp->frozen_cook_button);
                else if (child == PAGE_SOMECOOK && sp->some_cook_button)
                    lv_group_focus_obj(sp->some_cook_button);
            }
            current_group = g_special_menu;
        }
        lang_scr_load_anim(special_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        g_send.cook_mode = MODE_NONE;
        printf("[nav] back to special_menu\n");
        break;

    case PAGE_SOMECOOK:
        somecook_rebuild(child);
        break;

    case PAGE_SIXMENU:
        sixmenu_rebuild(child);
        break;

    case PAGE_BREAD6MENU:
        bread6menu_rebuild(child);
        break;

    case PAGE_CAKE6MENU:
        cake6menu_rebuild(child);
        break;

    case PAGE_CHICK6MENU:
        chick6menu_rebuild(child);
        break;

    case PAGE_CHICKENMENU:
        chickenmenu_rebuild(child);
        break;

    case PAGE_VEGETABLEMENU:
        vegetablemenu_rebuild(child);
        break;

    case PAGE_SIXSET2:
        sixset2_rebuild(child);
        break;

    case PAGE_DUCK6MENU:
        duckmenu_rebuild(child);
        break;

    case PAGE_SIXOP3PAGE:
        sixop3page_rebuild(child);
        break;

    case PAGE_SIXMENUTZ:
        sixmenutz_rebuild(child);
        break;

    case PAGE_CHICKMENUTZ:
        chickmenutz_rebuild(child);
        break;

    case PAGE_FAVORITES:
        goto pop_to_waitmenu;   /* 收藏页返回固定回 waitmenu(根级页面) */

    case PAGE_RISINGPAGE:
        risingpage_rebuild(child);
        break;

    case PAGE_DESCRIPTIONMENU:
        descriptionmenu_rebuild(child);
        break;

    case PAGE_SIX_COOKING:
        six_cooking_rebuild(child);
        break;

    case PAGE_TOASTCOLOR:
        toastcolor_rebuild(child);
        break;

    case PAGE_UPDOWN_BBQ_MENU:
        updown_bbq_menu_create(&ui_manager);
        {
            updown_bbq_menu_t *bbq = updown_bbq_menu_get(&ui_manager);
            if (bbq) {
                lv_obj_t *btns[] = {
                    bbq->tempnum_label, bbq->hournum_label, bbq->minnum_label,
                    bbq->next_button,
                };
                if (g_updown_bbq_menu) lv_group_del(g_updown_bbq_menu);
                g_updown_bbq_menu = group_create_for_page(btns, 4);

                /* 注册编辑字段 */
                edit_clear();
                edit_register(bbq->tempnum_label, bbq->templine_short, bbq->temeline_long,
                              &set_temp, 30, 300, 5, "%d");
                edit_register(bbq->hournum_label, bbq->hourline, NULL,
                              &set_hour, 0, 4, 1, "%02d");
                edit_register(bbq->minnum_label, bbq->minline, NULL,
                              &set_min, 0, 59, 1, "%02d");

                /* 绑定 focus 高亮 */
                lv_obj_add_event_cb(bbq->tempnum_label, on_edit_focus, LV_EVENT_FOCUSED, NULL);
                lv_obj_add_event_cb(bbq->hournum_label, on_edit_focus, LV_EVENT_FOCUSED, NULL);
                lv_obj_add_event_cb(bbq->minnum_label, on_edit_focus, LV_EVENT_FOCUSED, NULL);
                lv_obj_add_event_cb(bbq->next_button, on_edit_focus, LV_EVENT_FOCUSED, NULL);

                /* 初始化数值显示 */
                lv_label_set_text_fmt(bbq->tempnum_label, "%d", set_temp);
                lv_label_set_text_fmt(bbq->hournum_label, "%02d", set_hour);
                lv_label_set_text_fmt(bbq->minnum_label, "%02d", set_min);

                /* 初始显示温度线 */
                lv_obj_add_flag(bbq->templine_short, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(bbq->temeline_long, LV_OBJ_FLAG_HIDDEN);
                if (set_temp < 100)
                    lv_obj_clear_flag(bbq->templine_short, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_clear_flag(bbq->temeline_long, LV_OBJ_FLAG_HIDDEN);

                /* 新按钮重新绑定事件 */
                if (bbq->next_button)
                    lv_obj_add_event_cb(bbq->next_button, on_updown_next_click,
                                        LV_EVENT_CLICKED, NULL);

                /* 初始化约束（设置 min/max 匹配当前 hour） */
                validate_constraints();

                /* 根据 child 恢复焦点 */
                if (bbq->next_button)
                    lv_group_focus_obj(bbq->next_button);
            }
            current_group = g_updown_bbq_menu;
        }
        lang_scr_load_anim(updown_bbq_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to updown_bbq_menu\n");
        break;

    case PAGE_UPDOWN_BBQ_COOKING:
        if (child == PAGE_UPDOWN_BBQ_COMPLETE)
            goto pop_to_major_menu;
        if (child == PAGE_UPDOWN_BBQ_SETTING) {
            /* 恢复进入 setting 前的原始值 */
            set_temp_up = updown_setting_saved_temp_up;
            set_temp_down = updown_setting_saved_temp_down;
            set_hour = updown_setting_saved_hour;
            set_min = updown_setting_saved_min;
            /* 从设置页返回 → 重建 cooking，恢复定时器 */
            updown_bbq_cooking_create(&ui_manager);
            {
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
                    uint32_t elapsed = lv_tick_get() - cook_start_time;
                    int elapsed_sec = (elapsed + 500) / 1000;
                    int total_sec = cook_total_ms / 1000;
                    int remaining_sec = total_sec - elapsed_sec;
                    if (remaining_sec < 0) remaining_sec = 0;
                    int h = remaining_sec / 3600;
                    int m = (remaining_sec % 3600) / 60;
                    int s = remaining_sec % 60;
                    lv_label_set_text_fmt(cook->time_label, "%02d:%02d:%02d", h, m, s);
                    lv_bar_set_range(cook->bar, 0, 100);
                    int progress = (int)((int64_t)elapsed * 100 / (cook_total_ms ? cook_total_ms : 1));
                    if (progress > 100) progress = 100;
                    lv_bar_set_value(cook->bar, progress, LV_ANIM_OFF);
                    lv_anim_t a;
                    lv_anim_init(&a);
                    lv_anim_set_var(&a, cook->bar);
                    lv_anim_set_exec_cb(&a, anim_bar_set_value);
                    lv_anim_set_values(&a, progress, 100);
                    lv_anim_set_time(&a, ((int)(cook_total_ms - (int)elapsed) < 0) ? 0 : (cook_total_ms - (int)elapsed));
                    lv_anim_start(&a);
                }
                current_group = g_updown_bbq_cooking;
            }
            lang_scr_load_anim(updown_bbq_cooking_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0,
                             ui_manager.auto_del);
            printf("[nav] back from setting -> updown_bbq_cooking\n");
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                int rem = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
                g_send.remaining_ms = rem;
            }
        } else if (child == PAGE_UPDOWN_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            updown_bbq_cooking_create(&ui_manager);
            {
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
                    uint32_t elapsed = lv_tick_get() - cook_start_time;
                    int elapsed_sec = (elapsed + 500) / 1000;
                    int total_sec = cook_total_ms / 1000;
                    int remaining_sec = total_sec - elapsed_sec;
                    if (remaining_sec < 0) remaining_sec = 0;
                    int h = remaining_sec / 3600;
                    int m = (remaining_sec % 3600) / 60;
                    int s = remaining_sec % 60;
                    lv_label_set_text_fmt(cook->time_label, "%02d:%02d:%02d", h, m, s);
                    lv_bar_set_range(cook->bar, 0, 100);
                    int progress = stop_back_progress(elapsed, cook_total_ms);
                    if (progress > 100) progress = 100;
                    lv_bar_set_value(cook->bar, progress, LV_ANIM_OFF);
                    printf("[debug] SB->COOK: elapsed=%u timer=%p prog=%d\n", elapsed, (void*)cook_timer, progress);
                    lv_anim_t a;
                    lv_anim_init(&a);
                    lv_anim_set_var(&a, cook->bar);
                    lv_anim_set_exec_cb(&a, anim_bar_set_value);
                    lv_anim_set_values(&a, progress, 100);
                    lv_anim_set_time(&a, ((int)(cook_total_ms - (int)elapsed) < 0) ? 0 : (cook_total_ms - (int)elapsed));
                    lv_anim_start(&a);
                }
                current_group = g_updown_bbq_cooking;
            }
            lang_scr_load_anim(updown_bbq_cooking_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0,
                             ui_manager.auto_del);
            printf("[nav] back from stop_back -> updown_bbq_cooking\n");
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                int rem = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
                g_send.remaining_ms = rem;
            }
        } else {
            goto rebuild_updown_bbq_set;
        }
        break;

    case PAGE_UPDOWN_BBQ_STOP:
        g_on_stop_back = 0;
        g_stop_back_complete = NULL;
        if (child == PAGE_UPDOWN_BBQ_SETTING) {
            set_temp_up = updown_setting_saved_temp_up;
            set_temp_down = updown_setting_saved_temp_down;
            set_hour = updown_setting_saved_hour;
            set_min = updown_setting_saved_min;
        }
        updown_bbq_stop_create(&ui_manager);
        {
            updown_bbq_stop_t *stop = updown_bbq_stop_get(&ui_manager);
            if (stop) {
                lv_obj_t *btns[] = { stop->start_button, stop->little_button };
                if (g_updown_bbq_stop) lv_group_del(g_updown_bbq_stop);
                g_updown_bbq_stop = group_create_for_page(btns, 2);
                lv_obj_add_event_cb(stop->start_button, on_stop_start_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(stop->little_button, on_cook_setting_click,
                                    LV_EVENT_CLICKED, NULL);

                /* 同步显示（与暂停时刻一致） */
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
        }
        lang_scr_load_anim(updown_bbq_stop_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to updown_bbq_stop\n");
        g_send.iface_status = IFACE_PAUSE;
        break;

    case PAGE_UPDOWN_BBQ_STOP_BACK:
        g_on_stop_back = 1;
        g_stop_back_complete = jump_to_updown_bbq_complete;
        updown_bbq_stop_back_create(&ui_manager);
        {
            updown_bbq_stop_back_t *back = updown_bbq_stop_back_get(&ui_manager);
            if (back) {
                lv_obj_t *btns[] = { back->sure_button };
                if (g_updown_bbq_stop_back) lv_group_del(g_updown_bbq_stop_back);
                g_updown_bbq_stop_back = group_create_for_page(btns, 1);
                lv_obj_add_event_cb(back->sure_button, on_stop_back_sure_click,
                                    LV_EVENT_CLICKED, NULL);

                /* 同步 statu_label / bar_2（从耗时计算进度） */
                set_status_label_min(back->statu_label, set_temp_up, set_temp_down, set_hour, set_min);

                uint32_t elapsed = cook_timer ? (lv_tick_get() - cook_start_time) : cook_elapsed_saved;
                int p = stop_back_progress(elapsed, cook_total_ms);
                if (p > 100) p = 100;
                lv_bar_set_range(back->bar_2, 0, 100);
                lv_bar_set_value(back->bar_2, p, LV_ANIM_OFF);

                if (g_send.iface_status == IFACE_COOKING)
                    lv_label_set_text(back->label_8, tr("烹饪中..."));

                if (g_complete_to_stop_back) {
                    g_complete_to_stop_back = 0;
                    lv_label_set_text(back->label_8, tr("已完成"));
                    lv_bar_set_value(back->bar_2, 100, LV_ANIM_OFF);
                }
            }
            current_group = g_updown_bbq_stop_back;
        }
        lang_scr_load_anim(updown_bbq_stop_back_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to updown_bbq_stop_back\n");
        break;

    case PAGE_UPDOWN_BBQ_SETTING:
        updown_bbq_setting_create(&ui_manager);
        {
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

                uint32_t elapsed = lv_tick_get() - cook_start_time;
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
        }
        lang_scr_load_anim(updown_bbq_setting_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to updown_bbq_setting\n");
        break;

    case PAGE_UPDOWN_BBQ_COMPLETE:
        if (child == PAGE_UPDOWN_BBQ_SETTING) {
            set_temp_up = updown_setting_saved_temp_up;
            set_temp_down = updown_setting_saved_temp_down;
            set_hour = updown_setting_saved_hour;
            set_min = updown_setting_saved_min;
        }
        if (child == PAGE_UPDOWN_BBQ_SETTING || child == PAGE_UPDOWN_BBQ_STOP_BACK || child == PAGE_FAVORITES) {
            updown_bbq_complete_create(&ui_manager);
            updown_bbq_complete_t *done = updown_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little_button };
                if (g_updown_bbq_complete) lv_group_del(g_updown_bbq_complete);
                g_updown_bbq_complete = group_create_for_page(btns, 1);
                lv_obj_add_event_cb(done->little_button, on_cook_setting_click,
                                    LV_EVENT_CLICKED, NULL);
                set_status_label_min(done->statu_label, set_temp_up, set_temp_down, set_hour, set_min);
                lv_bar_set_value(done->bar_3, 100, LV_ANIM_OFF);
                /* 保温场景（stop_back 返回）：恢复"保温中..."显示，计时继续 */
                if (g_keepwarm_active)
                    lv_label_set_text(done->complete_label, tr("保温中..."));
                lv_group_focus_obj(done->little_button);
            }
            current_group = g_updown_bbq_complete;
            lang_scr_load_anim(updown_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            printf("[keepwarm] page_pop complete rebuild: child=%d active=%d sec=%d iface=%d contain=%d timer=%p\n",
                   child, g_keepwarm_active, g_keepwarm_sec, g_send.iface_status,
                   contain_on, (void *)cook_timer);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_UPDOWN_BBQ_MENU_PROBE:
        updown_bbq_probe_rebuild_menu(child);
        break;

    case PAGE_UPDOWN_BBQ_SET_PROBE:
        updown_bbq_probe_rebuild_set(child);
        break;

    case PAGE_UPDOWN_BBQ_COOKING_PROBE:
        if (child == PAGE_UPDOWN_BBQ_COMPLETE_PROBE)
            goto pop_to_major_menu;
        if (child == PAGE_UPDOWN_BBQ_STOP_BACK_PROBE) {
            g_on_stop_back = 0;
            updown_bbq_probe_rebuild_cooking(child);
        } else {
            updown_bbq_probe_rebuild_cooking(child);
        }
        break;

    case PAGE_UPDOWN_BBQ_STOP_PROBE:
        updown_bbq_probe_rebuild_stop();
        break;

    case PAGE_UPDOWN_BBQ_STOP_BACK_PROBE:
        updown_bbq_probe_rebuild_stop_back();
        break;

    case PAGE_UPDOWN_BBQ_COMPLETE_PROBE:
        if (child == PAGE_UPDOWN_BBQ_STOP_BACK_PROBE || child == PAGE_FAVORITES) {
            updown_bbq_complete_probe_create(&ui_manager);
            updown_bbq_complete_probe_t *done = updown_bbq_complete_probe_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->image_31 };
                if (g_updown_bbq_complete_probe) lv_group_del(g_updown_bbq_complete_probe);
                g_updown_bbq_complete_probe = group_create_for_page(btns, 1);
                updown_bbq_probe_complete_rebind(done->image_31);
                lv_group_focus_obj(done->image_31);
                lv_label_set_text_fmt(done->label_74, tr("| 上下烧烤 | %d℃ | %d℃"), set_temp, probe_target_temp);
                lv_bar_set_value(done->bar_4, 100, LV_ANIM_OFF);
            }
            current_group = g_updown_bbq_complete_probe;
            lang_scr_load_anim(updown_bbq_complete_probe_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_HOT_BBQ_MENU_PROBE:
        hot_bbq_probe_rebuild_menu(child);
        break;
    case PAGE_HOT_BBQ_SET_PROBE:
        hot_bbq_probe_rebuild_set(child);
        break;
    case PAGE_HOT_BBQ_COOKING_PROBE:
        if (child == PAGE_HOT_BBQ_COMPLETE_PROBE)
            goto pop_to_major_menu;
        if (child == PAGE_HOT_BBQ_STOP_BACK_PROBE) {
            g_on_stop_back = 0;
            hot_bbq_probe_rebuild_cooking(child);
        } else {
            hot_bbq_probe_rebuild_cooking(child);
        }
        break;
    case PAGE_HOT_BBQ_STOP_PROBE:
        hot_bbq_probe_rebuild_stop();
        break;
    case PAGE_HOT_BBQ_STOP_BACK_PROBE:
        hot_bbq_probe_rebuild_stop_back();
        break;
    case PAGE_HOT_BBQ_COMPLETE_PROBE:
        if (child == PAGE_HOT_BBQ_STOP_BACK_PROBE || child == PAGE_FAVORITES) {
            hot_bbq_complete_probe_create(&ui_manager);
            hot_bbq_complete_probe_t *done = hot_bbq_complete_probe_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->image_15 };
                if (g_hot_bbq_complete_probe) lv_group_del(g_hot_bbq_complete_probe);
                g_hot_bbq_complete_probe = group_create_for_page(btns, 1);
                hot_bbq_probe_complete_rebind(done->image_15);
                lv_group_focus_obj(done->image_15);
                lv_label_set_text_fmt(done->status, tr("| 热风烧烤 | %d℃ | %d℃"), set_temp, probe_target_temp);
                lv_bar_set_value(done->bar_4, 100, LV_ANIM_OFF);
            }
            current_group = g_hot_bbq_complete_probe;
            lang_scr_load_anim(hot_bbq_complete_probe_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_BOTTOM_BBQ_MENU_PROBE:
        bottom_bbq_probe_rebuild_menu(child);
        break;
    case PAGE_BOTTOM_BBQ_SET_PROBE:
        bottom_bbq_probe_rebuild_set(child);
        break;
    case PAGE_BOTTOM_BBQ_COOKING_PROBE:
        if (child == PAGE_BOTTOM_BBQ_COMPLETE_PROBE)
            goto pop_to_major_menu;
        if (child == PAGE_BOTTOM_BBQ_STOP_BACK_PROBE) {
            g_on_stop_back = 0;
            bottom_bbq_probe_rebuild_cooking(child);
        } else {
            bottom_bbq_probe_rebuild_cooking(child);
        }
        break;
    case PAGE_BOTTOM_BBQ_STOP_PROBE:
        bottom_bbq_probe_rebuild_stop();
        break;
    case PAGE_BOTTOM_BBQ_STOP_BACK_PROBE:
        bottom_bbq_probe_rebuild_stop_back();
        break;
    case PAGE_BOTTOM_BBQ_COMPLETE_PROBE:
        if (child == PAGE_BOTTOM_BBQ_STOP_BACK_PROBE || child == PAGE_FAVORITES) {
            bottom_bbq_complete_probe_create(&ui_manager);
            bottom_bbq_complete_probe_t *done = bottom_bbq_complete_probe_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->image_32 };
                if (g_bottom_bbq_complete_probe) lv_group_del(g_bottom_bbq_complete_probe);
                g_bottom_bbq_complete_probe = group_create_for_page(btns, 1);
                bottom_bbq_probe_complete_rebind(done->image_32);
                lv_group_focus_obj(done->image_32);
                lv_label_set_text_fmt(done->status, tr("| 底部烧烤 | %d℃ | %d℃"), set_temp, probe_target_temp);
                lv_bar_set_value(done->bar_8, 100, LV_ANIM_OFF);
            }
            current_group = g_bottom_bbq_complete_probe;
            lang_scr_load_anim(bottom_bbq_complete_probe_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_SLOWCOOK_MENU_PROBE:
        slowcook_probe_rebuild_menu(child);
        break;
    case PAGE_SLOWCOOK_SET_PROBE:
        slowcook_probe_rebuild_set(child);
        break;
    case PAGE_SLOWCOOK_COOKING_PROBE:
        if (child == PAGE_SLOWCOOK_COMPLETE_PROBE)
            goto pop_to_major_menu;
        if (child == PAGE_SLOWCOOK_STOP_BACK_PROBE) {
            g_on_stop_back = 0;
            slowcook_probe_rebuild_cooking(child);
        } else {
            slowcook_probe_rebuild_cooking(child);
        }
        break;
    case PAGE_SLOWCOOK_STOP_PROBE:
        slowcook_probe_rebuild_stop();
        break;
    case PAGE_SLOWCOOK_STOP_BACK_PROBE:
        slowcook_probe_rebuild_stop_back();
        break;
    case PAGE_SLOWCOOK_COMPLETE_PROBE:
        if (child == PAGE_SLOWCOOK_STOP_BACK_PROBE || child == PAGE_FAVORITES) {
            slowcook_complete_probe_create(&ui_manager);
            slowcook_complete_probe_t *done = slowcook_complete_probe_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->image_48 };
                if (g_slowcook_complete_probe) lv_group_del(g_slowcook_complete_probe);
                g_slowcook_complete_probe = group_create_for_page(btns, 1);
                slowcook_probe_complete_rebind(done->image_48);
                lv_group_focus_obj(done->image_48);
                lv_label_set_text_fmt(done->status, tr("| 慢煮 | %d℃ | %d℃"), set_temp, probe_target_temp);
                lv_bar_set_value(done->bar_12, 100, LV_ANIM_OFF);
            }
            current_group = g_slowcook_complete_probe;
            lang_scr_load_anim(slowcook_complete_probe_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_EXTRA_COLOR:
        if (child == PAGE_COLOR_STOP_BACK) {
            g_on_stop_back = 0;
            extra_color_create(&ui_manager);
            extra_color_t *ec = extra_color_get(&ui_manager);
            if (ec) {
                lv_obj_t *btns[] = { ec->start_button };
                if (g_extra_color) lv_group_del(g_extra_color);
                g_extra_color = group_create_for_page(btns, 1);
                lv_obj_add_event_cb(ec->start_button, on_color_start_click,
                                    LV_EVENT_CLICKED, NULL);
                if (ec->start_button) lv_group_focus_obj(ec->start_button);
            }
            current_group = g_extra_color;
            lang_scr_load_anim(extra_color_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        color_exit_to_home();
        break;

    case PAGE_COLOR_COOKING:
        if (child == PAGE_COLOR_STOP_BACK) {
            g_on_stop_back = 0;
            cook_is_color = 1;
            if (!cook_timer) {
                cook_start_time = lv_tick_get() - cook_elapsed_saved;
                cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
            }
            lv_obj_clean(lv_scr_act());
            color_cookoing_create(&ui_manager);
            color_cookoing_t *cc = color_cookoing_get(&ui_manager);
            if (cc) {
                lv_obj_t *btns[] = { cc->stop_button };
                if (g_color_cookoing) lv_group_del(g_color_cookoing);
                g_color_cookoing = group_create_for_page(btns, 1);
                lv_obj_add_event_cb(cc->stop_button, on_color_stop_click,
                                    LV_EVENT_CLICKED, NULL);
                uint32_t elapsed = lv_tick_get() - cook_start_time;
                int remaining_sec = (cook_total_ms - (int)elapsed) / 1000;
                if (remaining_sec < 0) remaining_sec = 0;
                int h = remaining_sec / 3600;
                int m = (remaining_sec % 3600) / 60;
                int s = remaining_sec % 60;
                lv_label_set_text_fmt(cc->time_label, "%02d:%02d:%02d", h, m, s);
                lv_label_set_text_fmt(cc->status_label, tr("| 额外上色 | %d℃ | 5分钟"), set_temp);
                lv_bar_set_range(cc->bar, 0, 100);
                    int progress = stop_back_progress(elapsed, cook_total_ms);
                    if (progress > 100) progress = 100;
                lv_bar_set_value(cc->bar, progress, LV_ANIM_OFF);
                lv_anim_t a;
                lv_anim_init(&a);
                lv_anim_set_var(&a, cc->bar);
                lv_anim_set_exec_cb(&a, anim_bar_set_value);
                lv_anim_set_values(&a, progress, 100);
                lv_anim_set_time(&a, ((int)(cook_total_ms - (int)elapsed) < 0) ? 0 : (cook_total_ms - (int)elapsed));
                lv_anim_start(&a);
            }
            current_group = g_color_cookoing;
            lang_scr_load_anim(color_cookoing_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            printf("[nav] back from stop_back -> color_cookoing\n");
            break;
        }
        color_exit_to_home();
        break;
    case PAGE_COLOR_COOKING_COMPLETE:
        if (child == PAGE_COLOR_STOP_BACK || child == PAGE_FAVORITES) {
            colorcooking_complete_create(&ui_manager);
            colorcooking_complete_t *cc = colorcooking_complete_get(&ui_manager);
            if (cc) {
                lv_label_set_text_fmt(cc->status_label, tr("| 额外上色 | %d℃ | 5分钟"), set_temp);
                lv_bar_set_range(cc->bar, 0, 100);
                lv_bar_set_value(cc->bar, 100, LV_ANIM_OFF);
            }
            current_group = NULL;
            lang_scr_load_anim(colorcooking_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        color_exit_to_home();
        break;

    case PAGE_UPDOWN_BBQ_MENU_TOP:
    case PAGE_UPDOWN_BBQ_MENU_LOW:
        goto rebuild_updown_bbq_set;   /* 上级是 updown set 页，按 child 恢复对应温度 */
    case PAGE_DELAYCOOKING:
        rebuild_delaycooking();
        break;
    case PAGE_DELAYSET:
    case PAGE_UPDOWN_BBQ_SET:
    rebuild_updown_bbq_set:
        if (child == PAGE_UPDOWN_BBQ_MENU_TOP)
            set_temp_up = updown_menu_top_saved;
        else if (child == PAGE_UPDOWN_BBQ_MENU_LOW)
            set_temp_down = updown_menu_low_saved;
        /* 关闭烹饪倒计时 */
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }

        updown_bbq_set_create(&ui_manager);
        {
            updown_bbq_set_t *set = updown_bbq_set_get(&ui_manager);
            if (set) {
                lv_obj_t *btns[] = {
                    set->sure_button, set->uptemp_button, set->downtemp_button,
                    set->preheat_button, set->preheat_on_button,
                    set->delay_button, set->delay_on_button,
                    set->contain_button, set->contain_on_button,
                };
                if (g_updown_bbq_set) lv_group_del(g_updown_bbq_set);
                g_updown_bbq_set = group_create_for_page(btns, 9);

                /* 强制清除所有按钮的 FOCUSED 状态，防止渲染缓存残留 */
                clear_focus_states(btns, 9);
                lv_group_focus_obj(set->sure_button);

                /* 温度显示 */
                setup_set_temp_display(set);
                printf("%d,%d\n",set_temp_up,set_temp_down);

                /* 时间显示 */
                lv_label_set_text_fmt(set->hour_label, "%02d", set_hour);
                lv_label_set_text_fmt(set->min_label, "%02d", set_min);

                if (set_hour == 0) {
                    lv_obj_add_flag(set->hour_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(set->shi_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_pos(set->min_label, 312, 254);
                    lv_obj_set_pos(set->fen_label, 365, 269);
                } else {
                    lv_obj_clear_flag(set->hour_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->shi_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_set_pos(set->min_label, 395, 254);
                    lv_obj_set_pos(set->fen_label, 448, 269);
                }

                /* 恢复 toggle 状态（跟随变量） */
                apply_toggle_state(set->preheat_button, set->preheat_on_button, preheat_on);
                apply_toggle_state(set->delay_button, set->delay_on_button, delay_on);
                apply_toggle_state(set->contain_button, set->contain_on_button, contain_on);
                updown_set_apply_delay_label(set);

                /* 绑定 toggle 事件 */
                lv_obj_add_event_cb(set->preheat_button, on_preheat_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->preheat_on_button, on_preheat_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->delay_button, on_delay_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->delay_on_button, on_delay_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->contain_button, on_contain_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->contain_on_button, on_contain_toggle, LV_EVENT_CLICKED, NULL);

                /* 绑定 sure_button 事件 */
                lv_obj_add_event_cb(set->sure_button, on_sure_click, LV_EVENT_CLICKED, NULL);

                /* 绑定温度按钮事件 */
                lv_obj_add_event_cb(set->uptemp_button, on_updown_uptemp_click,
                                    LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->downtemp_button, on_updown_downtemp_click,
                                    LV_EVENT_CLICKED, NULL);

                /* 焦点恢复 */
                if (child == PAGE_UPDOWN_BBQ_COOKING && set->sure_button)
                    lv_group_focus_obj(set->sure_button);
                else if (child == PAGE_UPDOWN_BBQ_MENU_TOP && set->uptemp_button)
                    lv_group_focus_obj(set->uptemp_button);
                else if (child == PAGE_UPDOWN_BBQ_MENU_LOW && set->downtemp_button)
                    lv_group_focus_obj(set->downtemp_button);
                else if (child == PAGE_DELAYSET) {
                    if (delay_on && set->delay_on_button)
                        lv_group_focus_obj(set->delay_on_button);
                    else if (set->delay_button)
                        lv_group_focus_obj(set->delay_button);
                }
            }
            current_group = g_updown_bbq_set;
        }
        lang_scr_load_anim(updown_bbq_set_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to updown_bbq_set\n");
        break;

    case PAGE_COLOR_STOP:
        g_on_stop_back = 0;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }   /* 暂停页不跑烹饪计时 */
        color_stop_create(&ui_manager);
        {
            color_stop_t *cs = color_stop_get(&ui_manager);
            if (cs) {
                lv_obj_t *btns[] = { cs->button_6 };
                if (g_color_stop) lv_group_del(g_color_stop);
                g_color_stop = group_create_for_page(btns, 1);
                lv_obj_add_event_cb(cs->button_6, on_color_stop_start_click,
                                    LV_EVENT_CLICKED, NULL);

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
        }
        lang_scr_load_anim(color_stop_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to color_stop\n");
        break;

    case PAGE_COLOR_STOP_BACK:
        color_stop_back_create(&ui_manager);
        {
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

                if (g_complete_to_stop_back) {
                    g_complete_to_stop_back = 0;
                    lv_label_set_text(csb->label_19, tr("已完成"));
                    lv_bar_set_value(csb->bar_4, 100, LV_ANIM_OFF);
                }
            }
            current_group = g_color_stop_back;
        }
        lang_scr_load_anim(color_stop_back_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to color_stop_back\n");
        break;

    case PAGE_TOP_BBQ_MENU:
        top_bbq_rebuild_menu(child);
        break;

    case PAGE_TOP_BBQ_SET:
        top_bbq_rebuild_set(child);
        break;

    case PAGE_TOP_BBQ_COOKING:
        if (child == PAGE_TOP_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_TOP_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            top_bbq_rebuild_cooking(PAGE_TOP_BBQ_SETTING);
        } else if (child == PAGE_TOP_BBQ_SETTING) {
            set_temp = top_setting_saved_temp;
            set_hour = top_setting_saved_hour;
            set_min = top_setting_saved_min;
            top_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            top_bbq_rebuild_cooking(0);
        break;

    case PAGE_TOP_BBQ_SETTING:
        top_bbq_rebuild_setting();
        break;

    case PAGE_TOP_BBQ_STOP:
        if (child == PAGE_TOP_BBQ_SETTING) {
            set_temp = top_setting_saved_temp;
            set_hour = top_setting_saved_hour;
            set_min = top_setting_saved_min;
        }
        top_bbq_rebuild_stop();
        break;

    case PAGE_TOP_BBQ_STOP_BACK:
        top_bbq_rebuild_stop_back();
        break;

    case PAGE_TOP_BBQ_COMPLETE:
        if (child == PAGE_TOP_BBQ_SETTING) {
            set_temp = top_setting_saved_temp;
            set_hour = top_setting_saved_hour;
            set_min = top_setting_saved_min;
        }
        if (child == PAGE_TOP_BBQ_SETTING || child == PAGE_TOP_BBQ_STOP_BACK || child == PAGE_FAVORITES) {
            top_bbq_complete_create(&ui_manager);
            top_bbq_complete_t *done = top_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_27 };
                if (g_top_bbq_complete) lv_group_del(g_top_bbq_complete);
                g_top_bbq_complete = group_create_for_page(btns, 1);
                top_bbq_complete_rebind(done->button_27);
                lv_group_focus_obj(done->button_27);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_108, tr("保温中..."));

                lv_label_set_text_fmt(done->label_107, tr("| 顶部烧烤 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_9, 100, LV_ANIM_OFF);
            }
            current_group = g_top_bbq_complete;
            lang_scr_load_anim(top_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_BOTTOM_BBQ_MENU:
        bottom_bbq_rebuild_menu(child);
        break;

    case PAGE_BOTTOM_BBQ_SET:
        bottom_bbq_rebuild_set(child);
        break;

    case PAGE_BOTTOM_BBQ_COOKING:
        if (child == PAGE_BOTTOM_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_BOTTOM_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            bottom_bbq_rebuild_cooking(PAGE_BOTTOM_BBQ_SETTING);
        } else if (child == PAGE_BOTTOM_BBQ_SETTING) {
            set_temp = bottom_bbq_setting_saved_temp;
            set_hour = bottom_bbq_setting_saved_hour;
            set_min = bottom_bbq_setting_saved_min;
            bottom_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            bottom_bbq_rebuild_cooking(0);
        break;

    case PAGE_BOTTOM_BBQ_SETTING:
        bottom_bbq_rebuild_setting();
        break;

    case PAGE_BOTTOM_BBQ_STOP:
        if (child == PAGE_BOTTOM_BBQ_SETTING) {
            set_temp = bottom_bbq_setting_saved_temp;
            set_hour = bottom_bbq_setting_saved_hour;
            set_min = bottom_bbq_setting_saved_min;
        }
        bottom_bbq_rebuild_stop();
        break;

    case PAGE_BOTTOM_BBQ_STOP_BACK:
        bottom_bbq_rebuild_stop_back();
        break;

    case PAGE_BOTTOM_BBQ_COMPLETE:
        if (child == PAGE_BOTTOM_BBQ_SETTING) {
            set_temp = bottom_bbq_setting_saved_temp;
            set_hour = bottom_bbq_setting_saved_hour;
            set_min = bottom_bbq_setting_saved_min;
        }
        if (child == PAGE_BOTTOM_BBQ_SETTING || child == PAGE_BOTTOM_BBQ_STOP_BACK || child == PAGE_FAVORITES) {
            bottom_bbq_complete_create(&ui_manager);
            bottom_bbq_complete_t *done = bottom_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_43 };
                if (g_bottom_bbq_complete) lv_group_del(g_bottom_bbq_complete);
                g_bottom_bbq_complete = group_create_for_page(btns, 1);
                bottom_bbq_complete_rebind(done->button_43);
                lv_group_focus_obj(done->button_43);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_156, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 底部烧烤 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_13, 100, LV_ANIM_OFF);
            }
            current_group = g_bottom_bbq_complete;
            lang_scr_load_anim(bottom_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_HOT_BBQ_MENU:
        hot_bbq_rebuild_menu(child);
        break;

    case PAGE_HOT_BBQ_SET:
        hot_bbq_rebuild_set(child);
        break;

    case PAGE_HOT_BBQ_COOKING:
        if (child == PAGE_HOT_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_HOT_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            hot_bbq_rebuild_cooking(PAGE_HOT_BBQ_SETTING);
        } else if (child == PAGE_HOT_BBQ_SETTING) {
            set_temp = hot_bbq_setting_saved_temp;
            set_hour = hot_bbq_setting_saved_hour;
            set_min = hot_bbq_setting_saved_min;
            hot_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            hot_bbq_rebuild_cooking(0);
        break;

    case PAGE_HOT_BBQ_SETTING:
        hot_bbq_rebuild_setting();
        break;

    case PAGE_HOT_BBQ_STOP:
        if (child == PAGE_HOT_BBQ_SETTING) {
            set_temp = hot_bbq_setting_saved_temp;
            set_hour = hot_bbq_setting_saved_hour;
            set_min = hot_bbq_setting_saved_min;
        }
        hot_bbq_rebuild_stop();
        break;

    case PAGE_HOT_BBQ_STOP_BACK:
        hot_bbq_rebuild_stop_back();
        break;

    case PAGE_HOT_BBQ_COMPLETE:
        if (child == PAGE_HOT_BBQ_SETTING) {
            set_temp = hot_bbq_setting_saved_temp;
            set_hour = hot_bbq_setting_saved_hour;
            set_min = hot_bbq_setting_saved_min;
        }
        if (child == PAGE_HOT_BBQ_SETTING || child == PAGE_HOT_BBQ_STOP_BACK || child == PAGE_FAVORITES) {
            hot_bbq_complete_create(&ui_manager);
            hot_bbq_complete_t *done = hot_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_59 };
                if (g_hot_bbq_complete) lv_group_del(g_hot_bbq_complete);
                g_hot_bbq_complete = group_create_for_page(btns, 1);
                hot_bbq_complete_rebind(done->button_59);
                lv_group_focus_obj(done->button_59);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_204, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 热风烧烤 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_17, 100, LV_ANIM_OFF);
            }
            current_group = g_hot_bbq_complete;
            lang_scr_load_anim(hot_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_HOTWIND_BBQ_MENU:
        hotwind_bbq_rebuild_menu(child);
        break;
    case PAGE_HOTWIND_BBQ_SET:
        hotwind_bbq_rebuild_set(child);
        break;
    case PAGE_HOTWIND_BBQ_COOKING:
        if (child == PAGE_HOTWIND_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_HOTWIND_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            hotwind_bbq_rebuild_cooking(PAGE_HOTWIND_BBQ_SETTING);
        } else if (child == PAGE_HOTWIND_BBQ_SETTING) {
            set_temp = hotwind_bbq_setting_saved_temp;
            set_hour = hotwind_bbq_setting_saved_hour;
            set_min = hotwind_bbq_setting_saved_min;
            hotwind_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            hotwind_bbq_rebuild_cooking(0);
        break;
    case PAGE_HOTWIND_BBQ_SETTING:
        hotwind_bbq_rebuild_setting();
        break;
    case PAGE_HOTWIND_BBQ_STOP:
        if (child == PAGE_HOTWIND_BBQ_SETTING) {
            set_temp = hotwind_bbq_setting_saved_temp;
            set_hour = hotwind_bbq_setting_saved_hour;
            set_min = hotwind_bbq_setting_saved_min;
        }
        hotwind_bbq_rebuild_stop();
        break;
    case PAGE_HOTWIND_BBQ_STOP_BACK:
        hotwind_bbq_rebuild_stop_back();
        break;
    case PAGE_HOTWIND_BBQ_COMPLETE:
        if (child == PAGE_HOTWIND_BBQ_SETTING) {
            set_temp = hotwind_bbq_setting_saved_temp;
            set_hour = hotwind_bbq_setting_saved_hour;
            set_min = hotwind_bbq_setting_saved_min;
        }
        if (child == PAGE_HOTWIND_BBQ_SETTING || child == PAGE_HOTWIND_BBQ_STOP_BACK || child == PAGE_FAVORITES) {
            hotwind_bbq_complete_create(&ui_manager);
            hotwind_bbq_complete_t *done = hotwind_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_74 };
                if (g_hotwind_bbq_complete) lv_group_del(g_hotwind_bbq_complete);
                g_hotwind_bbq_complete = group_create_for_page(btns, 1);
                hotwind_bbq_complete_rebind(done->button_74);
                lv_group_focus_obj(done->button_74);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_242, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 热风 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_21, 100, LV_ANIM_OFF);
            }
            current_group = g_hotwind_bbq_complete;
            lang_scr_load_anim(hotwind_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_SAVE_BBQ_MENU:
        save_bbq_rebuild_menu(child);
        break;
    case PAGE_SAVE_BBQ_SET:
        save_bbq_rebuild_set(child);
        break;
    case PAGE_SAVE_BBQ_COOKING:
        if (child == PAGE_SAVE_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_SAVE_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            save_bbq_rebuild_cooking(PAGE_SAVE_BBQ_SETTING);
        } else if (child == PAGE_SAVE_BBQ_SETTING) {
            set_temp = save_bbq_setting_saved_temp;
            set_hour = save_bbq_setting_saved_hour;
            set_min = save_bbq_setting_saved_min;
            save_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            save_bbq_rebuild_cooking(0);
        break;
    case PAGE_SAVE_BBQ_SETTING:
        save_bbq_rebuild_setting();
        break;
    case PAGE_SAVE_BBQ_STOP:
        if (child == PAGE_SAVE_BBQ_SETTING) {
            set_temp = save_bbq_setting_saved_temp;
            set_hour = save_bbq_setting_saved_hour;
            set_min = save_bbq_setting_saved_min;
        }
        save_bbq_rebuild_stop();
        break;
    case PAGE_SAVE_BBQ_STOP_BACK:
        save_bbq_rebuild_stop_back();
        break;
    case PAGE_SAVE_BBQ_COMPLETE:
        if (child == PAGE_SAVE_BBQ_SETTING) {
            set_temp = save_bbq_setting_saved_temp;
            set_hour = save_bbq_setting_saved_hour;
            set_min = save_bbq_setting_saved_min;
        }
        if (child == PAGE_SAVE_BBQ_SETTING || child == PAGE_SAVE_BBQ_STOP_BACK || child == PAGE_FAVORITES) {
            save_bbq_complete_create(&ui_manager);
            save_bbq_complete_t *done = save_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_89 };
                if (g_save_bbq_complete) lv_group_del(g_save_bbq_complete);
                g_save_bbq_complete = group_create_for_page(btns, 1);
                save_bbq_complete_rebind(done->button_89);
                lv_group_focus_obj(done->button_89);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_280, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 节能热风 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_25, 100, LV_ANIM_OFF);
            }
            current_group = g_save_bbq_complete;
            lang_scr_load_anim(save_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_CENTRAL_BBQ_MENU:
        central_bbq_rebuild_menu(child);
        break;
    case PAGE_CENTRAL_BBQ_SET:
        central_bbq_rebuild_set(child);
        break;
    case PAGE_CENTRAL_BBQ_COOKING:
        if (child == PAGE_CENTRAL_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_CENTRAL_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            central_bbq_rebuild_cooking(PAGE_CENTRAL_BBQ_SETTING);
        } else if (child == PAGE_CENTRAL_BBQ_SETTING) {
            set_temp = central_bbq_setting_saved_temp;
            set_hour = central_bbq_setting_saved_hour;
            set_min = central_bbq_setting_saved_min;
            central_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            central_bbq_rebuild_cooking(0);
        break;
    case PAGE_CENTRAL_BBQ_SETTING:
        central_bbq_rebuild_setting();
        break;
    case PAGE_CENTRAL_BBQ_STOP:
        if (child == PAGE_CENTRAL_BBQ_SETTING) {
            set_temp = central_bbq_setting_saved_temp;
            set_hour = central_bbq_setting_saved_hour;
            set_min = central_bbq_setting_saved_min;
        }
        central_bbq_rebuild_stop();
        break;
    case PAGE_CENTRAL_BBQ_STOP_BACK:
        central_bbq_rebuild_stop_back();
        break;
    case PAGE_CENTRAL_BBQ_COMPLETE:
        if (child == PAGE_CENTRAL_BBQ_SETTING) {
            set_temp = central_bbq_setting_saved_temp;
            set_hour = central_bbq_setting_saved_hour;
            set_min = central_bbq_setting_saved_min;
        }
        if (child == PAGE_CENTRAL_BBQ_SETTING || child == PAGE_CENTRAL_BBQ_STOP_BACK || child == PAGE_FAVORITES) {
            central_bbq_complete_create(&ui_manager);
            central_bbq_complete_t *done = central_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_104 };
                if (g_central_bbq_complete) lv_group_del(g_central_bbq_complete);
                g_central_bbq_complete = group_create_for_page(btns, 1);
                central_bbq_complete_rebind(done->button_104);
                lv_group_focus_obj(done->button_104);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_318, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 集中烧烤 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_29, 100, LV_ANIM_OFF);
            }
            current_group = g_central_bbq_complete;
            lang_scr_load_anim(central_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_WINDCHANGE_BBQ_MENU:
        windchange_bbq_rebuild_menu(child);
        break;
    case PAGE_WINDCHANGE_BBQ_SET:
        windchange_bbq_rebuild_set(child);
        break;
    case PAGE_WINDCHANGE_BBQ_COOKING:
        if (child == PAGE_WINDCHANGE_BBQ_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_WINDCHANGE_BBQ_STOP_BACK) {
            g_on_stop_back = 0;
            windchange_bbq_rebuild_cooking(PAGE_WINDCHANGE_BBQ_SETTING);
        } else if (child == PAGE_WINDCHANGE_BBQ_SETTING) {
            set_temp = windchange_bbq_setting_saved_temp;
            set_hour = windchange_bbq_setting_saved_hour;
            set_min = windchange_bbq_setting_saved_min;
            windchange_bbq_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            windchange_bbq_rebuild_cooking(0);
        break;
    case PAGE_WINDCHANGE_BBQ_SETTING:
        windchange_bbq_rebuild_setting();
        break;
    case PAGE_WINDCHANGE_BBQ_STOP:
        if (child == PAGE_WINDCHANGE_BBQ_SETTING) {
            set_temp = windchange_bbq_setting_saved_temp;
            set_hour = windchange_bbq_setting_saved_hour;
            set_min = windchange_bbq_setting_saved_min;
        }
        windchange_bbq_rebuild_stop();
        break;
    case PAGE_WINDCHANGE_BBQ_STOP_BACK:
        windchange_bbq_rebuild_stop_back();
        break;
    case PAGE_WINDCHANGE_BBQ_COMPLETE:
        if (child == PAGE_WINDCHANGE_BBQ_SETTING) {
            set_temp = windchange_bbq_setting_saved_temp;
            set_hour = windchange_bbq_setting_saved_hour;
            set_min = windchange_bbq_setting_saved_min;
        }
        if (child == PAGE_WINDCHANGE_BBQ_SETTING || child == PAGE_WINDCHANGE_BBQ_STOP_BACK || child == PAGE_FAVORITES) {
            windchange_bbq_complete_create(&ui_manager);
            windchange_bbq_complete_t *done = windchange_bbq_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->button_119 };
                if (g_windchange_bbq_complete) lv_group_del(g_windchange_bbq_complete);
                g_windchange_bbq_complete = group_create_for_page(btns, 1);
                windchange_bbq_complete_rebind(done->button_119);
                lv_group_focus_obj(done->button_119);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_356, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 热风对流 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_33, 100, LV_ANIM_OFF);
            }
            current_group = g_windchange_bbq_complete;
            lang_scr_load_anim(windchange_bbq_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_COOK4_MENU:
        cook4menu_rebuild(child);
        break;
    case PAGE_COOKIE_MENU:
        cookie_rebuild_menu(child);
        break;
    case PAGE_COOKIE_SET:
        cookie_rebuild_set(child);
        break;
    case PAGE_COOKIE_COOKING:
        if (child == PAGE_COOKIE_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_COOKIE_STOP_BACK) {
            g_on_stop_back = 0;
            cookie_rebuild_cooking(PAGE_COOKIE_SETTING);
        } else if (child == PAGE_COOKIE_SETTING) {
            set_temp = cookie_setting_saved_temp;
            set_hour = cookie_setting_saved_hour;
            set_min = cookie_setting_saved_min;
            cookie_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            cookie_rebuild_cooking(0);
        break;
    case PAGE_COOKIE_SETTING:
        cookie_rebuild_setting();
        break;
    case PAGE_COOKIE_STOP:
        if (child == PAGE_COOKIE_SETTING) {
            set_temp = cookie_setting_saved_temp;
            set_hour = cookie_setting_saved_hour;
            set_min = cookie_setting_saved_min;
        }
        cookie_rebuild_stop();
        break;
    case PAGE_COOKIE_STOP_BACK:
        cookie_rebuild_stop_back();
        break;
    case PAGE_COOKIE_COMPLETE:
        if (child == PAGE_COOKIE_SETTING) {
            set_temp = cookie_setting_saved_temp;
            set_hour = cookie_setting_saved_hour;
            set_min = cookie_setting_saved_min;
        }
        if (child == PAGE_COOKIE_SETTING || child == PAGE_COOKIE_STOP_BACK || child == PAGE_FAVORITES) {
            cookie_complete_create(&ui_manager);
            cookie_complete_t *done = cookie_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_cookie_complete) lv_group_del(g_cookie_complete);
                g_cookie_complete = group_create_for_page(btns, 1);
                cookie_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_143, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 饼干 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_7, 100, LV_ANIM_OFF);
            }
            current_group = g_cookie_complete;
            lang_scr_load_anim(cookie_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_WEST_MENU:
        west_rebuild_menu(child);
        break;
    case PAGE_WEST_SET:
        west_rebuild_set(child);
        break;
    case PAGE_WEST_COOKING:
        if (child == PAGE_WEST_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_WEST_STOP_BACK) {
            g_on_stop_back = 0;
            west_rebuild_cooking(PAGE_WEST_SETTING);
        } else if (child == PAGE_WEST_SETTING) {
            set_temp = west_setting_saved_temp;
            set_hour = west_setting_saved_hour;
            set_min = west_setting_saved_min;
            west_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            west_rebuild_cooking(0);
        break;
    case PAGE_WEST_SETTING:
        west_rebuild_setting();
        break;
    case PAGE_WEST_STOP:
        if (child == PAGE_WEST_SETTING) {
            set_temp = west_setting_saved_temp;
            set_hour = west_setting_saved_hour;
            set_min = west_setting_saved_min;
        }
        west_rebuild_stop();
        break;
    case PAGE_WEST_STOP_BACK:
        west_rebuild_stop_back();
        break;
    case PAGE_WEST_COMPLETE:
        if (child == PAGE_WEST_SETTING) {
            set_temp = west_setting_saved_temp;
            set_hour = west_setting_saved_hour;
            set_min = west_setting_saved_min;
        }
        if (child == PAGE_WEST_SETTING || child == PAGE_WEST_STOP_BACK || child == PAGE_FAVORITES) {
            west_complete_create(&ui_manager);
            west_complete_t *done = west_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_west_complete) lv_group_del(g_west_complete);
                g_west_complete = group_create_for_page(btns, 1);
                west_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_192, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 西式塔 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_11, 100, LV_ANIM_OFF);
            }
            current_group = g_west_complete;
            lang_scr_load_anim(west_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_PIZZA_MENU:
        pizza_rebuild_menu(child);
        break;
    case PAGE_PIZZA_SET:
        pizza_rebuild_set(child);
        break;
    case PAGE_PIZZA_COOKING:
        if (child == PAGE_PIZZA_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_PIZZA_STOP_BACK) {
            g_on_stop_back = 0;
            pizza_rebuild_cooking(PAGE_PIZZA_SETTING);
        } else if (child == PAGE_PIZZA_SETTING) {
            set_temp = pizza_setting_saved_temp;
            set_hour = pizza_setting_saved_hour;
            set_min = pizza_setting_saved_min;
            pizza_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            pizza_rebuild_cooking(0);
        break;
    case PAGE_PIZZA_SETTING:
        pizza_rebuild_setting();
        break;
    case PAGE_PIZZA_STOP:
        if (child == PAGE_PIZZA_SETTING) {
            set_temp = pizza_setting_saved_temp;
            set_hour = pizza_setting_saved_hour;
            set_min = pizza_setting_saved_min;
        }
        pizza_rebuild_stop();
        break;
    case PAGE_PIZZA_STOP_BACK:
        pizza_rebuild_stop_back();
        break;
    case PAGE_PIZZA_COMPLETE:
        if (child == PAGE_PIZZA_SETTING) {
            set_temp = pizza_setting_saved_temp;
            set_hour = pizza_setting_saved_hour;
            set_min = pizza_setting_saved_min;
        }
        if (child == PAGE_PIZZA_SETTING || child == PAGE_PIZZA_STOP_BACK || child == PAGE_FAVORITES) {
            pizza_complete_create(&ui_manager);
            pizza_complete_t *done = pizza_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_pizza_complete) lv_group_del(g_pizza_complete);
                g_pizza_complete = group_create_for_page(btns, 1);
                pizza_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_240, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 披萨 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_15, 100, LV_ANIM_OFF);
            }
            current_group = g_pizza_complete;
            lang_scr_load_anim(pizza_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_MENU_COOK_MENU:
        menu_rebuild_menu(child);
        break;
    case PAGE_MENU_COOK_SET:
        menu_rebuild_set(child);
        break;
    case PAGE_MENU_COOK_COOKING:
        if (child == PAGE_MENU_COOK_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_MENU_COOK_STOP_BACK) {
            g_on_stop_back = 0;
            menu_rebuild_cooking(PAGE_MENU_COOK_SETTING);
        } else if (child == PAGE_MENU_COOK_SETTING) {
            set_temp = menu_setting_saved_temp;
            set_hour = menu_setting_saved_hour;
            set_min = menu_setting_saved_min;
            menu_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            menu_rebuild_cooking(0);
        break;
    case PAGE_MENU_COOK_SETTING:
        menu_rebuild_setting();
        break;
    case PAGE_MENU_COOK_STOP:
        if (child == PAGE_MENU_COOK_SETTING) {
            set_temp = menu_setting_saved_temp;
            set_hour = menu_setting_saved_hour;
            set_min = menu_setting_saved_min;
        }
        menu_rebuild_stop();
        break;
    case PAGE_MENU_COOK_STOP_BACK:
        menu_rebuild_stop_back();
        break;
    case PAGE_MENU_COOK_COMPLETE:
        if (child == PAGE_MENU_COOK_SETTING) {
            set_temp = menu_setting_saved_temp;
            set_hour = menu_setting_saved_hour;
            set_min = menu_setting_saved_min;
        }
        if (child == PAGE_MENU_COOK_SETTING || child == PAGE_MENU_COOK_STOP_BACK || child == PAGE_FAVORITES) {
            menu_complete_create(&ui_manager);
            menu_complete_t *done = menu_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_menu_cook_complete) lv_group_del(g_menu_cook_complete);
                g_menu_cook_complete = group_create_for_page(btns, 1);
                menu_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_288, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 菜单 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_19, 100, LV_ANIM_OFF);
            }
            current_group = g_menu_cook_complete;
            lang_scr_load_anim(menu_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_AIR_MENU:
        air_rebuild_menu(child);
        break;
    case PAGE_AIR_SET:
        air_rebuild_set(child);
        break;
    case PAGE_AIR_COOKING:
        if (child == PAGE_AIR_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_AIR_STOP_BACK) {
            g_on_stop_back = 0;
            air_rebuild_cooking(PAGE_AIR_SETTING);
        } else if (child == PAGE_AIR_SETTING) {
            set_temp = air_setting_saved_temp;
            set_hour = air_setting_saved_hour;
            set_min = air_setting_saved_min;
            air_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            air_rebuild_cooking(0);
        break;
    case PAGE_AIR_SETTING:
        air_rebuild_setting();
        break;
    case PAGE_AIR_STOP:
        if (child == PAGE_AIR_SETTING) {
            set_temp = air_setting_saved_temp;
            set_hour = air_setting_saved_hour;
            set_min = air_setting_saved_min;
        }
        air_rebuild_stop();
        break;
    case PAGE_AIR_STOP_BACK:
        air_rebuild_stop_back();
        break;
    case PAGE_AIR_COMPLETE:
        if (child == PAGE_AIR_SETTING) {
            set_temp = air_setting_saved_temp;
            set_hour = air_setting_saved_hour;
            set_min = air_setting_saved_min;
        }
        if (child == PAGE_AIR_SETTING || child == PAGE_AIR_STOP_BACK || child == PAGE_FAVORITES) {
            air_complete_create(&ui_manager);
            air_complete_t *done = air_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_air_complete) lv_group_del(g_air_complete);
                g_air_complete = group_create_for_page(btns, 1);
                air_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_337, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 空气炸 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_23, 100, LV_ANIM_OFF);
            }
            current_group = g_air_complete;
            lang_scr_load_anim(air_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_PIZZA_2_MENU:
        pizza_2_rebuild_menu(child);
        break;
    case PAGE_PIZZA_2_SET:
        pizza_2_rebuild_set(child);
        break;
    case PAGE_PIZZA_2_COOKING:
        if (child == PAGE_PIZZA_2_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_PIZZA_2_STOP_BACK) {
            g_on_stop_back = 0;
            pizza_2_rebuild_cooking(PAGE_PIZZA_2_SETTING);
        } else if (child == PAGE_PIZZA_2_SETTING) {
            set_temp = pizza_2_setting_saved_temp;
            set_hour = pizza_2_setting_saved_hour;
            set_min = pizza_2_setting_saved_min;
            pizza_2_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            pizza_2_rebuild_cooking(0);
        break;
    case PAGE_PIZZA_2_SETTING:
        pizza_2_rebuild_setting();
        break;
    case PAGE_PIZZA_2_STOP:
        if (child == PAGE_PIZZA_2_SETTING) {
            set_temp = pizza_2_setting_saved_temp;
            set_hour = pizza_2_setting_saved_hour;
            set_min = pizza_2_setting_saved_min;
        }
        pizza_2_rebuild_stop();
        break;
    case PAGE_PIZZA_2_STOP_BACK:
        pizza_2_rebuild_stop_back();
        break;
    case PAGE_PIZZA_2_COMPLETE:
        if (child == PAGE_PIZZA_2_SETTING) {
            set_temp = pizza_2_setting_saved_temp;
            set_hour = pizza_2_setting_saved_hour;
            set_min = pizza_2_setting_saved_min;
        }
        if (child == PAGE_PIZZA_2_SETTING || child == PAGE_PIZZA_2_STOP_BACK || child == PAGE_FAVORITES) {
            pizza_2_complete_create(&ui_manager);
            pizza_2_complete_t *done = pizza_2_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_pizza_2_complete) lv_group_del(g_pizza_2_complete);
                g_pizza_2_complete = group_create_for_page(btns, 1);
                pizza_2_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_386, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 披萨 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_27, 100, LV_ANIM_OFF);
            }
            current_group = g_pizza_2_complete;
            lang_scr_load_anim(pizza_2_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_SLOWCOOK_MENU:
        slowcook_rebuild_menu(child);
        break;
    case PAGE_SLOWCOOK_SET:
        slowcook_rebuild_set(child);
        break;
    case PAGE_SLOWCOOK_COOKING:
        if (child == PAGE_SLOWCOOK_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_SLOWCOOK_STOP_BACK) {
            g_on_stop_back = 0;
            slowcook_rebuild_cooking(PAGE_SLOWCOOK_SETTING);
        } else if (child == PAGE_SLOWCOOK_SETTING) {
            set_temp = slowcook_setting_saved_temp;
            set_hour = slowcook_setting_saved_hour;
            set_min = slowcook_setting_saved_min;
            slowcook_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            slowcook_rebuild_cooking(0);
        break;
    case PAGE_SLOWCOOK_SETTING:
        slowcook_rebuild_setting();
        break;
    case PAGE_SLOWCOOK_STOP:
        if (child == PAGE_SLOWCOOK_SETTING) {
            set_temp = slowcook_setting_saved_temp;
            set_hour = slowcook_setting_saved_hour;
            set_min = slowcook_setting_saved_min;
        }
        slowcook_rebuild_stop();
        break;
    case PAGE_SLOWCOOK_STOP_BACK:
        slowcook_rebuild_stop_back();
        break;
    case PAGE_SLOWCOOK_COMPLETE:
        if (child == PAGE_SLOWCOOK_SETTING) {
            set_temp = slowcook_setting_saved_temp;
            set_hour = slowcook_setting_saved_hour;
            set_min = slowcook_setting_saved_min;
        }
        if (child == PAGE_SLOWCOOK_SETTING || child == PAGE_SLOWCOOK_STOP_BACK || child == PAGE_FAVORITES) {
            slowcook_complete_create(&ui_manager);
            slowcook_complete_t *done = slowcook_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_slowcook_complete) lv_group_del(g_slowcook_complete);
                g_slowcook_complete = group_create_for_page(btns, 1);
                slowcook_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_435, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 慢煮 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_31, 100, LV_ANIM_OFF);
            }
            current_group = g_slowcook_complete;
            lang_scr_load_anim(slowcook_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_UNFROZEN_MENU:
        unfrozen_rebuild_menu(child);
        break;
    case PAGE_UNFROZEN_SET:
        unfrozen_rebuild_set(child);
        break;
    case PAGE_UNFROZEN_COOKING:
        if (child == PAGE_UNFROZEN_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_UNFROZEN_STOP_BACK) {
            g_on_stop_back = 0;
            unfrozen_rebuild_cooking(PAGE_UNFROZEN_SETTING);
        } else if (child == PAGE_UNFROZEN_SETTING) {
            set_temp = unfrozen_setting_saved_temp;
            set_hour = unfrozen_setting_saved_hour;
            set_min = unfrozen_setting_saved_min;
            unfrozen_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            unfrozen_rebuild_cooking(0);
        break;
    case PAGE_UNFROZEN_SETTING:
        unfrozen_rebuild_setting();
        break;
    case PAGE_UNFROZEN_STOP:
        if (child == PAGE_UNFROZEN_SETTING) {
            set_temp = unfrozen_setting_saved_temp;
            set_hour = unfrozen_setting_saved_hour;
            set_min = unfrozen_setting_saved_min;
        }
        unfrozen_rebuild_stop();
        break;
    case PAGE_UNFROZEN_STOP_BACK:
        unfrozen_rebuild_stop_back();
        break;
    case PAGE_UNFROZEN_COMPLETE:
        if (child == PAGE_UNFROZEN_SETTING) {
            set_temp = unfrozen_setting_saved_temp;
            set_hour = unfrozen_setting_saved_hour;
            set_min = unfrozen_setting_saved_min;
        }
        if (child == PAGE_UNFROZEN_SETTING || child == PAGE_UNFROZEN_STOP_BACK || child == PAGE_FAVORITES) {
            unfrozen_complete_create(&ui_manager);
            unfrozen_complete_t *done = unfrozen_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_unfrozen_complete) lv_group_del(g_unfrozen_complete);
                g_unfrozen_complete = group_create_for_page(btns, 1);
                unfrozen_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_484, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 解冻 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_35, 100, LV_ANIM_OFF);
            }
            current_group = g_unfrozen_complete;
            lang_scr_load_anim(unfrozen_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_RISING_MENU:
        rising_rebuild_menu(child);
        break;
    case PAGE_RISING_SET:
        rising_rebuild_set(child);
        break;
    case PAGE_RISING_COOKING:
        if (child == PAGE_RISING_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_RISING_STOP_BACK) {
            g_on_stop_back = 0;
            rising_rebuild_cooking(PAGE_RISING_SETTING);
        } else if (child == PAGE_RISING_SETTING) {
            set_temp = rising_setting_saved_temp;
            set_hour = rising_setting_saved_hour;
            set_min = rising_setting_saved_min;
            rising_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            rising_rebuild_cooking(0);
        break;
    case PAGE_RISING_SETTING:
        rising_rebuild_setting();
        break;
    case PAGE_RISING_STOP:
        if (child == PAGE_RISING_SETTING) {
            set_temp = rising_setting_saved_temp;
            set_hour = rising_setting_saved_hour;
            set_min = rising_setting_saved_min;
        }
        rising_rebuild_stop();
        break;
    case PAGE_RISING_STOP_BACK:
        rising_rebuild_stop_back();
        break;
    case PAGE_RISING_COMPLETE:
        if (child == PAGE_RISING_SETTING) {
            set_temp = rising_setting_saved_temp;
            set_hour = rising_setting_saved_hour;
            set_min = rising_setting_saved_min;
        }
        if (child == PAGE_RISING_SETTING || child == PAGE_RISING_STOP_BACK || child == PAGE_FAVORITES) {
            rising_complete_create(&ui_manager);
            rising_complete_t *done = rising_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_rising_complete) lv_group_del(g_rising_complete);
                g_rising_complete = group_create_for_page(btns, 1);
                rising_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_533, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 发酵 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_39, 100, LV_ANIM_OFF);
            }
            current_group = g_rising_complete;
            lang_scr_load_anim(rising_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_CORN_MENU:
        corn_rebuild_menu(child);
        break;
    case PAGE_CORN_SET:
        corn_rebuild_set(child);
        break;
    case PAGE_CORN_COOKING:
        if (child == PAGE_CORN_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_CORN_STOP_BACK) {
            g_on_stop_back = 0;
            corn_rebuild_cooking(PAGE_CORN_SETTING);
        } else if (child == PAGE_CORN_SETTING) {
            set_temp = corn_setting_saved_temp;
            set_hour = corn_setting_saved_hour;
            set_min = corn_setting_saved_min;
            corn_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            corn_rebuild_cooking(0);
        break;
    case PAGE_CORN_SETTING:
        corn_rebuild_setting();
        break;
    case PAGE_CORN_STOP:
        if (child == PAGE_CORN_SETTING) {
            set_temp = corn_setting_saved_temp;
            set_hour = corn_setting_saved_hour;
            set_min = corn_setting_saved_min;
        }
        corn_rebuild_stop();
        break;
    case PAGE_CORN_STOP_BACK:
        corn_rebuild_stop_back();
        break;
    case PAGE_CORN_COMPLETE:
        if (child == PAGE_CORN_SETTING) {
            set_temp = corn_setting_saved_temp;
            set_hour = corn_setting_saved_hour;
            set_min = corn_setting_saved_min;
        }
        if (child == PAGE_CORN_SETTING || child == PAGE_CORN_STOP_BACK || child == PAGE_FAVORITES) {
            corn_complete_create(&ui_manager);
            corn_complete_t *done = corn_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_corn_complete) lv_group_del(g_corn_complete);
                g_corn_complete = group_create_for_page(btns, 1);
                corn_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_582, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 干果 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_43, 100, LV_ANIM_OFF);
            }
            current_group = g_corn_complete;
            lang_scr_load_anim(corn_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_HEATCONTAIN_MENU:
        heatcontain_rebuild_menu(child);
        break;
    case PAGE_HEATCONTAIN_SET:
        heatcontain_rebuild_set(child);
        break;
    case PAGE_HEATCONTAIN_COOKING:
        if (child == PAGE_HEATCONTAIN_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_HEATCONTAIN_STOP_BACK) {
            g_on_stop_back = 0;
            heatcontain_rebuild_cooking(PAGE_HEATCONTAIN_SETTING);
        } else if (child == PAGE_HEATCONTAIN_SETTING) {
            set_temp = heatcontain_setting_saved_temp;
            set_hour = heatcontain_setting_saved_hour;
            set_min = heatcontain_setting_saved_min;
            heatcontain_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            g_send.set_temp = set_temp;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            heatcontain_rebuild_cooking(0);
        break;
    case PAGE_HEATCONTAIN_SETTING:
        heatcontain_rebuild_setting();
        break;
    case PAGE_HEATCONTAIN_STOP:
        if (child == PAGE_HEATCONTAIN_SETTING) {
            set_temp = heatcontain_setting_saved_temp;
            set_hour = heatcontain_setting_saved_hour;
            set_min = heatcontain_setting_saved_min;
        }
        heatcontain_rebuild_stop();
        break;
    case PAGE_HEATCONTAIN_STOP_BACK:
        heatcontain_rebuild_stop_back();
        break;
    case PAGE_HEATCONTAIN_COMPLETE:
        if (child == PAGE_HEATCONTAIN_SETTING) {
            set_temp = heatcontain_setting_saved_temp;
            set_hour = heatcontain_setting_saved_hour;
            set_min = heatcontain_setting_saved_min;
        }
        if (child == PAGE_HEATCONTAIN_SETTING || child == PAGE_HEATCONTAIN_STOP_BACK || child == PAGE_FAVORITES) {
            heatcontain_complete_create(&ui_manager);
            heatcontain_complete_t *done = heatcontain_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_heatcontain_complete) lv_group_del(g_heatcontain_complete);
                g_heatcontain_complete = group_create_for_page(btns, 1);
                heatcontain_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_672, tr("保温中..."));

                lv_label_set_text_fmt(done->status, tr("| 保温 | %d℃ | %02d分钟"), set_temp, set_min);
                lv_bar_set_value(done->bar_48, 100, LV_ANIM_OFF);
            }
            current_group = g_heatcontain_complete;
            lang_scr_load_anim(heatcontain_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_FROZEN_COOK:
        frozen_rebuild(child);
        break;

    case PAGE_LASAGNA_MENU:
        lasagna_rebuild_menu(child);
        break;
    case PAGE_LASAGNA_SET:
        lasagna_rebuild_set(child);
        break;
    case PAGE_LASAGNA_COOKING:
        if (child == PAGE_LASAGNA_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_LASAGNA_STOP_BACK) {
            g_on_stop_back = 0;
            lasagna_rebuild_cooking(PAGE_LASAGNA_SETTING);
        } else if (child == PAGE_LASAGNA_SETTING) {
            set_hour = lasagna_setting_saved_hour;
            set_min = lasagna_setting_saved_min;
            lasagna_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            lasagna_rebuild_cooking(0);
        break;
    case PAGE_LASAGNA_SETTING:
        lasagna_rebuild_setting();
        break;
    case PAGE_LASAGNA_STOP:
        if (child == PAGE_LASAGNA_SETTING) {
            set_hour = lasagna_setting_saved_hour;
            set_min = lasagna_setting_saved_min;
        }
        lasagna_rebuild_stop();
        break;
    case PAGE_LASAGNA_STOP_BACK:
        lasagna_rebuild_stop_back();
        break;
    case PAGE_LASAGNA_COMPLETE:
        if (child == PAGE_LASAGNA_SETTING || child == PAGE_LASAGNA_STOP_BACK || child == PAGE_FAVORITES) {
            lasagna_complete_create(&ui_manager);
            lasagna_complete_t *done = lasagna_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_lasagna_complete) lv_group_del(g_lasagna_complete);
                g_lasagna_complete = group_create_for_page(btns, 1);
                lasagna_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_681, tr("保温中..."));

                if (set_hour == 0)
                    lv_label_set_text_fmt(done->status, tr("| 千层面 | %d℃ | %02d分钟"), set_temp, set_min);
                else
                    lv_label_set_text_fmt(done->status, tr("| 千层面 | %d℃ | %d小时%02d分钟"), set_temp, set_hour, set_min);
                lv_bar_set_value(done->bar_51, 100, LV_ANIM_OFF);
            }
            current_group = g_lasagna_complete;
            lang_scr_load_anim(lasagna_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_STRUDEL_MENU:
        strudel_rebuild_menu(child);
        break;
    case PAGE_STRUDEL_SET:
        strudel_rebuild_set(child);
        break;
    case PAGE_STRUDEL_COOKING:
        if (child == PAGE_STRUDEL_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_STRUDEL_STOP_BACK) {
            g_on_stop_back = 0;
            strudel_rebuild_cooking(PAGE_STRUDEL_SETTING);
        } else if (child == PAGE_STRUDEL_SETTING) {
            set_hour = strudel_setting_saved_hour;
            set_min = strudel_setting_saved_min;
            strudel_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            strudel_rebuild_cooking(0);
        break;
    case PAGE_STRUDEL_SETTING:
        strudel_rebuild_setting();
        break;
    case PAGE_STRUDEL_STOP:
        if (child == PAGE_STRUDEL_SETTING) {
            set_hour = strudel_setting_saved_hour;
            set_min = strudel_setting_saved_min;
        }
        strudel_rebuild_stop();
        break;
    case PAGE_STRUDEL_STOP_BACK:
        strudel_rebuild_stop_back();
        break;
    case PAGE_STRUDEL_COMPLETE:
        if (child == PAGE_STRUDEL_SETTING || child == PAGE_STRUDEL_STOP_BACK || child == PAGE_FAVORITES) {
            strudel_complete_create(&ui_manager);
            strudel_complete_t *done = strudel_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_strudel_complete) lv_group_del(g_strudel_complete);
                g_strudel_complete = group_create_for_page(btns, 1);
                strudel_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_722, tr("保温中..."));

                if (set_hour == 0)
                    lv_label_set_text_fmt(done->status, tr("| 果馅卷 | %d℃ | %02d分钟"), set_temp, set_min);
                else
                    lv_label_set_text_fmt(done->status, tr("| 果馅卷 | %d℃ | %d小时%02d分钟"), set_temp, set_hour, set_min);
                lv_bar_set_value(done->bar_55, 100, LV_ANIM_OFF);
            }
            current_group = g_strudel_complete;
            lang_scr_load_anim(strudel_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_BREAD_MENU:
        bread_rebuild_menu(child);
        break;
    case PAGE_BREAD_SET:
        bread_rebuild_set(child);
        break;
    case PAGE_BREAD_COOKING:
        if (child == PAGE_BREAD_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_BREAD_STOP_BACK) {
            g_on_stop_back = 0;
            bread_rebuild_cooking(PAGE_BREAD_SETTING);
        } else if (child == PAGE_BREAD_SETTING) {
            set_hour = bread_setting_saved_hour;
            set_min = bread_setting_saved_min;
            bread_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            bread_rebuild_cooking(0);
        break;
    case PAGE_BREAD_SETTING:
        bread_rebuild_setting();
        break;
    case PAGE_BREAD_STOP:
        if (child == PAGE_BREAD_SETTING) {
            set_hour = bread_setting_saved_hour;
            set_min = bread_setting_saved_min;
        }
        bread_rebuild_stop();
        break;
    case PAGE_BREAD_STOP_BACK:
        bread_rebuild_stop_back();
        break;
    case PAGE_BREAD_COMPLETE:
        if (child == PAGE_BREAD_SETTING || child == PAGE_BREAD_STOP_BACK || child == PAGE_FAVORITES) {
            bread_complete_create(&ui_manager);
            bread_complete_t *done = bread_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_bread_complete) lv_group_del(g_bread_complete);
                g_bread_complete = group_create_for_page(btns, 1);
                bread_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_763, tr("保温中..."));

                if (set_hour == 0)
                    lv_label_set_text_fmt(done->status, tr("| 面包 | %d℃ | %02d分钟"), set_temp, set_min);
                else
                    lv_label_set_text_fmt(done->status, tr("| 面包 | %d℃ | %d小时%02d分钟"), set_temp, set_hour, set_min);
                lv_bar_set_value(done->bar_59, 100, LV_ANIM_OFF);
            }
            current_group = g_bread_complete;
            lang_scr_load_anim(bread_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_PIZZA3_MENU:
        pizza3_rebuild_menu(child);
        break;
    case PAGE_PIZZA3_SET:
        pizza3_rebuild_set(child);
        break;
    case PAGE_PIZZA3_COOKING:
        if (child == PAGE_PIZZA3_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_PIZZA3_STOP_BACK) {
            g_on_stop_back = 0;
            pizza3_rebuild_cooking(PAGE_PIZZA3_SETTING);
        } else if (child == PAGE_PIZZA3_SETTING) {
            set_hour = pizza3_setting_saved_hour;
            set_min = pizza3_setting_saved_min;
            pizza3_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            pizza3_rebuild_cooking(0);
        break;
    case PAGE_PIZZA3_SETTING:
        pizza3_rebuild_setting();
        break;
    case PAGE_PIZZA3_STOP:
        if (child == PAGE_PIZZA3_SETTING) {
            set_hour = pizza3_setting_saved_hour;
            set_min = pizza3_setting_saved_min;
        }
        pizza3_rebuild_stop();
        break;
    case PAGE_PIZZA3_STOP_BACK:
        pizza3_rebuild_stop_back();
        break;
    case PAGE_PIZZA3_COMPLETE:
        if (child == PAGE_PIZZA3_SETTING || child == PAGE_PIZZA3_STOP_BACK || child == PAGE_FAVORITES) {
            pizza3_complete_create(&ui_manager);
            pizza3_complete_t *done = pizza3_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_pizza3_complete) lv_group_del(g_pizza3_complete);
                g_pizza3_complete = group_create_for_page(btns, 1);
                pizza3_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_804, tr("保温中..."));

                if (set_hour == 0)
                    lv_label_set_text_fmt(done->status, tr("| 披萨 | %d℃ | %02d分钟"), set_temp, set_min);
                else
                    lv_label_set_text_fmt(done->status, tr("| 披萨 | %d℃ | %d小时%02d分钟"), set_temp, set_hour, set_min);
                lv_bar_set_value(done->bar_63, 100, LV_ANIM_OFF);
            }
            current_group = g_pizza3_complete;
            lang_scr_load_anim(pizza3_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_CHIP_MENU:
        chip_rebuild_menu(child);
        break;
    case PAGE_CHIP_SET:
        chip_rebuild_set(child);
        break;
    case PAGE_CHIP_COOKING:
        if (child == PAGE_CHIP_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_CHIP_STOP_BACK) {
            g_on_stop_back = 0;
            chip_rebuild_cooking(PAGE_CHIP_SETTING);
        } else if (child == PAGE_CHIP_SETTING) {
            set_hour = chip_setting_saved_hour;
            set_min = chip_setting_saved_min;
            chip_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            chip_rebuild_cooking(0);
        break;
    case PAGE_CHIP_SETTING:
        chip_rebuild_setting();
        break;
    case PAGE_CHIP_STOP:
        if (child == PAGE_CHIP_SETTING) {
            set_hour = chip_setting_saved_hour;
            set_min = chip_setting_saved_min;
        }
        chip_rebuild_stop();
        break;
    case PAGE_CHIP_STOP_BACK:
        chip_rebuild_stop_back();
        break;
    case PAGE_CHIP_COMPLETE:
        if (child == PAGE_CHIP_SETTING || child == PAGE_CHIP_STOP_BACK || child == PAGE_FAVORITES) {
            chip_complete_create(&ui_manager);
            chip_complete_t *done = chip_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_chip_complete) lv_group_del(g_chip_complete);
                g_chip_complete = group_create_for_page(btns, 1);
                chip_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_845, tr("保温中..."));

                if (set_hour == 0)
                    lv_label_set_text_fmt(done->status, tr("| 薯条 | %d℃ | %02d分钟"), set_temp, set_min);
                else
                    lv_label_set_text_fmt(done->status, tr("| 薯条 | %d℃ | %d小时%02d分钟"), set_temp, set_hour, set_min);
                lv_bar_set_value(done->bar_67, 100, LV_ANIM_OFF);
            }
            current_group = g_chip_complete;
            lang_scr_load_anim(chip_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_CUSTOM_MENU:
        custom_rebuild_menu(child);
        break;
    case PAGE_CUSTOM_SET:
        custom_rebuild_set(child);
        break;
    case PAGE_CUSTOM_COOKING:
        if (child == PAGE_CUSTOM_COMPLETE)
            goto pop_to_major_menu;
    if (child == PAGE_CUSTOM_STOP_BACK) {
            g_on_stop_back = 0;
            custom_rebuild_cooking(PAGE_CUSTOM_SETTING);
        } else if (child == PAGE_CUSTOM_SETTING) {
            set_hour = custom_setting_saved_hour;
            set_min = custom_setting_saved_min;
            custom_rebuild_cooking(child);
            g_send.iface_status = IFACE_COOKING;
            {
                uint32_t e = lv_tick_get() - cook_start_time;
                g_send.remaining_ms = (int)(cook_total_ms > (int)e ? cook_total_ms - (int)e : 0);
            }
        } else
            custom_rebuild_cooking(0);
        break;
    case PAGE_CUSTOM_SETTING:
        custom_rebuild_setting();
        break;
    case PAGE_CUSTOM_STOP:
        if (child == PAGE_CUSTOM_SETTING) {
            set_hour = custom_setting_saved_hour;
            set_min = custom_setting_saved_min;
        }
        custom_rebuild_stop();
        break;
    case PAGE_CUSTOM_STOP_BACK:
        custom_rebuild_stop_back();
        break;
    case PAGE_CUSTOM_COMPLETE:
        if (child == PAGE_CUSTOM_SETTING || child == PAGE_CUSTOM_STOP_BACK || child == PAGE_FAVORITES) {
            custom_complete_create(&ui_manager);
            custom_complete_t *done = custom_complete_get(&ui_manager);
            if (done) {
                lv_obj_t *btns[] = { done->little };
                if (g_custom_complete) lv_group_del(g_custom_complete);
                g_custom_complete = group_create_for_page(btns, 1);
                custom_complete_rebind(done->little);
                lv_group_focus_obj(done->little);
                if (g_keepwarm_active)
                    lv_label_set_text(done->label_886, tr("保温中..."));

                if (set_hour == 0)
                    lv_label_set_text_fmt(done->status, tr("| 自定义 | %d℃ | %02d分钟"), set_temp, set_min);
                else
                    lv_label_set_text_fmt(done->status, tr("| 自定义 | %d℃ | %d小时%02d分钟"), set_temp, set_hour, set_min);
                lv_bar_set_value(done->bar_71, 100, LV_ANIM_OFF);
            }
            current_group = g_custom_complete;
            lang_scr_load_anim(custom_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_major_menu;

    case PAGE_CLEAN_MENU:
        clean_rebuild(child);
        break;
    case PAGE_HOTCLEAN_MENU:
        hotclean_rebuild(child);
        break;
    case PAGE_HOTCLEANSAVE_SET:
        hcs_rebuild_set(child);
        break;
    case PAGE_HOTCLEANSAVE_COOKING:
        if (child == PAGE_HOTCLEANSAVE_COMPLETE)
            goto pop_to_clean;
        if (child == PAGE_HOTCLEANSAVE_COOLING) {
            hcs_rebuild_cooking(child);
        } else if (child == PAGE_HOTCLEANSAVE_STOP_BACK) {
            g_on_stop_back = 0;
            hcs_rebuild_cooking(PAGE_HOTCLEANSAVE_STOP);
        } else
            hcs_rebuild_cooking(0);
        break;
    case PAGE_HOTCLEANSAVE_STOP:
        hcs_rebuild_stop();
        break;
    case PAGE_HOTCLEANSAVE_STOP_BACK:
        hcs_rebuild_stop_back();
        break;
    case PAGE_HOTCLEANSAVE_COOLING:
        if (child == PAGE_HOTCLEANSAVE_COMPLETE)
            goto pop_to_clean;
        hcs_rebuild_cooling();
        break;
    case PAGE_HOTCLEANSAVE_COMPLETE:
        if (child == PAGE_HOTCLEANSAVE_STOP_BACK || child == PAGE_FAVORITES) {
            hotcleansave_complete_create(&ui_manager);
            hotcleansave_complete_t *done = hotcleansave_complete_get(&ui_manager);
            if (done) {
                lv_bar_set_value(done->bar_1, 100, LV_ANIM_OFF);
            }
            current_group = g_hcs_complete;
            lang_scr_load_anim(hotcleansave_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_clean;
    case PAGE_HOTCLEANMIDDLE_SET:
        hcm_rebuild_set(child);
        break;
    case PAGE_HOTCLEANMIDDLE_COOKING:
        if (child == PAGE_HOTCLEANMIDDLE_COMPLETE)
            goto pop_to_clean;
        if (child == PAGE_HOTCLEANMIDDLE_COOLING) {
            hcm_rebuild_cooking(child);
        } else if (child == PAGE_HOTCLEANMIDDLE_STOP_BACK) {
            g_on_stop_back = 0;
            hcm_rebuild_cooking(PAGE_HOTCLEANMIDDLE_STOP);
        } else
            hcm_rebuild_cooking(0);
        break;
    case PAGE_HOTCLEANMIDDLE_STOP:
        hcm_rebuild_stop();
        break;
    case PAGE_HOTCLEANMIDDLE_STOP_BACK:
        hcm_rebuild_stop_back();
        break;
    case PAGE_HOTCLEANMIDDLE_COOLING:
        if (child == PAGE_HOTCLEANMIDDLE_COMPLETE)
            goto pop_to_clean;
        hcm_rebuild_cooling();
        break;
    case PAGE_HOTCLEANMIDDLE_COMPLETE:
        if (child == PAGE_HOTCLEANMIDDLE_STOP_BACK || child == PAGE_FAVORITES) {
            hotcleanmiddle_complete_create(&ui_manager);
            hotcleanmiddle_complete_t *done = hotcleanmiddle_complete_get(&ui_manager);
            if (done) {
                lv_bar_set_value(done->bar_5, 100, LV_ANIM_OFF);
            }
            current_group = g_hcm_complete;
            lang_scr_load_anim(hotcleanmiddle_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_clean;
    case PAGE_HOTCLEANHIGH_SET:
        hch_rebuild_set(child);
        break;
    case PAGE_HOTCLEANHIGH_COOKING:
        if (child == PAGE_HOTCLEANHIGH_COMPLETE)
            goto pop_to_clean;
        if (child == PAGE_HOTCLEANHIGH_COOLING) {
            hch_rebuild_cooking(child);
        } else if (child == PAGE_HOTCLEANHIGH_STOP_BACK) {
            g_on_stop_back = 0;
            hch_rebuild_cooking(PAGE_HOTCLEANHIGH_STOP);
        } else
            hch_rebuild_cooking(0);
        break;
    case PAGE_HOTCLEANHIGH_STOP:
        hch_rebuild_stop();
        break;
    case PAGE_HOTCLEANHIGH_STOP_BACK:
        hch_rebuild_stop_back();
        break;
    case PAGE_HOTCLEANHIGH_COOLING:
        if (child == PAGE_HOTCLEANHIGH_COMPLETE)
            goto pop_to_clean;
        hch_rebuild_cooling();
        break;
    case PAGE_HOTCLEANHIGH_COMPLETE:
        if (child == PAGE_HOTCLEANHIGH_STOP_BACK || child == PAGE_FAVORITES) {
            hotcleanhigh_complete_create(&ui_manager);
            hotcleanhigh_complete_t *done = hotcleanhigh_complete_get(&ui_manager);
            if (done) {
                lv_bar_set_value(done->bar_9, 100, LV_ANIM_OFF);
            }
            current_group = g_hch_complete;
            lang_scr_load_anim(hotcleanhigh_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_clean;
    case PAGE_WATER_CLEAN_SET:
        wc_rebuild_set(child);
        break;
    case PAGE_WATER_CLEAN_COOKING:
        if (child == PAGE_WATER_CLEAN_COMPLETE)
            goto pop_to_clean;
        if (child == PAGE_WATER_CLEAN_STOP_BACK) {
            g_on_stop_back = 0;
            wc_rebuild_cooking(PAGE_WATER_CLEAN_STOP);
        } else
            wc_rebuild_cooking(child);
        break;
    case PAGE_WATER_CLEAN_STOP:
        wc_rebuild_stop();
        break;
    case PAGE_WATER_CLEAN_STOP_BACK:
        wc_rebuild_stop_back();
        break;
    case PAGE_WATER_CLEAN_COMPLETE:
        if (child == PAGE_WATER_CLEAN_STOP_BACK || child == PAGE_FAVORITES) {
            waterclean_complete_create(&ui_manager);
            waterclean_complete_t *done = waterclean_complete_get(&ui_manager);
            if (done) {
                lv_bar_set_value(done->bar_4, 100, LV_ANIM_OFF);
            }
            current_group = g_wc_complete;
            lang_scr_load_anim(waterclean_complete_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
            break;
        }
        goto pop_to_clean;

    pop_to_clean:
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        set_hour = 0; set_min = 10;
        cook_elapsed_saved = 0; cook_bar_saved = 0;
        depth = 2;
        clean_rebuild(0);
        break;

    case PAGE_PREHEAT_MENU:
        preheat_rebuild_menu(child);
        break;
    case PAGE_COLOR_MENU:
        color_menu_rebuild(child);
        break;
    case PAGE_PREHEAT_COOKING:
        if (child == PAGE_PREHEAT_STOP_BACK) {
            g_on_stop_back = 0;
            if (!cook_timer) {
                cook_start_time = lv_tick_get() - cook_elapsed_saved;
                cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
            }
            preheat_rebuild_cooking();
            break;
        }
        goto pop_to_major_menu;
    case PAGE_PREHEAT_STOP:
        preheat_rebuild_stop();
        break;
    case PAGE_PREHEAT_STOP_BACK:
        preheat_rebuild_stop_back();
        break;
    case PAGE_PREHEAT_COMPLETE:
        if (child == PAGE_PREHEAT_STOP_BACK || child == PAGE_FAVORITES) {
            preheat_rebuild_complete();
            break;
        }
        goto pop_to_major_menu;

    pop_to_major_menu:
        /* 关闭定时器 */
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
        cook_is_color = 0;
        cook_elapsed_saved = 0; cook_bar_saved = 0;

        g_send.iface_status = IFACE_SETTING;
        g_send.cook_mode = MODE_NONE;
        g_send.cook_flag = 0;
        g_send.set_temp = 0;
        g_send.set_temp_lower = 0;
        g_send.remaining_ms = -1;

        if (is_probe_inserted()) {
            depth = 2;
            page_stack[1] = PAGE_MAJOR_MENU_TZ;
            major_menu_tz_rebuild(0);
        } else {
            depth = 2;
            page_stack[1] = PAGE_MAJOR_MENU;   /* goto 进入时 stack[1] 可能是任意 prev 页，显式修正 */
            lv_obj_clean(lv_scr_act());
            major_menu_create(&ui_manager);
            groups_create();
            bind_events();
                current_group = g_major_menu;
            lang_scr_load_anim(major_menu_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0,
                             ui_manager.auto_del);
            printf("[nav] pop to major_menu\n");
        }
        break;

    case PAGE_MAJOR_MENU_TZ:
        major_menu_tz_rebuild(child);
        break;

    case PAGE_COOK_MENU_TZ:
        cook_menu_tz_rebuild(child);
        break;

    case PAGE_SPECIAL_MENU_TZ:
        special_menu_tz_rebuild(child);
        break;

    case PAGE_PROBETIP:
        probetip_cancel_auto_dismiss();
        goto pop_to_waitmenu;

    case PAGE_WAITMENU_24:
    pop_to_waitmenu:
        waitmenu_24_create(&ui_manager);
        waitmenu_clock_cache_reset();   /* 强制刷新为真实时间 */
        current_group = NULL;
        lang_scr_load_anim(waitmenu_24_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        waitmenu_apply_clock();   /* 立即刷新为真实时间，不等 500ms 定时器 */
        g_send.iface_status = IFACE_STANDBY;
        printf("[nav] back to waitmenu_24\n");
        break;

    default:
        printf("[nav] unknown page to restore: %d, fallback to waitmenu_24\n", prev);
        goto pop_to_waitmenu;
    }
}

