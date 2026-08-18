#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"

/* ==============================
 * 第六感-烤色选择（toastcolor 页）
 * degree 标签(浅/中/深)入焦点组,编码器切换档位(默认中),
 * line 跟随 degree 焦点显隐(聚焦显示/离开隐藏,固定位置);
 * 确定(PRESS)从 degree 切到 next;next 点击 → 回六感运行页上色准备态
 * ============================== */

lv_group_t *g_toastcolor = NULL;
static int s_toast_color = 2;   /* 1浅 2中 3深 */

static void toastcolor_update_degree(void);

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

static void on_toastcolor_focus(lv_event_t *e)
{
    toastcolor_apply_line(lv_event_get_target(e));
}

static void on_toastcolor_next_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
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
            lv_label_set_text(tc->label_24, six_bread_name());   /* 左上角菜名 */
        /* 焦点组:degree(档位设置) + next */
        lv_obj_t *btns[] = { tc->degree, tc->next };
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
        }
        if (tc->degree) {
            lv_obj_add_event_cb(tc->degree, on_toastcolor_focus, LV_EVENT_FOCUSED, NULL);
        }

        s_toast_color = 2;                /* 每次进入默认"中" */
        toastcolor_update_degree();
    }
    current_group = g_toastcolor;

    lv_scr_load_anim(toastcolor_get(&ui_manager)->obj,
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
            lv_label_set_text(tc->label_24, six_bread_name());   /* 左上角菜名 */
        lv_obj_t *btns[] = { tc->degree, tc->next };
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
        }
        if (tc->degree) {
            lv_obj_add_event_cb(tc->degree, on_toastcolor_focus, LV_EVENT_FOCUSED, NULL);
        }

        s_toast_color = 2;                /* 每次进入默认"中" */
        toastcolor_update_degree();
    }
    current_group = g_toastcolor;

    lv_scr_load_anim(toastcolor_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[toastcolor] rebuild\n");
}

// degree 标签显示当前档位单字(默认"中",切换变浅/深)
static void toastcolor_update_degree(void)
{
    toastcolor_t *tc = toastcolor_get(&ui_manager);
    if (!tc || !tc->degree) return;
    lv_label_set_text(tc->degree, s_toast_color == 1 ? "浅" :
                                  s_toast_color == 3 ? "深" : "中");
}

// 编码器切换档位(CW/CCW 由 nav.c 调用,任意焦点都切程度)
void toastcolor_cycle(int dir)
{
    s_toast_color += dir;
    if (s_toast_color < 1) s_toast_color = 3;
    if (s_toast_color > 3) s_toast_color = 1;
    toastcolor_update_degree();
    printf("[toastcolor] degree -> %d\n", s_toast_color);
}
