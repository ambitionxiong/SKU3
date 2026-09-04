/*
 * nav_key.c - 按键分发处理 process_key
 *
 * 职责：
 *   1. 按键白名单（menu_clean_key_allowed：MENU/CLEAN 键仅在菜单/待机/探针页有效）
 *   2. 设置页功能键跳转/防御判断（screen_set_key_allowed）
 *   3. 核心按键分发 process_key：按 KEY 值分发到各功能入口
 *      （主菜单/清洁/快速预热/额外上色/第六感/设置页/BACK/编码器/确认）
 *
 * 按键状态机(key_state/active_key/active_key_time)定义于此，
 * 由 nav_keyio.c 的 nav_handle_key 驱动、nav_key1_long_press 处理长按。
 *
 * 注意：process_key 不做输入防抖（那是 nav_keyio.c 的职责），
 *   仅按"当前键 + 当前页面状态"决定有效动作与蜂鸣器反馈。
 */

#include "nav.h"
#include "nav_internal.h"
#include "nav_favorites.h"

// ==============================
// 按键处理（状态机防抖）
// ==============================

key_state_t key_state = KEY_IDLE;
uint8_t active_key = 0;
uint32_t active_key_time = 0;

// MENU/CLEAN 键白名单：仅在菜单/待机/探针提示类页面有效，
// 设置温度时间页、运行状态页、完成页等一律无效音
/* 是否为各模式 SETTING 页(烹饪中/完成后小按钮进入,不切状态、仍处烹饪态)。
 * 两处使用:menu_clean_key_allowed 排除功能键跳转;nav_show_invalid_hint 不弹窗只留无效音 */
int is_cook_setting_page(page_id_t cur)
{
    switch (cur) {
    case PAGE_UPDOWN_BBQ_SETTING: case PAGE_TOP_BBQ_SETTING:
    case PAGE_BOTTOM_BBQ_SETTING: case PAGE_HOT_BBQ_SETTING:
    case PAGE_HOTWIND_BBQ_SETTING: case PAGE_SAVE_BBQ_SETTING:
    case PAGE_CENTRAL_BBQ_SETTING: case PAGE_WINDCHANGE_BBQ_SETTING:
    case PAGE_PIZZA_2_SETTING: case PAGE_SLOWCOOK_SETTING:
    case PAGE_UNFROZEN_SETTING: case PAGE_RISING_SETTING:
    case PAGE_CORN_SETTING: case PAGE_HEATCONTAIN_SETTING:
    case PAGE_COOKIE_SETTING: case PAGE_WEST_SETTING: case PAGE_PIZZA_SETTING:
    case PAGE_MENU_COOK_SETTING: case PAGE_LASAGNA_SETTING:
    case PAGE_STRUDEL_SETTING: case PAGE_BREAD_SETTING: case PAGE_PIZZA3_SETTING:
    case PAGE_CHIP_SETTING: case PAGE_CUSTOM_SETTING:
        return 1;
    default:
        return 0;
    }
}

static int menu_clean_key_allowed(void)
{
    /* 2026-08-30:以运行状态取代页面白名单——待机/设置浏览态(IFACE_STANDBY/
     * IFACE_SETTING,含各模式 menu/set 流程页)功能键均可跳转;烹饪/暂停/完成/
     * 预约态(IFACE_DELAY_RESERVE,含预约等待页)功能键保持无效音,完成态的收藏
     * 保存/再次上色由各 case 自身逻辑先行处理。
     * 例外:各模式 SETTING 页(烹饪中/完成后小按钮进入)属 cooking 家族,
     * 显式排除,功能键无效音 */
    if (depth <= 0) return 0;
    if (page_stack[depth - 1] == PAGE_DELAYCOOKING)   /* 预约等待(DELAY_RESERVE 态,防御性排除) */
        return 0;
    if (is_cook_setting_page(page_stack[depth - 1]))
        return 0;
    return (g_send.iface_status == IFACE_SETTING || g_send.iface_status == IFACE_STANDBY);
}
/* 核心按键分发：按 key 值进入各功能分支。
   入口统一处理：SLEEP 拦截 → 重复收藏确认模态 → 无效提示弹窗屏蔽 → 设置页先关后处理。
   各 case 内再做白名单/防重入/运行态拦截，并设置 buzzer_req 反馈。 */
void process_key(uint8_t key)
{
    if (g_send.iface_status == IFACE_SLEEP) return;
    /* 重复收藏确认弹层:模态。PRESS=确认覆盖保存,BACK=取消回完成页,其余键忽略 */
    if (nav_favask_active()) {
        if (key == KEY_ENCODER_PRESS) nav_favask_confirm();
        else if (key == KEY_BACK) nav_favask_cancel();
        return;
    }
    /* 无效提示弹窗/收藏结果提示:仅 BACK 有效(KEY_BACK 分支关闭弹窗);长按关机由
       nav_handle_key 独立检测不受影响;其余键静默忽略,避免主动操作 */
    if ((nav_hint_active() || nav_favtip_active()) && key != KEY_BACK)
        return;
    /* 探针提示页:仅 BACK 有效(probetip_dismiss_now 提前结束),功能键静默忽略 */
    if (depth > 0 && page_stack[depth - 1] == PAGE_PROBETIP && key != KEY_BACK)
        return;
    /* 腔温过热提示页:仅 BACK 有效(temptip_dismiss_now 提前结束),功能键静默忽略 */
    if (depth > 0 && page_stack[depth - 1] == PAGE_TEMPTIP && key != KEY_BACK)
        return;
    /* 烤鸡探针提示页:仅 BACK / 确定(编码器按下=点击 sure)有效，其余功能键静默忽略 */
    if (depth > 0 && page_stack[depth - 1] == PAGE_PROBENEEDTIP &&
        key != KEY_BACK && key != KEY_ENCODER_PRESS)
        return;
    /* 设置层选项弹窗激活:按键全部由弹窗消化(编码器切选项/PRESS确认/BACK取消,
     * 其余键无效音),不触发覆盖层的功能键关层 */
    if (screen_set_popup_active() && screen_set_popup_key(key))
        return;
    /* 声音设置页提示音弹窗激活:按键全部由弹窗消化(编码器切选项/PRESS确认/
     * BACK取消,其余无效音),不影响覆盖层/下层页面 */
    if (loudness_popup_active() && loudness_popup_key(key))
        return;
    /* 设置页覆盖层打开时按功能键:先关闭设置页(弹栈恢复下层页面),再按下层
     * 页面正常处理——功能键在设置页也能生效;键自身的白名单/防重入/运行态
     * 拦截(烹饪中等)由各 case 照常判断 */
    if (depth > 1 && page_stack[depth - 1] == PAGE_SCREEN_SET &&
        (key == KEY_MENU || key == KEY_SIXMENU || key == KEY_PREHEAT ||
         key == KEY_EXTRA_COLOR || key == KEY_CLEAN || key == KEY_FAV)) {
        screen_set_reset();   /* 清覆盖层对象/指针,防悬空 */
        depth--;              /* 弹掉 PAGE_SCREEN_SET,栈顶恢复下层页面 */
        topflag_update_visibility();   /* 与 page_pop 一致刷新 topflag 显隐 */
    }
    /* 功能键离开收藏页:先安全清理收藏组/默认组引用——收藏页出口原只有 BACK 与
     * Fav_Start 走 favo_safety_group_delete,功能键直接跳转会留下僵尸默认组,
     * 污染后续页面首个控件的焦点事件(favo_safety_group_delete 注记载的同类问题)。
     * 仅对确定跳离的键清理:预热/清洁在探针插入时只弹提示留在本页,不清理;
     * KEY_SET 把设置覆盖层压在收藏页之上,收藏页仍存活,也不清理 */
    if (g_favorites && current_group == g_favorites &&
        (key == KEY_MENU || key == KEY_SIXMENU || key == KEY_EXTRA_COLOR ||
         key == KEY_CLEAN || key == KEY_PREHEAT) &&
        !(is_probe_inserted() && (key == KEY_PREHEAT || key == KEY_CLEAN))) {
        favo_safety_group_delete();
    }
    uart_data_receive[Receive_data_Touch_Key] = 0;

    switch (key) {
    case KEY1:              // 1: 开关机键，短按无操作
        break;
    case KEY_MENU:          // 3: 进入主菜单
        if (g_send.iface_status == IFACE_COOKING) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            nav_show_invalid_hint();   /* 烹饪中:无效提示弹窗 */
            uart_print();
            break;
        }
        if (!menu_clean_key_allowed()) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            break;
        }
        if (depth > 0) {
            page_id_t cur = page_stack[depth - 1];
            if (cur == PAGE_MAJOR_MENU || cur == PAGE_MAJOR_MENU_TZ) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;
                break;
            }
        }
        g_send.buzzer_req = BUZZER_KEY_VALID;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        g_on_stop_back = 0;
        g_complete_to_stop_back = 0;
        g_cooling_to_stop_back = 0;
        g_extra_color_to_stop_back = 0;
        g_keepwarm_active = 0;
        cook_is_color = 0;
        g_stop_back_complete = NULL;
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        if (is_probe_inserted()) {
            jump_to_major_menu_tz();
        } else {
            page_push(PAGE_MAJOR_MENU);
            lv_obj_clean(lv_scr_act());
            major_menu_create(&ui_manager);
            groups_create();
            bind_events();
            current_group = g_major_menu;
            lang_scr_load_anim(major_menu_get(&ui_manager)->obj,
                             LV_SCR_LOAD_ANIM_NONE, 0, 0,
                             ui_manager.auto_del);
        }
        g_send.iface_status = IFACE_SETTING;
        g_send.cook_mode = MODE_NONE;
        g_send.set_temp = 0;
        g_send.set_temp_lower = 0;
        g_send.remaining_ms = -1;
        uart_print();
        break;
    case KEY_EXTRA_COLOR:   // 5: 进入额外上色
        if (depth > 0) {
            page_id_t cur = page_stack[depth - 1];
            if (cur == PAGE_SCREEN_SET || cur == PAGE_SIX_COOKING ||
                cur == PAGE_TOASTCOLOR) {
                /* 设置页覆盖层/六感流程:拦截,防止跳转破坏流程 */
                g_send.buzzer_req = BUZZER_KEY_INVALID;
                /* 六感运行页且正在烹饪:与其他功能键一致弹无效提示 */
                if (cur == PAGE_SIX_COOKING && g_send.iface_status == IFACE_COOKING)
                    nav_show_invalid_hint();
                uart_print();
                break;
            }
        }
        if (g_send.iface_status == IFACE_COMPLETE) {
            /* 完成状态：特定模式完成页可再次上色（进 extra_color 确认页） */
            if (depth > 0 && page_stack[depth - 1] == PAGE_EXTRA_COLOR) {
                /* 已在 extra_color 确认页，防重复入栈 */
                g_send.buzzer_req = BUZZER_KEY_INVALID;
                break;
            }
            if (g_send.cook_mode != MODE_UPDOWN_BBQ &&
                g_send.cook_mode != MODE_HOTWIND_BBQ &&
                g_send.cook_mode != MODE_WINDCHANGE_BBQ &&
                g_send.cook_mode != MODE_PIZZA_2 &&
                g_send.cook_mode != MODE_COOK4 &&
                g_send.cook_mode != MODE_EXTRA_COLOR) {   /* 额外上色完成后可再次上色 */
                g_send.buzzer_req = BUZZER_KEY_INVALID;
                break;
            }
            g_send.buzzer_req = BUZZER_KEY_VALID;
            g_color_from_probe = is_probe_inserted();   /* 记录进入时的探针状态 */
            cook_elapsed_saved = 0; cook_bar_saved = 0;  /* 新一轮上色，清旧会话保存值 */
            page_push(PAGE_EXTRA_COLOR);
            lv_obj_clean(lv_scr_act());
            extra_color_create(&ui_manager);
        {
            extra_color_t *ec = extra_color_get(&ui_manager);
            if (ec) {
                lv_obj_t *btns[] = { ec->start_button };
                if (g_extra_color) lv_group_del(g_extra_color);
                g_extra_color = group_create_for_page(btns, 1);
                lv_obj_add_event_cb(ec->start_button, on_color_start_click,
                                    LV_EVENT_CLICKED, NULL);
            }
            current_group = g_extra_color;
        }
        lang_scr_load_anim(extra_color_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        printf("[nav] jump: -> extra_color\n");
        uart_print();
        break;
        }
        /* 非完成状态:重置栈式功能键入口(与其他功能键一致,返回回待机页;不拦截探针) */
        if (depth > 0) {
            page_id_t cur = page_stack[depth - 1];
            if (cur == PAGE_COLOR_MENU || cur == PAGE_EXTRA_COLOR ||
                cur == PAGE_COLOR_COOKING || cur == PAGE_COLOR_STOP ||
                cur == PAGE_COLOR_STOP_BACK || cur == PAGE_COLOR_COOKING_COMPLETE) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;   /* 防重入 */
                uart_print();
                break;
            }
        }
        if (g_send.iface_status == IFACE_COOKING) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            nav_show_invalid_hint();   /* 烹饪中:无效提示弹窗 */
            uart_print();
            break;
        }
        if (!menu_clean_key_allowed()) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            uart_print();
            break;
        }
        g_send.buzzer_req = BUZZER_KEY_VALID;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        g_on_stop_back = 0;
        g_complete_to_stop_back = 0;
        g_cooling_to_stop_back = 0;
        g_extra_color_to_stop_back = 0;
        g_keepwarm_active = 0;
        g_keepwarm_sec = 0;
        cook_is_color = 0;
        g_stop_back_complete = NULL;
        g_somecook_running = 0;
        g_somecook_run_idx = 0;
        six_cook_reset();
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        jump_to_color_menu();
        g_send.iface_status = IFACE_SETTING;
        g_send.remaining_ms = -1;
        uart_print();
        break;
    case KEY_FAV:           // 6: 收藏键（完成态收藏当前参数，非完成态进入收藏页）
        if (g_favorites && current_group == g_favorites) {   /* 收藏页内：无效音（须判空:待机页两者皆 NULL 会误等） */
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            uart_print();
            break;
        }
        if (g_send.iface_status == IFACE_COMPLETE) {
            /* 排除：预热/额外上色/保温（这三种完成态不支持收藏） */
            if (g_send.cook_mode == MODE_PREHEAT ||
                g_send.cook_mode == MODE_EXTRA_COLOR ||
                g_send.cook_mode == MODE_HEATCONTAIN) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;
                uart_print();
                break;
            }
            favorites_save_current();   /* 收藏当前运行参数（进 cooking 时的设置值） */
            uart_print();
            break;
        }
        /* 非完成态：收藏页是根级页面,统一挂在 waitmenu 之下(与 KEY_MENU 建栈方式
         * 一致,待机时钟态 depth=0 时同样先建根页再进),BACK 即回 waitmenu */
        if (!menu_clean_key_allowed()) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            nav_show_invalid_hint();   /* 烹饪中:与其他功能键一致弹无效提示(非烹饪态内部自动跳过) */
            uart_print();
            break;
        }
        g_send.buzzer_req = BUZZER_KEY_VALID;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        g_on_stop_back = 0;
        g_complete_to_stop_back = 0;
        g_cooling_to_stop_back = 0;
        g_extra_color_to_stop_back = 0;
        g_keepwarm_active = 0;
        cook_is_color = 0;
        g_stop_back_complete = NULL;
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        jump_to_favorites();
        uart_print();
        break;
    case KEY_CLEAN:         // 7: 进入清洁菜单
        if (g_send.iface_status == IFACE_COOKING) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            nav_show_invalid_hint();   /* 烹饪中:无效提示弹窗 */
            uart_print();
            break;
        }
        if (!menu_clean_key_allowed()) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            break;
        }
        if (is_probe_inserted()) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            if (depth > 0 && page_stack[depth - 1] != PAGE_PROBETIP)
                jump_to_probetip(tr("该功能不支持探针，请拔出探针！"));
            break;
        }
        if (depth > 0) {
            page_id_t cur = page_stack[depth - 1];
            if (cur == PAGE_CLEAN_MENU) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;
                break;
            }
        }
        g_send.buzzer_req = BUZZER_KEY_VALID;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        g_on_stop_back = 0;
        g_complete_to_stop_back = 0;
        g_delay_cancel_to_stop_back = 0;
        g_delay_cancel_btn = 0;
        g_keepwarm_active = 0;
        g_keepwarm_sec = 0;
        delay_on = 0;
        contain_on = 0;
        preheat_on = 0;
        g_stop_back_complete = NULL;
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        jump_to_clean_menu();
        uart_print();
        break;
    case KEY_SET:           // 11: 进入设置页（覆盖层）
        if (depth > 0 && page_stack[depth - 1] == PAGE_SCREEN_SET) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;   /* 防重入 */
            break;
        }
        /* 与其他功能键相同入口限制(菜单白名单),运行态放行 */
        if (g_send.iface_status != IFACE_COOKING && g_send.iface_status != IFACE_PAUSE &&
            g_send.iface_status != IFACE_COMPLETE && g_send.iface_status != IFACE_DELAY_RESERVE) {
            if (!menu_clean_key_allowed()) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;   /* 非运行态:仅菜单页可进 */
                uart_print();
                break;
            }
        }
        g_send.buzzer_req = BUZZER_KEY_VALID;
        jump_to_screen_set();
        uart_print();
        break;
    case KEY_PREHEAT:       // 4: 进入快速预热
        if (depth > 0) {
            page_id_t cur = page_stack[depth - 1];
            if (cur == PAGE_PREHEAT_MENU || cur == PAGE_PREHEAT_COOKING ||
                cur == PAGE_PREHEAT_STOP || cur == PAGE_PREHEAT_STOP_BACK ||
                cur == PAGE_PREHEAT_COMPLETE) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;   /* 防重入 */
                uart_print();
                break;
            }
        }
        if (g_send.iface_status == IFACE_COOKING) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            nav_show_invalid_hint();   /* 烹饪中:无效提示弹窗 */
            uart_print();
            break;
        }
        if (!menu_clean_key_allowed()) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            break;
        }
        if (is_probe_inserted()) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            if (depth > 0 && page_stack[depth - 1] != PAGE_PROBETIP)
                jump_to_probetip(tr("该功能不支持探针，请拔出探针！"));   /* 与 KEY_CLEAN 一致 */
            break;
        }
        g_send.buzzer_req = BUZZER_KEY_VALID;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        g_on_stop_back = 0;
        g_complete_to_stop_back = 0;
        g_delay_cancel_to_stop_back = 0;
        g_delay_cancel_btn = 0;
        g_keepwarm_active = 0;
        g_keepwarm_sec = 0;
        delay_on = 0;
        contain_on = 0;
        preheat_on = 0;
        g_stop_back_complete = NULL;
        g_somecook_running = 0;
        g_somecook_run_idx = 0;
        six_cook_reset();
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        jump_to_preheat_menu();
        g_send.iface_status = IFACE_SETTING;
        g_send.remaining_ms = -1;
        uart_print();
        break;
    case KEY_SIXMENU:       // 2: 进入第六感菜单
        if (depth > 0) {
            page_id_t cur = page_stack[depth - 1];
            if (cur == PAGE_SIXMENU || cur == PAGE_BREAD6MENU ||
                cur == PAGE_CAKE6MENU || cur == PAGE_CHICK6MENU || cur == PAGE_CHICKENMENU ||
                cur == PAGE_DUCK6MENU || cur == PAGE_VEGETABLEMENU ||
                cur == PAGE_SIXMENUTZ || cur == PAGE_CHICKMENUTZ ||
                cur == PAGE_RISINGPAGE || cur == PAGE_DESCRIPTIONMENU ||
                cur == PAGE_SIX_COOKING || cur == PAGE_TOASTCOLOR) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;   /* 防重入 */
                /* 六感运行页(非遮罩确认态):与烹饪中行为一致,弹无效提示 */
                if (cur == PAGE_SIX_COOKING && g_send.iface_status == IFACE_COOKING)
                    nav_show_invalid_hint();
                uart_print();
                break;
            }
        }
        if (g_send.iface_status == IFACE_COOKING) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            nav_show_invalid_hint();   /* 烹饪中:无效提示弹窗,不中止烹饪 */
            uart_print();
            break;
        }
        if (!menu_clean_key_allowed()) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;   /* 暂停态等非菜单页:防卫,与其他功能键一致 */
            uart_print();
            break;
        }
        /* 探针模式下也允许进入第六感（2026-08-15 需求调整） */
        g_send.buzzer_req = BUZZER_KEY_VALID;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        g_on_stop_back = 0;
        g_complete_to_stop_back = 0;
        g_delay_cancel_to_stop_back = 0;
        g_delay_cancel_btn = 0;
        g_keepwarm_active = 0;
        g_keepwarm_sec = 0;
        delay_on = 0;
        contain_on = 0;
        preheat_on = 0;
        g_stop_back_complete = NULL;
        g_somecook_running = 0;
        g_somecook_run_idx = 0;
        g_rising_choice = -1;
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        if (is_probe_inserted())
            jump_to_sixmenutz();   /* 探针模式：进探针版第六感菜单 */
        else
            jump_to_sixmenu();
        g_send.iface_status = IFACE_SETTING;
        g_send.cook_mode = MODE_NONE;
        g_send.set_temp = 0;
        g_send.set_temp_lower = 0;
        g_send.remaining_ms = -1;
        uart_print();
        break;
    case KEY_BACK:          // 21: 返回
        if (nav_favtip_active()) {
            /* 收藏结果提示中:BACK 直接关闭提示,不执行返回 */
            nav_favtip_cancel();
            uart_print();
            break;
        }
        if (nav_hint_active()) {
            /* 无效提示弹窗中:BACK 直接结束提示恢复页面,不执行返回 */
            nav_hint_cancel();
            uart_print();
            break;
        }
        if (depth <= 1) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            uart_print();
            break;
        }
        g_send.buzzer_req = BUZZER_KEY_VALID;
        {
            page_id_t cur = page_stack[depth - 1];
            if (cur == PAGE_CHICK6MENU && six_chick_get_fish_mode() == 2) {
                six_chick_fish_go_back();   /* 烤鱼子页:原地重绘回鱼/海鲜首页,不弹页 */
                g_send.buzzer_req = BUZZER_KEY_VALID;
                uart_print();
                break;
            }
            if (cur == PAGE_UPDOWN_BBQ_COOKING) {
                // 不暂停，后台继续cooking，直接跳stopback确认退出
                jump_to_updown_bbq_stop_back();
            } else if (cur == PAGE_UPDOWN_BBQ_STOP)
                jump_to_updown_bbq_stop_back();
            else if (cur == PAGE_UPDOWN_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_updown_bbq_stop_back();
            } else if (cur == PAGE_TOP_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_top_bbq_stop_back();
            } else if (cur == PAGE_BOTTOM_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_bottom_bbq_stop_back();
            } else if (cur == PAGE_HOT_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_hot_bbq_stop_back();
            } else if (cur == PAGE_HOTWIND_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_hotwind_bbq_stop_back();
            } else if (cur == PAGE_SAVE_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_save_bbq_stop_back();
            } else if (cur == PAGE_CENTRAL_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_central_bbq_stop_back();
            } else if (cur == PAGE_WINDCHANGE_BBQ_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_windchange_bbq_stop_back();
            } else if (cur == PAGE_AIR_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_air_stop_back();
            } else if (cur == PAGE_COOKIE_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_cookie_stop_back();
            } else if (cur == PAGE_WEST_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_west_stop_back();
            } else if (cur == PAGE_PIZZA_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_pizza_stop_back();
            } else if (cur == PAGE_PIZZA_2_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_pizza_2_stop_back();
            } else if (cur == PAGE_MENU_COOK_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_menu_stop_back();
            } else if (cur == PAGE_SLOWCOOK_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_slowcook_stop_back();
            } else if (cur == PAGE_UNFROZEN_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_unfrozen_stop_back();
            } else if (cur == PAGE_RISING_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_rising_stop_back();
            } else if (cur == PAGE_CORN_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_corn_stop_back();
            } else if (cur == PAGE_HEATCONTAIN_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_heatcontain_stop_back();
            } else if (cur == PAGE_UPDOWN_BBQ_COOKING_PROBE) {
                jump_to_updown_bbq_stop_back_probe();
            } else if (cur == PAGE_UPDOWN_BBQ_STOP_PROBE)
                jump_to_updown_bbq_stop_back_probe();
            else if (cur == PAGE_HOT_BBQ_COOKING_PROBE) {
                jump_to_hot_bbq_stop_back_probe();
            } else if (cur == PAGE_HOT_BBQ_STOP_PROBE)
                jump_to_hot_bbq_stop_back_probe();
            else if (cur == PAGE_BOTTOM_BBQ_COOKING_PROBE) {
                jump_to_bottom_bbq_stop_back_probe();
            } else if (cur == PAGE_BOTTOM_BBQ_STOP_PROBE)
                jump_to_bottom_bbq_stop_back_probe();
            else if (cur == PAGE_SLOWCOOK_COOKING_PROBE) {
                jump_to_slowcook_stop_back_probe();
            } else if (cur == PAGE_SLOWCOOK_STOP_PROBE)
                jump_to_slowcook_stop_back_probe();
            else if (cur == PAGE_COLOR_COOKING)
                jump_to_color_stop_back();
            else if (cur == PAGE_COLOR_STOP)
                jump_to_color_stop_back();
            else if (cur == PAGE_EXTRA_COLOR) {
                g_extra_color_to_stop_back = 1;
                jump_to_color_stop_back();
            }
            else if (cur == PAGE_COLOR_COOKING_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_color_stop_back();
            }
            else if (cur == PAGE_TOP_BBQ_COOKING)
                jump_to_top_bbq_stop_back();
            else if (cur == PAGE_TOP_BBQ_STOP)
                jump_to_top_bbq_stop_back();
            else if (cur == PAGE_BOTTOM_BBQ_COOKING)
                jump_to_bottom_bbq_stop_back();
            else if (cur == PAGE_BOTTOM_BBQ_STOP)
                jump_to_bottom_bbq_stop_back();
            else if (cur == PAGE_HOT_BBQ_COOKING)
                jump_to_hot_bbq_stop_back();
            else if (cur == PAGE_HOT_BBQ_STOP)
                jump_to_hot_bbq_stop_back();
            else if (cur == PAGE_HOTWIND_BBQ_COOKING)
                jump_to_hotwind_bbq_stop_back();
            else if (cur == PAGE_HOTWIND_BBQ_STOP)
                jump_to_hotwind_bbq_stop_back();
            else if (cur == PAGE_SAVE_BBQ_COOKING)
                jump_to_save_bbq_stop_back();
            else if (cur == PAGE_SAVE_BBQ_STOP)
                jump_to_save_bbq_stop_back();
            else if (cur == PAGE_CENTRAL_BBQ_COOKING)
                jump_to_central_bbq_stop_back();
            else if (cur == PAGE_CENTRAL_BBQ_STOP)
                jump_to_central_bbq_stop_back();
            else if (cur == PAGE_WINDCHANGE_BBQ_COOKING)
                jump_to_windchange_bbq_stop_back();
            else if (cur == PAGE_WINDCHANGE_BBQ_STOP)
                jump_to_windchange_bbq_stop_back();
            else if (cur == PAGE_PREHEAT_COOKING)
                jump_to_preheat_stop_back();
            else if (cur == PAGE_DELAYCOOKING) {
                g_delay_cancel_to_stop_back = 1;
                delay_cancel_to_stop_back();
            }
            else if (cur == PAGE_SOMECOOK_COOKING) {
                somecook_cooking_handle_back();
            }
            else if (cur == PAGE_SIX_COOKING) {
                six_cook_handle_back();
            }
            else if (cur == PAGE_CHICKENCOOKING) {
                six_chick_handle_back();   /* 烤全鸡烹饪:BACK → 遮罩确认/完成退出 */
            }
            else if (cur == PAGE_PROBETIP) {
                probetip_dismiss_now();   /* 探针提示:BACK 提前结束(恢复 iface + 回 prev) */
            }
            else if (cur == PAGE_TEMPTIP) {
                temptip_dismiss_now();   /* 温度提示:BACK 提前结束(恢复 iface + 回 prev) */
            }
            else if (cur == PAGE_LOUDNESS) {
                return_Loudness_action();   /* 声音页 BACK:弹窗开则关弹窗,否则回设置覆盖层 */
            }
            else if (cur == PAGE_SET_VAL) {
                set_val_return_action();    /* 数值条页 BACK:回声音页/设置层 */
            }
            else if (cur == PAGE_SET_COUNT) {
                count_down_back_action();   /* 计时器 BACK:回设置层(运行中后台继续) */
            }
            else if (cur == PAGE_SET_SYSTIME) {
                systime_back_action();      /* 日期时间 BACK:不写 RTC 回设置层 */
            }
            else if (cur == PAGE_SCREEN_SET) {
                screen_set_back();
            }
            else if (cur == PAGE_SOMECOOK) {
                if (current_group == g_somecook_edit) {
                    /* 步骤容器编辑组 BACK → 回对应大按钮（不弹栈） */
                    somecook_t *sc = somecook_get(&ui_manager);
                    lv_obj_t *df = lv_group_get_focused(current_group);
                    int step = 0;
                    if (sc && (df == sc->edit2 || df == sc->delete2)) step = 1;
                    else if (sc && (df == sc->edit3 || df == sc->delete3)) step = 2;
                    somecook_back_to_btns(step);
                } else {
                    page_pop();   /* 回 special_menu */
                }
            }
            else if (cur == PAGE_PREHEAT_STOP)
                jump_to_preheat_stop_back();
            else if (cur == PAGE_PREHEAT_COMPLETE) {
                if (g_send.cook_mode != MODE_PREHEAT) {
                    g_complete_to_stop_back = 1;
                    jump_to_preheat_stop_back();
                } else {
                    preheat_complete_exit();
                }
            }
            else if (cur == PAGE_COOKIE_COOKING)
                jump_to_cookie_stop_back();
            else if (cur == PAGE_COOKIE_STOP)
                jump_to_cookie_stop_back();
            else if (cur == PAGE_WEST_COOKING)
                jump_to_west_stop_back();
            else if (cur == PAGE_WEST_STOP)
                jump_to_west_stop_back();
            else if (cur == PAGE_PIZZA_COOKING)
                jump_to_pizza_stop_back();
            else if (cur == PAGE_PIZZA_STOP)
                jump_to_pizza_stop_back();
            else if (cur == PAGE_MENU_COOK_COOKING)
                jump_to_menu_stop_back();
            else if (cur == PAGE_MENU_COOK_STOP)
                jump_to_menu_stop_back();
            else if (cur == PAGE_AIR_COOKING)
                jump_to_air_stop_back();
            else if (cur == PAGE_AIR_STOP)
                jump_to_air_stop_back();
            else if (cur == PAGE_PIZZA_2_COOKING)
                jump_to_pizza_2_stop_back();
            else if (cur == PAGE_PIZZA_2_STOP)
                jump_to_pizza_2_stop_back();
            else if (cur == PAGE_SLOWCOOK_COOKING)
                jump_to_slowcook_stop_back();
            else if (cur == PAGE_SLOWCOOK_STOP)
                jump_to_slowcook_stop_back();
            else if (cur == PAGE_UNFROZEN_COOKING)
                jump_to_unfrozen_stop_back();
            else if (cur == PAGE_UNFROZEN_STOP)
                jump_to_unfrozen_stop_back();
            else if (cur == PAGE_RISING_COOKING)
                jump_to_rising_stop_back();
            else if (cur == PAGE_RISING_STOP)
                jump_to_rising_stop_back();
            else if (cur == PAGE_CORN_COOKING)
                jump_to_corn_stop_back();
            else if (cur == PAGE_CORN_STOP)
                jump_to_corn_stop_back();
            else if (cur == PAGE_HEATCONTAIN_COOKING)
                jump_to_heatcontain_stop_back();
            else if (cur == PAGE_HEATCONTAIN_STOP)
                jump_to_heatcontain_stop_back();
            else if (cur == PAGE_LASAGNA_COOKING)
                jump_to_lasagna_stop_back();
            else if (cur == PAGE_LASAGNA_STOP)
                jump_to_lasagna_stop_back();
            else if (cur == PAGE_STRUDEL_COOKING)
                jump_to_strudel_stop_back();
            else if (cur == PAGE_STRUDEL_STOP)
                jump_to_strudel_stop_back();
            else if (cur == PAGE_BREAD_COOKING)
                jump_to_bread_stop_back();
            else if (cur == PAGE_BREAD_STOP)
                jump_to_bread_stop_back();
            else if (cur == PAGE_PIZZA3_COOKING)
                jump_to_pizza3_stop_back();
            else if (cur == PAGE_PIZZA3_STOP)
                jump_to_pizza3_stop_back();
            else if (cur == PAGE_CHIP_COOKING)
                jump_to_chip_stop_back();
            else if (cur == PAGE_CHIP_STOP)
                jump_to_chip_stop_back();
            else if (cur == PAGE_CUSTOM_COOKING)
                jump_to_custom_stop_back();
            else if (cur == PAGE_CUSTOM_STOP)
                jump_to_custom_stop_back();
            else if (cur == PAGE_WATER_CLEAN_COOKING)
                jump_to_wc_stop_back();
            else if (cur == PAGE_WATER_CLEAN_STOP)
                jump_to_wc_stop_back();
            else if (cur == PAGE_HOTCLEANSAVE_COOKING)
                jump_to_hcs_stop_back();
            else if (cur == PAGE_HOTCLEANSAVE_STOP)
                jump_to_hcs_stop_back();
            else if (cur == PAGE_HOTCLEANSAVE_COOLING) {
                g_cooling_to_stop_back = 1;
                jump_to_hcs_stop_back();
                g_on_stop_back = 0;
            }
            else if (cur == PAGE_HOTCLEANMIDDLE_COOKING)
                jump_to_hcm_stop_back();
            else if (cur == PAGE_HOTCLEANMIDDLE_STOP)
                jump_to_hcm_stop_back();
            else if (cur == PAGE_HOTCLEANMIDDLE_COOLING) {
                g_cooling_to_stop_back = 1;
                jump_to_hcm_stop_back();
                g_on_stop_back = 0;
            }
            else if (cur == PAGE_HOTCLEANHIGH_COOKING)
                jump_to_hch_stop_back();
            else if (cur == PAGE_HOTCLEANHIGH_STOP)
                jump_to_hch_stop_back();
            else if (cur == PAGE_HOTCLEANHIGH_COOLING) {
                g_cooling_to_stop_back = 1;
                jump_to_hch_stop_back();
                g_on_stop_back = 0;
            }
            // B-2 clean complete
            else if (cur == PAGE_WATER_CLEAN_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_wc_stop_back();
            } else if (cur == PAGE_HOTCLEANSAVE_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_hcs_stop_back();
            } else if (cur == PAGE_HOTCLEANMIDDLE_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_hcm_stop_back();
            } else if (cur == PAGE_HOTCLEANHIGH_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_hch_stop_back();
            }
            // B-1 frozen bake complete
            else if (cur == PAGE_LASAGNA_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_lasagna_stop_back();
            } else if (cur == PAGE_STRUDEL_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_strudel_stop_back();
            } else if (cur == PAGE_BREAD_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_bread_stop_back();
            } else if (cur == PAGE_PIZZA3_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_pizza3_stop_back();
            } else if (cur == PAGE_CHIP_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_chip_stop_back();
            } else if (cur == PAGE_CUSTOM_COMPLETE) {
                g_complete_to_stop_back = 1;
                jump_to_custom_stop_back();
            }
            // B-3 probe complete
            else if (cur == PAGE_UPDOWN_BBQ_COMPLETE_PROBE) {
                g_complete_to_stop_back = 1;
                jump_to_updown_bbq_stop_back_probe();
            } else if (cur == PAGE_BOTTOM_BBQ_COMPLETE_PROBE) {
                g_complete_to_stop_back = 1;
                jump_to_bottom_bbq_stop_back_probe();
            } else if (cur == PAGE_HOT_BBQ_COMPLETE_PROBE) {
                g_complete_to_stop_back = 1;
                jump_to_hot_bbq_stop_back_probe();
            } else if (cur == PAGE_SLOWCOOK_COMPLETE_PROBE) {
                g_complete_to_stop_back = 1;
                jump_to_slowcook_stop_back_probe();
            } else if (cur == PAGE_FAVORITES) {
                return_favorites_action();   /* 收藏页返回 */
            }
            else
                page_pop();
        }
        uart_print();
        break;
    case KEY_ENCODER_CW: {  // 31: 焦点下移 / 数值+
#ifdef LV_USE_AIC_SIMULATOR
        if (current_group == g_preheat_cooking) {
            g_sim_cavity_temp += 5;
            if (g_sim_cavity_temp > 300) g_sim_cavity_temp = 300;
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (current_group == g_preheat_stop_back) {
            g_sim_cavity_temp += 5;
            if (g_sim_cavity_temp > 300) g_sim_cavity_temp = 300;
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (depth > 0 && (page_stack[depth-1] == PAGE_HOTCLEANSAVE_COOLING ||
            page_stack[depth-1] == PAGE_HOTCLEANMIDDLE_COOLING ||
            page_stack[depth-1] == PAGE_HOTCLEANHIGH_COOLING ||
            page_stack[depth-1] == PAGE_HOTCLEANSAVE_STOP_BACK ||
            page_stack[depth-1] == PAGE_HOTCLEANMIDDLE_STOP_BACK ||
            page_stack[depth-1] == PAGE_HOTCLEANHIGH_STOP_BACK)) {
            g_sim_cavity_temp += 5;
            if (g_sim_cavity_temp > 400) g_sim_cavity_temp = 400;
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
#endif
        /* 数值条页(按键音/亮度)无组:按栈分流,须在 current_group 空守卫之前 */
        if (depth > 0 && page_stack[depth - 1] == PAGE_SET_VAL) {
            set_val_encoder_action(KEY_ENCODER_CW);
            uart_print();
            break;
        }
        if (!current_group) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            uart_print();
            break;
        }
        if (current_group == g_favorites) {
            encoder_favorites_action(KEY_ENCODER_CW);   /* 收藏页：翻页/移焦点 */
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (current_group == loudness_page_group()) {
            encoder_Loudness_action(KEY_ENCODER_CW);   /* 声音页:行间移焦点 */
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (current_group == count_down_page_group()) {
            g_send.buzzer_req = BUZZER_ENCODER;
            encoder_count_down_action(KEY_ENCODER_CW);   /* 计时器页:移焦点/数值+ */
            uart_print();
            break;
        }
        if (current_group == systime_page_group()) {
            g_send.buzzer_req = BUZZER_ENCODER;
            encoder_systime_action(KEY_ENCODER_CW);   /* 日期时间页:移字段/数值+ */
            uart_print();
            break;
        }
        if (current_group == g_sixset2) {
            sixset2_cycle(+1);   /* 按焦点分派:选择对象上切值,其他移焦点 */
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (current_group == g_toastcolor) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            toastcolor_t *tc = toastcolor_get(&ui_manager);
            if (tc && df != tc->degree && df != tc->weight && df != tc->Maturity) {
                /* 第一次转动:仅移焦点到当前组标签（烤色程度/份量/成熟度），不切档位 */
                lv_group_focus_obj((g_toast_mode == TOAST_MODE_WEIGHT) ? tc->weight :
                                   (g_toast_mode == TOAST_MODE_MATURITY) ? tc->Maturity : tc->degree);
            } else {
                toastcolor_cycle(+1);
            }
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (current_group == g_delayset) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            delayset_t *ds = delayset_get(&ui_manager);
            if (ds && df == ds->hour) {
                delay_hour++;
                if (delay_hour > 23) delay_hour = 0;
            } else if (ds && df == ds->min) {
                delay_min++;
                if (delay_min > 59) delay_min = 0;
            } else {
                g_send.buzzer_req = BUZZER_ENCODER;
                lv_group_focus_next(current_group);
                uart_print();
                break;
            }
            g_send.buzzer_req = BUZZER_ENCODER;
            delayset_refresh_display(ds);
            uart_print();
            break;
        }
        if (current_group == g_stepset) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            stepset_t *ss = stepset_get(&ui_manager);
            if (ss && df == ss->roller_main) {
                uint32_t sel = lv_roller_get_selected(ss->roller_main);
                uint32_t cnt = lv_roller_get_option_count(ss->roller_main);
                if (sel >= cnt - 1) {
                    g_send.buzzer_req = BUZZER_KEY_INVALID;
                    uart_print();
                    break;
                }
                lv_roller_set_selected(ss->roller_main, sel + 1, LV_ANIM_ON);
                lv_obj_send_event(ss->roller_main, LV_EVENT_VALUE_CHANGED, NULL);
                g_send.buzzer_req = BUZZER_ENCODER;
                uart_print();
                break;
            } else if (ss && df == ss->roller_mode) {
                uint32_t sel = lv_roller_get_selected(ss->roller_mode);
                uint32_t cnt = lv_roller_get_option_count(ss->roller_mode);
                if (sel >= cnt - 1) {
                    g_send.buzzer_req = BUZZER_KEY_INVALID;
                    uart_print();
                    break;
                }
                lv_roller_set_selected(ss->roller_mode, sel + 1, LV_ANIM_ON);
                lv_obj_send_event(ss->roller_mode, LV_EVENT_VALUE_CHANGED, NULL);
                g_send.buzzer_req = BUZZER_ENCODER;
                uart_print();
                break;
            }
            /* temp/hour/min/next:放行到通用逻辑(编辑字段调值 / 焦点移动) */
        }
        lv_obj_t *focused = lv_group_get_focused(current_group);
        edit_field_t *ef = find_edit_field(focused);
        if (ef) {
            if (ef->min == ef->max) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;
            } else {
                g_send.buzzer_req = BUZZER_ENCODER;
                adjust_value(ef, +1);
                printf("[nav] adjust +: %d\n", *ef->value);
            }
        } else {
            if (!current_group) { g_send.buzzer_req = BUZZER_KEY_INVALID; uart_print(); break; }
            g_send.buzzer_req = BUZZER_ENCODER;
            lv_group_focus_next(current_group);
            printf("[nav] focus next\n");
        }
        uart_print();
        break;
    }
    case KEY_ENCODER_CCW: {  // 41: 焦点上移 / 数值-
#ifdef LV_USE_AIC_SIMULATOR
        if (current_group == g_preheat_cooking) {
            g_sim_cavity_temp -= 5;
            if (g_sim_cavity_temp > 300) g_sim_cavity_temp = 300;
            if (g_sim_cavity_temp < 0) g_sim_cavity_temp = 0;
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (current_group == g_preheat_stop_back) {
            g_sim_cavity_temp -= 5;
            if (g_sim_cavity_temp > 300) g_sim_cavity_temp = 300;
            if (g_sim_cavity_temp < 0) g_sim_cavity_temp = 0;
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (depth > 0 && (page_stack[depth-1] == PAGE_HOTCLEANSAVE_COOLING ||
            page_stack[depth-1] == PAGE_HOTCLEANMIDDLE_COOLING ||
            page_stack[depth-1] == PAGE_HOTCLEANHIGH_COOLING ||
            page_stack[depth-1] == PAGE_HOTCLEANSAVE_STOP_BACK ||
            page_stack[depth-1] == PAGE_HOTCLEANMIDDLE_STOP_BACK ||
            page_stack[depth-1] == PAGE_HOTCLEANHIGH_STOP_BACK)) {
            g_sim_cavity_temp -= 5;
            if (g_sim_cavity_temp > 400) g_sim_cavity_temp = 400;
            if (g_sim_cavity_temp < 0) g_sim_cavity_temp = 0;
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
#endif
        /* 数值条页(按键音/亮度)无组:按栈分流,须在 current_group 空守卫之前 */
        if (depth > 0 && page_stack[depth - 1] == PAGE_SET_VAL) {
            set_val_encoder_action(KEY_ENCODER_CCW);
            uart_print();
            break;
        }
        if (!current_group) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            uart_print();
            break;
        }
        if (current_group == g_favorites) {
            encoder_favorites_action(KEY_ENCODER_CCW);   /* 收藏页：翻页/移焦点 */
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (current_group == loudness_page_group()) {
            encoder_Loudness_action(KEY_ENCODER_CCW);   /* 声音页:行间移焦点 */
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (current_group == count_down_page_group()) {
            g_send.buzzer_req = BUZZER_ENCODER;
            encoder_count_down_action(KEY_ENCODER_CCW);   /* 计时器页:移焦点/数值- */
            uart_print();
            break;
        }
        if (current_group == systime_page_group()) {
            g_send.buzzer_req = BUZZER_ENCODER;
            encoder_systime_action(KEY_ENCODER_CCW);   /* 日期时间页:移字段/数值- */
            uart_print();
            break;
        }
        if (current_group == g_sixset2) {
            sixset2_cycle(-1);   /* 按焦点分派:选择对象上切值,其他移焦点 */
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (current_group == g_toastcolor) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            toastcolor_t *tc = toastcolor_get(&ui_manager);
            if (tc && df != tc->degree && df != tc->weight && df != tc->Maturity) {
                /* 第一次转动:仅移焦点到当前组标签（烤色程度/份量/成熟度），不切档位 */
                lv_group_focus_obj((g_toast_mode == TOAST_MODE_WEIGHT) ? tc->weight :
                                   (g_toast_mode == TOAST_MODE_MATURITY) ? tc->Maturity : tc->degree);
            } else {
                toastcolor_cycle(-1);
            }
            g_send.buzzer_req = BUZZER_ENCODER;
            uart_print();
            break;
        }
        if (current_group == g_delayset) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            delayset_t *ds = delayset_get(&ui_manager);
            if (ds && df == ds->hour) {
                delay_hour--;
                if (delay_hour < 0) delay_hour = 23;
            } else if (ds && df == ds->min) {
                delay_min--;
                if (delay_min < 0) delay_min = 59;
            } else {
                g_send.buzzer_req = BUZZER_ENCODER;
                lv_group_focus_prev(current_group);
                uart_print();
                break;
            }
            g_send.buzzer_req = BUZZER_ENCODER;
            delayset_refresh_display(ds);
            uart_print();
            break;
        }
        if (current_group == g_stepset) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            stepset_t *ss = stepset_get(&ui_manager);
            if (ss && df == ss->next) {
                /* 仿 updown menu:从 next 左转跳到 mainroller */
                lv_group_focus_obj(ss->roller_main);
                g_send.buzzer_req = BUZZER_ENCODER;
                uart_print();
                break;
            }
            if (ss && df == ss->roller_main) {
                uint32_t sel = lv_roller_get_selected(ss->roller_main);
                if (sel <= 0) {
                    g_send.buzzer_req = BUZZER_KEY_INVALID;
                    uart_print();
                    break;
                }
                lv_roller_set_selected(ss->roller_main, sel - 1, LV_ANIM_ON);
                lv_obj_send_event(ss->roller_main, LV_EVENT_VALUE_CHANGED, NULL);
                g_send.buzzer_req = BUZZER_ENCODER;
                uart_print();
                break;
            } else if (ss && df == ss->roller_mode) {
                uint32_t sel = lv_roller_get_selected(ss->roller_mode);
                if (sel <= 0) {
                    g_send.buzzer_req = BUZZER_KEY_INVALID;
                    uart_print();
                    break;
                }
                lv_roller_set_selected(ss->roller_mode, sel - 1, LV_ANIM_ON);
                lv_obj_send_event(ss->roller_mode, LV_EVENT_VALUE_CHANGED, NULL);
                g_send.buzzer_req = BUZZER_ENCODER;
                uart_print();
                break;
            }
            /* temp/hour/min/next:放行到通用逻辑(编辑字段调值 / 焦点移动) */
        }
        lv_obj_t *focused = lv_group_get_focused(current_group);
        edit_field_t *ef = find_edit_field(focused);
        g_send.buzzer_req = BUZZER_ENCODER;
        if (ef) {
            if (ef->min == ef->max) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;
            } else {
                g_send.buzzer_req = BUZZER_ENCODER;
                adjust_value(ef, -1);
                printf("[nav] adjust -: %d\n", *ef->value);
            }
        } else if (current_group == g_updown_bbq_menu) {
            updown_bbq_menu_t *bbq = updown_bbq_menu_get(&ui_manager);
            if (bbq && focused == bbq->next_button) {
                lv_group_focus_obj(bbq->tempnum_label);
                printf("[nav] focus wrap to tempnum\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[nav] focus prev\n");
            }
        } else if (current_group == g_updown_bbq_menu_probe) {
            updown_bbq_menu_probe_t *menu = updown_bbq_menu_probe_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[updown_bbq_probe] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[updown_bbq_probe] focus prev\n");
            }
        } else if (current_group == g_hot_bbq_menu_probe) {
            hot_bbq_menu_probe_t *menu = hot_bbq_menu_probe_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[hot_bbq_probe] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[hot_bbq_probe] focus prev\n");
            }
        } else if (current_group == g_bottom_bbq_menu_probe) {
            bottom_bbq_menu_probe_t *menu = bottom_bbq_menu_probe_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[bottom_bbq_probe] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[bottom_bbq_probe] focus prev\n");
            }
        } else if (current_group == g_slowcook_menu_probe) {
            slowcook_menu_probe_t *menu = slowcook_menu_probe_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[slowcook_probe] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[slowcook_probe] focus prev\n");
            }
        } else if (current_group == g_updown_bbq_setting) {
            updown_bbq_setting_t *set = updown_bbq_setting_get(&ui_manager);
            if (set && focused == set->sure_button) {
                lv_group_focus_obj(set->tempup_label);
                printf("[nav] setting focus wrap to tempup\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[nav] focus prev\n");
            }
        } else if (current_group == g_top_bbq_menu) {
            top_bbq_menu_t *menu = top_bbq_menu_get(&ui_manager);
            if (menu && focused == menu->next_button) {
                lv_group_focus_obj(menu->temp_label);
                printf("[top_bbq] focus wrap to temp_label\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[top_bbq] focus prev\n");
            }
        } else if (current_group == g_top_bbq_setting) {
            top_bbq_setting_t *set = top_bbq_setting_get(&ui_manager);
            if (set && focused == set->surebutton) {
                lv_group_focus_obj(set->temp);
                printf("[top_bbq] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[top_bbq] focus prev\n");
            }
        } else if (current_group == g_bottom_bbq_menu) {
            bottom_bbq_menu_t *menu = bottom_bbq_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[bottom_bbq] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[bottom_bbq] focus prev\n");
            }
        } else if (current_group == g_bottom_bbq_setting) {
            bottom_bbq_setting_t *set = bottom_bbq_setting_get(&ui_manager);
            if (set && focused == set->surebutton) {
                lv_group_focus_obj(set->temp);
                printf("[bottom_bbq] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[bottom_bbq] focus prev\n");
            }
        } else if (current_group == g_hot_bbq_menu) {
            hot_bbq_menu_t *menu = hot_bbq_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[hot_bbq] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[hot_bbq] focus prev\n");
            }
        } else if (current_group == g_hot_bbq_setting) {
            hot_bbq_setting_t *set = hot_bbq_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[hot_bbq] setting focus wrap to temp\n");
            } else {
            lv_group_focus_prev(current_group);
            printf("[hot_bbq] focus prev\n");
        }
    } else if (current_group == g_hotwind_bbq_menu) {
        hotwind_bbq_menu_t *menu = hotwind_bbq_menu_get(&ui_manager);
        if (menu && focused == menu->next) {
            lv_group_focus_obj(menu->temp);
            printf("[hotwind_bbq] focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[hotwind_bbq] focus prev\n");
        }
    } else if (current_group == g_hotwind_bbq_setting) {
        hotwind_bbq_setting_t *set = hotwind_bbq_setting_get(&ui_manager);
        if (set && focused == set->sure) {
            lv_group_focus_obj(set->temp);
            printf("[hotwind_bbq] setting focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[hotwind_bbq] focus prev\n");
        }
    } else if (current_group == g_save_bbq_menu) {
        save_bbq_menu_t *menu = save_bbq_menu_get(&ui_manager);
        if (menu && focused == menu->next) {
            lv_group_focus_obj(menu->temp);
            printf("[save_bbq] focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[save_bbq] focus prev\n");
        }
    } else if (current_group == g_save_bbq_setting) {
        save_bbq_setting_t *set = save_bbq_setting_get(&ui_manager);
        if (set && focused == set->sure) {
            lv_group_focus_obj(set->temp);
            printf("[save_bbq] setting focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[save_bbq] focus prev\n");
        }
    } else if (current_group == g_central_bbq_menu) {
        central_bbq_menu_t *menu = central_bbq_menu_get(&ui_manager);
        if (menu && focused == menu->next) {
            lv_group_focus_obj(menu->temp);
            printf("[central_bbq] focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[central_bbq] focus prev\n");
        }
    } else if (current_group == g_central_bbq_setting) {
        central_bbq_setting_t *set = central_bbq_setting_get(&ui_manager);
        if (set && focused == set->sure) {
            lv_group_focus_obj(set->temp);
            printf("[central_bbq] setting focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[central_bbq] focus prev\n");
        }
    } else if (current_group == g_windchange_bbq_menu) {
        windchange_bbq_menu_t *menu = windchange_bbq_menu_get(&ui_manager);
        if (menu && focused == menu->next) {
            lv_group_focus_obj(menu->temp);
            printf("[windchange_bbq] focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[windchange_bbq] focus prev\n");
        }
    } else if (current_group == g_windchange_bbq_setting) {
        windchange_bbq_setting_t *set = windchange_bbq_setting_get(&ui_manager);
        if (set && focused == set->sure) {
            lv_group_focus_obj(set->temp);
            printf("[windchange_bbq] setting focus wrap to temp\n");
        } else {
            lv_group_focus_prev(current_group);
            printf("[windchange_bbq] focus prev\n");
        }
        } else if (current_group == g_cookie_menu) {
            cookie_menu_t *menu = cookie_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[cookie] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[cookie] focus prev\n");
            }
        } else if (current_group == g_cookie_setting) {
            cookie_setting_t *set = cookie_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[cookie] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[cookie] setting focus prev\n");
            }
        } else if (current_group == g_west_menu) {
            west_menu_t *menu = west_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[west] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[west] focus prev\n");
            }
        } else if (current_group == g_west_setting) {
            west_setting_t *set = west_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[west] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[west] setting focus prev\n");
            }
        } else if (current_group == g_pizza_menu) {
            pizza_menu_t *menu = pizza_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[pizza] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[pizza] focus prev\n");
            }
        } else if (current_group == g_pizza_setting) {
            pizza_setting_t *set = pizza_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[pizza] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[pizza] setting focus prev\n");
            }
        } else if (current_group == g_menu_cook_menu) {
            menu_menu_t *menu = menu_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[menu] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[menu] focus prev\n");
            }
        } else if (current_group == g_menu_cook_setting) {
            menu_setting_t *set = menu_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[menu] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[menu] setting focus prev\n");
            }
        } else if (current_group == g_air_menu) {
            air_menu_t *menu = air_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[air] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[air] focus prev\n");
            }
        } else if (current_group == g_air_setting) {
            air_setting_t *set = air_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[air] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[air] setting focus prev\n");
            }
        } else if (current_group == g_pizza_2_menu) {
            pizza_2_menu_t *menu = pizza_2_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[pizza_2] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[pizza_2] focus prev\n");
            }
        } else if (current_group == g_pizza_2_setting) {
            pizza_2_setting_t *set = pizza_2_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[pizza_2] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[pizza_2] setting focus prev\n");
            }
        } else if (current_group == g_slowcook_menu) {
            slowcook_menu_t *menu = slowcook_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[slowcook] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[slowcook] focus prev\n");
            }
        } else if (current_group == g_slowcook_setting) {
            slowcook_setting_t *set = slowcook_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[slowcook] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[slowcook] setting focus prev\n");
            }
        } else if (current_group == g_unfrozen_menu) {
            unfrozen_menu_t *menu = unfrozen_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[unfrozen] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[unfrozen] focus prev\n");
            }
        } else if (current_group == g_unfrozen_setting) {
            unfrozen_setting_t *set = unfrozen_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[unfrozen] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[unfrozen] setting focus prev\n");
            }
        } else if (current_group == g_rising_menu) {
            rising_menu_t *menu = rising_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[rising] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[rising] focus prev\n");
            }
        } else if (current_group == g_rising_setting) {
            rising_setting_t *set = rising_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[rising] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[rising] setting focus prev\n");
            }
        } else if (current_group == g_corn_menu) {
            corn_menu_t *menu = corn_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[corn] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[corn] focus prev\n");
            }
        } else if (current_group == g_corn_setting) {
            corn_setting_t *set = corn_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[corn] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[corn] setting focus prev\n");
            }
        } else if (current_group == g_heatcontain_menu) {
            heatcontain_menu_t *menu = heatcontain_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->temp);
                printf("[heatcontain] focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[heatcontain] focus prev\n");
            }
        } else if (current_group == g_heatcontain_setting) {
            heatcontain_setting_t *set = heatcontain_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->temp);
                printf("[heatcontain] setting focus wrap to temp\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[heatcontain] setting focus prev\n");
            }
        } else if (current_group == g_lasagna_menu) {
            lasagna_menu_t *menu = lasagna_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->hour);
                printf("[lasagna] focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[lasagna] focus prev\n");
            }
        } else if (current_group == g_lasagna_setting) {
            lasagna_setting_t *set = lasagna_setting_get(&ui_manager);
            if (set && focused == set->button_200) {
                lv_group_focus_obj(set->hour);
                printf("[lasagna] setting focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[lasagna] setting focus prev\n");
            }
        } else if (current_group == g_strudel_menu) {
            strudel_menu_t *menu = strudel_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->hour);
                printf("[strudel] focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[strudel] focus prev\n");
            }
        } else if (current_group == g_strudel_setting) {
            strudel_setting_t *set = strudel_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->hour);
                printf("[strudel] setting focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[strudel] setting focus prev\n");
            }
        } else if (current_group == g_bread_menu) {
            bread_menu_t *menu = bread_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->hour);
                printf("[bread] focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[bread] focus prev\n");
            }
        } else if (current_group == g_bread_setting) {
            bread_setting_t *set = bread_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->hour);
                printf("[bread] setting focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[bread] setting focus prev\n");
            }
        } else if (current_group == g_pizza3_menu) {
            pizza3_menu_t *menu = pizza3_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->hour);
                printf("[pizza3] focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[pizza3] focus prev\n");
            }
        } else if (current_group == g_pizza3_setting) {
            pizza3_setting_t *set = pizza3_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->hour);
                printf("[pizza3] setting focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[pizza3] setting focus prev\n");
            }
        } else if (current_group == g_chip_menu) {
            chip_menu_t *menu = chip_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->hour);
                printf("[chip] focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[chip] focus prev\n");
            }
        } else if (current_group == g_chip_setting) {
            chip_setting_t *set = chip_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->hour);
                printf("[chip] setting focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[chip] setting focus prev\n");
            }
        } else if (current_group == g_custom_menu) {
            custom_menu_t *menu = custom_menu_get(&ui_manager);
            if (menu && focused == menu->next) {
                lv_group_focus_obj(menu->hour);
                printf("[custom] focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[custom] focus prev\n");
            }
        } else if (current_group == g_custom_setting) {
            custom_setting_t *set = custom_setting_get(&ui_manager);
            if (set && focused == set->sure) {
                lv_group_focus_obj(set->hour);
                printf("[custom] setting focus wrap to hour\n");
            } else {
                lv_group_focus_prev(current_group);
                printf("[custom] setting focus prev\n");
            }
        } else {
            if (!current_group) { g_send.buzzer_req = BUZZER_KEY_INVALID; uart_print(); break; }
            lv_group_focus_prev(current_group);
            printf("[nav] focus prev\n");
        }
        uart_print();
        break;
    }
    case KEY_ENCODER_PRESS: { // 51: 确认 / 跳到下一焦点
        /* 数值条页(按键音/亮度)无组:按栈分流,须在 current_group 空守卫之前 */
        if (depth > 0 && page_stack[depth - 1] == PAGE_SET_VAL) {
            set_val_encoder_action(KEY_ENCODER_PRESS);   /* 返回声音页/设置层 */
            uart_print();
            break;
        }
        if (!current_group) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            break;
        }
        if (current_group == g_delayset) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            delayset_t *ds = delayset_get(&ui_manager);
            if (ds && df == ds->start) {
                g_send.buzzer_req = BUZZER_KEY_VALID;
                lv_obj_send_event(ds->start, LV_EVENT_CLICKED, NULL);
            } else {
                g_send.buzzer_req = BUZZER_KEY_VALID;
                lv_group_focus_next(current_group);
            }
            uart_print();
            break;
        }
        if (current_group == g_stepset) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            stepset_t *ss = stepset_get(&ui_manager);
            if (ss && df == ss->next) {
                g_send.buzzer_req = BUZZER_KEY_VALID;
                lv_obj_send_event(ss->next, LV_EVENT_CLICKED, NULL);
            } else {
                g_send.buzzer_req = BUZZER_KEY_VALID;
                lv_group_focus_next(current_group);
            }
            uart_print();
            break;
        }
        if (current_group == loudness_page_group()) {
            encoder_Loudness_action(KEY_ENCODER_PRESS);   /* 声音页:进弹窗/确认 */
            g_send.buzzer_req = BUZZER_KEY_VALID;
            uart_print();
            break;
        }
        if (current_group == count_down_page_group()) {
            g_send.buzzer_req = BUZZER_KEY_VALID;
            encoder_count_down_action(KEY_ENCODER_PRESS);   /* 计时器页:启动/清零/移光标 */
            uart_print();
            break;
        }
        if (current_group == systime_page_group()) {
            g_send.buzzer_req = BUZZER_KEY_VALID;
            encoder_systime_action(KEY_ENCODER_PRESS);   /* 日期时间页:写 RTC/移字段 */
            uart_print();
            break;
        }
        if (current_group == g_sixset2) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            sixset2_t *pg = sixset2_get(&ui_manager);
            if (pg && (df == pg->weight || df == pg->maturity || df == pg->degree)) {
                g_send.buzzer_req = BUZZER_KEY_VALID;
                sixset2_press_focus();   /* 焦点循环:weight/maturity→degree→next */
                uart_print();
                break;
            }
            /* next:放行通用逻辑(点击 → 描述页) */
        }
        if (current_group == g_toastcolor) {
            lv_obj_t *df = lv_group_get_focused(current_group);
            toastcolor_t *tc = toastcolor_get(&ui_manager);
            if (tc && (df == tc->degree || df == tc->weight || df == tc->Maturity)) {
                g_send.buzzer_req = BUZZER_KEY_VALID;
                lv_group_focus_obj(tc->next);   /* 确定:从当前组标签切到下一焦点 */
                uart_print();
                break;
            }
            /* next:放行通用逻辑(点击 → 回上色准备态) */
        }
        if (current_group == g_favorites) {
            g_send.buzzer_req = BUZZER_KEY_VALID;
            encoder_favorites_action(KEY_ENCODER_PRESS);   /* 收藏页：选中卡片/启动/删除 */
            uart_print();
            break;
        }
        lv_obj_t *focused = lv_group_get_focused(current_group);
        edit_field_t *ef = find_edit_field(focused);
        if (ef) {
            g_send.buzzer_req = BUZZER_KEY_VALID;
            lv_group_focus_next(current_group);
            printf("[nav] press -> next focus\n");
        } else if (focused) {
            g_send.buzzer_req = BUZZER_KEY_VALID;
            lv_obj_send_event(focused, LV_EVENT_CLICKED, NULL);
            printf("[nav] press -> click\n");
        } else {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
        }
        uart_print();
        break;
    }
    default:
        printf("[nav] unknown key: %d\n", key);
        break;
    }
}

