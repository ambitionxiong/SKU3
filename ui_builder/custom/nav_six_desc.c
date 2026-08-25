/*
 * nav_six_desc.c - 第六感-烹饪描述页
 * 显示配方描述/温度时间，确认进入六感运行页。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"

/* ==============================
 * 第六感-烹饪说明页（descriptionmenu）
 * 菜单流程: risingpage → descriptionmenu → 开始/延迟
 * ============================== */

lv_group_t *g_descriptionmenu = NULL;

static void on_description_start_click(lv_event_t *e);
static void on_description_delay_click(lv_event_t *e);

static void descriptionmenu_layout(descriptionmenu_t *dm)
{
    if (!dm || !dm->container_1) return;

    /* 左上角菜名 */
    if (dm->label_19)
        lv_label_set_text(dm->label_19, six_current_name());

    /* 摘要：探针菜/程度→时间显示烧烤程度；烤鸡翅类显示份量；面包有发酵显示发酵小结；其余隐藏 */
    if (dm->summary) {
        if (six_chick_is_probe()) {
            lv_obj_clear_flag(dm->summary, LV_OBJ_FLAG_HIDDEN);
            if (six_chick_is_matdeg())
                lv_label_set_text_fmt(dm->summary, tr("小结：\n熟度：%s\n烧烤程度：%s\n"),
                                      six_2d_mat_text(), six_2d_deg_text());   /* 牛肉/羊腿/羊排:熟度+程度 */
            else if (g_six_bread_type == SIX_MEAT_GRILL_STEAK)
                lv_label_set_text_fmt(dm->summary, tr("小结：\n熟度：%s\n"),
                                      six_chick_degree_short());                     /* 烤牛排:熟度 */
            else
                lv_label_set_text_fmt(dm->summary, tr("小结：\n烧烤程度：%s\n"), six_chick_degree_short());   /* 烤全鸡/鸭 */
        } else if (six_chick_is_degree_time()) {
            int d = toastcolor_degree_value();
            const char *dt = (d == 1) ? tr("浅") : (d == 3) ? tr("深") : tr("中等");
            lv_obj_clear_flag(dm->summary, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->summary, tr("小结：\n烧烤程度：%s\n"), dt);
        } else if (six_chick_is_kind()) {
            int w = toastcolor_weight_value();
            if (w < 0) w = 800;   /* 兜底 */
            lv_obj_clear_flag(dm->summary, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->summary, tr("小结：\n份量/种类：%d%s\n"), w, toastcolor_weight_unit());
        } else if (six_chick_is_pizza()) {
            lv_obj_clear_flag(dm->summary, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(dm->summary, tr("小结：\n份量/种类：1份\n"));   /* 披萨:固定1份 */
        } else if (six_chick_is_2d()) {
            lv_obj_clear_flag(dm->summary, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->summary, tr("小结：\n份量/种类：%dg\n烧烤程度：%s\n"),
                                  six_2d_weight(), six_2d_deg_text());   /* 二维菜:份量+程度 */
        } else if (six_bread_has_rising()) {
            lv_label_set_text(dm->summary, g_rising_choice == 1 ?
                              tr("小结：\n有发酵阶段\n") : tr("小结：\n没有发酵阶段\n"));
        } else {
            lv_obj_add_flag(dm->summary, LV_OBJ_FLAG_HIDDEN);
        }
    }

    /* 烹饪时间：探针菜不显示；程度→时间/份量按各自时间显示；其余原逻辑 */
    if (dm->cooktime) {
        if (six_chick_is_probe()) {
            lv_obj_add_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
        } else if (six_chick_is_degree_time()) {
            int d = toastcolor_degree_value();
            if (d < 1 || d > 3) d = 2;
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), six_chick_degree_min(d));
        } else if (six_chick_is_kind()) {
            int w = toastcolor_weight_value();
            if (w < 0) w = 800;
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), six_chick_cook_min(w));
        } else if (six_chick_is_seafood()) {
            const seafood_dish_t *sd = seafood_dish_cfg();
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), sd ? sd->cook_min : 18);

        } else if (six_chick_is_veg()) {
            const seafood_dish_t *sd = veg_fixed_cfg();
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), sd ? sd->cook_min : 22);
        } else if (six_chick_is_pizza()) {
            const seafood_dish_t *sd = pizza_fixed_cfg();
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), sd ? sd->cook_min : 23);

        } else if (six_chick_is_2d()) {
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), six_2d_cook_min());   /* 二维菜 */
        } else {
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            /* 选了发酵:发酵分钟+烹饪分钟分开显示 */
            if (six_bread_has_rising() && g_rising_choice == 1)
                lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：发酵%d分钟+烹饪%d分钟"),
                                      six_rising_min(), six_bread_cook_min());
            else
                lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), six_bread_cook_min());
        }
    }
    if (dm->cookdescriptin) {
        lv_label_set_text(dm->cookdescriptin, six_current_desc());
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
    /* 探针菜:隐藏时间后，说明与小结整体上移顶对齐（隐藏项不占 flex 位） */
    if (six_chick_is_probe()) {
        if (dm->cookdescriptin) lv_obj_move_to_index(dm->cookdescriptin, 0);
        if (dm->summary) lv_obj_move_to_index(dm->summary, 1);
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
    printf("[six_desc] jump: descriptionmenu (choice=%d)\n", g_rising_choice);
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
    printf("[six_desc] rebuild: descriptionmenu\n");
}

static void on_description_start_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    if (six_chick_is_probe())
        jump_to_chick_cooking();   /* 探针菜(烤全鸡/烤全鸭):探针温度驱动烹饪页 */
    else
        jump_to_six_cooking();
}

static void on_description_delay_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_delayset();   /* 来源页自动记录为 PAGE_DESCRIPTIONMENU */
}
