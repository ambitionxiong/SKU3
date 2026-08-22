/*
 * nav_six_rising.c - 第六感-发酵判断页
 * 面包是否发酵选择，进入描述页。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"

/* ==============================
 * 第六感-发酵阶段选择页（risingpage）
 * 菜单流程: bread6menu → risingpage → descriptionmenu
 * ============================== */

lv_group_t *g_risingpage = NULL;
int g_rising_choice = -1;   /* -1 未选 0=否 1=是 */

static void on_rising_yes_click(lv_event_t *e);
static void on_rising_no_click(lv_event_t *e);

void jump_to_risingpage(void)
{
    g_rising_choice = -1;   /* 每次进入默认未选,焦点回"是" */
    page_push(PAGE_RISINGPAGE);
    lv_obj_clean(lv_scr_act());
    risingpage_create(&ui_manager);

    risingpage_t *rp = risingpage_get(&ui_manager);
    if (rp) {
        lv_obj_t *btns[] = { rp->yes, rp->no };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_risingpage) lv_group_del(g_risingpage);
        g_risingpage = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (rp->label_17)
            lv_label_set_text(rp->label_17, six_bread_name());   /* 左上角菜名 */
        if (rp->yes) {
            lv_obj_add_event_cb(rp->yes, on_rising_yes_click, LV_EVENT_CLICKED, NULL);
        }
        if (rp->no) {
            lv_obj_add_event_cb(rp->no, on_rising_no_click, LV_EVENT_CLICKED, NULL);
        }

        /* 焦点：按上次选择恢复，未选过默认 yes */
        lv_obj_t *focus_rp = (g_rising_choice == 0 ? rp->no : rp->yes);
        if (focus_rp) lv_group_focus_obj(focus_rp);
    }
    current_group = g_risingpage;

    lv_scr_load_anim(risingpage_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_rising] jump: risingpage\n");
}

void risingpage_rebuild(page_id_t child)
{
    if (g_risingpage) { lv_group_del(g_risingpage); g_risingpage = NULL; }
    lv_obj_clean(lv_scr_act());
    risingpage_create(&ui_manager);

    risingpage_t *rp = risingpage_get(&ui_manager);
    if (rp) {
        lv_obj_t *btns[] = { rp->yes, rp->no };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_risingpage = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (rp->label_17)
            lv_label_set_text(rp->label_17, six_bread_name());   /* 左上角菜名 */
        if (rp->yes) {
            lv_obj_add_event_cb(rp->yes, on_rising_yes_click, LV_EVENT_CLICKED, NULL);
        }
        if (rp->no) {
            lv_obj_add_event_cb(rp->no, on_rising_no_click, LV_EVENT_CLICKED, NULL);
        }

        lv_obj_t *focus_rp = (g_rising_choice == 0 ? rp->no : rp->yes);
        if (focus_rp) lv_group_focus_obj(focus_rp);
    }
    current_group = g_risingpage;

    lv_scr_load_anim(risingpage_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_rising] rebuild: risingpage\n");
}

static void on_rising_yes_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    g_rising_choice = 1;
    jump_to_descriptionmenu();
}

static void on_rising_no_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    g_rising_choice = 0;
    jump_to_descriptionmenu();
}
