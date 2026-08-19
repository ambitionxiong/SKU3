#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"

/* ==============================
 * 第六感-蛋糕菜单（cake6menu）
 * 无发酵、无烤色; 选菜后直接进 descriptionmenu → cooking → 完成
 * ============================== */

lv_group_t *g_cake6menu = NULL;

static void on_cake6menu_click(lv_event_t *e);

void jump_to_cake6menu(void)
{
    page_push(PAGE_CAKE6MENU);
    lv_obj_clean(lv_scr_act());
    cake6menu_create(&ui_manager);

    cake6menu_t *ck = cake6menu_get(&ui_manager);
    if (ck) {
        lv_obj_t *btns[] = {
            ck->swissroll, ck->strudel, ck->madeleine, ck->spongecake,
            ck->cupcake, ck->chocolatecake, ck->muffin,
            ck->creampuff, ck->eggtart, ck->millefeuille, ck->cookies,
        };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_cake6menu) lv_group_del(g_cake6menu);
        g_cake6menu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        /* 绑定 11 个蛋糕按钮, user_data = 菜类型 */
        if (ck->swissroll)     lv_obj_add_event_cb(ck->swissroll, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_SWISSROLL);
        if (ck->strudel)       lv_obj_add_event_cb(ck->strudel, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_STRUDEL);
        if (ck->muffin)        lv_obj_add_event_cb(ck->muffin, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_MUFFIN);
        if (ck->madeleine)     lv_obj_add_event_cb(ck->madeleine, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_MADELEINE);
        if (ck->spongecake)    lv_obj_add_event_cb(ck->spongecake, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_SPONGECAKE);
        if (ck->cupcake)       lv_obj_add_event_cb(ck->cupcake, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_CUPCAKE);
        if (ck->chocolatecake) lv_obj_add_event_cb(ck->chocolatecake, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_CHOCOLATECAKE);
        if (ck->creampuff)     lv_obj_add_event_cb(ck->creampuff, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_CREAMPUFF);
        if (ck->eggtart)       lv_obj_add_event_cb(ck->eggtart, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_EGGTART);
        if (ck->millefeuille)  lv_obj_add_event_cb(ck->millefeuille, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_MILLEFEUILLE);
        if (ck->cookies)       lv_obj_add_event_cb(ck->cookies, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_COOKIES);
        if (ck->swissroll)     lv_group_focus_obj(ck->swissroll);
    }
    current_group = g_cake6menu;

    lv_scr_load_anim(cake6menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixmenu] jump: cake6menu\n");
}

void cake6menu_rebuild(page_id_t child)
{
    if (g_cake6menu) { lv_group_del(g_cake6menu); g_cake6menu = NULL; }
    lv_obj_clean(lv_scr_act());
    cake6menu_create(&ui_manager);

    cake6menu_t *ck = cake6menu_get(&ui_manager);
    if (ck) {
        lv_obj_t *btns[] = {
            ck->swissroll, ck->strudel, ck->madeleine, ck->spongecake,
            ck->cupcake, ck->chocolatecake, ck->muffin,
            ck->creampuff, ck->eggtart, ck->millefeuille, ck->cookies,
        };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_cake6menu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        /* 绑定 11 个蛋糕按钮, user_data = 菜类型 */
        if (ck->swissroll)     lv_obj_add_event_cb(ck->swissroll, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_SWISSROLL);
        if (ck->strudel)       lv_obj_add_event_cb(ck->strudel, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_STRUDEL);
        if (ck->muffin)        lv_obj_add_event_cb(ck->muffin, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_MUFFIN);
        if (ck->madeleine)     lv_obj_add_event_cb(ck->madeleine, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_MADELEINE);
        if (ck->spongecake)    lv_obj_add_event_cb(ck->spongecake, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_SPONGECAKE);
        if (ck->cupcake)       lv_obj_add_event_cb(ck->cupcake, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_CUPCAKE);
        if (ck->chocolatecake) lv_obj_add_event_cb(ck->chocolatecake, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_CHOCOLATECAKE);
        if (ck->creampuff)     lv_obj_add_event_cb(ck->creampuff, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_CREAMPUFF);
        if (ck->eggtart)       lv_obj_add_event_cb(ck->eggtart, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_EGGTART);
        if (ck->millefeuille)  lv_obj_add_event_cb(ck->millefeuille, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_MILLEFEUILLE);
        if (ck->cookies)       lv_obj_add_event_cb(ck->cookies, on_cake6menu_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CAKE_COOKIES);
        /* 返回时恢复进入时的菜焦点 */
        lv_obj_t *focus_cake = NULL;
        switch (g_six_bread_type) {
        case SIX_CAKE_SWISSROLL:     focus_cake = ck->swissroll; break;
        case SIX_CAKE_STRUDEL:       focus_cake = ck->strudel; break;
        case SIX_CAKE_MUFFIN:        focus_cake = ck->muffin; break;
        case SIX_CAKE_MADELEINE:     focus_cake = ck->madeleine; break;
        case SIX_CAKE_SPONGECAKE:    focus_cake = ck->spongecake; break;
        case SIX_CAKE_CUPCAKE:       focus_cake = ck->cupcake; break;
        case SIX_CAKE_CHOCOLATECAKE: focus_cake = ck->chocolatecake; break;
        case SIX_CAKE_CREAMPUFF:     focus_cake = ck->creampuff; break;
        case SIX_CAKE_EGGTART:       focus_cake = ck->eggtart; break;
        case SIX_CAKE_MILLEFEUILLE:  focus_cake = ck->millefeuille; break;
        case SIX_CAKE_COOKIES:       focus_cake = ck->cookies; break;
        default:                     focus_cake = ck->swissroll; break;
        }
        if (focus_cake) lv_group_focus_obj(focus_cake);
    }
    current_group = g_cake6menu;

    lv_scr_load_anim(cake6menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixmenu] rebuild: cake6menu\n");
}

static void on_cake6menu_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = (uint8_t)(intptr_t)lv_event_get_user_data(e);
    jump_to_descriptionmenu();   /* 蛋糕无发酵,直接进 description */
}
