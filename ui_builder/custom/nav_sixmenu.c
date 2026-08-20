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

static void on_sixmenu_bread_click(lv_event_t *e);
static void on_sixmenu_cake_click(lv_event_t *e);
static void on_bread6menu_breadroll_click(lv_event_t *e);
static void on_bread6menu_wheat_click(lv_event_t *e);
static void on_bread6menu_toast_click(lv_event_t *e);
static void on_bread6menu_croissant_click(lv_event_t *e);

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
        /* 其余按钮：功能未实现，不绑事件（点击静默无效） */
    }
    current_group = g_sixmenu;

    lv_scr_load_anim(sixmenu_get(&ui_manager)->obj,
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
            /* 从 bread6menu 返回时焦点回到面包按钮 */
            lv_group_focus_obj(sm->bread);
        }
        if (sm->cake) {
            lv_obj_add_event_cb(sm->cake, on_sixmenu_cake_click, LV_EVENT_CLICKED, NULL);
        }
    }
    current_group = g_sixmenu;

    lv_scr_load_anim(sixmenu_get(&ui_manager)->obj,
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

/* ================= bread6menu ================= */

void jump_to_bread6menu(void)
{
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

    lv_scr_load_anim(bread6menu_get(&ui_manager)->obj,
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
    current_group = g_bread6menu;

    lv_scr_load_anim(bread6menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixmenu] rebuild: bread6menu\n");
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

    lv_scr_load_anim(risingpage_get(&ui_manager)->obj,
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

    lv_scr_load_anim(risingpage_get(&ui_manager)->obj,
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
                          "小结：\n有发酵阶段\n" : "小结：\n没有发酵阶段\n");

    /* 烹饪时间 + 烹饪说明按当前菜填充 */
    if (dm->cooktime) {
        lv_label_set_text_fmt(dm->cooktime, "预计烹饪时间：%d分钟", six_bread_cook_min());
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

    lv_scr_load_anim(descriptionmenu_get(&ui_manager)->obj,
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

    lv_scr_load_anim(descriptionmenu_get(&ui_manager)->obj,
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
