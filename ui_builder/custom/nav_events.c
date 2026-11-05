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
        lv_obj_t *btns[] = { c4->cookie, c4->piza, c4->west, c4->menu };   /* 组序=视觉序(西式塔/披萨换位,2026-11-03) */
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
        lv_obj_t *btns[] = { c4->cookie, c4->piza, c4->west, c4->menu };   /* 组序=视觉序(西式塔/披萨换位,2026-11-03) */
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
            if (edit_disp(f, *f->value) < 100)   /* 按显示值位数选线:℉ 下 80℃ 显 176 三位走长线 */
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

/* 刚进入 delayset 的定向标志(2026-10-21):进入后首次旋转(顺/逆皆)把焦点定到
   小时并直接进编辑;BACK 退出编辑后的浏览态不在此列,旋转按 时/分/start 整组循环 */
uint8_t nav_delayset_fresh = 0;

// 12 小时制后缀标签缓存(随 delayset 页实例重建,防悬空)
static lv_obj_t *s_delay_ampm = NULL;
static void *s_delay_ampm_host = NULL;

/* 今天/明天实时判定(2026-11-04 用户:改实时,不再用进入时刻做冻结基准):
   所选时刻(时:分)早于当前 RTC → 明天;相等按今天(点开始即当场执行,与 start 校正同口径)。
   delay_hour>=24 是点开始后"已确认明天"的归一值,恒明天 */
static uint8_t delayset_pick_tomorrow(void)
{
    rtc_time_t now;
    if (delay_hour >= 24) return 1;
    if (rtc_get_time(&now) != 0) return 0;
    return (delay_hour < now.hour ||
            (delay_hour == now.hour && delay_min < now.min)) ? 1 : 0;
}

/* ===================== 预约时间窗(2026-11-05 用户) =====================
 * 时/分是两组独立循环(时 0-23、分 0-59 各自回绕,分不进位到时——用户明确不做
 * 时/分联动),但两端都按"以当前 RTC 为起点的 24h 窗口"拦住:
 *   逆向最多到"现在"(offset 0),顺向最多到"明天此刻-1分"(offset 1439)。
 * 分字段只在正好这两个端点挡;时字段按方向判——绕回窗口另一端(=时间反而变早/
 * 变晚)就挡,所以朝两端走都会停住。今天/明天标签仍由 delayset_pick_tomorrow
 * 实时比较得出。 */
static int delayset_offset_of(int h, int m)
{
    rtc_time_t now;
    int off;
    if (rtc_get_time(&now) != 0) return -1;
    off = (h * 60 + m) - (now.hour * 60 + now.min);
    if (off < 0) off += 1440;
    return off;
}

static int delayset_window_offset(void)
{
    return delayset_offset_of(delay_hour % 24, delay_min);
}

/* 分字段 ±1:0-59 独立回绕(不进位到时);仅窗口两端挡一下。
   返回 0=不动(调用方发无效音),1=已更新 */
int delayset_step_minute(int delta)
{
    int off = delayset_window_offset();

    if (off == 0 && delta < 0) return 0;        /* 已到"现在":不能再往早调 */
    if (off == 1439 && delta > 0) return 0;     /* 已到"明天此刻-1分":不能再往晚调 */
    delay_min += delta;
    if (delay_min > 59) delay_min = 0;
    if (delay_min < 0) delay_min = 59;
    return 1;
}

/* 时字段 ±1:0-23 独立循环,分钟不动;绕回窗口另一端(=偏移反而变小/变大)即拦。
   返回 0=不动(调用方发无效音),1=已更新 */
int delayset_step_hour(int delta)
{
    int cur = delayset_window_offset();
    int nh  = (delay_hour + delta) % 24;
    int no;

    if (nh < 0) nh += 24;
    no = delayset_offset_of(nh, delay_min);
    if (cur < 0 || no < 0) {          /* RTC 读失败:退回无界循环,不影响可调性 */
        delay_hour = nh;
        return 1;
    }
    if (delta > 0 && no <= cur) return 0;   /* 顺时针绕回窗口起点:实际变早 → 拦 */
    if (delta < 0 && no >= cur) return 0;   /* 逆时针绕回窗口终点:实际变晚 → 拦 */
    delay_hour = nh;
    return 1;
}

// 刷新 delayset 页的时间显示（hour 显示 0-23，min 0-59；今天/明天按实时时钟比较）
void delayset_refresh_display(delayset_t *ds)
{
    if (!ds) return;
    uint8_t h24 = (uint8_t)(delay_hour % 24);
    if (SET_Data.Set_TimeType == 1) {
        int h12 = h24 % 12;
        if (h12 == 0) h12 = 12;
        lv_label_set_text_fmt(ds->hour, "%02d", h12);
    } else {
        lv_label_set_text_fmt(ds->hour, "%02d", delay_hour);
    }
    lv_label_set_text_fmt(ds->min, "%02d", delay_min);
    lv_label_set_text(ds->day, delayset_pick_tomorrow() ? tr("明天") : tr("今天"));

    /* 12 小时制:min 右侧"上午/下午"后缀。
       英文:后缀由 delayset_lang_tune 挂在 flex 时间组尾(第 4 子),这里只翻文字(跨 12 点 AM/PM);
       中文:绝对定位+基线对齐+时间组 translate 左移半后缀宽(整体居中) */
    if (SET_Data.Set_TimeType == 1 && ds->obj && is_english()) {
        lv_obj_t *tc = lv_obj_get_parent(ds->hour);
        lv_obj_t *sfx = (tc && tc != ds->obj) ? lv_obj_get_child(tc, 3) : NULL;
        if (sfx)
            lv_label_set_text(sfx, tr(h24 < 12 ? "上午" : "下午"));
    } else if (SET_Data.Set_TimeType == 1 && ds->obj) {
        if (s_delay_ampm_host != (void *)ds->obj ||
            (s_delay_ampm && !lv_obj_is_valid(s_delay_ampm))) {
            s_delay_ampm_host = (void *)ds->obj;
            s_delay_ampm = NULL;
        }
        if (s_delay_ampm == NULL) {
            s_delay_ampm = lv_label_create(ds->obj);
            lv_obj_set_style_text_color(s_delay_ampm, lv_color_hex(0xffffff), 0);
        }
        const lv_font_t *amf = is_english() ? &c_aktivgroteskmedium_36 : &c_taiwanpearl_regular_36;
        const lv_font_t *big72 = is_english() ? &c_aktivgroteskmedium_72 : &c_taiwanpearl_regular_72;
        lv_obj_set_style_text_font(s_delay_ampm, amf, 0);
        lv_label_set_text(s_delay_ampm, tr(h24 < 12 ? "上午" : "下午"));
        int min_w = lv_txt_get_width("00", 2, big72, 0);
        const char *am_txt = lv_label_get_text(s_delay_ampm);
        int am_w = lv_txt_get_width(am_txt, (uint32_t)strlen(am_txt), amf, 0);
        int gap = 3;
        int dx = (am_w + gap) / 2;   /* 时间组(hour+冒号+min)左移半后缀宽:"时间+后缀"整体居中 */
        lv_obj_set_style_translate_x(ds->hour, -dx, 0);
        lv_obj_set_style_translate_x(ds->label_10, -dx, 0);
        lv_obj_set_style_translate_x(ds->min, -dx, 0);
        lv_obj_set_style_translate_x(ds->image_9, -dx, 0);
        lv_obj_set_style_translate_x(ds->image_10, -dx, 0);
        /* 后缀与时间数字基线对齐(数字无降部,基线即数字下缘);min 标签顶 y=248 */
        int base_y = 248 + big72->line_height - big72->base_line;
        lv_obj_set_pos(s_delay_ampm, 651 + (111 + min_w) / 2 + gap - dx,
                       base_y - (amf->line_height - amf->base_line));
        lv_obj_clear_flag(s_delay_ampm, LV_OBJ_FLAG_HIDDEN);
    } else if (s_delay_ampm) {
        if (lv_obj_is_valid(s_delay_ampm))
            lv_obj_add_flag(s_delay_ampm, LV_OBJ_FLAG_HIDDEN);
        s_delay_ampm = NULL;   /* 失效即弃,防地址复用后误写悬空指针 */
        lv_obj_set_style_translate_x(ds->hour, 0, 0);
        lv_obj_set_style_translate_x(ds->label_10, 0, 0);
        lv_obj_set_style_translate_x(ds->min, 0, 0);
        lv_obj_set_style_translate_x(ds->image_9, 0, 0);
        lv_obj_set_style_translate_x(ds->image_10, 0, 0);
    }
}
/* 1s 常驻:delayset 在栈顶时按实时时钟刷新"今天/明天"——调节中时间走过所选时刻
   会自动翻;只在文案真变时重刷整页,避免每秒无谓重绘。
   定时器只建一个,页面反复重建不新增;页面已销毁时由栈顶守卫+对象有效性挡住 */
static lv_timer_t *s_delayset_live_timer = NULL;
static void delayset_live_tick_cb(lv_timer_t *t)
{
    (void)t;
    if (depth <= 0 || page_stack[depth - 1] != PAGE_DELAYSET) return;
    delayset_t *ds = delayset_get(&ui_manager);
    if (!ds || !ds->day || !lv_obj_is_valid(ds->day)) return;
    const char *want = delayset_pick_tomorrow() ? tr("明天") : tr("今天");
    const char *cur = lv_label_get_text(ds->day);
    if (!cur || strcmp(cur, want) != 0)
        delayset_refresh_display(ds);
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
        /* 与页面显示同一实时口径:已过 → 归一成明天(target 多加一天) */
        if (delay_hour < 24 && delayset_pick_tomorrow())
            delay_hour += 24;
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
// 设置 updown set 页 ondelay 按钮文字（"今天19:00开始"，taiwan 字体滚动显示；12 小时制带上午/下午）
void updown_set_apply_delay_label(updown_bbq_set_t *set)
{
    if (!set) return;
    lv_obj_t *lbl = lv_obj_get_child(set->delay_on_button, 0);
    if (lbl) {
        lv_obj_set_style_text_font(lbl, &c_taiwanpearl_regular_24,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_long_mode(lbl, LV_LABEL_LONG_SCROLL_CIRCULAR);
        if (SET_Data.Set_TimeType == 1) {
            int h12 = (delay_hour % 24) % 12;
            if (h12 == 0) h12 = 12;
            lv_label_set_text_fmt(lbl, tr("%s%s%02d:%02d开始"),
                                  delay_hour >= 24 ? tr("明天") : tr("今天"),
                                  tr(delay_hour % 24 < 12 ? "上午" : "下午"),
                                  h12, delay_min);
        } else {
            lv_label_set_text_fmt(lbl, tr("%s%02d:%02d开始"),
                                  delay_hour >= 24 ? tr("明天") : tr("今天"),
                                  delay_hour % 24, delay_min);
        }
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
        else if (six_chick_is_veg())
            g_send.cook_mode = six_chick_mode();   /* 蔬菜:按菜谱(热风对流/上下烧烤) */
        else if (six_chick_is_pizza())
            g_send.cook_mode = six_chick_mode();   /* 披萨:披萨230 */
        else if (six_chick_is_2d())
            g_send.cook_mode = six_chick_mode();   /* 二维菜:热风对流(带皮土豆230/意面200) */
        else
            g_send.cook_mode = MODE_UPDOWN_BBQ;
        break;
    default: break;
    }
}
/* delayset 页面构建（进入与 pop 回退共用）：清屏+建页+焦点组+刷新+加载动画。
   单独抽出是因为 pop 揭示 delayset 时也要重建（原误建 set 页，英文 tune 摸悬空对象崩溃）；
   pop 路径不走 jump 的 delay_hour 默认值重置，用户已调的预约时间得以保留 */
void delayset_page_build(void)
{
    lv_obj_clean(lv_scr_act());
    s_delay_ampm = NULL;          /* 新页实例:12h 后缀缓存一律作废(malloc 地址复用防悬空) */
    s_delay_ampm_host = NULL;
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

        /* 数值字段呼吸组:焦点落 时/分/开始 时,字段+对应下划线整组呼吸显隐(与其它数值页一致) */
        lv_obj_t *g_hour[2] = { ds->hour, ds->image_9 };
        nav_blink_group_register(ds->hour, g_hour, 2);
        lv_obj_t *g_min[2] = { ds->min, ds->image_10 };
        nav_blink_group_register(ds->min, g_min, 2);
        lv_obj_t *g_start[2] = { ds->start, ds->startline };
        nav_blink_group_register(ds->start, g_start, 2);
        /* 两态会话:时/分是可编辑对象(浏览模式确认进编辑),start 是按钮不登记 */
        nav_editable_extra_register(ds->hour);
        nav_editable_extra_register(ds->min);

        lv_group_focus_obj(ds->start);
        nav_delayset_fresh = 1;   /* 进入定向:首次旋转直接进小时编辑(jump/pop 重建共用此构建) */

        /* 六感已设"面包卷",其他模式显示模式名 */
        if (g_delay_source_page != PAGE_DESCRIPTIONMENU)
            lv_label_set_text(ds->name, mode_display_name());
        delayset_refresh_display(ds);

        /* 实时判断:1s 常驻定时器(只建一个,页面反复重建不新增) */
        if (!s_delayset_live_timer)
            s_delayset_live_timer = lv_timer_create(delayset_live_tick_cb, 1000, NULL);
    }
    current_group = g_delayset;
    /* 延迟页入口定义:焦点在 start 的浏览态。族分类(edit_browse_reset)对本页走
       "其余页面进页即编辑"默认,会话带进来后首次旋转会落兜底分支绕错方向——
       此处显式退浏览:首次旋转(顺/逆)定向进小时编辑,BACK 退编辑后整组循环 */
    nav_edit_session_exit();

    lang_scr_load_anim(delayset_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
}

void jump_to_delayset(void)
{
    edit_clear();
    /* 记录预约来源页（cook4 多组共用 MODE_COOK4，靠来源页区分） */
    if (depth > 0)
        g_delay_source_page = page_stack[depth - 1];
    apply_delay_cook_mode(g_delay_source_page);
    /* 默认当前时间+5 分钟（当前 23:59 → 明天 00:04）；今天/明天由实时判断,不再记进入时刻 */
    rtc_time_t now;
    if (rtc_get_time(&now) == 0) {
        delay_hour = now.hour;
        delay_min = now.min + 5;
        if (delay_min > 59) { delay_min -= 60; delay_hour++; }
        if (delay_hour > 23) delay_hour = 0;   /* 23:59+5min → 00:04,内部回 0,标签自动"明天" */
    }
    page_push(PAGE_DELAYSET);
    delayset_page_build();
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
    /* 额外上色(2026-11-04):此前落到 default 显示成"上下烧烤"——上色是完成页的
       后续小任务,童锁第三行等按模式名显示的地方都该是"额外上色" */
    case MODE_EXTRA_COLOR:    return tr("额外上色");
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
        case PAGE_COOKIE_SET:    return tr("饼干");
        case PAGE_WEST_SET:      return tr("西式塔");
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
        default:               lv_img_set_src(icon, lang_img_src("frozencookfr.png")); break;   /* 繁體换 _tw 图 */
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
    nav_topflag_popups_yield();   /* 到点跳烹饪页让位:弹窗悬浮 layer_top,换屏前先收(2026-10-30) */
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
            if (six_chick_is_probe()) {
                if (six_chick_is_matdeg())
                    lv_label_set_text_fmt(dc->status, tr("| %s | %s | %s"),
                                          six_chick_name(), six_2d_deg_text(),
                                          six_2d_mat_text());   /* 牛肉/羊腿/羊排:菜名+烤色+成熟度(与烹饪页一致) */
                else
                    lv_label_set_text_fmt(dc->status, tr("| %s | %s"),
                                          six_chick_name(), six_chick_degree_text());   /* 探针菜:菜名+烤色程度 */
            }
            else if (six_chick_is_degree_time()) {
                int d = toastcolor_degree_value();
                if (d < 1 || d > 3) d = 2;
                const char *dt = (d == 1) ? tr("浅色") : (d == 3) ? tr("深色") : tr("中等色");   /* 逐项 tr,与 nav_stop.c 六感分支一致 */
                lv_label_set_text_fmt(dc->status, tr("| %s | %s | %d分钟"),
                                      six_chick_name(), dt, six_chick_degree_min(d));   /* 烤羊肉串:菜名+程度+时间 */
            } else if (six_chick_is_kind()) {
                int w = toastcolor_weight_value();
                if (w < 0) w = 800;
                lv_label_set_text_fmt(dc->status, tr("| %s | %d%s | %d分钟"),
                                      six_chick_name(), w, toastcolor_weight_unit_n(w), six_chick_cook_min(w));   /* 烤鸡翅类:菜名+单位+时间 */
            } else if (six_chick_is_seafood()) {
                const seafood_dish_t *sd = seafood_dish_cfg();
                lv_label_set_text_fmt(dc->status, tr("| %s | %d分钟"), six_chick_name(),
                                      sd ? sd->cook_min : 18);   /* 烤海鲜:菜名+固定时间 */

            } else if (six_chick_is_veg()) {
                const seafood_dish_t *sd = veg_fixed_cfg();
                lv_label_set_text_fmt(dc->status, tr("| %s | %d分钟"), six_chick_name(),
                                      sd ? sd->cook_min : 22);   /* 蔬菜:菜名+固定时间 */

            } else if (six_chick_is_pizza()) {
                const seafood_dish_t *sd = pizza_fixed_cfg();
                lv_label_set_text_fmt(dc->status, tr("| %s | %d分钟"), six_chick_name(),
                                      sd ? sd->cook_min : 23);   /* 披萨:菜名+固定时间 */

            } else if (six_chick_is_2d()) {
                lv_label_set_text_fmt(dc->status, tr("| %s | %dg | %s色"), six_chick_name(),
                                      six_2d_weight(), six_2d_deg_text());   /* 二维菜:菜名|克数|程度色 */
            } else
                lv_label_set_text_fmt(dc->status, tr("| %s | %d分钟"), six_bread_name(), six_bread_cook_min());
            lv_img_set_src(dc->icon, LVGL_IMAGE_PATH(sixicon.png));
            lv_obj_set_pos(dc->icon, 163, 161);
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
                lv_label_set_text_fmt(dc->status, tr("| %s | %d℃ | %d分钟"),
                                      mode_display_name(), set_temp, set_min);
            else
                nav_status_info_fmt(dc->status, tr("| %s | %d℃ | %d小时%d分钟"),
                                      mode_display_name(), set_temp, set_hour, set_min);
        }
        if (g_delay_source_page != PAGE_DESCRIPTIONMENU) {
            mode_apply_icon(dc->icon);   /* 六感已单独设 sixicon */
            /* 标语/状态行的 x 必须与同模式的烹饪页、延迟返回(取消)页一致:
             *   干果/保温/解冻/披萨四家按模式微调过 icon,它们的烹饪/停止/确认页
             *   统一走 nav_cook_icon_align(图标 115、标语=图标右缘+35、状态再+1),
             *   本页同步;
             *   其余模式图标同为 126 宽左右,各页沿用生成稿的 273/274,本页也保持
             *   生成稿坐标不动(英文版由 delaycooking_lang_tune 维持同一口径)。
             * 2026-11-05 用户:延迟中页与延迟返回页图标/状态栏必须同位置 */
            switch (g_send.cook_mode) {
            case MODE_CORN:
            case MODE_HEATCONTAIN:
            case MODE_UNFROZEN:
            case MODE_PIZZA_2:
                nav_cook_icon_align(dc->icon, dc->label_14, dc->status);
                break;
            default:
                lv_obj_set_pos(dc->icon, 115, 161);   /* 图标左缘与其它页同基准 */
                break;
            }
        }
        lv_label_set_text(dc->label_14, tr("预约中..."));
        if (SET_Data.Set_TimeType == 1) {
            int h12 = (delay_hour % 24) % 12;
            if (h12 == 0) h12 = 12;
            lv_label_set_text_fmt(dc->tip2, tr("%s%s%02d:%02d"),
                                  delay_hour >= 24 ? tr("明天") : tr("今天"),
                                  tr(delay_hour % 24 < 12 ? "上午" : "下午"),
                                  h12, delay_min);
        } else {
            lv_label_set_text_fmt(dc->tip2, tr("%s%02d:%02d"),
                                  delay_hour >= 24 ? tr("明天") : tr("今天"),
                                  delay_hour % 24, delay_min);
        }
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

