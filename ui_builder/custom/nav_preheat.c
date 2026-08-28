/*
 * nav_preheat.c - 快速预热模式 完整流程
 *
 * 页面流程：菜单(menu) → 设置(set) → 烹饪(cooking) → 暂停(stop) → 确认退出(stop_back) → 完成(complete)
 *   cooking 页可点设置按钮进入 setting 页(运行时改参数)。
 * 模式标识：MODE_PREHEAT，通讯/图标/描述均按此分发。
 * 支持：延迟预约(ondelay)、保温(contain)、预热(preheat)入口与聚焦移动。
 */
#include "nav.h"
#include "protocol.h"

static uint8_t g_preheat_solo;   /* 单独进入预热(来源 preheat_menu)标志 */
static uint8_t g_preheat_fast = 0;   /* 按键4快速预热入口标志 */

// 预热进度：以进入 cooking 时的起始腔温为 13 基准，线性映射到目标温度 100
static int preheat_progress(void)
{
    uint16_t cavity = get_cavity_temp();
    int range = set_temp - preheat_start_cavity;
    if (range <= 0) return 100;
    int p = 13 + (int)((int64_t)87 * (cavity - preheat_start_cavity) / range);
    if (p < 13) p = 13;
    if (p > 100) p = 100;
    return p;
}

// 同步 bar 值 + bartemp 位置/文字（保持 bartemp 与指示器右边缘 80px 间距，文字 cap 到设置值）
static void preheat_update_bar(lv_obj_t *bar, lv_obj_t *bartemp)
{
    int p = preheat_progress();
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, p, LV_ANIM_OFF);
    if (bartemp) {
        uint16_t cavity = get_cavity_temp();
        int disp = cavity > set_temp ? set_temp : cavity;
        lv_label_set_text_fmt(bartemp, tr("%d℃"), disp);
        int bx = 122 + (637 * p) / 100 - 80 + lang_dyn_dx();
        lv_obj_set_pos(bartemp, bx, 323 + lang_dyn_dy());
    }
}

// 按来源模式设置预热页的 icon 和 status（模式自己的 icon + 模式名，无温度）
static void preheat_apply_mode_ui(lv_obj_t *icon, lv_obj_t *status)
{
    /* 单独进入预热:icon 右移 48px(排版修正,不改生成文件;模式预热保持原位) */
    if (g_preheat_solo && icon) lv_obj_set_pos(icon, 163, 161);
    const char *txt = NULL;
    switch (g_send.cook_mode) {
    case MODE_UPDOWN_BBQ:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(updown_img.png));
        txt = tr("| 上下烧烤 |");
        break;
    case MODE_TOP_BBQ:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(topicon.png));
        txt = tr("| 顶部烧烤 |");
        break;
    case MODE_BOTTOM_BBQ:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(dwbbqicon.png));
        txt = tr("| 底部烧烤 |");
        break;
    case MODE_HOT_BBQ:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(hotbbqicon.png));
        txt = tr("| 热风烧烤 |");
        break;
    case MODE_HOTWIND_BBQ:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(hotwindicon.png));
        txt = tr("| 热风 |");
        break;
    case MODE_SAVE_BBQ:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(savewindicon.png));
        txt = tr("| 节能热风 |");
        break;
    case MODE_CENTRAL_BBQ:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(centralbbqicon.png));
        txt = tr("| 集中烧烤 |");
        break;
    case MODE_WINDCHANGE_BBQ:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(windchange.png));
        txt = tr("| 热风对流 |");
        break;
    case MODE_AIR:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(airicon.png));
        txt = tr("| 空气炸 |");
        break;
    case MODE_PIZZA_2:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(pizza2icon.png));
        txt = tr("| 披萨 |");
        break;
    case MODE_SLOWCOOK:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(slowcookicon.png));
        txt = tr("| 慢煮 |");
        break;
    case MODE_UNFROZEN:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(unfrozenicon.png));
        txt = tr("| 解冻 |");
        break;
    case MODE_RISING:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(risingicon.png));
        txt = tr("| 发酵 |");
        break;
    case MODE_CORN:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(cornicon.png));
        txt = tr("| 干果 |");
        break;
    case MODE_HEATCONTAIN:
        if (icon) lv_img_set_src(icon, LVGL_IMAGE_PATH(heatcontainicon.png));
        txt = tr("| 保温 |");
        break;
    default:
        /* 预热菜单入口（MODE_PREHEAT）:仅按键4入口显示"快速预热",cookmenu 入口保持"| 预热 |" */
        if (g_preheat_fast && status)
            lv_label_set_text(status, tr("| 快速预热 |"));
        return;
    }
    if (status) lv_label_set_text(status, txt);
}

static void on_preheat_menu_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_preheat_cooking();
}

static void on_preheat_cooking_stop_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_preheat_stop();
}

static void on_preheat_stop_start_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        preheat_resume_cooking();
}

static void on_preheat_stop_back_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    g_on_stop_back = 0;
    preheat_wait_door = 0;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.cook_flag = 0;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    g_stop_back_complete = NULL;
    preheat_start_cavity = 0;
    /* 显式重置栈(按键4入口栈[1]是 PREHEAT_MENU,不能 depth=2 截栈) */
    depth = 0;
    page_push(PAGE_WAITMENU_24);
    page_push(PAGE_MAJOR_MENU);
    lv_obj_clean(lv_scr_act());
    major_menu_create(&ui_manager);
    groups_create();
    bind_events();
    current_group = g_major_menu;
    lang_scr_load_anim(major_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] stop_back sure -> major_menu\n");
}

static void on_preheat_complete_sure_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    if (g_send.cook_mode != MODE_PREHEAT) {
        // 一阶段（等食材）：sure 不可用
        if (preheat_wait_door) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            return;
        }
        // 二阶段：清理 preheat 栈 → 进入该模式正常 cooking
        if (depth > 0 && page_stack[depth - 1] == PAGE_PREHEAT_COMPLETE)
            depth--;
        if (depth > 0 && page_stack[depth - 1] == PAGE_PREHEAT_COOKING)
            depth--;
        g_on_stop_back = 0;
        switch (g_send.cook_mode) {
        case MODE_UPDOWN_BBQ:     jump_to_updown_bbq_cooking(); break;
        case MODE_TOP_BBQ:        jump_to_top_bbq_cooking(); break;
        case MODE_BOTTOM_BBQ:     jump_to_bottom_bbq_cooking(); break;
        case MODE_HOT_BBQ:        jump_to_hot_bbq_cooking(); break;
        case MODE_HOTWIND_BBQ:    jump_to_hotwind_bbq_cooking(); break;
        case MODE_SAVE_BBQ:       jump_to_save_bbq_cooking(); break;
        case MODE_CENTRAL_BBQ:    jump_to_central_bbq_cooking(); break;
        case MODE_WINDCHANGE_BBQ: jump_to_windchange_bbq_cooking(); break;
        case MODE_AIR:            jump_to_air_cooking(); break;
        case MODE_PIZZA_2:        jump_to_pizza_2_cooking(); break;
        case MODE_SLOWCOOK:       jump_to_slowcook_cooking(); break;
        case MODE_UNFROZEN:       jump_to_unfrozen_cooking(); break;
        case MODE_RISING:         jump_to_rising_cooking(); break;
        case MODE_CORN:           jump_to_corn_cooking(); break;
        case MODE_HEATCONTAIN:    jump_to_heatcontain_cooking(); break;
        default:                  preheat_complete_exit(); break;
        }
        return;
    }
    preheat_complete_exit();
}

void preheat_complete_exit(void)
{
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    g_on_stop_back = 0;
    g_stop_back_complete = NULL;
    preheat_wait_door = 0;
    preheat_start_cavity = 0;
    g_keepwarm_active = 0;
    g_keepwarm_sec = 0;
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    cook_elapsed_saved = 0; cook_bar_saved = 0; cook_total_ms = 0; cook_start_time = 0;
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.cook_flag = 0;
    g_send.set_temp = 0; g_send.set_temp_lower = 0; g_send.remaining_ms = -1;
    if (g_preheat_fast) {
        /* 按键4入口栈[1]是 PREHEAT_MENU,显式重置为 [WAITMENU, MAJOR_MENU] 再显示主菜单 */
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        page_push(PAGE_MAJOR_MENU);
        lv_obj_clean(lv_scr_act());
        major_menu_create(&ui_manager);
        groups_create();
        bind_events();
        current_group = g_major_menu;
        lang_scr_load_anim(major_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav_preheat] complete exit -> major_menu\n");
    } else {
        /* cookmenu 入口:回 cookmenu(栈[1]=MAJOR_MENU,depth=2 截栈正确) */
        depth = 2;
        jump_to_cookmenu();
    }
}

void jump_to_preheat_menu(void)
{
    g_preheat_fast = (depth <= 2);   /* 按键4入口(depth=1)vs cookmenu 入口(depth=3),须在 push 前判断 */
    g_send.cook_mode = MODE_PREHEAT;
    g_send.cook_flag = 1;
    set_temp = 180;   /* 单独进入预热:重置默认温度(预热无时间概念) */

    page_push(PAGE_PREHEAT_MENU);
    lv_obj_clean(lv_scr_act());
    preheatmenu_create(&ui_manager);

    preheatmenu_t *menu = preheatmenu_get(&ui_manager);
    if (menu) {
        if (g_preheat_fast) lv_label_set_text(menu->label_69, tr("快速预热"));
        lv_obj_t *btns[] = { menu->temp, menu->next };
        if (g_preheat_menu) lv_group_del(g_preheat_menu);
        g_preheat_menu = group_create_for_page(btns, 2);

        edit_clear();
        edit_register(menu->temp, menu->line2, menu->line3,
                      &set_temp, 120, 250, 5, "%d");

        lv_obj_add_event_cb(menu->temp, on_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_edit_focus, LV_EVENT_FOCUSED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp);

        lv_obj_add_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);

        lv_group_focus_obj(menu->next);
    }
    current_group = g_preheat_menu;

    if (menu && menu->next)
        lv_obj_add_event_cb(menu->next, on_preheat_menu_next_click,
                            LV_EVENT_CLICKED, NULL);

    lang_scr_load_anim(preheatmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] cookmenu -> preheat_menu\n");
}

void jump_to_preheat_cooking(void)
{
    fav_snapshot_save();   /* 收藏:进入 cooking 时快照初始参数 */
    edit_clear();
    /* 单独进入(来源 preheat_menu)vs 模式预热(来源各模式 set 页),需在 page_push 前判断 */
    g_preheat_solo = (depth > 0 && page_stack[depth - 1] == PAGE_PREHEAT_MENU);
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }

    page_push(PAGE_PREHEAT_COOKING);
    lv_obj_clean(lv_scr_act());
    preheatcooking_create(&ui_manager);

#ifdef LV_USE_AIC_SIMULATOR
    g_sim_cavity_temp = 25;
#endif
    preheat_start_cavity = get_cavity_temp();

    preheatcooking_t *cook = preheatcooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_preheat_cooking) lv_group_del(g_preheat_cooking);
        g_preheat_cooking = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cook->stop, on_preheat_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(cook->temp, tr("%d℃"), set_temp);
        lv_label_set_text(cook->name, tr("预热中..."));
        preheat_apply_mode_ui(cook->icon, cook->status);
        preheat_update_bar(cook->bar_1, cook->bartemp);
    }
    current_group = g_preheat_cooking;

    g_send.iface_status = IFACE_COOKING;
    g_send.set_temp = set_temp;
    g_send.set_temp_lower = (g_send.cook_mode == MODE_UPDOWN_BBQ) ? set_temp : 0;
    g_send.remaining_ms = 0;
    g_send.cook_flag = (g_send.cook_mode == MODE_PREHEAT) ? 1 : 0;

    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    lang_scr_load_anim(preheatcooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] menu -> preheat_cooking\n");
}

void jump_to_preheat_stop(void)
{
    edit_clear();
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    page_push(PAGE_PREHEAT_STOP);
    lv_obj_clean(lv_scr_act());
    preheatstop_create(&ui_manager);

    preheatstop_t *stop = preheatstop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start };
        if (g_preheat_stop) lv_group_del(g_preheat_stop);
        g_preheat_stop = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(stop->start, on_preheat_stop_start_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(stop->temp, tr("%d℃"), set_temp);
        preheat_apply_mode_ui(stop->icon, stop->status);
        preheat_update_bar(stop->bar_2, stop->bartemp);
    }
    current_group = g_preheat_stop;
    g_send.iface_status = IFACE_PAUSE;
    g_send.remaining_ms = 0;

    lang_scr_load_anim(preheatstop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] cooking -> preheat_stop\n");
}

void preheat_resume_cooking(void)
{
    edit_clear();
    g_on_stop_back = 0;

    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    if (depth > 1) depth--;
    lv_obj_clean(lv_scr_act());
    preheatcooking_create(&ui_manager);

    preheatcooking_t *cook = preheatcooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_preheat_cooking) lv_group_del(g_preheat_cooking);
        g_preheat_cooking = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cook->stop, on_preheat_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(cook->temp, tr("%d℃"), set_temp);
        lv_label_set_text(cook->name, tr("预热中..."));
        preheat_apply_mode_ui(cook->icon, cook->status);
        preheat_update_bar(cook->bar_1, cook->bartemp);
    }
    current_group = g_preheat_cooking;
    g_send.iface_status = IFACE_COOKING;
    g_send.remaining_ms = 0;

    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);

    lang_scr_load_anim(preheatcooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] resume: stop -> cooking\n");
}

void jump_to_preheat_stop_back(void)
{
    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_preheat_complete;
    page_push(PAGE_PREHEAT_STOP_BACK);
    lv_obj_clean(lv_scr_act());
    preheat_stop_back_create(&ui_manager);

    preheat_stop_back_t *back = preheat_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_preheat_stop_back) lv_group_del(g_preheat_stop_back);
        g_preheat_stop_back = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(back->sure, on_preheat_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        preheat_update_bar(back->bar_1, back->bartemp);
        preheat_apply_mode_ui(back->icon, back->status);

        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(back->name, tr("预热中..."));

        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            lv_label_set_text(back->name, tr("预热完成"));
            lv_bar_set_value(back->bar_1, 100, LV_ANIM_OFF);
            if (back->bartemp) lv_obj_set_pos(back->bartemp, 679 + lang_dyn_dx(), 323 + lang_dyn_dy());
        }
    }
    current_group = g_preheat_stop_back;

    lang_scr_load_anim(preheat_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] cooking/stop/complete -> stop_back\n");
}

void preheat_rebuild_stop_back(void)
{
    g_on_stop_back = 1;
    g_stop_back_complete = jump_to_preheat_complete;
    preheat_stop_back_create(&ui_manager);
    preheat_stop_back_t *back = preheat_stop_back_get(&ui_manager);
    if (back) {
        lv_obj_t *btns[] = { back->sure };
        if (g_preheat_stop_back) lv_group_del(g_preheat_stop_back);
        g_preheat_stop_back = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(back->sure, on_preheat_stop_back_sure_click,
                            LV_EVENT_CLICKED, NULL);

        preheat_update_bar(back->bar_1, back->bartemp);
        preheat_apply_mode_ui(back->icon, back->status);

        if (g_send.iface_status == IFACE_COOKING)
            lv_label_set_text(back->name, tr("预热中..."));

        if (g_complete_to_stop_back) {
            g_complete_to_stop_back = 0;
            lv_label_set_text(back->name, tr("预热完成"));
            lv_bar_set_value(back->bar_1, 100, LV_ANIM_OFF);
            if (back->bartemp) lv_obj_set_pos(back->bartemp, 679 + lang_dyn_dx(), 323 + lang_dyn_dy());
        }
    }
    current_group = g_preheat_stop_back;
    lang_scr_load_anim(preheat_stop_back_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] back to stop_back\n");
}

// 预热完成页 UI：set 场景分两阶段（等食材 → 门关后确认）
static void preheat_complete_update_ui(preheatcomplete_t *cook)
{
    if (g_send.cook_mode != MODE_PREHEAT) {
        lv_obj_add_flag(cook->tip2, LV_OBJ_FLAG_HIDDEN);
        if (preheat_wait_door) {
            lv_obj_add_flag(cook->sure, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(cook->tip1, tr("请放入食材！"));
        } else {
            lv_obj_clear_flag(cook->sure, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(cook->tip1, tr("已放入食材！"));
        }
    }
}

void jump_to_preheat_complete(void)
{
    // 门关重建场景：已在 complete 页，只更新 UI（不重复 push）
    if (depth > 0 && page_stack[depth - 1] == PAGE_PREHEAT_COMPLETE) {
        preheatcomplete_t *cook = preheatcomplete_get(&ui_manager);
        if (cook) preheat_complete_update_ui(cook);
        return;
    }
    edit_clear();
    int from_stop_back = 0;
    if (depth > 0 && page_stack[depth - 1] == PAGE_PREHEAT_STOP_BACK) {
        depth--;
        if (depth > 0 && page_stack[depth - 1] == PAGE_PREHEAT_COMPLETE)
            from_stop_back = 1;
    }
    if (!from_stop_back) {
        if (depth > 0 && page_stack[depth - 1] == PAGE_PREHEAT_STOP)
            depth--;
        page_push(PAGE_PREHEAT_COMPLETE);
    }
    lv_obj_clean(lv_scr_act());
    preheatcomplete_create(&ui_manager);

    // set 场景：首次进入等待放食材阶段（回跳场景 wait_door 已被门检测清零 → 二阶段）
    if (g_send.cook_mode != MODE_PREHEAT && !from_stop_back)
        preheat_wait_door = 1;

    {
        preheatcomplete_t *cook = preheatcomplete_get(&ui_manager);
        if (cook) {
            lv_obj_t *btns[] = { cook->sure };
            if (g_preheat_complete) lv_group_del(g_preheat_complete);
            g_preheat_complete = group_create_for_page(btns, 1);
            lv_obj_add_event_cb(cook->sure, on_preheat_complete_sure_click,
                                LV_EVENT_CLICKED, NULL);
            lv_bar_set_value(cook->bar_3, 100, LV_ANIM_OFF);
            lv_label_set_text(cook->name, tr("预热完成"));
            preheat_apply_mode_ui(cook->icon, cook->status);
            if (cook->bartemp) {
                lv_label_set_text_fmt(cook->bartemp, tr("%d℃"), set_temp);
                lv_obj_set_pos(cook->bartemp, 679 + lang_dyn_dx(), 323 + lang_dyn_dy());
            }
            preheat_complete_update_ui(cook);
        }
    }
    current_group = g_preheat_complete;
    g_send.iface_status = IFACE_COMPLETE;
    g_send.remaining_ms = 0;
    g_send.cook_flag = 0;

    lang_scr_load_anim(preheatcomplete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] cooking -> preheat_complete\n");
}

// ==============================
// Rebuild 函数（供 page_pop 调用）
// ==============================

void preheat_rebuild_menu(page_id_t child)
{
    preheatmenu_create(&ui_manager);
    preheatmenu_t *menu = preheatmenu_get(&ui_manager);
    if (menu) {
        if (g_preheat_fast) lv_label_set_text(menu->label_69, tr("快速预热"));
        lv_obj_t *btns[] = { menu->temp, menu->next };
        if (g_preheat_menu) lv_group_del(g_preheat_menu);
        g_preheat_menu = group_create_for_page(btns, 2);

        edit_clear();
        edit_register(menu->temp, menu->line2, menu->line3,
                      &set_temp, 120, 250, 5, "%d");

        lv_obj_add_event_cb(menu->temp, on_edit_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(menu->next, on_edit_focus, LV_EVENT_FOCUSED, NULL);

        lv_label_set_text_fmt(menu->temp, "%d", set_temp);

        lv_obj_add_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);
        if (set_temp < 100)
            lv_obj_clear_flag(menu->line2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(menu->line3, LV_OBJ_FLAG_HIDDEN);

        if (menu->next)
            lv_group_focus_obj(menu->next);
    }
    current_group = g_preheat_menu;
    if (menu && menu->next)
        lv_obj_add_event_cb(menu->next, on_preheat_menu_next_click,
                            LV_EVENT_CLICKED, NULL);
    lang_scr_load_anim(preheatmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] back to preheat_menu\n");
}

void preheat_rebuild_cooking(void)
{
    edit_clear();
    preheatcooking_create(&ui_manager);
    preheatcooking_t *cook = preheatcooking_get(&ui_manager);
    if (cook) {
        lv_obj_t *btns[] = { cook->stop };
        if (g_preheat_cooking) lv_group_del(g_preheat_cooking);
        g_preheat_cooking = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(cook->stop, on_preheat_cooking_stop_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(cook->temp, tr("%d℃"), set_temp);
        lv_label_set_text(cook->name, tr("预热中..."));
        preheat_apply_mode_ui(cook->icon, cook->status);
        preheat_update_bar(cook->bar_1, cook->bartemp);
    }
    current_group = g_preheat_cooking;
    lang_scr_load_anim(preheatcooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] back to preheat_cooking\n");
}

void preheat_rebuild_stop(void)
{
    edit_clear();
    g_on_stop_back = 0;
    preheatstop_create(&ui_manager);
    preheatstop_t *stop = preheatstop_get(&ui_manager);
    if (stop) {
        lv_obj_t *btns[] = { stop->start };
        if (g_preheat_stop) lv_group_del(g_preheat_stop);
        g_preheat_stop = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(stop->start, on_preheat_stop_start_click,
                            LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(stop->temp, tr("%d℃"), set_temp);
        preheat_apply_mode_ui(stop->icon, stop->status);
        preheat_update_bar(stop->bar_2, stop->bartemp);
    }
    current_group = g_preheat_stop;
    lang_scr_load_anim(preheatstop_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] back to preheat_stop\n");
}

void preheat_rebuild_complete(void)
{
    edit_clear();
    preheatcomplete_create(&ui_manager);
    {
        preheatcomplete_t *cook = preheatcomplete_get(&ui_manager);
        if (cook) {
            lv_obj_t *btns[] = { cook->sure };
            if (g_preheat_complete) lv_group_del(g_preheat_complete);
            g_preheat_complete = group_create_for_page(btns, 1);
            lv_obj_add_event_cb(cook->sure, on_preheat_complete_sure_click,
                                LV_EVENT_CLICKED, NULL);
            lv_bar_set_value(cook->bar_3, 100, LV_ANIM_OFF);
            lv_label_set_text(cook->name, tr("预热完成"));
            preheat_apply_mode_ui(cook->icon, cook->status);
            if (cook->bartemp) {
                lv_label_set_text_fmt(cook->bartemp, tr("%d℃"), set_temp);
                lv_obj_set_pos(cook->bartemp, 679 + lang_dyn_dx(), 323 + lang_dyn_dy());
            }
            preheat_complete_update_ui(cook);
        }
    }
    current_group = g_preheat_complete;
    lang_scr_load_anim(preheatcomplete_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[nav_preheat] back to preheat_complete\n");
}
