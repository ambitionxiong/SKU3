/*
 * nav_toastcolor.c - 第六感-烤色选择页
 * 浅/中/深三档烤色，编码器切换，确认进入额外上色。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"
#include <string.h>

/* ==============================
 * 第六感-烤色选择（toastcolor 页）
 * 页面包含三组互斥的设置信息，同一时间仅显示一组，由 label_23 标题标明当前组：
 *   1) 烤色程度：degree 标签(浅/中/深) + line（跟随 degree 聚焦显隐）
 *   2) 成熟度：  Maturity 标签 + 其下划线（maturityline2/3 按文字字数择一显示）
 *   3) 份量/种类： weight 标签 + 其下划线（weightline3/4 按数值位数择一显示）
 * 编码器切换档位(默认中)；确定(PRESS)从当前组切到 next；next 点击进入下一步。
 * ============================== */

lv_group_t *g_toastcolor = NULL;
static int s_toast_color = 2;   /* 1浅 2中 3深 */

/* 当前激活组：三组互斥（默认烤色程度；进入前由流程设置 g_toast_mode） */
int g_toast_mode = TOAST_MODE_DEGREE;

/* 份量/种类组选项（克数，由调用方按菜谱表传入） */
static const int *s_weight_opts = NULL;
static int s_weight_count = 0;
static int s_weight_index = 0;   /* 当前选中份量下标 */

static void toastcolor_update_degree(void);
static void toastcolor_update_weight(void);

/* 设置份量选项并选定默认下标（default_idx <0 时取 0） */
void toastcolor_set_weight_options(const int *opts, int count, int default_idx)
{
    s_weight_opts = opts;
    s_weight_count = count > 0 ? count : 0;
    s_weight_index = default_idx;
    if (s_weight_index < 0 || s_weight_index >= s_weight_count)
        s_weight_index = 0;
}

/* 当前选中份量克数；非份量组/未设置返回 -1 */
int toastcolor_weight_value(void)
{
    if (g_toast_mode != TOAST_MODE_WEIGHT || s_weight_count <= 0 || !s_weight_opts)
        return -1;
    return s_weight_opts[s_weight_index];
}

/* 当前选中程度(1浅2中3深)；非 degree 组返回 -1 */
int toastcolor_degree_value(void)
{
    if (g_toast_mode != TOAST_MODE_DEGREE)
        return -1;
    return s_toast_color;
}

/* 互斥显示：仅显示当前激活组，其余两组整体隐藏 */
static void toastcolor_apply_mode_visibility(void)
{
    toastcolor_t *tc = toastcolor_get(&ui_manager);
    if (!tc) return;

    int show_deg = (g_toast_mode == TOAST_MODE_DEGREE);
    int show_mat = (g_toast_mode == TOAST_MODE_MATURITY);
    int show_wt  = (g_toast_mode == TOAST_MODE_WEIGHT);

    /* 烤色程度组 */
    if (tc->degree) {
        if (show_deg) lv_obj_clear_flag(tc->degree, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(tc->degree, LV_OBJ_FLAG_HIDDEN);
    }
    if (tc->line) {
        if (show_deg) lv_obj_clear_flag(tc->line, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(tc->line, LV_OBJ_FLAG_HIDDEN);
    }
    /* 成熟度组 */
    if (tc->Maturity) {
        if (show_mat) lv_obj_clear_flag(tc->Maturity, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(tc->Maturity, LV_OBJ_FLAG_HIDDEN);
    }
    if (tc->maturityline2) {
        if (show_mat) lv_obj_clear_flag(tc->maturityline2, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(tc->maturityline2, LV_OBJ_FLAG_HIDDEN);
    }
    if (tc->maturityline3) {
        if (show_mat) lv_obj_clear_flag(tc->maturityline3, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(tc->maturityline3, LV_OBJ_FLAG_HIDDEN);
    }
    /* 份量/种类组 */
    if (tc->weight) {
        if (show_wt) lv_obj_clear_flag(tc->weight, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(tc->weight, LV_OBJ_FLAG_HIDDEN);
    }
    if (tc->weighticon) {
        if (show_wt) lv_obj_clear_flag(tc->weighticon, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(tc->weighticon, LV_OBJ_FLAG_HIDDEN);
    }
    if (tc->weightline3) {
        if (!show_wt) lv_obj_add_flag(tc->weightline3, LV_OBJ_FLAG_HIDDEN);
        /* weight 组激活时下划线交给焦点控制（apply_weight_line） */
    }
    if (tc->weightline4) {
        if (!show_wt) lv_obj_add_flag(tc->weightline4, LV_OBJ_FLAG_HIDDEN);
    }
    if (tc->maturityline2) {
        if (!show_mat) lv_obj_add_flag(tc->maturityline2, LV_OBJ_FLAG_HIDDEN);
        else {
            int len = (int)strlen(lv_label_get_text(tc->Maturity));
            if (len >= 2) lv_obj_add_flag(tc->maturityline2, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_clear_flag(tc->maturityline2, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (tc->maturityline3) {
        if (!show_mat) lv_obj_add_flag(tc->maturityline3, LV_OBJ_FLAG_HIDDEN);
        else {
            int len = (int)strlen(lv_label_get_text(tc->Maturity));
            if (len >= 2) lv_obj_clear_flag(tc->maturityline3, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_add_flag(tc->maturityline3, LV_OBJ_FLAG_HIDDEN);
        }
    }

    /* 标题跟随当前组 */
    if (tc->label_23)
        lv_label_set_text(tc->label_23, show_deg ? tr("选择烤色程度") :
                                          show_mat ? "成熟度" : "份量/种类");
}

// line 固定位置(生成默认),仅按 degree 是否聚焦显隐
static void toastcolor_apply_line(lv_obj_t *focused)
{
    toastcolor_t *tc = toastcolor_get(&ui_manager);
    if (!tc || !tc->line) return;
    if (focused == tc->degree)
        lv_obj_clear_flag(tc->line, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(tc->line, LV_OBJ_FLAG_HIDDEN);
}

// weight 组下划线:仅 weight 聚焦时显示(按位数择一)，其他情况隐藏
static void toastcolor_apply_weight_line(lv_obj_t *focused)
{
    toastcolor_t *tc = toastcolor_get(&ui_manager);
    if (!tc) return;
    int on = (focused == tc->weight);
    if (on && tc->weight) {
        int len = (int)strlen(lv_label_get_text(tc->weight));
        if (tc->weightline4) {
            if (len >= 4) lv_obj_clear_flag(tc->weightline4, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_add_flag(tc->weightline4, LV_OBJ_FLAG_HIDDEN);
        }
        if (tc->weightline3) {
            if (len >= 4) lv_obj_add_flag(tc->weightline3, LV_OBJ_FLAG_HIDDEN);
            else lv_obj_clear_flag(tc->weightline3, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        if (tc->weightline3) lv_obj_add_flag(tc->weightline3, LV_OBJ_FLAG_HIDDEN);
        if (tc->weightline4) lv_obj_add_flag(tc->weightline4, LV_OBJ_FLAG_HIDDEN);
    }
}

static void on_toastcolor_focus(lv_event_t *e)
{
    if (g_toast_mode == TOAST_MODE_WEIGHT)
        toastcolor_apply_weight_line(lv_event_get_target(e));
    else
        toastcolor_apply_line(lv_event_get_target(e));
}

static void on_toastcolor_next_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    if (six_chick_is_probe()) {
        /* 探针菜(烤全鸡/烤全鸭):浅/中/深 → 探针目标温度(按菜谱),进 descriptionmenu */
        g_six_probe_temp = six_chick_probe_temp(s_toast_color);
        jump_to_descriptionmenu();
        return;
    }
    if (six_chick_is_kind()) {
        /* 烤鸡翅类:份量已选（toastcolor_weight_value()），进 cooking 说明页 */
        jump_to_descriptionmenu();
        return;
    }
    g_six_color_min = six_bread_color_min(s_toast_color);   /* 1浅 2中 3深,按菜查表 */
    six_cook_goto_setup();   /* 重建后显示上色准备态 */
    /* 回运行页:弹 toastcolor */
    page_pop();
}

void jump_to_toastcolor(void)
{
    page_push(PAGE_TOASTCOLOR);
    lv_obj_clean(lv_scr_act());
    toastcolor_create(&ui_manager);

    toastcolor_t *tc = toastcolor_get(&ui_manager);
    if (tc) {
        if (tc->label_24)
            lv_label_set_text(tc->label_24, six_current_name());   /* 左上角菜名（烤鸡走独立名） */
        /* 模式：面包/蛋糕固定烤色程度；鸡流程进入前已设置（探针菜=烤色程度/份量菜=份量） */
        if (!six_chick_is_probe() && !six_chick_is_kind())
            g_toast_mode = TOAST_MODE_DEGREE;
        toastcolor_apply_mode_visibility(); /* 三组互斥：仅显示当前组 */
        /* 焦点组:当前组标签 + next（烤色程度→degree，份量→weight） */
        lv_obj_t *btns[] = { (g_toast_mode == TOAST_MODE_WEIGHT) ? tc->weight : tc->degree,
                             tc->next };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_toastcolor) lv_group_del(g_toastcolor);
        g_toastcolor = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (tc->next) {
            lv_obj_add_event_cb(tc->next, on_toastcolor_next_click, LV_EVENT_CLICKED, NULL);
            lv_obj_add_event_cb(tc->next, on_toastcolor_focus, LV_EVENT_FOCUSED, NULL);
            lv_group_focus_obj(tc->next);     /* 默认焦点 next */
            toastcolor_apply_line(tc->next);  /* 显式隐藏 line(不依赖事件时序) */
            if (g_toast_mode == TOAST_MODE_WEIGHT)
                toastcolor_apply_weight_line(tc->next);   /* 初始焦点不在 weight:隐藏份量下划线 */
        }
        if (tc->degree) {
            lv_obj_add_event_cb(tc->degree, on_toastcolor_focus, LV_EVENT_FOCUSED, NULL);
        }
        if (tc->weight && g_toast_mode == TOAST_MODE_WEIGHT) {
            lv_obj_add_event_cb(tc->weight, on_toastcolor_focus, LV_EVENT_FOCUSED, NULL);
        }

        s_toast_color = 2;                /* 每次进入默认"中" */
        toastcolor_update_degree();
        if (g_toast_mode == TOAST_MODE_WEIGHT)
            toastcolor_update_weight();   /* 份量显示当前选中项 */
    }
    current_group = g_toastcolor;

    lang_scr_load_anim(toastcolor_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[toastcolor] jump\n");
}

void toastcolor_rebuild(page_id_t child)
{
    if (g_toastcolor) { lv_group_del(g_toastcolor); g_toastcolor = NULL; }
    lv_obj_clean(lv_scr_act());
    toastcolor_create(&ui_manager);

    toastcolor_t *tc = toastcolor_get(&ui_manager);
    if (tc) {
        if (tc->label_24)
            lv_label_set_text(tc->label_24, six_current_name());   /* 左上角菜名（烤鸡走独立名） */
        /* 模式：面包/蛋糕固定烤色程度；鸡流程重建时保持进入时设置的组 */
        if (!six_chick_is_probe() && !six_chick_is_kind())
            g_toast_mode = TOAST_MODE_DEGREE;
        toastcolor_apply_mode_visibility(); /* 三组互斥：仅显示当前组 */
        lv_obj_t *btns[] = { (g_toast_mode == TOAST_MODE_WEIGHT) ? tc->weight : tc->degree,
                             tc->next };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_toastcolor = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (tc->next) {
            lv_obj_add_event_cb(tc->next, on_toastcolor_next_click, LV_EVENT_CLICKED, NULL);
            lv_obj_add_event_cb(tc->next, on_toastcolor_focus, LV_EVENT_FOCUSED, NULL);
            lv_group_focus_obj(tc->next);
            toastcolor_apply_line(tc->next);
            if (g_toast_mode == TOAST_MODE_WEIGHT)
                toastcolor_apply_weight_line(tc->next);
        }
        if (tc->degree) {
            lv_obj_add_event_cb(tc->degree, on_toastcolor_focus, LV_EVENT_FOCUSED, NULL);
        }
        if (tc->weight && g_toast_mode == TOAST_MODE_WEIGHT) {
            lv_obj_add_event_cb(tc->weight, on_toastcolor_focus, LV_EVENT_FOCUSED, NULL);
        }

        s_toast_color = 2;                /* 每次进入默认"中" */
        toastcolor_update_degree();
        if (g_toast_mode == TOAST_MODE_WEIGHT)
            toastcolor_update_weight();
    }
    current_group = g_toastcolor;

    lang_scr_load_anim(toastcolor_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[toastcolor] rebuild\n");
}

// degree 标签显示当前档位单字(默认"中",切换变浅/深)
static void toastcolor_update_degree(void)
{
    toastcolor_t *tc = toastcolor_get(&ui_manager);
    if (!tc || !tc->degree) return;
    lv_label_set_text(tc->degree, s_toast_color == 1 ? tr("浅") :
                                  s_toast_color == 3 ? "深" : "中");
}

// weight 标签显示当前选中份量(g)；下划线由焦点+位数控制（见 toastcolor_apply_weight_line）
static void toastcolor_update_weight(void)
{
    toastcolor_t *tc = toastcolor_get(&ui_manager);
    if (!tc || !tc->weight) return;
    if (s_weight_count <= 0 || !s_weight_opts) return;

    lv_label_set_text_fmt(tc->weight, "%d", s_weight_opts[s_weight_index]);
}

// 编码器切换(CW/CCW 由 nav_key.c 调用)：按当前组切换档位或份量
void toastcolor_cycle(int dir)
{
    if (g_toast_mode == TOAST_MODE_WEIGHT) {
        toastcolor_t *tc = toastcolor_get(&ui_manager);
        if (s_weight_count <= 1) return;
        s_weight_index = (s_weight_index + dir + s_weight_count) % s_weight_count;
        toastcolor_update_weight();
        if (tc)
            toastcolor_apply_weight_line(tc->weight);   /* 切档后刷新下划线（焦点在 weight 上） */
        printf("[toastcolor] weight -> %dg\n", s_weight_opts[s_weight_index]);
        return;
    }
    s_toast_color += dir;
    if (s_toast_color < 1) s_toast_color = 3;
    if (s_toast_color > 3) s_toast_color = 1;
    toastcolor_update_degree();
    printf("[toastcolor] degree -> %d\n", s_toast_color);
}
