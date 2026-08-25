/*
 * nav_sixset2.c - 第六感-双维选择页（复用 sixset2 UI）
 *
 * 当前用于烤带皮土豆：degree(浅/中/深) × weight(500/1000/1500g) 二维选择。
 *
 * 页面控件分工：
 *   degree 组(label_13"选择烤色程度" + degree 单字 + degreeline) = 程度,常显
 *   weight 组(weight + weighticon"g" + weightline3/4) = 份量
 *   maturity 组(name1 标题 + maturity + maturityline2/3) = 成熟度(5档,预留)
 *   中间区二选一:按菜单显示 weight 或 maturity,name1 标题跟随组类型;
 *   右边 degree 常显。焦点循环:next → 中间组 → degree → next(3 个焦点切换)。
 *
 * 数据: t[重量档][程度档] 分钟,默认 1000g×中等=50min。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"

lv_group_t *g_sixset2 = NULL;

/* 烤带皮土豆数据 */
static const int jw[3] = { 500, 1000, 1500 };
static const char *jdeg_short[3] = { "浅", "中", "深" };      /* degree 大字显示(单字) */
static const char *jdeg[3]     = { "浅", "中等", "深" };      /* 小结用全称 */
static const int jt[3][3] = {
    { 38, 40, 45 },   /* 500g  浅/中/深 */
    { 48, 50, 55 },   /* 1000g 浅/中/深 */
    { 58, 60, 65 },   /* 1500g 浅/中/深 */
};
static int s_jw = 1;   /* 重量档 idx,默认 1000g */
static int s_jd = 1;   /* 程度档 idx(0浅/1中/2深),默认 中等 */
static int s_show_maturity = 0;   /* 中间区显示 maturity(true) 还是 weight(false) */

/* 成熟度 5 档(预留;具体菜接入时补时间映射) */
static const char *mat_txt[5] = {
    "一成熟", "三成熟", "五成熟", "七成熟", "全熟",
};
static int s_mat = 4;   /* 默认 五成熟 */

int six_chick_is_jacket(void)
{
    return (g_six_bread_type == SIX_VEG_JACKET_POTATO);
}
int jacket_cook_min(void)         { return jt[s_jw][s_jd]; }
int jacket_weight(void)           { return jw[s_jw]; }
const char *jacket_deg_text(void) { return jdeg[s_jd]; }

static void apply_display(void);
static void on_next_click(lv_event_t *e);

/* 中间区当前控件(按场景返回 weight 或 maturity;next 场景返回 NULL) */
static lv_obj_t *mid_obj(sixset2_t *pg)
{
    return s_show_maturity ? pg->maturity : pg->weight;
}

/* 统一刷新三组显示 */
static void apply_display(void)
{
    sixset2_t *pg = sixset2_get(&ui_manager);
    if (!pg) return;

    /* degree:单字 浅/中/深 */
    if (pg->degree)
        lv_label_set_text(pg->degree, jdeg_short[s_jd]);

    /* weight:纯数字 */
    if (pg->weight)
        lv_label_set_text_fmt(pg->weight, "%d", jw[s_jw]);

    /* maturity:文本 */
    if (s_show_maturity && pg->maturity)
        lv_label_set_text(pg->maturity, mat_txt[s_mat]);
}

/* 下划线显隐:全隐藏 → 只亮当前焦点字段的线(值决定该字段用哪条线)
   与 updown_bbq_menu 的 on_edit_focus 同思路(焦点事件驱动) */
static void apply_line_for(lv_obj_t *focused)
{
    sixset2_t *pg = sixset2_get(&ui_manager);
    if (!pg) return;

    /* ① 全隐藏 */
    if (pg->weightline3)   lv_obj_add_flag(pg->weightline3, LV_OBJ_FLAG_HIDDEN);
    if (pg->weightline4)   lv_obj_add_flag(pg->weightline4, LV_OBJ_FLAG_HIDDEN);
    if (pg->maturityline2) lv_obj_add_flag(pg->maturityline2, LV_OBJ_FLAG_HIDDEN);
    if (pg->maturityline3) lv_obj_add_flag(pg->maturityline3, LV_OBJ_FLAG_HIDDEN);
    if (pg->degreeline)    lv_obj_add_flag(pg->degreeline, LV_OBJ_FLAG_HIDDEN);

    /* ② 只亮焦点字段的线 */
    if (focused == pg->weight && pg->weightline3 && pg->weightline4) {
        int len = snprintf(NULL, 0, "%d", jw[s_jw]);
        if (len >= 4) lv_obj_clear_flag(pg->weightline4, LV_OBJ_FLAG_HIDDEN);
        else          lv_obj_clear_flag(pg->weightline3, LV_OBJ_FLAG_HIDDEN);
    } else if (focused == pg->maturity && pg->maturityline2 && pg->maturityline3) {
        int len = (int)strlen(mat_txt[s_mat]) / 3;   /* 汉字字数 */
        if (len >= 3) lv_obj_clear_flag(pg->maturityline3, LV_OBJ_FLAG_HIDDEN);
        else          lv_obj_clear_flag(pg->maturityline2, LV_OBJ_FLAG_HIDDEN);
    } else if (focused == pg->degree && pg->degreeline) {
        lv_obj_clear_flag(pg->degreeline, LV_OBJ_FLAG_HIDDEN);
    }
}

/* 焦点事件:进入选择对象时刷新下划线(跟随焦点) */
static void sixset2_focus_cb(lv_event_t *e)
{
    apply_line_for(lv_event_get_target(e));
}

/* 焦点推进: next → 中间组 → degree → next(PRESS 用) */
void sixset2_press_focus(void)
{
    lv_obj_t *df = lv_group_get_focused(current_group);
    sixset2_t *pg = sixset2_get(&ui_manager);
    if (!pg) return;
    if (df == pg->next && mid_obj(pg)) {
        lv_group_focus_obj(mid_obj(pg));
        apply_line_for(mid_obj(pg));   /* 立即亮线,不依赖事件时序 */
    } else if (df == mid_obj(pg) && pg->degree) {
        lv_group_focus_obj(pg->degree);
        apply_line_for(pg->degree);
    } else if (pg->next) {
        lv_group_focus_obj(pg->next);
        apply_line_for(pg->next);   /* next:无线 */
    }
}

/* 编码器:按焦点切对应维;焦点在 next 时仅移到中间组(不切值)。
   返回 1 表示已处理 */
int sixset2_cycle(int dir)
{
    sixset2_t *pg = sixset2_get(&ui_manager);
    if (!pg) return 0;
    lv_obj_t *df = lv_group_get_focused(current_group);

    if (df == pg->degree) {
        s_jd += dir;
        if (s_jd < 0) s_jd = 2;
        if (s_jd > 2) s_jd = 0;
        apply_display();
        apply_line_for(df);
        printf("[sixset2] degree -> %s\n", jdeg[s_jd]);
        return 1;
    }
    if (df == pg->weight) {
        s_jw += dir;
        if (s_jw < 0) s_jw = 2;
        if (s_jw > 2) s_jw = 0;
        apply_display();
        apply_line_for(df);
        printf("[sixset2] weight -> %dg\n", jw[s_jw]);
        return 1;
    }
    if (s_show_maturity && df == pg->maturity) {
        s_mat += dir;
        if (s_mat < 0) s_mat = 4;
        if (s_mat > 4) s_mat = 0;
        apply_display();
        apply_line_for(df);
        printf("[sixset2] maturity -> %s\n", mat_txt[s_mat]);
        return 1;
    }
    /* next 或其他:移到中间组,不切数值 */
    if (mid_obj(pg)) {
        lv_group_focus_obj(mid_obj(pg));
        apply_line_for(mid_obj(pg));
        return 1;
    }
    return 0;
}

static void on_next_click(lv_event_t *e)
{
    (void)e;
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_descriptionmenu();
}

void jump_to_sixset2(void)
{
    page_push(PAGE_SIXSET2);
    lv_obj_clean(lv_scr_act());
    sixset2_create(&ui_manager);

    sixset2_t *pg = sixset2_get(&ui_manager);
    if (pg) {
        lv_obj_t *btns[] = { pg->degree, mid_obj(pg), pg->next };
        const int n = 3;
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_sixset2) { lv_group_del(g_sixset2); g_sixset2 = NULL; }
        g_sixset2 = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        /* 中间区互斥:jacket 显示 weight 组、隐藏 maturity 组 */
        if (pg->maturity)      lv_obj_add_flag(pg->maturity, LV_OBJ_FLAG_HIDDEN);
        if (pg->maturityline2) lv_obj_add_flag(pg->maturityline2, LV_OBJ_FLAG_HIDDEN);
        if (pg->maturityline3) lv_obj_add_flag(pg->maturityline3, LV_OBJ_FLAG_HIDDEN);

        /* name1 跟随中间组类型 */
        if (pg->name1)    lv_label_set_text(pg->name1, tr("份量/种类"));
        if (pg->label_18) lv_label_set_text(pg->label_18, tr("烤带皮土豆"));

        /* degree 组恢复常显 */
        if (pg->label_13)   lv_obj_clear_flag(pg->label_13, LV_OBJ_FLAG_HIDDEN);
        if (pg->degree)     lv_obj_clear_flag(pg->degree, LV_OBJ_FLAG_HIDDEN);
        if (pg->degreeline) lv_obj_clear_flag(pg->degreeline, LV_OBJ_FLAG_HIDDEN);

        if (pg->next) lv_obj_add_event_cb(pg->next, on_next_click, LV_EVENT_CLICKED, NULL);

        /* 三个选择对象绑 FOCUSED → 下划线跟随焦点 */
        if (pg->weight)   lv_obj_add_event_cb(pg->weight,   sixset2_focus_cb, LV_EVENT_FOCUSED, NULL);
        if (pg->maturity) lv_obj_add_event_cb(pg->maturity, sixset2_focus_cb, LV_EVENT_FOCUSED, NULL);
        if (pg->degree)   lv_obj_add_event_cb(pg->degree,   sixset2_focus_cb, LV_EVENT_FOCUSED, NULL);

        s_jw = 1;   /* 默认 1000g */
        s_jd = 1;   /* 默认 中等 */
        apply_display();
        apply_line_for(NULL);   /* 初始无线 */

        if (pg->next) lv_group_focus_obj(pg->next);   /* 初始焦点 next */
    }
    current_group = g_sixset2;

    lang_scr_load_anim(sixset2_get(&ui_manager)->obj,
                       LV_SCR_LOAD_ANIM_NONE, 0, 0,
                       ui_manager.auto_del);
    printf("[sixset2] jump\n");
}

void sixset2_rebuild(page_id_t child)
{
    if (g_sixset2) { lv_group_del(g_sixset2); g_sixset2 = NULL; }
    lv_obj_clean(lv_scr_act());
    sixset2_create(&ui_manager);

    sixset2_t *pg = sixset2_get(&ui_manager);
    if (pg) {
        lv_obj_t *btns[] = { pg->degree, mid_obj(pg), pg->next };
        const int n = 3;
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_sixset2 = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (pg->maturity)      lv_obj_add_flag(pg->maturity, LV_OBJ_FLAG_HIDDEN);
        if (pg->maturityline2) lv_obj_add_flag(pg->maturityline2, LV_OBJ_FLAG_HIDDEN);
        if (pg->maturityline3) lv_obj_add_flag(pg->maturityline3, LV_OBJ_FLAG_HIDDEN);

        if (pg->name1)    lv_label_set_text(pg->name1, tr("份量/种类"));
        if (pg->label_18) lv_label_set_text(pg->label_18, tr("烤带皮土豆"));

        if (pg->label_13)   lv_obj_clear_flag(pg->label_13, LV_OBJ_FLAG_HIDDEN);
        if (pg->degree)     lv_obj_clear_flag(pg->degree, LV_OBJ_FLAG_HIDDEN);
        if (pg->degreeline) lv_obj_clear_flag(pg->degreeline, LV_OBJ_FLAG_HIDDEN);

        if (pg->next) lv_obj_add_event_cb(pg->next, on_next_click, LV_EVENT_CLICKED, NULL);

        if (pg->weight)   lv_obj_add_event_cb(pg->weight,   sixset2_focus_cb, LV_EVENT_FOCUSED, NULL);
        if (pg->maturity) lv_obj_add_event_cb(pg->maturity, sixset2_focus_cb, LV_EVENT_FOCUSED, NULL);
        if (pg->degree)   lv_obj_add_event_cb(pg->degree,   sixset2_focus_cb, LV_EVENT_FOCUSED, NULL);

        apply_display();

        /* 返回时焦点恢复到进入前的维(jacket:degree 在后,先回中间组) */
        if (mid_obj(pg)) {
            lv_group_focus_obj(mid_obj(pg));
            apply_line_for(mid_obj(pg));   /* 恢复时直接亮线 */
        }
    }
    current_group = g_sixset2;

    lang_scr_load_anim(sixset2_get(&ui_manager)->obj,
                       LV_SCR_LOAD_ANIM_NONE, 0, 0,
                       ui_manager.auto_del);
    printf("[sixset2] rebuild\n");
}
