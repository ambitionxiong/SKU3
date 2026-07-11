#include "nav.h"

// === 页面 ID 枚举 ===
typedef enum {
    PAGE_WAITMENU_24,     // 等待界面（根页，开机首页）
    PAGE_MAJOR_MENU,
    PAGE_COOKMENU,
    PAGE_COOKMENU_4,
    PAGE_SPECIAL_MENU,
    PAGE_UPDOWN_BBQ_MENU,
    PAGE_UPDOWN_BBQ_SET,
    PAGE_UPDOWN_BBQ_COOKING,
} page_id_t;

// === 页面栈 ===
#define MAX_STACK 8
static page_id_t page_stack[MAX_STACK];  // 栈数组，stack[0]=根页，stack[depth-1]=当前页
static int depth = 0;                     // 栈高度，depth=1 时只有根页

// === 各页面焦点组（NULL=未创建）===
lv_group_t *g_major_menu;
lv_group_t *g_cookmenu;
lv_group_t *g_special_menu;
lv_group_t *g_cook_menu_tz;
lv_group_t *g_major_menu_tz;
lv_group_t *g_special_menu_tz;
lv_group_t *g_updown_bbq_menu;
lv_group_t *g_updown_bbq_set;
lv_group_t *g_updown_bbq_cooking;

lv_group_t *current_group = NULL;  // 当前活跃的焦点组，nav_handle_key 操作的就是这个组

// 前向声明（page_pop/groups_create 互相引用）
static void groups_create(void);
static lv_group_t *group_create_for_page(lv_obj_t **btns, int count);
static void bind_events(void);
static void on_cook_updown_click(lv_event_t *e);
static void on_updown_next_click(lv_event_t *e);
static void on_edit_focus(lv_event_t *e);
static void validate_constraints(void);
static void on_preheat_toggle(lv_event_t *e);
static void on_delay_toggle(lv_event_t *e);
static void on_contain_toggle(lv_event_t *e);
static void on_sure_click(lv_event_t *e);
static void cooking_timer_cb(lv_timer_t *timer);

// ==============================
// 可编辑字段注册表
// ==============================

#define MAX_EDIT_FIELDS 8
static edit_field_t edit_fields[MAX_EDIT_FIELDS];
static int edit_count = 0;

void edit_clear(void)
{
    edit_count = 0;
}

void edit_register(lv_obj_t *label, lv_obj_t *ind_s, lv_obj_t *ind_l,
                   int *value, int min, int max, int step, const char *fmt)
{
    if (edit_count < MAX_EDIT_FIELDS) {
        edit_fields[edit_count].label = label;
        edit_fields[edit_count].ind_short = ind_s;
        edit_fields[edit_count].ind_long = ind_l;
        edit_fields[edit_count].value = value;
        edit_fields[edit_count].min = min;
        edit_fields[edit_count].max = max;
        edit_fields[edit_count].step = step;
        edit_fields[edit_count].fmt = fmt;
        edit_count++;
    }
}

static edit_field_t *find_edit_field(lv_obj_t *obj)
{
    for (int i = 0; i < edit_count; i++)
        if (edit_fields[i].label == obj)
            return &edit_fields[i];
    return NULL;
}

static void adjust_value(edit_field_t *f, int delta)
{
    int new_val = *f->value + f->step * delta;

    /* 循环 */
    if (new_val > f->max) new_val = f->min;
    if (new_val < f->min) new_val = f->max;

    *f->value = new_val;
    lv_label_set_text_fmt(f->label, f->fmt, new_val);

    /* 温度线切换 */
    if (f->ind_short && f->ind_long) {
        lv_obj_add_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
        if (new_val < 100)
            lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
    }

    validate_constraints();
}

static void validate_constraints(void)
{
    /* 找到 minute 字段 */
    edit_field_t *min_field = NULL;
    for (int i = 0; i < edit_count; i++) {
        if (edit_fields[i].value == &set_min) {
            min_field = &edit_fields[i];
            break;
        }
    }
    if (!min_field) return;

    /* 根据 hour 动态调整 minute 的循环范围 */
    if (set_hour == 0) {
        min_field->min = 5;    // 5-59 循环，锁住 0-4
    } else if (set_hour == 4) {
        min_field->min = 0;    // hour=4 时 minute 不可调
        min_field->max = 0;
    } else {
        min_field->min = 0;    // 0-59 正常循环
        min_field->max = 59;
    }

    /* 纠正 minute 值（如果当前值超出新范围） */
    if (set_hour == 0 && set_min < 5) {
        set_min = 5;
        lv_label_set_text_fmt(min_field->label, min_field->fmt, set_min);
    }
    if (set_hour == 4 && set_min != 0) {
        set_min = 0;
        lv_label_set_text_fmt(min_field->label, min_field->fmt, set_min);
    }
}

// ==============================
// 页面栈操作
// ==============================

// 跳转子页前调用，记录"当前页"到栈顶
static void page_push(page_id_t id)
{
    if (depth < MAX_STACK) {
        page_stack[depth++] = id;  // 写入栈顶，depth 自增
        printf("[nav] page push: depth=%d id=%d\n", depth, id);
    }
}

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
static void page_pop(void)
{
    /* ① 根页保护 */
    if (depth <= 1) {
        printf("[nav] at root, cannot pop\n");
        return;
    }

    /* ②~④ 算出 child(子页) 和 prev(父页) */
    depth--;
    page_id_t child = page_stack[depth];
    page_id_t prev = page_stack[depth - 1];
    printf("[nav] page pop: depth=%d, back to id=%d (from id=%d)\n", depth, prev, child);

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
            }
        }
        lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
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

            /* 新按钮需要重新绑定事件 */
            if (cook && cook->up_down_button)
                lv_obj_add_event_cb(cook->up_down_button, on_cook_updown_click,
                                    LV_EVENT_CLICKED, NULL);

            /* 退出 updown_bbq 流程后清空临时值 */
            set_temp = 180;
            set_hour = 0;
            set_min = 30;
        }
        lv_scr_load_anim(cookmenu_get(&ui_manager)->obj,
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
                    sp->slow_cook_button, sp->unfrozen_button, sp->fajiao_button,
                    sp->corn_button, sp->heat_contain_button, sp->some_cook_button,
                };
                if (g_special_menu) lv_group_del(g_special_menu);
                g_special_menu = group_create_for_page(btns, sizeof(btns) / sizeof(btns[0]));
            }
            current_group = g_special_menu;
        }
        lv_scr_load_anim(special_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to special_menu\n");
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
        lv_scr_load_anim(updown_bbq_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to updown_bbq_menu\n");
        break;

    case PAGE_UPDOWN_BBQ_SET:
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
                lv_obj_clear_state(set->sure_button, LV_STATE_FOCUSED);
                lv_obj_clear_state(set->uptemp_button, LV_STATE_FOCUSED);
                lv_obj_clear_state(set->downtemp_button, LV_STATE_FOCUSED);
                lv_obj_clear_state(set->preheat_button, LV_STATE_FOCUSED);
                lv_obj_clear_state(set->preheat_on_button, LV_STATE_FOCUSED);
                lv_obj_clear_state(set->delay_button, LV_STATE_FOCUSED);
                lv_obj_clear_state(set->delay_on_button, LV_STATE_FOCUSED);
                lv_obj_clear_state(set->contain_button, LV_STATE_FOCUSED);
                lv_obj_clear_state(set->contain_on_button, LV_STATE_FOCUSED);
                lv_group_focus_obj(set->sure_button);

                /* 隐藏所有温度相关组件 */
                lv_obj_add_flag(set->up2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->up2_dir_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->up2_icon_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->down2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->down2_dir_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->down2_icon_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->up3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->up3_dir_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->up3_icon_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->down3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->down3_dir_label, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(set->down3_icon_label, LV_OBJ_FLAG_HIDDEN);

                /* 根据温度选 2 位或 3 位 */
                if (set_temp < 100) {
                    lv_label_set_text_fmt(set->up2_tempnum_label, "%d", set_temp);
                    lv_label_set_text_fmt(set->down2_tempnum_label, "%d", set_temp);
                    lv_obj_clear_flag(set->up2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->up2_dir_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->up2_icon_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->down2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->down2_dir_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->down2_icon_label, LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_label_set_text_fmt(set->up3_tempnum_label, "%d", set_temp);
                    lv_label_set_text_fmt(set->down3_tempnum_label, "%d", set_temp);
                    lv_obj_clear_flag(set->up3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->up3_dir_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->up3_icon_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->down3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->down3_dir_label, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->down3_icon_label, LV_OBJ_FLAG_HIDDEN);
                }

                /* 时间显示 */
                lv_label_set_text_fmt(set->hour_label, "%02d", set_hour);
                lv_label_set_text_fmt(set->min_label, "%02d", set_min);

                /* 恢复 toggle 状态（跟随变量） */
                if (preheat_on) {
                    lv_obj_add_flag(set->preheat_button, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->preheat_on_button, LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_clear_flag(set->preheat_button, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(set->preheat_on_button, LV_OBJ_FLAG_HIDDEN);
                }
                if (delay_on) {
                    lv_obj_add_flag(set->delay_button, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->delay_on_button, LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_clear_flag(set->delay_button, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(set->delay_on_button, LV_OBJ_FLAG_HIDDEN);
                }
                if (contain_on) {
                    lv_obj_add_flag(set->contain_button, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(set->contain_on_button, LV_OBJ_FLAG_HIDDEN);
                } else {
                    lv_obj_clear_flag(set->contain_button, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(set->contain_on_button, LV_OBJ_FLAG_HIDDEN);
                }

                /* 绑定 toggle 事件 */
                lv_obj_add_event_cb(set->preheat_button, on_preheat_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->preheat_on_button, on_preheat_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->delay_button, on_delay_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->delay_on_button, on_delay_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->contain_button, on_contain_toggle, LV_EVENT_CLICKED, NULL);
                lv_obj_add_event_cb(set->contain_on_button, on_contain_toggle, LV_EVENT_CLICKED, NULL);

                /* 绑定 sure_button 事件 */
                lv_obj_add_event_cb(set->sure_button, on_sure_click, LV_EVENT_CLICKED, NULL);

                /* 焦点恢复 */
                if (child == PAGE_UPDOWN_BBQ_COOKING && set->sure_button)
                    lv_group_focus_obj(set->sure_button);
            }
            current_group = g_updown_bbq_set;
        }
        lv_scr_load_anim(updown_bbq_set_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to updown_bbq_set\n");
        break;

    case PAGE_WAITMENU_24:
        waitmenu_24_create(&ui_manager);
        current_group = NULL;
        lv_scr_load_anim(waitmenu_24_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] back to waitmenu_24\n");
        break;

    default:
        printf("[nav] unknown page to restore: %d\n", prev);
        break;
    }
}

// ==============================
// 焦点组创建
// ==============================

// 创建 g_major_menu（只在初始化时调一次，返回时也会调）
static void groups_create(void)
{
    major_menu_t *major = major_menu_get(&ui_manager);
    if (!major) {
        printf("[nav] major_menu struct is NULL\n");
        return;
    }

    if (g_major_menu) lv_group_del(g_major_menu);
    g_major_menu = lv_group_create();
    if (!g_major_menu) {
        printf("[nav] failed to create major_menu group\n");
        return;
    }

    /* 按焦点顺序依次加入按钮 */
    if (major->cook_button)
        lv_group_add_obj(g_major_menu, major->cook_button);
    else
        printf("[nav] major->cook_button is NULL\n");

    if (major->cook4_button)
        lv_group_add_obj(g_major_menu, major->cook4_button);
    else
        printf("[nav] major->cook4_button is NULL\n");

    if (major->special_button)
        lv_group_add_obj(g_major_menu, major->special_button);
    else
        printf("[nav] major->special_button is NULL\n");

    printf("[nav] major_menu group created\n");
}

// 将 buttons 数组中的非 NULL 对象全部加入 group（统一 NULL 检查）
static void group_add_all_btns(lv_group_t *g, lv_obj_t **btns, int count)
{
    for (int i = 0; i < count; i++) {
        if (btns[i])
            lv_group_add_obj(g, btns[i]);
    }
}

// 创建 group 并加入所有按钮（跳转子页时调用）
static lv_group_t *group_create_for_page(lv_obj_t **btns, int count)
{
    lv_group_t *g = lv_group_create();
    if (g)
        group_add_all_btns(g, btns, count);
    return g;
}

// ==============================
// 页面跳转
// ==============================

// major_menu → cookmenu
static void jump_to_cookmenu(void)
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

    lv_scr_load_anim(cookmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    printf("[nav] jump: major_menu -> cookmenu\n");
}

// major_menu → special_menu
static void jump_to_special_menu(void)
{
    page_push(PAGE_SPECIAL_MENU);
    lv_obj_clean(lv_scr_act());
    special_menu_create(&ui_manager);

    special_menu_t *sp = special_menu_get(&ui_manager);
    if (sp) {
        lv_obj_t *btns[] = {
            sp->air_button, sp->piza_button, sp->frozen_cook_button,
            sp->slow_cook_button, sp->unfrozen_button, sp->fajiao_button,
            sp->corn_button, sp->heat_contain_button, sp->some_cook_button,
        };

        if (g_special_menu) lv_group_del(g_special_menu);
        g_special_menu = group_create_for_page(btns, sizeof(btns) / sizeof(btns[0]));
    }

    current_group = g_special_menu;

    lv_scr_load_anim(special_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);

    printf("[nav] jump: major_menu -> special_menu\n");
}

// cookmenu → updown_bbq_menu
static void jump_to_updown_bbq_menu(void)
{
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

    printf("[nav] jump: cookmenu -> updown_bbq_menu\n");
}

// updown_bbq_menu → updown_bbq_set
static void jump_to_updown_bbq_set(void)
{
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
        lv_obj_clear_state(set->sure_button, LV_STATE_FOCUSED);
        lv_obj_clear_state(set->uptemp_button, LV_STATE_FOCUSED);
        lv_obj_clear_state(set->downtemp_button, LV_STATE_FOCUSED);
        lv_obj_clear_state(set->preheat_button, LV_STATE_FOCUSED);
        lv_obj_clear_state(set->preheat_on_button, LV_STATE_FOCUSED);
        lv_obj_clear_state(set->delay_button, LV_STATE_FOCUSED);
        lv_obj_clear_state(set->delay_on_button, LV_STATE_FOCUSED);
        lv_obj_clear_state(set->contain_button, LV_STATE_FOCUSED);
        lv_obj_clear_state(set->contain_on_button, LV_STATE_FOCUSED);
        lv_group_focus_obj(set->sure_button);

        /* 隐藏所有温度相关组件 */
        lv_obj_add_flag(set->up2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up2_dir_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up2_icon_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down2_dir_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down2_icon_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up3_dir_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->up3_icon_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down3_dir_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(set->down3_icon_label, LV_OBJ_FLAG_HIDDEN);

        /* 根据温度选 2 位或 3 位 */
        if (set_temp < 100) {
            lv_label_set_text_fmt(set->up2_tempnum_label, "%d", set_temp);
            lv_label_set_text_fmt(set->down2_tempnum_label, "%d", set_temp);
            lv_obj_clear_flag(set->up2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up2_dir_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up2_icon_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down2_dir_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down2_icon_label, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_label_set_text_fmt(set->up3_tempnum_label, "%d", set_temp);
            lv_label_set_text_fmt(set->down3_tempnum_label, "%d", set_temp);
            lv_obj_clear_flag(set->up3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up3_dir_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->up3_icon_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down3_dir_label, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->down3_icon_label, LV_OBJ_FLAG_HIDDEN);
        }

        /* 时间显示 */
        lv_label_set_text_fmt(set->hour_label, "%02d", set_hour);
        lv_label_set_text_fmt(set->min_label, "%02d", set_min);

        /* 按钮状态重置（跟随变量） */
        if (preheat_on) {
            lv_obj_add_flag(set->preheat_button, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->preheat_on_button, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(set->preheat_button, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(set->preheat_on_button, LV_OBJ_FLAG_HIDDEN);
        }
        if (delay_on) {
            lv_obj_add_flag(set->delay_button, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->delay_on_button, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(set->delay_button, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(set->delay_on_button, LV_OBJ_FLAG_HIDDEN);
        }
        if (contain_on) {
            lv_obj_add_flag(set->contain_button, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(set->contain_on_button, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(set->contain_button, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(set->contain_on_button, LV_OBJ_FLAG_HIDDEN);
        }

        /* 绑定 toggle 事件 */
        lv_obj_add_event_cb(set->preheat_button, on_preheat_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->preheat_on_button, on_preheat_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->delay_button, on_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->delay_on_button, on_delay_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->contain_button, on_contain_toggle, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(set->contain_on_button, on_contain_toggle, LV_EVENT_CLICKED, NULL);

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
static void jump_to_updown_bbq_cooking(void)
{
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

        /* 更新界面显示 */
        if (set_hour == 0) {
            lv_label_set_text_fmt(cook->updown_label,
                "| 上下烧烤 | %d℃ | %02d分钟",
                set_temp, set_min);
        } else {
            lv_label_set_text_fmt(cook->updown_label,
                "| 上下烧烤 | %d℃ | %d小时%02d分钟",
                set_temp, set_hour, set_min);
        }
    }

    /* 立即显示倒计时初始值（覆盖 UiBuilder 默认值） */
    if (cook)
        lv_label_set_text_fmt(cook->time_label, "%02d:%02d:%02d", set_hour, set_min, 0);

    /* 初始化进度条 + 启动动画（连续无级平滑） */
    cook_total_ms = (set_hour * 3600 + set_min * 60) * 1000;
    if (cook) {
        lv_bar_set_range(cook->bar, 0, 100);
        lv_bar_set_value(cook->bar, 3, LV_ANIM_OFF);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, cook->bar);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_bar_set_value);
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

    printf("[nav] jump: updown_bbq_set -> updown_bbq_cooking\n");
}

// ==============================
// 按键处理（状态机防抖）
// ==============================

typedef enum { KEY_IDLE, KEY_PRESSED } key_state_t;
static key_state_t key_state = KEY_IDLE;
static uint8_t active_key = 0;
static uint32_t active_key_time = 0;

#define ENC_REPEAT_MS   50    // 编码器长按时重复间隔

static void process_key(uint8_t key)
{
    uart_data_receive[Receive_data_Touch_Key] = 0;

    switch (key) {
    case KEY_MENU:          // 3: 从 waitmenu_24 进入主菜单
        if (depth == 1 && page_stack[0] == PAGE_WAITMENU_24) {
            page_push(PAGE_MAJOR_MENU);
            lv_obj_clean(lv_scr_act());
            major_menu_create(&ui_manager);
            groups_create();
            bind_events();
            current_group = g_major_menu;
            lv_scr_load_anim(major_menu_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0,
                             ui_manager.auto_del);
            printf("[nav] jump: waitmenu_24 -> major_menu\n");
        }
        break;
    case KEY_BACK:          // 21: 返回
        page_pop();
        break;
    case KEY_ENCODER_CW: {  // 31: 焦点下移 / 数值+
        if (!current_group) break;
        lv_obj_t *focused = lv_group_get_focused(current_group);
        edit_field_t *ef = find_edit_field(focused);
        if (ef) {
            adjust_value(ef, +1);
            printf("[nav] adjust +: %d\n", *ef->value);
        } else {
            lv_group_focus_next(current_group);
            printf("[nav] focus next\n");
        }
        break;
    }
    case KEY_ENCODER_CCW: {  // 41: 焦点上移 / 数值-
        if (!current_group) break;
        lv_obj_t *focused = lv_group_get_focused(current_group);
        edit_field_t *ef = find_edit_field(focused);
        if (ef) {
            adjust_value(ef, -1);
            printf("[nav] adjust -: %d\n", *ef->value);
        } else {
            lv_group_focus_prev(current_group);
            printf("[nav] focus prev\n");
        }
        break;
    }
    case KEY_ENCODER_PRESS: { // 51: 确认 / 跳到下一焦点
        if (!current_group) break;
        lv_obj_t *focused = lv_group_get_focused(current_group);
        edit_field_t *ef = find_edit_field(focused);
        if (ef) {
            lv_group_focus_next(current_group);
            printf("[nav] press -> next focus\n");
        } else {
            lv_obj_send_event(focused, LV_EVENT_CLICKED, NULL);
            printf("[nav] press -> click\n");
        }
        break;
    }
    default:
        printf("[nav] unknown key: %d\n", key);
        break;
    }
}

void nav_handle_key(uint8_t key)
{
    uint32_t now = lv_tick_get();

    switch (key_state) {
    case KEY_IDLE:
        if (key != 0) {
            active_key = key;
            active_key_time = now;
            key_state = KEY_PRESSED;
            process_key(key);
        }
        break;

    case KEY_PRESSED:
        if (key == 0) {
            // 松开 → 回到空闲
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

// ==============================
// 事件绑定
// ==============================

static void on_major_cook_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))  // 防重入：动画未完成时不跳转
        jump_to_cookmenu();
}

static void on_major_special_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_special_menu();
}

static void on_major_cook4_click(lv_event_t *e)
{
    printf("[nav] cook4 jump not implemented yet\n");
}

static void on_cook_updown_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_menu();
}

static void on_updown_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_set();
}

static void on_edit_focus(lv_event_t *e)
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
}

static void on_preheat_toggle(lv_event_t *e)
{
    updown_bbq_set_t *set = updown_bbq_set_get(&ui_manager);
    if (!set) return;
    preheat_on = !preheat_on;
    if (preheat_on) {
        lv_obj_add_flag(set->preheat_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->preheat_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->preheat_on_button);
    } else {
        lv_obj_add_flag(set->preheat_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->preheat_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->preheat_button);
    }
}

static void on_delay_toggle(lv_event_t *e)
{
    updown_bbq_set_t *set = updown_bbq_set_get(&ui_manager);
    if (!set) return;
    delay_on = !delay_on;
    if (delay_on) {
        lv_obj_add_flag(set->delay_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->delay_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->delay_on_button);
    } else {
        lv_obj_add_flag(set->delay_on_button, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->delay_button, LV_OBJ_FLAG_HIDDEN);
        lv_group_focus_obj(set->delay_button);
    }
}

static void on_contain_toggle(lv_event_t *e)
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

static void on_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_updown_bbq_cooking();
}

static void cooking_timer_cb(lv_timer_t *timer)
{
    updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
    if (!cook) return;

    uint32_t elapsed = lv_tick_get() - cook_start_time;
    if (elapsed >= (uint32_t)cook_total_ms) {
        lv_timer_del(cook_timer);
        cook_timer = NULL;
        lv_label_set_text_fmt(cook->time_label, "%02d:%02d:%02d", 0, 0, 0);
        return;
    }

    /* 时间显示（半秒四舍五入） */
    int elapsed_sec = (elapsed + 500) / 1000;
    int total_sec = set_hour * 3600 + set_min * 60;
    int remaining_sec = total_sec - elapsed_sec;
    if (remaining_sec < 0) remaining_sec = 0;
    int h = remaining_sec / 3600;
    int m = (remaining_sec % 3600) / 60;
    int s = remaining_sec % 60;
    lv_label_set_text_fmt(cook->time_label, "%02d:%02d:%02d", h, m, s);
}

// 为 major_menu 的三个按钮绑定 LV_EVENT_CLICKED 回调
// 注意：每次 groups_create 后必须调用 bind_events，新按钮需要重新绑定
static void bind_events(void)
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
    // groups_create / bind_events 在 KEY_MENU 跳转时调用
    current_group = NULL;               // waitmenu_24 无焦点组
    depth = 0;
    page_push(PAGE_WAITMENU_24);        // 根页 = waitmenu_24
    printf("[nav] init done\n");
}
