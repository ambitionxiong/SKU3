/*
 * nav_six_tz.c - 第六感-探针版菜单（sixmenutz / chickmenutz）
 *
 * 探针模式下按第六感功能键进入 sixmenutz（探针版第六感菜单）；
 * sixmenutz 的 chick → chickmenutz（探针版家禽菜单）；
 * chickmenutz 的 wholechicken/wholeduck → 走烤全鸡/烤全鸭完整探针流程
 *   （toastcolor → descriptionmenu → chickencooking）。
 * 而非探针模式的 chickenmenu/duckmenu 里，烤全鸡/烤全鸭仅跳探针提示页。
 */
#include "protocol.h"
#include "nav.h"

lv_group_t *g_sixmenutz = NULL;
lv_group_t *g_chickmenutz = NULL;

static void on_sixmenutz_chick_click(lv_event_t *e);
static void on_chickmenutz_wholechicken_click(lv_event_t *e);
static void on_chickmenutz_wholeduck_click(lv_event_t *e);

/* ================= sixmenutz（探针版第六感菜单） ================= */

void jump_to_sixmenutz(void)
{
    page_push(PAGE_SIXMENUTZ);
    lv_obj_clean(lv_scr_act());
    sixmenutz_create(&ui_manager);

    sixmenutz_t *mz = sixmenutz_get(&ui_manager);
    if (mz) {
        lv_obj_t *btns[] = { mz->chick, mz->meat };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_sixmenutz) lv_group_del(g_sixmenutz);
        g_sixmenutz = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (mz->chick) {
            lv_obj_add_event_cb(mz->chick, on_sixmenutz_chick_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(mz->chick);
        }
        /* meat 暂未接入：不绑事件（点击静默） */
    }
    current_group = g_sixmenutz;

    lang_scr_load_anim(sixmenutz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_tz] jump: sixmenutz\n");
}

void sixmenutz_rebuild(page_id_t child)
{
    if (g_sixmenutz) { lv_group_del(g_sixmenutz); g_sixmenutz = NULL; }
    lv_obj_clean(lv_scr_act());
    sixmenutz_create(&ui_manager);

    sixmenutz_t *mz = sixmenutz_get(&ui_manager);
    if (mz) {
        lv_obj_t *btns[] = { mz->chick, mz->meat };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_sixmenutz = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (mz->chick) {
            lv_obj_add_event_cb(mz->chick, on_sixmenutz_chick_click, LV_EVENT_CLICKED, NULL);
            /* 从家禽菜单返回时焦点回到鸡按钮 */
            lv_group_focus_obj(mz->chick);
        }
    }
    current_group = g_sixmenutz;

    lang_scr_load_anim(sixmenutz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_tz] rebuild: sixmenutz\n");
}

static void on_sixmenutz_chick_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_chickmenutz();
}

/* ================= chickmenutz（探针版家禽菜单） ================= */

void jump_to_chickmenutz(void)
{
    page_push(PAGE_CHICKMENUTZ);
    lv_obj_clean(lv_scr_act());
    chickmenutz_create(&ui_manager);

    chickmenutz_t *cm = chickmenutz_get(&ui_manager);
    if (cm) {
        lv_obj_t *btns[] = { cm->wholechicken, cm->wholeduck };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_chickmenutz) lv_group_del(g_chickmenutz);
        g_chickmenutz = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (cm->wholechicken) {
            lv_obj_add_event_cb(cm->wholechicken, on_chickmenutz_wholechicken_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(cm->wholechicken);
        }
        if (cm->wholeduck) {
            lv_obj_add_event_cb(cm->wholeduck, on_chickmenutz_wholeduck_click, LV_EVENT_CLICKED, NULL);
        }
    }
    current_group = g_chickmenutz;

    lang_scr_load_anim(chickmenutz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_tz] jump: chickmenutz\n");
}

void chickmenutz_rebuild(page_id_t child)
{
    if (g_chickmenutz) { lv_group_del(g_chickmenutz); g_chickmenutz = NULL; }
    lv_obj_clean(lv_scr_act());
    chickmenutz_create(&ui_manager);

    chickmenutz_t *cm = chickmenutz_get(&ui_manager);
    if (cm) {
        lv_obj_t *btns[] = { cm->wholechicken, cm->wholeduck };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_chickmenutz = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (cm->wholechicken)
            lv_obj_add_event_cb(cm->wholechicken, on_chickmenutz_wholechicken_click, LV_EVENT_CLICKED, NULL);
        if (cm->wholeduck)
            lv_obj_add_event_cb(cm->wholeduck, on_chickmenutz_wholeduck_click, LV_EVENT_CLICKED, NULL);

        /* 返回时恢复进入时的菜焦点（探针流程会设置 g_six_bread_type） */
        if (g_six_bread_type == SIX_CHICK_DUCK_WHOLE && cm->wholeduck)
            lv_group_focus_obj(cm->wholeduck);
        else if (cm->wholechicken)
            lv_group_focus_obj(cm->wholechicken);
    }
    current_group = g_chickmenutz;

    lang_scr_load_anim(chickmenutz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_tz] rebuild: chickmenutz\n");
}

/* 烤全鸡：探针模式必有探针 → 直接进 toastcolor(degree 组) 走完整流程 */
static void on_chickmenutz_wholechicken_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = SIX_CHICK_WHOLE;
    g_toast_mode = TOAST_MODE_DEGREE;
    jump_to_toastcolor();
}

/* 烤全鸭：同上 */
static void on_chickmenutz_wholeduck_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = SIX_CHICK_DUCK_WHOLE;
    g_toast_mode = TOAST_MODE_DEGREE;
    jump_to_toastcolor();
}
