/*
 * nav_clean.c - 清洁入口
 * 主菜单清洁入口分发：自清洁(水) / 高温清洁三档(节能/常规/强力)。
 */
#include "protocol.h"
#include "nav.h"

void on_clean_water_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_wc_set();
}

void on_clean_hot_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hotclean_menu();
}

void on_hc_save_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hcs_set();
}

void on_hc_middle_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hcm_set();
}

void on_hc_high_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (!screen_is_loading(act_scr))
        jump_to_hch_set();
}

void jump_to_hotclean_menu(void)
{
    page_push(PAGE_HOTCLEAN_MENU);
    lv_obj_clean(lv_scr_act());
    hotclean_menu_create(&ui_manager);

    hotclean_menu_t *hm = hotclean_menu_get(&ui_manager);
    if (hm) {
        lv_obj_t *btns[] = { hm->hotcleansave, hm->hotclean_middle, hm->hotclean_high };
        if (g_hotclean_menu) lv_group_del(g_hotclean_menu);
        g_hotclean_menu = group_create_for_page(btns, 3);

        lv_obj_add_event_cb(hm->hotcleansave, on_hc_save_click, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(hm->hotclean_middle, on_hc_middle_click, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(hm->hotclean_high, on_hc_high_click, LV_EVENT_CLICKED, NULL);

        if (hm->hotcleansave) lv_group_focus_obj(hm->hotcleansave);
    }
    current_group = g_hotclean_menu;

    lang_scr_load_anim(hotclean_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.cook_mode = MODE_HOT_CLEAN;
    printf("[clean] jump: clean_menu -> hotclean_menu\n");
}

void hotclean_rebuild(page_id_t child)
{
    hotclean_menu_create(&ui_manager);
    hotclean_menu_t *hm = hotclean_menu_get(&ui_manager);
    if (hm) {
        lv_obj_t *btns[] = { hm->hotcleansave, hm->hotclean_middle, hm->hotclean_high };
        if (g_hotclean_menu) lv_group_del(g_hotclean_menu);
        g_hotclean_menu = group_create_for_page(btns, 3);

        lv_obj_add_event_cb(hm->hotcleansave, on_hc_save_click, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(hm->hotclean_middle, on_hc_middle_click, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(hm->hotclean_high, on_hc_high_click, LV_EVENT_CLICKED, NULL);

        if (child >= PAGE_HOTCLEANSAVE_SET && child <= PAGE_HOTCLEANSAVE_COMPLETE && hm->hotcleansave)
            lv_group_focus_obj(hm->hotcleansave);
        else if (child >= PAGE_HOTCLEANMIDDLE_SET && child <= PAGE_HOTCLEANMIDDLE_COMPLETE && hm->hotclean_middle)
            lv_group_focus_obj(hm->hotclean_middle);
        else if (child >= PAGE_HOTCLEANHIGH_SET && child <= PAGE_HOTCLEANHIGH_COMPLETE && hm->hotclean_high)
            lv_group_focus_obj(hm->hotclean_high);
        else if (hm->hotcleansave)
            lv_group_focus_obj(hm->hotcleansave);
    }
    current_group = g_hotclean_menu;
    lang_scr_load_anim(hotclean_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[clean] back to hotclean_menu\n");
}

void jump_to_clean_menu(void)
{
    page_push(PAGE_CLEAN_MENU);
    lv_obj_clean(lv_scr_act());
    clean_menu_create(&ui_manager);

    clean_menu_t *cm = clean_menu_get(&ui_manager);
    if (cm) {
        lv_obj_t *btns[] = { cm->waterclean, cm->button_2 };
        if (g_clean_menu) lv_group_del(g_clean_menu);
        g_clean_menu = group_create_for_page(btns, 2);

        lv_obj_add_event_cb(cm->waterclean, on_clean_water_click, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cm->button_2, on_clean_hot_click, LV_EVENT_CLICKED, NULL);

        if (cm->waterclean) lv_group_focus_obj(cm->waterclean);
    }
    current_group = g_clean_menu;

    lang_scr_load_anim(clean_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    printf("[clean] jump: waitmenu_24 -> clean_menu\n");
}

void clean_rebuild(page_id_t child)
{
    clean_menu_create(&ui_manager);
    clean_menu_t *cm = clean_menu_get(&ui_manager);
    if (cm) {
        lv_obj_t *btns[] = { cm->waterclean, cm->button_2 };
        if (g_clean_menu) lv_group_del(g_clean_menu);
        g_clean_menu = group_create_for_page(btns, 2);

        lv_obj_add_event_cb(cm->waterclean, on_clean_water_click, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cm->button_2, on_clean_hot_click, LV_EVENT_CLICKED, NULL);

        if (child >= PAGE_WATER_CLEAN_SET && child <= PAGE_WATER_CLEAN_COMPLETE && cm->waterclean)
            lv_group_focus_obj(cm->waterclean);
        else if (((child >= PAGE_HOTCLEANSAVE_SET && child <= PAGE_HOTCLEANHIGH_COMPLETE) ||
                  child == PAGE_HOTCLEAN_MENU) && cm->button_2)
            lv_group_focus_obj(cm->button_2);
        else if (cm->waterclean)
            lv_group_focus_obj(cm->waterclean);
    }
    current_group = g_clean_menu;
    lang_scr_load_anim(clean_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    printf("[clean] back to clean_menu\n");
}

/* ==============================
 * 热解自清洁"结束任务"收尾(三档共用)
 * 原样搬自 nav_hotclean(save|middle|high).c 的 on_h*_stop_back_sure_click:
 * 收计时器/复位进度 → 栈回到深度 2 → 重建清洁菜单。行为逐字一致,只是三份副本合一,
 * 因为"提前结束 + 冷却等待"这条新路径(见 hotclean_abort_to_cooling)结束时也要落到同一处。
 * set_hour_def/set_min_def:该档的默认时长(节能 1:15 / 常规 1:40 / 强力 2:00)
 * ============================== */
void hotclean_end_to_clean_menu(int set_hour_def, int set_min_def)
{
    g_on_stop_back = 0;
    g_hotclean_cool_to_menu = 0;   /* 一次性标志:无论从哪条路进来都清掉 */
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    set_hour = set_hour_def;
    set_min = set_min_def;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    depth = 2;
    lv_obj_clean(lv_scr_act());
    clean_menu_create(&ui_manager);
    clean_menu_t *cm = clean_menu_get(&ui_manager);
    if (cm) {
        lv_obj_t *btns[] = { cm->waterclean, cm->button_2 };
        if (g_clean_menu) lv_group_del(g_clean_menu);
        g_clean_menu = group_create_for_page(btns, 2);
        lv_obj_add_event_cb(cm->waterclean, on_clean_water_click, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(cm->button_2, on_clean_hot_click, LV_EVENT_CLICKED, NULL);
        if (cm->waterclean) lv_group_focus_obj(cm->waterclean);
    }
    current_group = g_clean_menu;
    lang_scr_load_anim(clean_menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    printf("[clean] hotclean end -> clean_menu\n");
}

/* 热解提前结束且腔温仍高(>HOTCLEAN_COOL_TEMP):不直接回菜单,先进冷却页等降温。
 * 调用处为三档 stop_back 确认页的"确定"(见 nav_hotclean*.c)。
 * 关键点:必须让电源板停火——冷却页默认按"整段做完后的冷却"发 IFACE_COOKING + 保留
 * 烹调模式,而那会让电源板继续按热解程序加热;这里改成暂停态(模式保留、目标温度清零、
 * 不再倒计时),电源板按暂停处理:停止加热、门锁保持,显示侧因 iface 非设置态也不会
 * 被空闲策略拽进待机。降到 HOTCLEAN_COOL_TEMP 以下由 nav_cooktimer 的冷却分支
 * 调用 hotclean_end_to_clean_menu() 回清洁菜单。 */
void hotclean_abort_to_cooling(void)
{
    page_id_t top;

    if (depth <= 0) return;
    top = page_stack[depth - 1];

    g_on_stop_back = 0;
    g_hotclean_cool_to_menu = 1;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    cook_elapsed_saved = 0; cook_bar_saved = 0;

    if (top == PAGE_HOTCLEANSAVE_STOP_BACK)        jump_to_hcs_cooling();
    else if (top == PAGE_HOTCLEANMIDDLE_STOP_BACK) jump_to_hcm_cooling();
    else                                           jump_to_hch_cooling();

    g_send.iface_status = IFACE_PAUSE;   /* 覆盖冷却页默认的 IFACE_COOKING:停火 */
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;
    printf("[clean] hotclean abort (cavity > %dC) -> cooling page\n", HOTCLEAN_COOL_TEMP);
}

/* 冷却页 BACK 进返回确认、再按"确定"的收尾(三档共用)。
 * 冷却页上任务其实已经结束,这里只是等腔温降到 HOTCLEAN_COOL_TEMP 以下自动回菜单;
 * 用户按确定=不再等,直接收尾回清洁菜单。原实现不看来源,一律 hotclean_abort_to_cooling()
 * 再压一层冷却页,每按一次返回确定就多两层,页面栈 16 层溢出后卡死(2026-11-24 用户实测)。
 * 判定用页面栈:冷却页 BACK 会 push 一层 stop_back,所以 stop_back 底下那层就是同档冷却页。
 * 返回 1 = 已处理,调用方直接 return。 */
int hotclean_stop_back_sure_from_cooling(page_id_t cooling, int set_hour_def, int set_min_def)
{
    if (depth < 2 || page_stack[depth - 2] != cooling) return 0;
    printf("[clean] cooling stop_back sure -> end task (back to clean_menu)\n");
    hotclean_end_to_clean_menu(set_hour_def, set_min_def);
    return 1;
}

/* 当前确认页是不是从冷却页进来的(任务已结束、只在等降温)。
 * 判定同上:页栈里 stop_back 底下那层就是同档冷却页——冷却页 BACK 会 push 一层确认页,
 * 重建路径(nav_pop 的 *_rebuild_stop_back)栈形也一样。
 * 冷却页的 cook_timer 还在跑(冷却轮询用)、cook_start_time 还是清洁开始时刻,
 * 确认页若照常按 elapsed 算进度,会显示一段"烹饪进度"且每开一次都变长
 * (2026-11-24 用户:返回确认下面显示的是烹饪时的长度,还会变长,计时没停吗)。 */
int hotclean_stop_back_from_cooling(page_id_t cooling)
{
    return (depth >= 2 && page_stack[depth - 2] == cooling);
}
