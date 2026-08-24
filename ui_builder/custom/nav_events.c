/*
 * nav_events.c - 事件绑定与回调
 *
 * 职责：
 *   1. bind_events：为主菜单三个按钮绑定 LV_EVENT_CLICKED 回调
 *      (每次 groups_create 后必须调用，新按钮需重新绑定)
 *   2. 各 on_xxx_click 回调：主菜单/烹调菜单/六感前两层等入口按钮 → 跳转
 *   3. 延迟预约(delayset/delaycooking) 完整流程回调
 *   4. 额外上色 color 入口回调、模式名/图标工具(mode_display_name/mode_apply_icon)
 *
 * 回调统一防重入：screen_is_loading(lv_scr_act()) 时忽略点击。
 */

#include "nav.h"
#include "nav_internal.h"

// ==============================
// 事件绑定
// ==============================

static void on_major_cook_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))  // 防重入：动画未完成时不跳转
        jump_to_cookmenu();
}
void jump_to_cook4_menu(void);
static void on_major_special_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_special_menu();
}
static void on_major_cook4_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_cook4_menu();
}
static void on_cook4_cookie_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_cookie_menu();
}
static void on_cook4_west_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_west_menu();
}
static void on_cook4_pizza_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_pizza_menu();
}
static void on_cook4_menu_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_menu_menu();
}
void jump_to_cook4_menu(void)
{
    page_push(PAGE_COOK4_MENU);
    lv_obj_clean(lv_scr_act());
    cook4menu_create(&ui_manager);

    cook4menu_t *c4 = cook4menu_get(&ui_manager);
    if (c4) {
        lv_obj_t *btns[] = { c4->cookie, c4->west, c4->piza, c4->menu };
        if (g_cook4_menu) lv_group_del(g_cook4_menu);
        g_cook4_menu = group_create_for_page(btns, 4);

        lv_obj_add_event_cb(c4->cookie, on_cook4_cookie_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(c4->west, on_cook4_west_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(c4->piza, on_cook4_pizza_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(c4->menu, on_cook4_menu_click,
                            LV_EVENT_CLICKED, NULL);
    }
    current_group = g_cook4_menu;

    lang_scr_load_anim(cook4menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] major_menu -> cook4_menu\n");
    g_send.cook_mode = MODE_COOK4;
}
void cook4menu_rebuild(page_id_t child)
{
    lv_obj_clean(lv_scr_act());
    cook4menu_create(&ui_manager);

    cook4menu_t *c4 = cook4menu_get(&ui_manager);
    if (c4) {
        lv_obj_t *btns[] = { c4->cookie, c4->west, c4->piza, c4->menu };
        if (g_cook4_menu) lv_group_del(g_cook4_menu);
        g_cook4_menu = group_create_for_page(btns, 4);

        lv_obj_add_event_cb(c4->cookie, on_cook4_cookie_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(c4->west, on_cook4_west_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(c4->piza, on_cook4_pizza_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(c4->menu, on_cook4_menu_click,
                            LV_EVENT_CLICKED, NULL);

        if (child == PAGE_COOKIE_MENU && c4->cookie)
            lv_group_focus_obj(c4->cookie);
        else if (child == PAGE_WEST_MENU && c4->west)
            lv_group_focus_obj(c4->west);
        else if (child == PAGE_PIZZA_MENU && c4->piza)
            lv_group_focus_obj(c4->piza);
        else if (child == PAGE_MENU_COOK_MENU && c4->menu)
            lv_group_focus_obj(c4->menu);
    }
    current_group = g_cook4_menu;

    lang_scr_load_anim(cook4menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] back to cook4_menu\n");
    g_send.cook_mode = MODE_COOK4;
}
void on_cook_updown_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_menu();
}
void on_top_bbq_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_top_bbq_menu();
}
void on_bottom_bbq_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_bottom_bbq_menu();
}
void on_hot_bbq_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hot_bbq_menu();
}
void on_hotwind_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hotwind_bbq_menu();
}
void on_save_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_save_bbq_menu();
}
void on_central_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_central_bbq_menu();
}
void on_windchange_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_windchange_bbq_menu();
}
void on_preheat_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_preheat_menu();
}
void on_air_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_air_menu();
}
void on_pizza_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_pizza_2_menu();
}
void on_some_cook_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_somecook();
}
void on_slowcook_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_slowcook_menu();
}
void on_unfrozen_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_unfrozen_menu();
}
void on_rising_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    /* 发酵需冷却状态:腔温>50℃ 跳温度提示页(5秒自动返回,可 BACK 提前返回) */
    if (get_cavity_temp() > 50) {
        jump_to_temptip();
        return;
    }
    jump_to_rising_menu();
}
void on_corn_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_corn_menu();
}
void on_heatcontain_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_heatcontain_menu();
}
void on_frozen_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_frozen_cook();
}
void on_updown_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_set();
}
void on_edit_focus(lv_event_t *e)
{
    /* 隐藏所有指示线 */
    for (int i = 0; i < edit_count; i++) {
        if (edit_fields[i].ind_short)
            lv_obj_add_flag(edit_fields[i].ind_short, LV_OBJ_FLAG_HIDDEN);
        if (edit_fields[i].ind_long)
            lv_obj_add_flag(edit_fields[i].ind_long, LV_OBJ_FLAG_HIDDEN);
    }
    /* 显示当前焦点的指示线 */
    lv_obj_t *target = lv_event_get_target(e);
    edit_field_t *f = find_edit_field(target);
    if (f) {
        if (f->ind_short && f->ind_long) {
            if (*f->value < 100)
                lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
            else
                lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
        } else if (f->ind_short) {
            lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
        }
    }
    /* 强制整个页面重绘 */
    lv_obj_invalidate(lv_scr_act());
}
void on_preheat_toggle(lv_event_t *e)
{
    updown_bbq_set_t *set = updown_bbq_set_get(&ui_manager);
    if (!set) return;
    preheat_on = !preheat_on;
    if (preheat_on) {
        /* 互斥：开预热关延时 */
        delay_on = 0;
        apply_toggle_state(set->delay_button, set->delay_on_button, delay_on);
        lv_obj_add_flag(set->preheat_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->preheat_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->preheat_on_button);
    } else {
        lv_obj_add_flag(set->preheat_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->preheat_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->preheat_button);
    }
}

int delay_hour = 19;
int delay_min = 0;

// 进入 delayset 的时刻（今天/明天切换基准，不随实时时间变化）
static int delayset_enter_hour = -1;
static int delayset_enter_min = 0;

// 刷新 delayset 页的时间显示（hour 显示 0-23，超 23:59 切换明天）
// 今天/明天标签：调节期按"进入时刻"双向比较（已过→明天），确定后 delay_hour>=24 恒明天。
// 刷新 delayset 页的时间显示（hour 显示 0-23，min 0-59）
// 今天/明天标签：与进入时刻纯比较（小时+分钟分开判断），小于 → 明天，否则 → 今天
void delayset_refresh_display(delayset_t *ds)
{
    if (!ds) return;
    lv_label_set_text_fmt(ds->hour, "%02d", delay_hour);
    lv_label_set_text_fmt(ds->min, "%02d", delay_min);
    const char *day;
    if (delayset_enter_hour >= 0 &&
        (delay_hour < delayset_enter_hour ||
         (delay_hour == delayset_enter_hour && delay_min < delayset_enter_min)))
        day = tr("明天");   /* 小时或分钟小于时间戳 → 明天 */
    else
        day = tr("今天");   /* 大于或相等 → 今天 */
    lv_label_set_text(ds->day, day);
}
// delayset 焦点切换：显示对应字段的下划线
void on_delayset_focus(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    delayset_t *ds = delayset_get(&ui_manager);
    if (!ds) return;
    lv_obj_add_flag(ds->image_9, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ds->image_10, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ds->startline, LV_OBJ_FLAG_HIDDEN);
    if (obj == ds->hour)
        lv_obj_clear_flag(ds->image_9, LV_OBJ_FLAG_HIDDEN);
    else if (obj == ds->min)
        lv_obj_clear_flag(ds->image_10, LV_OBJ_FLAG_HIDDEN);
    else if (obj == ds->start)
        lv_obj_clear_flag(ds->startline, LV_OBJ_FLAG_HIDDEN);
}
// delayset 点开始：与实时时间比较校正（已过 → 明天），用确定时日期一次性算好
// 预约目标 g_delay_target 并保存（此后 delaycooking 重建不再重算，避免跨天漂移）
void on_delayset_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    rtc_time_t now;
    if (rtc_get_time(&now) == 0) {
        if (delay_hour < 24 &&
            (delay_hour < now.hour ||
             (delay_hour == now.hour && delay_min < now.min)))
            delay_hour += 24;                       /* 实时已过 → 明天 */
        int64_t base = (int64_t)rtc_days_from_epoch(now.year, now.month, now.day) * 86400000LL;
        g_delay_target = base
                       + (delay_hour >= 24 ? 86400000LL : 0)
                       + (int64_t)(delay_hour % 24) * 3600000LL
                       + (int64_t)delay_min * 60000LL;
    } else {
        g_delay_target = -1;                        /* RTC 失败哨兵，到点检测不触发 */
    }
    delay_on = 1;
    /* 互斥：开延时关预热（回 set 页重建时显示关） */
    preheat_on = 0;
    if (g_delay_source_page == PAGE_DESCRIPTIONMENU)
        jump_to_delaycooking();   /* 六感:直接进预约运行页 */
    else
        page_pop();               /* 其他模式:回 set 页 */
}
// 设置 updown set 页 ondelay 按钮文字（"今天19:00开始"，taiwan 字体滚动显示）
void updown_set_apply_delay_label(updown_bbq_set_t *set)
{
    if (!set) return;
    lv_obj_t *lbl = lv_obj_get_child(set->delay_on_button, 0);
    if (lbl) {
        lv_obj_set_style_text_font(lbl, &c_taiwanpearl_regular_24,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_label_set_text_fmt(lbl, tr("%s%02d:%02d开始"),
                              delay_hour >= 24 ? tr("明天") : tr("今天"),
                              delay_hour % 24, delay_min);
        lv_obj_set_width(lbl, 110);
    }
}
// 延时开关：关态点击进入 delayset 设置时间；开态点击直接关闭
void on_delay_toggle(lv_event_t *e)
{
    updown_bbq_set_t *set = updown_bbq_set_get(&ui_manager);
    if (!set) return;
    lv_obj_t *tgt = lv_event_get_target(e);
    if (tgt == set->delay_button) {
        jump_to_delayset();
        return;
    }
    /* delay_on_button：直接关闭 */
    delay_on = 0;
    lv_obj_add_flag(set->delay_on_button, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(set->delay_button, LV_OBJ_FLAG_HIDDEN);
    lv_group_focus_obj(set->delay_button);
}
// 按预约来源页设置 cook_mode（delay 流程所有显示/分发依赖 cook_mode）
static void apply_delay_cook_mode(page_id_t src)
{
    switch (src) {
    case PAGE_UPDOWN_BBQ_SET: case PAGE_UPDOWN_BBQ_SET_PROBE:
        g_send.cook_mode = MODE_UPDOWN_BBQ; break;
    case PAGE_TOP_BBQ_SET:        g_send.cook_mode = MODE_TOP_BBQ; break;
    case PAGE_BOTTOM_BBQ_SET: case PAGE_BOTTOM_BBQ_SET_PROBE:
        g_send.cook_mode = MODE_BOTTOM_BBQ; break;
    case PAGE_HOT_BBQ_SET: case PAGE_HOT_BBQ_SET_PROBE:
        g_send.cook_mode = MODE_HOT_BBQ; break;
    case PAGE_HOTWIND_BBQ_SET:    g_send.cook_mode = MODE_HOTWIND_BBQ; break;
    case PAGE_SAVE_BBQ_SET:       g_send.cook_mode = MODE_SAVE_BBQ; break;
    case PAGE_CENTRAL_BBQ_SET:    g_send.cook_mode = MODE_CENTRAL_BBQ; break;
    case PAGE_WINDCHANGE_BBQ_SET: g_send.cook_mode = MODE_WINDCHANGE_BBQ; break;
    case PAGE_AIR_SET:            g_send.cook_mode = MODE_AIR; break;
    case PAGE_PIZZA_2_SET:        g_send.cook_mode = MODE_PIZZA_2; break;
    case PAGE_SLOWCOOK_SET: case PAGE_SLOWCOOK_SET_PROBE:
        g_send.cook_mode = MODE_SLOWCOOK; break;
    case PAGE_UNFROZEN_SET:       g_send.cook_mode = MODE_UNFROZEN; break;
    case PAGE_RISING_SET:         g_send.cook_mode = MODE_RISING; break;
    case PAGE_CORN_SET:           g_send.cook_mode = MODE_CORN; break;
    case PAGE_HEATCONTAIN_SET:    g_send.cook_mode = MODE_HEATCONTAIN; break;
    case PAGE_COOKIE_SET: case PAGE_WEST_SET: case PAGE_PIZZA_SET: case PAGE_MENU_COOK_SET:
        g_send.cook_mode = MODE_COOK4; break;
    case PAGE_LASAGNA_SET: case PAGE_STRUDEL_SET: case PAGE_BREAD_SET:
    case PAGE_PIZZA3_SET: case PAGE_CHIP_SET: case PAGE_CUSTOM_SET:
        g_send.cook_mode = MODE_FROZEN_BAKE; break;
    case PAGE_DESCRIPTIONMENU:
        /* 六感:探针菜热风对流；烤鸡翅类按菜谱(热风对流/空气炸)；其余保持前面模式 */
        if (six_chick_is_probe())
            g_send.cook_mode = MODE_WINDCHANGE_BBQ;
        else if (six_chick_is_kind())
            g_send.cook_mode = six_chick_mode();
        else if (six_chick_is_seafood())
            g_send.cook_mode = six_chick_mode();   /* 烤海鲜:按菜谱(热风对流/上下烧烤) */
        else
            g_send.cook_mode = MODE_UPDOWN_BBQ;
        break;
    default: break;
    }
}
void jump_to_delayset(void)
{
    edit_clear();
    /* 记录预约来源页（cook4 多组共用 MODE_COOK4，靠来源页区分） */
    if (depth > 0)
        g_delay_source_page = page_stack[depth - 1];
    apply_delay_cook_mode(g_delay_source_page);
    /* 记录进入时刻（今天/明天切换基准）+ 默认当前时间+5 分钟（当前 23:59 → 明天 00:04） */
    rtc_time_t now;
    if (rtc_get_time(&now) == 0) {
        delayset_enter_hour = now.hour;
        delayset_enter_min = now.min;
        delay_hour = now.hour;
        delay_min = now.min + 5;
        if (delay_min > 59) { delay_min -= 60; delay_hour++; }
        if (delay_hour > 23) delay_hour = 0;   /* 23:59+5min → 00:04,内部回 0,标签自动"明天" */
    }
    page_push(PAGE_DELAYSET);
    lv_obj_clean(lv_scr_act());
    delayset_create(&ui_manager);

    delayset_t *ds = delayset_get(&ui_manager);
    if (ds) {
        if (g_delay_source_page == PAGE_DESCRIPTIONMENU)
            lv_label_set_text(ds->name, six_current_name());   /* 六感菜名（烤全鸡走独立名） */
        lv_obj_t *btns[] = { ds->hour, ds->min, ds->start };
        if (g_delayset) lv_group_del(g_delayset);
        g_delayset = group_create_for_page(btns, 3);
        clear_focus_states(btns, 3);

        lv_obj_add_flag(ds->start, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(ds->start, on_delayset_start_click,
                            LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(ds->hour, on_delayset_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(ds->min, on_delayset_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(ds->start, on_delayset_focus, LV_EVENT_FOCUSED, NULL);

        lv_group_focus_obj(ds->start);

        /* 六感已设"面包卷",其他模式显示模式名 */
        if (g_delay_source_page != PAGE_DESCRIPTIONMENU)
            lv_label_set_text(ds->name, mode_display_name());
        delayset_refresh_display(ds);
    }
    current_group = g_delayset;

    lang_scr_load_anim(delayset_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] jump: updown_bbq_set -> delayset\n");
}
// delaycooking 取消：取消延时 → 跳 stop_back 确认（预约中态）
void on_delaycooking_cancel_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    g_delay_cancel_to_stop_back = 1;
    g_delay_cancel_btn = 1;
    delay_cancel_to_stop_back();
}
// 按 g_send.cook_mode 返回模式显示名（无 "| |"）
const char *mode_display_name(void)
{
    switch (g_send.cook_mode) {
    case MODE_UPDOWN_BBQ:     return tr("上下烧烤");
    case MODE_TOP_BBQ:        return tr("顶部烧烤");
    case MODE_BOTTOM_BBQ:     return tr("底部烧烤");
    case MODE_HOT_BBQ:        return tr("热风烧烤");
    case MODE_HOTWIND_BBQ:    return tr("热风");
    case MODE_SAVE_BBQ:       return tr("节能热风");
    case MODE_CENTRAL_BBQ:    return tr("集中烧烤");
    case MODE_WINDCHANGE_BBQ: return tr("热风对流");
    case MODE_AIR:            return tr("空气炸");
    case MODE_PIZZA_2:        return tr("披萨");
    case MODE_SLOWCOOK:       return tr("慢煮");
    case MODE_UNFROZEN:       return tr("解冻");
    case MODE_RISING:         return tr("发酵");
    case MODE_CORN:           return tr("干果");
    case MODE_HEATCONTAIN:    return tr("保温");
    case MODE_FROZEN_BAKE:
        switch (g_delay_source_page) {
        case PAGE_LASAGNA_SET: return tr("千层面");
        case PAGE_STRUDEL_SET: return tr("果馅卷");
        case PAGE_BREAD_SET:   return tr("面包");
        case PAGE_PIZZA3_SET:  return tr("披萨");
        case PAGE_CHIP_SET:    return tr("薯条");
        case PAGE_CUSTOM_SET:  return tr("自定义");
        default:               return tr("冷冻烘焙");
        }
    case MODE_COOK4:
        switch (g_delay_source_page) {
        case PAGE_COOKIE_SET:    return tr("曲奇");
        case PAGE_WEST_SET:      return tr("西式");
        case PAGE_PIZZA_SET:     return tr("披萨");
        case PAGE_MENU_COOK_SET: return tr("菜单");
        default:                 return tr("上下烧烤");
        }
    default:                  return tr("上下烧烤");
    }
}
// 按 g_send.cook_mode 设置模式图标（与预热页映射一致）
void mode_apply_icon(lv_obj_t *icon)
{
    if (!icon) return;
    switch (g_send.cook_mode) {
    case MODE_UPDOWN_BBQ:     lv_img_set_src(icon, LVGL_IMAGE_PATH(updown_img.png)); break;
    case MODE_TOP_BBQ:        lv_img_set_src(icon, LVGL_IMAGE_PATH(topicon.png)); break;
    case MODE_BOTTOM_BBQ:     lv_img_set_src(icon, LVGL_IMAGE_PATH(dwbbqicon.png)); break;
    case MODE_HOT_BBQ:        lv_img_set_src(icon, LVGL_IMAGE_PATH(hotbbqicon.png)); break;
    case MODE_HOTWIND_BBQ:    lv_img_set_src(icon, LVGL_IMAGE_PATH(hotwindicon.png)); break;
    case MODE_SAVE_BBQ:       lv_img_set_src(icon, LVGL_IMAGE_PATH(savewindicon.png)); break;
    case MODE_CENTRAL_BBQ:    lv_img_set_src(icon, LVGL_IMAGE_PATH(centralbbqicon.png)); break;
    case MODE_WINDCHANGE_BBQ: lv_img_set_src(icon, LVGL_IMAGE_PATH(windchange.png)); break;
    case MODE_AIR:            lv_img_set_src(icon, LVGL_IMAGE_PATH(airicon.png)); break;
    case MODE_PIZZA_2:        lv_img_set_src(icon, LVGL_IMAGE_PATH(pizza2icon.png)); break;
    case MODE_SLOWCOOK:       lv_img_set_src(icon, LVGL_IMAGE_PATH(slowcookicon.png)); break;
    case MODE_UNFROZEN:       lv_img_set_src(icon, LVGL_IMAGE_PATH(unfrozenicon.png)); break;
    case MODE_RISING:         lv_img_set_src(icon, LVGL_IMAGE_PATH(risingicon.png)); break;
    case MODE_CORN:           lv_img_set_src(icon, LVGL_IMAGE_PATH(cornicon.png)); break;
    case MODE_HEATCONTAIN:    lv_img_set_src(icon, LVGL_IMAGE_PATH(heatcontainicon.png)); break;
    case MODE_FROZEN_BAKE:
        switch (g_delay_source_page) {
        case PAGE_LASAGNA_SET: lv_img_set_src(icon, LVGL_IMAGE_PATH(lasagnaicon.png)); break;
        case PAGE_STRUDEL_SET: lv_img_set_src(icon, LVGL_IMAGE_PATH(strudelicon.png)); break;
        case PAGE_BREAD_SET:   lv_img_set_src(icon, LVGL_IMAGE_PATH(breadicon_1.png)); break;
        case PAGE_PIZZA3_SET:  lv_img_set_src(icon, LVGL_IMAGE_PATH(pizza3icon.png)); break;
        case PAGE_CHIP_SET:    lv_img_set_src(icon, LVGL_IMAGE_PATH(chipicon.png)); break;
        case PAGE_CUSTOM_SET:  lv_img_set_src(icon, LVGL_IMAGE_PATH(customicon.png)); break;
        default:               lv_img_set_src(icon, LVGL_IMAGE_PATH(frozencookfr.png)); break;
        }
        break;
    case MODE_COOK4:
        switch (g_delay_source_page) {
        case PAGE_COOKIE_SET:    lv_img_set_src(icon, LVGL_IMAGE_PATH(cookieicon.png)); break;
        case PAGE_WEST_SET:      lv_img_set_src(icon, LVGL_IMAGE_PATH(westicon.png)); break;
        case PAGE_PIZZA_SET:     lv_img_set_src(icon, LVGL_IMAGE_PATH(pizzaicon.png)); break;
        case PAGE_MENU_COOK_SET: lv_img_set_src(icon, LVGL_IMAGE_PATH(menuicon.png)); break;
        default:                 lv_img_set_src(icon, LVGL_IMAGE_PATH(updown_img.png)); break;
        }
        break;
    default:                  lv_img_set_src(icon, LVGL_IMAGE_PATH(updown_img.png)); break;
    }
}
// 14 模式 set 页 ondelay 按钮（背景 switchbg30/80 + "今天19:00开始"滚动文字）
void mode_set_apply_delay_label(lv_obj_t *ondelay_btn)
{
    if (!ondelay_btn) return;
    lv_obj_set_style_bg_img_src(ondelay_btn, LVGL_IMAGE_PATH(switchbg30.png),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(ondelay_btn, LVGL_IMAGE_PATH(switchbg80.png),
                                LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_t *lbl = lv_obj_get_child(ondelay_btn, 0);
    if (lbl) {
        lv_obj_set_style_text_font(lbl, &c_taiwanpearl_regular_24,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_label_set_text_fmt(lbl, tr("%s%02d:%02d开始"),
                              delay_hour >= 24 ? tr("明天") : tr("今天"),
                              delay_hour % 24, delay_min);
        lv_obj_set_width(lbl, 110);
    }
}
// 延时预约到点：按 cook_mode 进入对应模式烹饪（探针插入走探针烹饪）
void delay_start_cook(void)
{
    if (g_delay_source_page == PAGE_DESCRIPTIONMENU) {
        if (six_chick_is_probe())
            jump_to_chick_cooking();   /* 探针菜(烤全鸡/烤全鸭):到点直接进烹饪页（探针驱动） */
        else
            jump_to_six_cooking();     /* 六感:到点进入六感烹饪 */
        return;
    }
    if (is_probe_inserted()) {
        switch (g_send.cook_mode) {
        case MODE_UPDOWN_BBQ: jump_to_updown_bbq_cooking_probe(); return;
        case MODE_HOT_BBQ:    jump_to_hot_bbq_cooking_probe(); return;
        case MODE_BOTTOM_BBQ: jump_to_bottom_bbq_cooking_probe(); return;
        case MODE_SLOWCOOK:   jump_to_slowcook_cooking_probe(); return;
        default: break; /* 非探针模式：探针插入不影响，按普通分发 */
        }
    }
    if (g_send.cook_mode == MODE_FROZEN_BAKE) {
        /* 冷冻食谱六组共用 MODE_FROZEN_BAKE：按预约来源页分发 */
        switch (g_delay_source_page) {
        case PAGE_LASAGNA_SET: jump_to_lasagna_cooking(); break;
        case PAGE_STRUDEL_SET: jump_to_strudel_cooking(); break;
        case PAGE_BREAD_SET:   jump_to_bread_cooking(); break;
        case PAGE_PIZZA3_SET:  jump_to_pizza3_cooking(); break;
        case PAGE_CHIP_SET:    jump_to_chip_cooking(); break;
        case PAGE_CUSTOM_SET:  jump_to_custom_cooking(); break;
        default:               jump_to_lasagna_cooking(); break;
        }
    } else if (g_send.cook_mode == MODE_COOK4) {
        /* cook4 四组共用 MODE_COOK4：按预约来源页分发 */
        switch (g_delay_source_page) {
        case PAGE_COOKIE_SET: jump_to_cookie_cooking(); break;
        case PAGE_WEST_SET:   jump_to_west_cooking(); break;
        case PAGE_PIZZA_SET:  jump_to_pizza_cooking(); break;
        case PAGE_MENU_COOK_SET: jump_to_menu_cooking(); break;
        default:              jump_to_cookie_cooking(); break;
        }
    } else {
        switch (g_send.cook_mode) {
        case MODE_UPDOWN_BBQ:     jump_to_updown_bbq_cooking(); break;
        case MODE_TOP_BBQ:        jump_to_top_bbq_cooking(); break;
        case MODE_BOTTOM_BBQ:     jump_to_bottom_bbq_cooking(); break;
        case MODE_HOT_BBQ:        jump_to_hot_bbq_cooking(); break;
        case MODE_HOTWIND_BBQ:    jump_to_hotwind_bbq_cooking(); break;
        case MODE_SAVE_BBQ:       jump_to_save_bbq_cooking(); break;
        case MODE_CENTRAL_BBQ:    jump_to_central_bbq_cooking(); break;
        case MODE_WINDCHANGE_BBQ: jump_to_windchange_bbq_cooking(); break;
        case MODE_AIR:            jump_to_air_cooking(); break;
        case MODE_PIZZA_2:        jump_to_pizza_2_cooking(); break;
        case MODE_SLOWCOOK:       jump_to_slowcook_cooking(); break;
        case MODE_UNFROZEN:       jump_to_unfrozen_cooking(); break;
        case MODE_RISING:         jump_to_rising_cooking(); break;
        case MODE_CORN:           jump_to_corn_cooking(); break;
        case MODE_HEATCONTAIN:    jump_to_heatcontain_cooking(); break;
        default:                  jump_to_updown_bbq_cooking(); break;
        }
    }
}
// delaycooking 取消/返回：按 cook_mode 跳对应模式 stop_back
void delay_cancel_to_stop_back(void)
{
    if (g_delay_source_page == PAGE_DESCRIPTIONMENU) {
        jump_to_updown_bbq_stop_back();   /* 六感:复用 updown stop_back 页 */
        return;
    }
    if (is_probe_inserted()) {
        switch (g_send.cook_mode) {
        case MODE_UPDOWN_BBQ: jump_to_updown_bbq_stop_back_probe(); return;
        case MODE_HOT_BBQ:    jump_to_hot_bbq_stop_back_probe(); return;
        case MODE_BOTTOM_BBQ: jump_to_bottom_bbq_stop_back_probe(); return;
        case MODE_SLOWCOOK:   jump_to_slowcook_stop_back_probe(); return;
        default: break; /* 非探针模式：探针插入不影响，按普通分发 */
        }
    }
    if (g_send.cook_mode == MODE_FROZEN_BAKE) {
        /* 冷冻食谱六组共用 MODE_FROZEN_BAKE：按预约来源页分发 */
        switch (g_delay_source_page) {
        case PAGE_LASAGNA_SET: jump_to_lasagna_stop_back(); break;
        case PAGE_STRUDEL_SET: jump_to_strudel_stop_back(); break;
        case PAGE_BREAD_SET:   jump_to_bread_stop_back(); break;
        case PAGE_PIZZA3_SET:  jump_to_pizza3_stop_back(); break;
        case PAGE_CHIP_SET:    jump_to_chip_stop_back(); break;
        case PAGE_CUSTOM_SET:  jump_to_custom_stop_back(); break;
        default:               jump_to_lasagna_stop_back(); break;
        }
    } else if (g_send.cook_mode == MODE_COOK4) {
        switch (g_delay_source_page) {
        case PAGE_COOKIE_SET:    jump_to_cookie_stop_back(); break;
        case PAGE_WEST_SET:      jump_to_west_stop_back(); break;
        case PAGE_PIZZA_SET:     jump_to_pizza_stop_back(); break;
        case PAGE_MENU_COOK_SET: jump_to_menu_stop_back(); break;
        default:                 jump_to_cookie_stop_back(); break;
        }
    } else {
        switch (g_send.cook_mode) {
        case MODE_UPDOWN_BBQ:     jump_to_updown_bbq_stop_back(); break;
        case MODE_TOP_BBQ:        jump_to_top_bbq_stop_back(); break;
        case MODE_BOTTOM_BBQ:     jump_to_bottom_bbq_stop_back(); break;
        case MODE_HOT_BBQ:        jump_to_hot_bbq_stop_back(); break;
        case MODE_HOTWIND_BBQ:    jump_to_hotwind_bbq_stop_back(); break;
        case MODE_SAVE_BBQ:       jump_to_save_bbq_stop_back(); break;
        case MODE_CENTRAL_BBQ:    jump_to_central_bbq_stop_back(); break;
        case MODE_WINDCHANGE_BBQ: jump_to_windchange_bbq_stop_back(); break;
        case MODE_AIR:            jump_to_air_stop_back(); break;
        case MODE_PIZZA_2:        jump_to_pizza_2_stop_back(); break;
        case MODE_SLOWCOOK:       jump_to_slowcook_stop_back(); break;
        case MODE_UNFROZEN:       jump_to_unfrozen_stop_back(); break;
        case MODE_RISING:         jump_to_rising_stop_back(); break;
        case MODE_CORN:           jump_to_corn_stop_back(); break;
        case MODE_HEATCONTAIN:    jump_to_heatcontain_stop_back(); break;
        default:                  jump_to_updown_bbq_stop_back(); break;
        }
    }
}
// 重建 delaycooking 预约页（stop_back 返回场景复用，不重复压栈）
void rebuild_delaycooking(void)
{
    g_on_stop_back = 0;
    g_stop_back_complete = NULL;
    apply_delay_cook_mode(g_delay_source_page);
    lv_obj_clean(lv_scr_act());
    delaycooking_create(&ui_manager);

    delaycooking_t *dc = delaycooking_get(&ui_manager);
    if (dc) {
        lv_obj_t *btns[] = { dc->cancel };
        if (g_delaycooking) lv_group_del(g_delaycooking);
        g_delaycooking = group_create_for_page(btns, 1);
        clear_focus_states(btns, 1);
        lv_group_focus_obj(dc->cancel);

        lv_obj_add_event_cb(dc->cancel, on_delaycooking_cancel_click,
                            LV_EVENT_CLICKED, NULL);

        if (g_delay_source_page == PAGE_DESCRIPTIONMENU) {
            /* 六感:status 显示菜+信息,icon 用 sixicon(与运行页一致) */
            if (six_chick_is_probe())
                lv_label_set_text_fmt(dc->status, tr("| %s | %s |"),
                                      six_chick_name(), six_chick_degree_text());   /* 探针菜:菜名+烤色程度 */
            else if (six_chick_is_degree_time()) {
                int d = toastcolor_degree_value();
                if (d < 1 || d > 3) d = 2;
                const char *dt = (d == 1) ? "浅色" : (d == 3) ? "深色" : "中等色";
                lv_label_set_text_fmt(dc->status, tr("| %s | %s | %d分钟 |"),
                                      six_chick_name(), dt, six_chick_degree_min(d));   /* 烤羊肉串:菜名+程度+时间 */
            } else if (six_chick_is_kind()) {
                int w = toastcolor_weight_value();
                if (w < 0) w = 800;
                lv_label_set_text_fmt(dc->status, tr("| %s | %dg | %d分钟 |"),
                                      six_chick_name(), w, six_chick_cook_min(w));   /* 烤鸡翅类:菜名+克重+时间 */
            } else if (six_chick_is_seafood()) {
                const seafood_dish_t *sd = seafood_dish_cfg();
                lv_label_set_text_fmt(dc->status, tr("| %s | %d分钟"), six_chick_name(),
                                      sd ? sd->cook_min : 18);   /* 烤海鲜:菜名+固定时间 */
            } else
                lv_label_set_text_fmt(dc->status, tr("| %s | %d分钟"), six_bread_name(), six_bread_cook_min());
            lv_img_set_src(dc->icon, LVGL_IMAGE_PATH(sixicon.png));
            lv_obj_set_pos(dc->icon, 163 + lang_dyn_dx(), 161 + lang_dyn_dy());
        } else if (g_delay_source_page == PAGE_UPDOWN_BBQ_SET_PROBE ||
            g_delay_source_page == PAGE_HOT_BBQ_SET_PROBE ||
            g_delay_source_page == PAGE_BOTTOM_BBQ_SET_PROBE ||
            g_delay_source_page == PAGE_SLOWCOOK_SET_PROBE) {
            /* 探针来源：与探针烹饪页一致的格式（温度 + 探针目标温） */
            lv_label_set_text_fmt(dc->status, tr("| %s | %d℃ | %d℃"),
                                  mode_display_name(), set_temp, probe_target_temp);
        } else if (g_send.cook_mode == MODE_UPDOWN_BBQ)
            set_status_label_min(dc->status, set_temp_up, set_temp_down, set_hour, set_min);
        else {
            if (set_hour == 0)
                lv_label_set_text_fmt(dc->status, tr("| %s | %d℃ | %02d分钟"),
                                      mode_display_name(), set_temp, set_min);
            else
                lv_label_set_text_fmt(dc->status, tr("| %s | %d℃ | %d小时%02d分钟"),
                                      mode_display_name(), set_temp, set_hour, set_min);
        }
        if (g_delay_source_page != PAGE_DESCRIPTIONMENU)
            mode_apply_icon(dc->icon);   /* 六感已单独设 sixicon */
        lv_label_set_text(dc->label_14, tr("预约中..."));
        lv_label_set_text_fmt(dc->tip2, tr("%s%02d:%02d"),
                              delay_hour >= 24 ? tr("明天") : tr("今天"),
                              delay_hour % 24, delay_min);
    }
    current_group = g_delaycooking;

    /* 预约目标 g_delay_target 已在 on_delayset_start_click 用确定时日期算好并保存，
       此处不再重算（避免 delaycooking 重建/跨 0 点导致目标漂移一天） */
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    g_send.iface_status = IFACE_DELAY_RESERVE;
    g_send.remaining_ms = 0;
    g_send.cook_flag = 0;

    lang_scr_load_anim(delaycooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav] back to delaycooking\n");
}
void jump_to_delaycooking(void)
{
    edit_clear();
    page_push(PAGE_DELAYCOOKING);
    rebuild_delaycooking();
    printf("[nav] jump: updown_bbq_set -> delaycooking\n");
}
void on_contain_toggle(lv_event_t *e)
{
    updown_bbq_set_t *set = updown_bbq_set_get(&ui_manager);
    if (!set) return;
    contain_on = !contain_on;
    if (contain_on) {
        lv_obj_add_flag(set->contain_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->contain_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->contain_on_button);
    } else {
        lv_obj_add_flag(set->contain_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->contain_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->contain_button);
    }
}
void anim_bar_set_value(void *obj, int32_t v)
{
    lv_bar_set_value((lv_obj_t *)obj, v, LV_ANIM_OFF);
}
void on_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr)) {
        if (delay_on)
            jump_to_delaycooking();
        else if (preheat_on)
            jump_to_preheat_cooking();
        else
            jump_to_updown_bbq_cooking();
    }
}
void on_color_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_color_cookoing();
}
// 为 major_menu 的三个按钮绑定 LV_EVENT_CLICKED 回调
// 注意：每次 groups_create 后必须调用 bind_events，新按钮需要重新绑定
void bind_events(void)
{
    major_menu_t *major = major_menu_get(&ui_manager);
    if (!major) {
        printf("[nav] bind_events: major_menu is NULL\n");
        return;
    }

    if (major->cook_button)
        lv_obj_add_event_cb(major->cook_button, on_major_cook_click,
                            LV_EVENT_CLICKED, NULL);
    else
        printf("[nav] cannot bind cook_button: NULL\n");

    if (major->special_button)
        lv_obj_add_event_cb(major->special_button, on_major_special_click,
                            LV_EVENT_CLICKED, NULL);
    else
        printf("[nav] cannot bind special_button: NULL\n");

    if (major->cook4_button)
        lv_obj_add_event_cb(major->cook4_button, on_major_cook4_click,
                            LV_EVENT_CLICKED, NULL);
    else
        printf("[nav] cannot bind cook4_button: NULL\n");
}

