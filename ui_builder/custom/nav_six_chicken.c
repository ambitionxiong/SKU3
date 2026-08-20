/*
 * nav_six_chicken.c - 第六感-家禽/鸡
 *
 * 链路：sixmenu → (chick) → chick6menu → (chicken) → chickenmenu → 烤鸡菜谱
 * 本文件当前只做菜单链路（sixmenu→chick6menu→chickenmenu）；
 * 菜谱选择（烤全鸡等）与 chickencooking 烹饪页在后续步骤接入。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"

lv_group_t *g_chick6menu = NULL;
lv_group_t *g_chickenmenu = NULL;

static void on_chick6menu_chicken_click(lv_event_t *e);

/* ================= chick6menu（家禽：鸡/鸭） ================= */

void jump_to_chick6menu(void)
{
    page_push(PAGE_CHICK6MENU);
    lv_obj_clean(lv_scr_act());
    chick6menu_create(&ui_manager);

    chick6menu_t *cm = chick6menu_get(&ui_manager);
    if (cm) {
        lv_obj_t *btns[] = { cm->chicken, cm->duck };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_chick6menu) lv_group_del(g_chick6menu);
        g_chick6menu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (cm->chicken) {
            lv_obj_add_event_cb(cm->chicken, on_chick6menu_chicken_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(cm->chicken);
        }
        /* duck 暂未接入：不绑事件（点击静默） */
    }
    current_group = g_chick6menu;

    lv_scr_load_anim(chick6menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] jump: chick6menu\n");
}

void chick6menu_rebuild(page_id_t child)
{
    if (g_chick6menu) { lv_group_del(g_chick6menu); g_chick6menu = NULL; }
    lv_obj_clean(lv_scr_act());
    chick6menu_create(&ui_manager);

    chick6menu_t *cm = chick6menu_get(&ui_manager);
    if (cm) {
        lv_obj_t *btns[] = { cm->chicken, cm->duck };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_chick6menu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (cm->chicken) {
            lv_obj_add_event_cb(cm->chicken, on_chick6menu_chicken_click, LV_EVENT_CLICKED, NULL);
            /* 从 chickenmenu 返回时焦点回到鸡按钮 */
            lv_group_focus_obj(cm->chicken);
        }
    }
    current_group = g_chick6menu;

    lv_scr_load_anim(chick6menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] rebuild: chick6menu (child=%d)\n", (int)child);
}

static void on_chick6menu_chicken_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_chickenmenu();
}

/* ================= chickenmenu（鸡：烤全鸡/烤鸡翅/炸鸡翅/炸鸡腿/烤鸡胸） ================= */

void jump_to_chickenmenu(void)
{
    page_push(PAGE_CHICKENMENU);
    lv_obj_clean(lv_scr_act());
    chickenmenu_create(&ui_manager);

    chickenmenu_t *cm = chickenmenu_get(&ui_manager);
    if (cm) {
        lv_obj_t *btns[] = {
            cm->wholechicken, cm->grillchickenwing, cm->friedchickenwing,
            cm->friedchickenleg, cm->grillchickenbreast,
        };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_chickenmenu) lv_group_del(g_chickenmenu);
        g_chickenmenu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        /* 菜谱事件逻辑后续步骤接入；当前仅建立焦点顺序 */
        if (cm->wholechicken) lv_group_focus_obj(cm->wholechicken);
    }
    current_group = g_chickenmenu;

    lv_scr_load_anim(chickenmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] jump: chickenmenu\n");
}

void chickenmenu_rebuild(page_id_t child)
{
    if (g_chickenmenu) { lv_group_del(g_chickenmenu); g_chickenmenu = NULL; }
    lv_obj_clean(lv_scr_act());
    chickenmenu_create(&ui_manager);

    chickenmenu_t *cm = chickenmenu_get(&ui_manager);
    if (cm) {
        lv_obj_t *btns[] = {
            cm->wholechicken, cm->grillchickenwing, cm->friedchickenwing,
            cm->friedchickenleg, cm->grillchickenbreast,
        };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_chickenmenu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        /* 菜谱选择后续接入前，统一焦点到烤全鸡 */
        if (cm->wholechicken) lv_group_focus_obj(cm->wholechicken);
    }
    current_group = g_chickenmenu;

    lv_scr_load_anim(chickenmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] rebuild: chickenmenu (child=%d)\n", (int)child);
}