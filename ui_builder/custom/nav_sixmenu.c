/*
 * nav_sixmenu.c - 第六感菜单
 * KEY2 进入。面包/蛋糕两个子入口，面包走发酵判断页(rising)，
 * 蛋糕直接走烤色。菜单重置页面栈后进入。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"

/* ==============================
 * 第六感菜单流程
 * 按键2(KEY_SIXMENU) → sixmenu → bread6menu → risingpage → descriptionmenu
 * KEY_CLEAN 式入口（重置栈，BACK 逐级返回，最终回待机页）
 * ============================== */

lv_group_t *g_sixmenu = NULL;
lv_group_t *g_bread6menu = NULL;

static int s_six_meat_mode = 0;   /* 肉菜单复用 bread6menu 页面标记 */
static int s_six_meat_kind = 0;   /* 肉菜单进入的子类: 0牛肉 1羊肉 2猪肉 3肉菜(返回焦点恢复用) */
int six_menu_get_meat_kind(void)   { return s_six_meat_kind; }   /* bread6menu 复用肉菜单:tune 排版分支用 */
int six_menu_is_meat_mode(void)    { return s_six_meat_mode; }

static void on_sixmenu_bread_click(lv_event_t *e);
static void on_sixmenu_cake_click(lv_event_t *e);
static void on_sixmenu_chick_click(lv_event_t *e);
static void on_sixmenu_vegetable_click(lv_event_t *e);
static void on_sixmenu_meat_click(lv_event_t *e);
static void on_sixmenu_fish_click(lv_event_t *e);
static void on_sixmenu_pizza_click(lv_event_t *e);
static void on_sixmenu_pasta_click(lv_event_t *e);
static void on_sixmenu_snack_click(lv_event_t *e);
static void on_bread6menu_breadroll_click(lv_event_t *e);
static void on_bread6menu_wheat_click(lv_event_t *e);
static void on_bread6menu_toast_click(lv_event_t *e);
static void on_bread6menu_croissant_click(lv_event_t *e);
static void jump_to_meat6menu(void);
static void meat6menu_apply_labels(bread6menu_t *b6);
static void on_meat6menu_beef_click(lv_event_t *e);
static void on_meat6menu_mutton_click(lv_event_t *e);
static void on_meat6menu_pork_click(lv_event_t *e);
static void on_meat6menu_meatdish_click(lv_event_t *e);

/* ================= sixmenu ================= */

void jump_to_sixmenu(void)
{
    page_push(PAGE_SIXMENU);
    lv_obj_clean(lv_scr_act());
    sixmenu_create(&ui_manager);

    sixmenu_t *sm = sixmenu_get(&ui_manager);
    if (sm) {
        lv_obj_t *btns[] = {
            sm->bread, sm->cake, sm->chick, sm->meat, sm->fish,
            sm->vegetable, sm->pizza6, sm->pasta, sm->snack,
        };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_sixmenu) lv_group_del(g_sixmenu);
        g_sixmenu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (sm->bread) {
            lv_obj_add_event_cb(sm->bread, on_sixmenu_bread_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(sm->bread);
        }
        if (sm->cake) {
            lv_obj_add_event_cb(sm->cake, on_sixmenu_cake_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->vegetable) {
            lv_obj_add_event_cb(sm->vegetable, on_sixmenu_vegetable_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->chick) {
            lv_obj_add_event_cb(sm->chick, on_sixmenu_chick_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->meat) {
            lv_obj_add_event_cb(sm->meat, on_sixmenu_meat_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->fish) {
            lv_obj_add_event_cb(sm->fish, on_sixmenu_fish_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->pizza6) {
            lv_obj_add_event_cb(sm->pizza6, on_sixmenu_pizza_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->pasta) {
            lv_obj_add_event_cb(sm->pasta, on_sixmenu_pasta_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->snack) {
            lv_obj_add_event_cb(sm->snack, on_sixmenu_snack_click, LV_EVENT_CLICKED, NULL);
        }
        /* 其余按钮：功能未实现，不绑事件（点击静默无效） */
    }
    current_group = g_sixmenu;

    lang_scr_load_anim(sixmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixmenu] jump: enter sixmenu\n");
}

void sixmenu_rebuild(page_id_t child)
{
    if (g_sixmenu) { lv_group_del(g_sixmenu); g_sixmenu = NULL; }
    lv_obj_clean(lv_scr_act());
    sixmenu_create(&ui_manager);

    sixmenu_t *sm = sixmenu_get(&ui_manager);
    if (sm) {
        lv_obj_t *btns[] = {
            sm->bread, sm->cake, sm->chick, sm->meat, sm->fish,
            sm->vegetable, sm->pizza6, sm->pasta, sm->snack,
        };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_sixmenu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (sm->bread) {
            lv_obj_add_event_cb(sm->bread, on_sixmenu_bread_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->cake) {
            lv_obj_add_event_cb(sm->cake, on_sixmenu_cake_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->vegetable) {
            lv_obj_add_event_cb(sm->vegetable, on_sixmenu_vegetable_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->chick) {
            lv_obj_add_event_cb(sm->chick, on_sixmenu_chick_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->meat) {
            lv_obj_add_event_cb(sm->meat, on_sixmenu_meat_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->fish) {
            lv_obj_add_event_cb(sm->fish, on_sixmenu_fish_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->pizza6) {
            lv_obj_add_event_cb(sm->pizza6, on_sixmenu_pizza_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->pasta) {
            lv_obj_add_event_cb(sm->pasta, on_sixmenu_pasta_click, LV_EVENT_CLICKED, NULL);
        }
        if (sm->snack) {
            lv_obj_add_event_cb(sm->snack, on_sixmenu_snack_click, LV_EVENT_CLICKED, NULL);
        }

        /* 焦点恢复：按 child + 模式恢复到进入时的按钮 */
        if (s_six_meat_mode && child == PAGE_BREAD6MENU && sm->meat)
            lv_group_focus_obj(sm->meat);
        else if (six_chick_get_fish_mode() && child == PAGE_CHICK6MENU && sm->fish)
            lv_group_focus_obj(sm->fish);      /* 从鱼/海鲜菜单返回 */
        else if (six_chick_get_vegetable_mode() && child == PAGE_CHICK6MENU && sm->vegetable)
            lv_group_focus_obj(sm->vegetable); /* 从蔬菜/配菜菜单返回 */
        else if (six_chick_get_pasta_mode() && child == PAGE_CHICK6MENU && sm->pasta)
            lv_group_focus_obj(sm->pasta);     /* 从砂锅菜/烤意面菜单返回 */
        else if (six_chick_get_snack_mode() && child == PAGE_DUCK6MENU && sm->snack)
            lv_group_focus_obj(sm->snack);     /* 从零食子页返回 */
        else if (!six_chick_get_fish_mode() && child == PAGE_CHICK6MENU && sm->chick)
            lv_group_focus_obj(sm->chick);     /* 从鸡菜单返回 */
        else if (child == PAGE_CAKE6MENU && sm->cake)
            lv_group_focus_obj(sm->cake);
        else if (g_six_bread_type == SIX_PIZZA && child == PAGE_DESCRIPTIONMENU && sm->pizza6)
            lv_group_focus_obj(sm->pizza6);      /* 从披萨描述页返回 */
        else if (sm->bread)
            lv_group_focus_obj(sm->bread);
    }
    current_group = g_sixmenu;

    lang_scr_load_anim(sixmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixmenu] rebuild: child=%d\n", (int)child);
}

static void on_sixmenu_bread_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_bread6menu();
}

static void on_sixmenu_cake_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_cake6menu();
}

/* 鸡入口：sixmenu → chick6menu */
static void on_sixmenu_chick_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    six_chick_reset_fish_mode();   /* 重置鱼模式，进鸡流程 */
    jump_to_chick6menu();
}

/* 蔬菜入口：sixmenu → vegetable (复用 chick6menu 页面) */
static void on_sixmenu_vegetable_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    six_chick_reset_fish_mode();   /* 重置鱼/海鲜等模式,进蔬菜流程 */
    jump_to_vegetable_menu();
}

/* 肉入口：sixmenu → meat (复用 bread6menu 页面) */
static void on_sixmenu_meat_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    six_chick_reset_fish_mode();
    jump_to_meat6menu();
}

/* 鱼/海鲜入口 */
static void on_sixmenu_fish_click(lv_event_t *e)
{
    (void)e;
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_fish_menu();
}

/* 披萨:固定参数菜,点击直接进描述页(无菜单/份量/烤色) */
static void on_sixmenu_pizza_click(lv_event_t *e)
{
    (void)e;
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = SIX_PIZZA;
    jump_to_descriptionmenu();
}

/* 砂锅菜/烤意面:复用 chick6menu(千层面/卡内罗尼,二维菜) */
static void on_sixmenu_pasta_click(lv_event_t *e)
{
    (void)e;
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_pasta_menu();
}

/* 零食:复用 duckmenu(炸鸡米花,份量驱动) */
static void on_sixmenu_snack_click(lv_event_t *e)
{
    (void)e;
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_snack_menu();
}

/* 肉菜单：复用 bread6menu 页面,换文字 */
static void meat6menu_apply_labels(bread6menu_t *b6)
{
    if (!b6) return;
    if (b6->label_11) lv_label_set_text(b6->label_11, tr("肉"));
    if (b6->breadroll)   lv_label_set_text(lv_obj_get_child(b6->breadroll, 0), tr("牛肉"));
    if (b6->wheat_bread) lv_label_set_text(lv_obj_get_child(b6->wheat_bread, 0), tr("羊肉"));
    if (b6->toast)       lv_label_set_text(lv_obj_get_child(b6->toast, 0), tr("猪肉"));
    if (b6->croissant)   lv_label_set_text(lv_obj_get_child(b6->croissant, 0), tr("肉菜"));
}

/* 肉菜单按钮点击：牛肉/羊肉/猪肉进六选项3页；肉菜进肉菜页 */
static void on_meat6menu_beef_click(lv_event_t *e)
{
    (void)e;
    s_six_meat_kind = 0;
    jump_to_sixop3page(tr("牛肉"), tr("烤牛排"), tr("炸牛排"), tr("烤牛肉"),  (1 | 4), SIX_OP3_KIND_BEEF);   /* (1|4)=101: 显示 probe1/3, 隐藏 probe2 */
}
static void on_meat6menu_mutton_click(lv_event_t *e)
{
    (void)e;
    s_six_meat_kind = 1;
    jump_to_sixop3page(tr("羊肉"), tr("烤羊腿"), tr("烤羊排"), tr("烤羊肉串"),  (1 | 2), SIX_OP3_KIND_MUTTON);   /* (1|2)=011: 显示 probe1/2, 隐藏 probe3 */
}
static void on_meat6menu_pork_click(lv_event_t *e)
{
    (void)e;
    s_six_meat_kind = 2;
    jump_to_sixop3page(tr("猪肉"), tr("烤猪里脊肉"), tr("烤五花肉"), tr("炸排骨"),  (1 | 2), SIX_OP3_KIND_PORK);  /* (1|2)=011: 显示 probe1/2, 隐藏 probe3 */
}
static void on_meat6menu_meatdish_click(lv_event_t *e)
{
    (void)e;
    s_six_meat_kind = 3;
    jump_to_meatdish_menu();
}

static void jump_to_meat6menu(void)
{
    s_six_meat_mode = 1;
    s_six_meat_kind = 0;   /* 新进入默认牛肉 */
    page_push(PAGE_BREAD6MENU);
    lv_obj_clean(lv_scr_act());
    bread6menu_create(&ui_manager);

    bread6menu_t *b6 = bread6menu_get(&ui_manager);
    if (b6) {
        meat6menu_apply_labels(b6);

        lv_obj_t *btns[] = {
            b6->breadroll, b6->wheat_bread, b6->toast, b6->croissant,
        };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_bread6menu) lv_group_del(g_bread6menu);
        g_bread6menu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (b6->breadroll)   lv_obj_add_event_cb(b6->breadroll,   on_meat6menu_beef_click,    LV_EVENT_CLICKED, NULL);
        if (b6->wheat_bread) lv_obj_add_event_cb(b6->wheat_bread, on_meat6menu_mutton_click, LV_EVENT_CLICKED, NULL);
        if (b6->toast)       lv_obj_add_event_cb(b6->toast,       on_meat6menu_pork_click,   LV_EVENT_CLICKED, NULL);
        if (b6->croissant)   lv_obj_add_event_cb(b6->croissant,   on_meat6menu_meatdish_click, LV_EVENT_CLICKED, NULL);
        lv_group_focus_obj(b6->breadroll);
    }
    current_group = g_bread6menu;

    lang_scr_load_anim(bread6menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixmenu] jump: meat6menu (reuse bread6menu page)\n");
}

/* ================= bread6menu ================= */

void jump_to_bread6menu(void)
{
    s_six_meat_mode = 0;
    page_push(PAGE_BREAD6MENU);
    lv_obj_clean(lv_scr_act());
    bread6menu_create(&ui_manager);

    bread6menu_t *b6 = bread6menu_get(&ui_manager);
    if (b6) {
        lv_obj_t *btns[] = {
            b6->breadroll, b6->wheat_bread, b6->toast, b6->croissant,
        };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_bread6menu) lv_group_del(g_bread6menu);
        g_bread6menu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (b6->breadroll) {
            lv_obj_add_event_cb(b6->breadroll, on_bread6menu_breadroll_click, LV_EVENT_CLICKED, NULL);
        }
        if (b6->wheat_bread) {
            lv_obj_add_event_cb(b6->wheat_bread, on_bread6menu_wheat_click, LV_EVENT_CLICKED, NULL);
        }
        if (b6->toast) {
            lv_obj_add_event_cb(b6->toast, on_bread6menu_toast_click, LV_EVENT_CLICKED, NULL);
        }
        if (b6->croissant) {
            lv_obj_add_event_cb(b6->croissant, on_bread6menu_croissant_click, LV_EVENT_CLICKED, NULL);
        }
        lv_group_focus_obj(b6->breadroll);
    }
    current_group = g_bread6menu;

    lang_scr_load_anim(bread6menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixmenu] jump: bread6menu\n");
}

void bread6menu_rebuild(page_id_t child)
{
    if (g_bread6menu) { lv_group_del(g_bread6menu); g_bread6menu = NULL; }
    lv_obj_clean(lv_scr_act());
    bread6menu_create(&ui_manager);

    bread6menu_t *b6 = bread6menu_get(&ui_manager);
    if (b6) {
        lv_obj_t *btns[] = {
            b6->breadroll, b6->wheat_bread, b6->toast, b6->croissant,
        };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_bread6menu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (s_six_meat_mode) {
            /* 肉菜单重建：换文字+肉事件 */
            meat6menu_apply_labels(b6);
            if (b6->breadroll)   lv_obj_add_event_cb(b6->breadroll,   on_meat6menu_beef_click,    LV_EVENT_CLICKED, NULL);
            if (b6->wheat_bread) lv_obj_add_event_cb(b6->wheat_bread, on_meat6menu_mutton_click, LV_EVENT_CLICKED, NULL);
            if (b6->toast)       lv_obj_add_event_cb(b6->toast,       on_meat6menu_pork_click,   LV_EVENT_CLICKED, NULL);
            if (b6->croissant)   lv_obj_add_event_cb(b6->croissant,   on_meat6menu_meatdish_click, LV_EVENT_CLICKED, NULL);
            /* 返回时恢复进入的肉类按钮焦点 */
            if (s_six_meat_kind == 1 && b6->wheat_bread)
                lv_group_focus_obj(b6->wheat_bread);   /* 羊肉 */
            else if (s_six_meat_kind == 2 && b6->toast)
                lv_group_focus_obj(b6->toast);         /* 猪肉 */
            else if (s_six_meat_kind == 3 && b6->croissant)
                lv_group_focus_obj(b6->croissant);     /* 肉菜 */
            else if (b6->breadroll)
                lv_group_focus_obj(b6->breadroll);     /* 牛肉 */
        } else {
            if (b6->breadroll)   lv_obj_add_event_cb(b6->breadroll,   on_bread6menu_breadroll_click, LV_EVENT_CLICKED, NULL);
            if (b6->wheat_bread) lv_obj_add_event_cb(b6->wheat_bread, on_bread6menu_wheat_click,     LV_EVENT_CLICKED, NULL);
            if (b6->toast)       lv_obj_add_event_cb(b6->toast,       on_bread6menu_toast_click,     LV_EVENT_CLICKED, NULL);
            if (b6->croissant)   lv_obj_add_event_cb(b6->croissant,   on_bread6menu_croissant_click, LV_EVENT_CLICKED, NULL);
            /* 返回时恢复进入时的菜焦点 */
            if (g_six_bread_type == SIX_BREAD_WHEAT && b6->wheat_bread)
                lv_group_focus_obj(b6->wheat_bread);
            else if (g_six_bread_type == SIX_BREAD_TOAST && b6->toast)
                lv_group_focus_obj(b6->toast);
            else if (g_six_bread_type == SIX_BREAD_CROISSANT && b6->croissant)
                lv_group_focus_obj(b6->croissant);
            else if (b6->breadroll)
                lv_group_focus_obj(b6->breadroll);
        }
    }
    current_group = g_bread6menu;

    lang_scr_load_anim(bread6menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixmenu] rebuild: bread6menu (meat=%d)\n", s_six_meat_mode);
}

static void on_bread6menu_breadroll_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = SIX_BREAD_ROLL;
    jump_to_risingpage();
}

static void on_bread6menu_wheat_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = SIX_BREAD_WHEAT;
    jump_to_risingpage();
}

static void on_bread6menu_toast_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = SIX_BREAD_TOAST;
    jump_to_risingpage();
}

static void on_bread6menu_croissant_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = SIX_BREAD_CROISSANT;
    jump_to_risingpage();
}

#if 0 /* 已拆分到 nav_six_rising.c / nav_six_desc.c */

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

    lang_scr_load_anim(risingpage_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixmenu] jump: risingpage\n");
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

    lang_scr_load_anim(risingpage_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixmenu] rebuild: risingpage\n");
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

/* ================= descriptionmenu ================= */

static void descriptionmenu_layout(descriptionmenu_t *dm)
{
    if (!dm || !dm->container_1) return;

    /* 左上角菜名 */
    if (dm->label_19)
        lv_label_set_text(dm->label_19, six_bread_name());

    /* 摘要文本按是否发酵选择 */
    if (dm->summary)
        lv_label_set_text(dm->summary, g_rising_choice == 1 ?
                          tr("小结：\n有发酵阶段\n") : tr("小结：\n没有发酵阶段\n"));

    /* 烹饪时间 + 烹饪说明按当前菜填充 */
    if (dm->cooktime) {
        lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), six_bread_cook_min());
    }
    if (dm->cookdescriptin) {
        lv_label_set_text(dm->cookdescriptin, six_bread_desc());
    }

    /* 容器 Flex 竖排：左/上边距 24，垂直间距 16 */
    lv_obj_set_flex_flow(dm->container_1, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_left(dm->container_1, 24, 0);
    lv_obj_set_style_pad_top(dm->container_1, 24, 0);
    lv_obj_set_style_pad_row(dm->container_1, 16, 0);

    /* Flex 按子对象顺序排列:调整顺序为 cooktime → cookdescriptin → summary */
    if (dm->cooktime) lv_obj_move_to_index(dm->cooktime, 0);
    if (dm->cookdescriptin) lv_obj_move_to_index(dm->cookdescriptin, 1);
    /* summary 自然落位 idx 2,不动 */

    /* 三标签：高度按内容自适应 + 行高对齐字号(30 号字体 line_height=35,负 5 对齐上位机) */
    lv_obj_t *labels[] = { dm->cooktime, dm->cookdescriptin, dm->summary };
    for (int k = 0; k < 3; k++) {
        if (!labels[k]) continue;
        lv_obj_set_height(labels[k], LV_SIZE_CONTENT);
        lv_obj_set_style_text_line_space(labels[k], -5, 0);
    }
    lv_obj_update_layout(dm->container_1);
}

void jump_to_descriptionmenu(void)
{
    page_push(PAGE_DESCRIPTIONMENU);
    lv_obj_clean(lv_scr_act());
    descriptionmenu_create(&ui_manager);

    descriptionmenu_t *dm = descriptionmenu_get(&ui_manager);
    if (dm) {
        descriptionmenu_layout(dm);

        lv_obj_t *btns[] = { dm->start, dm->delay };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_descriptionmenu) lv_group_del(g_descriptionmenu);
        g_descriptionmenu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (dm->start) {
            lv_obj_add_event_cb(dm->start, on_description_start_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(dm->start);
        }
        if (dm->delay) {
            lv_obj_add_event_cb(dm->delay, on_description_delay_click, LV_EVENT_CLICKED, NULL);
        }
    }
    current_group = g_descriptionmenu;

    lang_scr_load_anim(descriptionmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixmenu] jump: descriptionmenu (choice=%d)\n", g_rising_choice);
}

void descriptionmenu_rebuild(page_id_t child)
{
    if (g_descriptionmenu) { lv_group_del(g_descriptionmenu); g_descriptionmenu = NULL; }
    lv_obj_clean(lv_scr_act());
    descriptionmenu_create(&ui_manager);

    descriptionmenu_t *dm = descriptionmenu_get(&ui_manager);
    if (dm) {
        descriptionmenu_layout(dm);

        lv_obj_t *btns[] = { dm->start, dm->delay };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_descriptionmenu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (dm->start) {
            lv_obj_add_event_cb(dm->start, on_description_start_click, LV_EVENT_CLICKED, NULL);
        }
        if (dm->delay) {
            lv_obj_add_event_cb(dm->delay, on_description_delay_click, LV_EVENT_CLICKED, NULL);
        }
        /* 从延迟设置返回时恢复焦点到“延迟”，否则默认“开始” */
        if (child == PAGE_DELAYSET && dm->delay)
            lv_group_focus_obj(dm->delay);
        else if (dm->start)
            lv_group_focus_obj(dm->start);
    }
    current_group = g_descriptionmenu;

    lang_scr_load_anim(descriptionmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixmenu] rebuild: descriptionmenu\n");
}

static void on_description_start_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_six_cooking();
}

static void on_description_delay_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_delayset();   /* 来源页自动记录为 PAGE_DESCRIPTIONMENU */
}
#endif /* 已拆分到 nav_six_rising.c / nav_six_desc.c */
