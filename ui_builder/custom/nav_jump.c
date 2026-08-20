/*
 * nav_jump.c - 页面跳转
 *
 * 职责：各入口/回调调用的跳转函数——从当前页进入下一个页面。
 *   统一模式：page_push(目标页) + lv_obj_clean 清屏 + 创建目标页 UI +
 *   建焦点组 + 设 current_group + lv_scr_load_anim。
 *
 * 包含：
 *   - 主菜单 → 烹调菜单/特色菜单
 *   - 上下烧烤全流程（menu/set/menu_top/menu_low/cooking/complete）
 *   - 额外上色流程（color_menu/cookoing/complete/exit_to_home）
 */

#include "nav.h"
#include "nav_internal.h"

/* 文件内前向声明(定义在后方) */
void jump_to_updown_bbq_menu_top(void);
void jump_to_updown_bbq_menu_low(void);
void on_color_menu_next_click(lv_event_t *e);
static void color_menu_open(void);
void jump_to_color_cookoing(void);

// ==============================
// 页面跳转
// ==============================

// major_menu → cookmenu
void jump_to_cookmenu(void)
{
    page_push(PAGE_COOKMENU);  // 推栈
    lv_obj_clean(lv_scr_act());
    cookmenu_create(&ui_manager);

    cookmenu_t *cook = cookmenu_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = {
            cook->up_down_button, cook->top_bbq_button, cook->hot_bbq_button,
            cook->hot_wind_button, cook->save_button, cook->bottom_button,
            cook->central_button, cook->windchange_buttonn, cook->preheater_button,
        };
        if (g_cookmenu) lv_group_del(g_cookmenu);  // 旧 group 可能引用已销毁的按钮
        g_cookmenu = group_create_for_page(btns, sizeof(btns) / sizeof(btns[0]));
    }

    current_group = g_cookmenu;

    /* 绑定 cookmenu 按钮的点击事件 */
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

    cookmenu_apply_lang();   /* i18n: 英文模式设置文本+字体 */

    lv_scr_load_anim(cookmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    printf("[nav] jump: major_menu -> cookmenu\n");
}
// major_menu → special_menu
void jump_to_special_menu(void)
{
    page_push(PAGE_SPECIAL_MENU);
    lv_obj_clean(lv_scr_act());
    special_menu_create(&ui_manager);

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
    }

    current_group = g_special_menu;

    lv_scr_load_anim(special_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    printf("[nav] jump: major_menu -> special_menu\n");
}
// cookmenu → updown_bbq_menu
void jump_to_updown_bbq_menu(void)
{
    set_temp = 180; set_hour = 0; set_min = 30;
    page_push(PAGE_UPDOWN_BBQ_MENU);
    lv_obj_clean(lv_scr_act());
    updown_bbq_menu_create(&ui_manager);

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

        /* 初始化数值显示（覆盖 UiBuilder 默认值） */
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
    }

    /* 初始化约束（设置 min/max 匹配当前 hour） */
    validate_constraints();

    current_group = g_updown_bbq_menu;

    /* 默认焦点在 next_button */
    if (bbq && bbq->next_button)
        lv_group_focus_obj(bbq->next_button);

    /* 绑定 next_button 事件 */
    if (bbq && bbq->next_button)
        lv_obj_add_event_cb(bbq->next_button, on_updown_next_click,
                            LV_EVENT_CLICKED, NULL);

    lv_scr_load_anim(updown_bbq_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    g_send.cook_mode = MODE_UPDOWN_BBQ;
    printf("[nav] jump: cookmenu -> updown_bbq_menu\n");
}
// updown_bbq_menu → updown_bbq_set
void jump_to_updown_bbq_set(void)
{
    set_temp_up = set_temp; set_temp_down = set_temp;
    page_push(PAGE_UPDOWN_BBQ_SET);
    lv_obj_clean(lv_scr_act());
    updown_bbq_set_create(&ui_manager);

    /* 复位 toggle 状态（全新进入，默认全关） */
    preheat_on = 0;
    delay_on = 0;
    contain_on = 0;

    updown_bbq_set_t *set = updown_bbq_set_get(&ui_manager);
    if (set) {
        lv_obj_t *btns[] = {
            set->sure_button, set->uptemp_button, set->downtemp_button,
            set->preheat_button, set->preheat_on_button,
            set->delay_button, set->delay_on_button,
            set->contain_button, set->contain_on_button,
        };
        if (g_updown_bbq_set) lv_group_del(g_updown_bbq_set);
        g_updown_bbq_set = group_create_for_page(btns, sizeof(btns) / sizeof(btns[0]));

        /* 强制清除所有按钮的 FOCUSED 状态，防止渲染缓存残留 */
        clear_focus_states(btns, sizeof(btns)/sizeof(btns[0]));
        lv_group_focus_obj(set->sure_button);

        /* 温度显示 */
        setup_set_temp_display(set);

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

        /* 按钮状态重置（跟随变量） */
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

        /* 绑定温度按钮事件 */
        lv_obj_add_event_cb(set->uptemp_button, on_updown_uptemp_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->downtemp_button, on_updown_downtemp_click,
                            LV_EVENT_CLICKED, NULL);

        /* 绑定 sure_button 事件 */
        lv_obj_add_event_cb(set->sure_button, on_sure_click, LV_EVENT_CLICKED, NULL);
    }

    current_group = g_updown_bbq_set;

    lv_scr_load_anim(updown_bbq_set_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    printf("[nav] jump: updown_bbq_menu -> updown_bbq_set\n");
}
// updown_bbq_set → updown_bbq_cooking
void jump_to_updown_bbq_cooking(void)
{
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    g_on_stop_back = 0;
    g_keepwarm_active = 0;
    g_stop_back_complete = NULL;
    page_push(PAGE_UPDOWN_BBQ_COOKING);
    lv_obj_clean(lv_scr_act());
    updown_bbq_cooking_create(&ui_manager);

    updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = {
            cook->stop_button, cook->little_button,
        };
        if (g_updown_bbq_cooking) lv_group_del(g_updown_bbq_cooking);
        g_updown_bbq_cooking = group_create_for_page(btns, sizeof(btns) / sizeof(btns[0]));

        /* 绑定停止按钮 */
        lv_obj_add_event_cb(cook->stop_button, on_cook_stop_click,
                            LV_EVENT_CLICKED, NULL);

        /* 绑定设置按钮 */
        lv_obj_add_event_cb(cook->little_button, on_cook_setting_click,
                            LV_EVENT_CLICKED, NULL);

        /* 更新界面显示 */
        set_status_label_min(cook->updown_label, set_temp_up, set_temp_down, set_hour, set_min);
    }

    /* 立即显示倒计时初始值（覆盖 UiBuilder 默认值） */
    if (cook)
        lv_label_set_text_fmt(cook->time_label, "%02d:%02d:%02d", set_hour, set_min, 0);

    /* 初始化上下温度（从 set 页带入） */
    set_temp_up = set_temp;
    set_temp_down = set_temp;

    /* 初始化进度条 + 启动动画（连续无级平滑） */
    cook_total_ms = (set_hour * 3600 + set_min * 60) * 1000;
    if (cook) {
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

    /* 启动时间显示定时器 */
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

    printf("[nav] jump: updown_bbq_set -> updown_bbq_cooking\n");
}
// updown_bbq_cooking → updown_bbq_complete
void jump_to_updown_bbq_complete(void)
{
    if (depth > 0 && page_stack[depth - 1] == PAGE_UPDOWN_BBQ_STOP_BACK)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_UPDOWN_BBQ_STOP)
        depth--;
    page_push(PAGE_UPDOWN_BBQ_COMPLETE);
    lv_obj_clean(lv_scr_act());
    updown_bbq_complete_create(&ui_manager);

    updown_bbq_complete_t *cook = updown_bbq_complete_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->little_button };
        if (g_updown_bbq_complete) lv_group_del(g_updown_bbq_complete);
        g_updown_bbq_complete = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cook->little_button, on_cook_setting_click,
                            LV_EVENT_CLICKED, NULL);
        set_status_label_min(cook->statu_label, set_temp_up, set_temp_down, set_hour, set_min);
        lv_bar_set_value(cook->bar_3, 100, LV_ANIM_OFF);
    }

    current_group = g_updown_bbq_complete;

    lv_scr_load_anim(updown_bbq_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    g_send.iface_status = IFACE_COMPLETE;
    g_send.remaining_ms = 0;

    /* 自动保温：保温开关开启时 complete 页停留 1 分钟无操作 → 保温（15 分钟） */
    if (contain_on) {
        g_keepwarm_active = 0;
        g_keepwarm_sec = 0;
        if (cook_timer) lv_timer_del(cook_timer);
        cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
        printf("[keepwarm] complete enter (contain on): active=%d sec=%d timer=%p\n",
               g_keepwarm_active, g_keepwarm_sec, (void *)cook_timer);
    } else {
        printf("[keepwarm] complete enter (contain off): no timer\n");
    }
    printf("[nav] jump: updown_bbq_cooking -> updown_bbq_complete\n");
}
// updown_bbq_set → updown_bbq_menu_top（设置上层温度）
void on_updown_uptemp_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_menu_top();
}
void on_updown_downtemp_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_menu_low();
}
void on_updown_top_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr)) {
        updown_menu_top_saved = set_temp_up;
        page_pop();
    }
}
void on_updown_low_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr)) {
        updown_menu_low_saved = set_temp_down;
        page_pop();
    }
}
void jump_to_updown_bbq_menu_top(void)
{
    updown_menu_top_saved = set_temp_up;
    page_push(PAGE_UPDOWN_BBQ_MENU_TOP);
    lv_obj_clean(lv_scr_act());
    updown_bbq_menu_top_create(&ui_manager);

    updown_bbq_menu_top_t *menu = updown_bbq_menu_top_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = { menu->temp, menu->next };
        if (g_updown_bbq_menu_top) lv_group_del(g_updown_bbq_menu_top);
        g_updown_bbq_menu_top = group_create_for_page(btns, 2);

        edit_clear();
        edit_register(menu->temp, menu->line2, menu->line3,
                      &set_temp_up, 30, 300, 5, "%d");

        lv_obj_add_event_cb(menu->temp, on_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_edit_focus, LV_EVENT_FOCUSED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp_up);

        lv_obj_add_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp_up < 100) {
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        }

        lv_group_focus_obj(menu->next);
    }
    current_group = g_updown_bbq_menu_top;

    if (menu && menu->next)
        lv_obj_add_event_cb(menu->next, on_updown_top_next_click,
                            LV_EVENT_CLICKED, NULL);

    lv_scr_load_anim(updown_bbq_menu_top_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] jump: updown_bbq_set -> updown_bbq_menu_top\n");
}
void jump_to_updown_bbq_menu_low(void)
{
    updown_menu_low_saved = set_temp_down;
    page_push(PAGE_UPDOWN_BBQ_MENU_LOW);
    lv_obj_clean(lv_scr_act());
    updown_bbq_menu_low_create(&ui_manager);

    updown_bbq_menu_low_t *menu = updown_bbq_menu_low_get(&ui_manager);
    if (menu) {
        lv_obj_t *btns[] = { menu->temp, menu->next };
        if (g_updown_bbq_menu_low) lv_group_del(g_updown_bbq_menu_low);
        g_updown_bbq_menu_low = group_create_for_page(btns, 2);

        edit_clear();
        edit_register(menu->temp, menu->line2, menu->line3,
                      &set_temp_down, 30, 300, 5, "%d");

        lv_obj_add_event_cb(menu->temp, on_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_edit_focus, LV_EVENT_FOCUSED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp_down);

        lv_obj_add_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        if (set_temp_down < 100) {
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir2, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(menu->dir3, LV_OBJ_FLAG_HIDDEN);
        }

        lv_group_focus_obj(menu->next);
    }
    current_group = g_updown_bbq_menu_low;

    if (menu && menu->next)
        lv_obj_add_event_cb(menu->next, on_updown_low_next_click,
                            LV_EVENT_CLICKED, NULL);

    lv_scr_load_anim(updown_bbq_menu_low_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] jump: updown_bbq_set -> updown_bbq_menu_low\n");
}
// 额外上色设置页（复用 preheatmenu 结构）：label_69="额外上色"，温度 30-300，next → color cooking
static void color_menu_open(void)
{
    preheatmenu_create(&ui_manager);
    preheatmenu_t *menu = preheatmenu_get(&ui_manager);
    if (menu) {
        lv_label_set_text(menu->label_69, "额外上色");
        lv_obj_t *btns[] = { menu->temp, menu->next };
        if (g_preheat_menu) lv_group_del(g_preheat_menu);
        g_preheat_menu = group_create_for_page(btns, 2);

        edit_clear();
        edit_register(menu->temp, menu->line2, menu->line3,
                      &set_temp, 30, 300, 5, "%d");

        lv_obj_add_event_cb(menu->temp, on_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_edit_focus, LV_EVENT_FOCUSED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp);

        lv_obj_add_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_event_cb(menu->next, on_color_menu_next_click,
                            LV_EVENT_CLICKED, NULL);
        lv_group_focus_obj(menu->next);
    }
    current_group = g_preheat_menu;

    lv_scr_load_anim(preheatmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] color_menu open\n");
}
void on_color_menu_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    jump_to_color_cookoing();
}
void jump_to_color_menu(void)
{
    set_temp = 180;                              /* 默认温度 */
    g_color_from_probe = is_probe_inserted();   /* 记录进入时的探针状态（返回时区分） */
    page_push(PAGE_COLOR_MENU);
    lv_obj_clean(lv_scr_act());
    color_menu_open();
}
void color_menu_rebuild(page_id_t child)
{
    color_menu_open();
}
// extra_color → color_cookoing（固定 5 分钟倒计时）
void jump_to_color_cookoing(void)
{
    edit_clear();   /* 清除编辑注册表残留（color_menu_open 注册了 preheatmenu 的字段） */
    g_on_stop_back = 0;
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    page_push(PAGE_COLOR_COOKING);
    lv_obj_clean(lv_scr_act());
    color_cookoing_create(&ui_manager);

    color_cookoing_t *cc = color_cookoing_get(&ui_manager);
    if (cc) {
        lv_obj_t *btns[] = { cc->stop_button };
        if (g_color_cookoing) lv_group_del(g_color_cookoing);
        g_color_cookoing = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cc->stop_button, on_color_stop_click,
                            LV_EVENT_CLICKED, NULL);
    }

    /* 初始化显示 */
    if (cc) {
        lv_label_set_text_fmt(cc->status_label, "| 额外上色 | %d℃ | 5分钟", set_temp);
        lv_label_set_text_fmt(cc->time_label, "%02d:%02d:%02d", 0, 5, 0);
        lv_bar_set_range(cc->bar, 0, 100);
        lv_bar_set_value(cc->bar, 3, LV_ANIM_OFF);
    }

    /* 启动进度条动画 */
    cook_total_ms = 5 * 60 * 1000;
    if (cc) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cc->bar);
        lv_anim_set_exec_cb(&a, anim_bar_set_value);
        lv_anim_set_values(&a, 3, 100);
        lv_anim_set_time(&a, cook_total_ms);
        lv_anim_start(&a);
    }

    /* 启动定时器 */
    cook_is_color = 1;
    cook_start_time = lv_tick_get();
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    g_send.iface_status = IFACE_COOKING;
    g_send.cook_mode = MODE_EXTRA_COLOR;
    g_send.set_temp = set_temp;   /* 跟随原烹饪模式温度 */
    g_send.set_temp_lower = set_temp_down;
    g_send.remaining_ms = cook_total_ms;
    g_send.cook_flag = 0;

    current_group = g_color_cookoing;

    lv_scr_load_anim(color_cookoing_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    printf("[nav] jump: extra_color -> color_cookoing\n");
}
// color_cookoing → colorcooking_complete
void jump_to_color_complete(void)
{
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }   /* 防御:防残留 */
    cook_is_color = 0;
    if (depth > 0 && page_stack[depth - 1] == PAGE_COLOR_STOP_BACK)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_COLOR_STOP)
        depth--;
    /* 压缩本轮上色层（EXTRA_COLOR/COLOR_COOKING 仅本轮使用）：
       防止完成态连续再上色时栈无限累积（MAX_STACK=16） */
    if (depth > 0 && page_stack[depth - 1] == PAGE_COLOR_COOKING)
        depth--;
    if (depth > 0 && page_stack[depth - 1] == PAGE_EXTRA_COLOR)
        depth--;
    page_push(PAGE_COLOR_COOKING_COMPLETE);
    lv_obj_clean(lv_scr_act());
    colorcooking_complete_create(&ui_manager);

    {
        colorcooking_complete_t *cc = colorcooking_complete_get(&ui_manager);
        if (cc) {
            lv_label_set_text_fmt(cc->status_label, "| 额外上色 | %d℃ | 5分钟", set_temp);
            lv_bar_set_range(cc->bar, 0, 100);
            lv_bar_set_value(cc->bar, 100, LV_ANIM_OFF);
        }
    }
    current_group = NULL;

    lv_scr_load_anim(colorcooking_complete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_COMPLETE;
    g_send.remaining_ms = 0;
    printf("[nav] jump: color_cookoing -> colorcooking_complete\n");
}
// color（额外上色）流程退出回主页：
// 探针模式进入 → 探针主菜单；否则普通主菜单。完整清理定时器/标志/栈。
void color_exit_to_home(void)
{
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    g_on_stop_back = 0;
    g_extra_color_to_stop_back = 0;
    g_complete_to_stop_back = 0;
    g_cooling_to_stop_back = 0;
    g_keepwarm_active = 0;
    g_keepwarm_sec = 0;
    cook_is_color = 0;
    g_stop_back_complete = NULL;
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.cook_flag = 0;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    if (g_color_from_probe) {
        depth = 2;
        page_stack[1] = PAGE_MAJOR_MENU_TZ;
        lv_obj_clean(lv_scr_act());   /* 与普通分支对称，避免依赖 auto_del 回收旧屏 */
        major_menu_tz_rebuild(0);
        printf("[color] exit -> major_menu_tz (probe)\n");
    } else {
        depth = 2;
        page_stack[1] = PAGE_MAJOR_MENU;
        lv_obj_clean(lv_scr_act());
        major_menu_create(&ui_manager);
        groups_create();
        bind_events();
        major_menu_apply_lang();   /* i18n */
        current_group = g_major_menu;
        lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[color] exit -> major_menu\n");
    }
}

