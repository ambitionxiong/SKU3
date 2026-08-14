#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"

/* ==============================
 * 第六感-面包卷 运行流程（复用 somecook_cooking 页面 UI）
 * 发酵(45min) → 烹饪(20min, 160℃) → 完成询问烤色 → 上色准备 → 上色(2/4/6min) → 再询问(循环)
 * 独立状态机/组/timer，不干扰多段烹饪的 cooking_timer_cb 逻辑
 * ============================== */

lv_group_t *g_six_cooking = NULL;
uint8_t g_six_running = 0;
int g_six_color_min = 4;   /* 烤色分钟:浅2 中4 深6 */

enum {
    SIX_PHASE_RISING,      /* 发酵 45min */
    SIX_PHASE_COOKING,     /* 烹饪 20min */
    SIX_PHASE_ASK,         /* 完成:询问是否需要烤色 */
    SIX_PHASE_COLOR_SETUP, /* 上色准备:等待"开 始" */
    SIX_PHASE_COLOR_COOKING, /* 上色烹饪 2/4/6min */
    SIX_PHASE_ASK_COLOR,   /* 上色完成:询问是否还需要烤色 */
};
static uint8_t g_six_phase = SIX_PHASE_COOKING;
static uint8_t g_six_paused = 0;
static uint8_t g_six_overlay = 0;   /* 遮罩确认态 */
static uint8_t g_six_from = SIX_PHASE_COOKING;  /* 遮罩进入源 */
static uint8_t g_six_has_rising = 0;   /* 是否有发酵段 */

#define SIX_RISING_SEC     (1 * 60)
#define SIX_COOKING_SEC    (1 * 60)
#define SIX_TOTAL_SEC      (SIX_RISING_SEC + SIX_COOKING_SEC)

static void six_cook_apply_display(void);
static void six_cook_exit(void);
static void six_cook_set_phase(int phase);

// 当前阶段秒数
static int32_t six_phase_sec(int phase)
{
    switch (phase) {
    case SIX_PHASE_RISING:        return SIX_RISING_SEC;
    case SIX_PHASE_COOKING:       return SIX_COOKING_SEC;
    case SIX_PHASE_COLOR_COOKING: return g_six_color_min * 60;
    default:                      return 0;
    }
}

// 已走秒数（暂停时用保存值）
static int32_t six_elapsed_sec(void)
{
    if (g_six_paused)
        return (int32_t)(cook_elapsed_saved / 1000);
    return (int32_t)((lv_tick_get() - cook_start_time) / 1000);
}

// 倒计时显示秒（发酵+烹饪走总时长；上色独立）
static int32_t six_remaining_sec(void)
{
    int32_t e = six_elapsed_sec();
    int32_t ph = six_phase_sec(g_six_phase);
    if (e > ph) e = ph;
    switch (g_six_phase) {
    case SIX_PHASE_RISING:        return SIX_TOTAL_SEC - e;
    case SIX_PHASE_COOKING:
        return g_six_has_rising ? SIX_TOTAL_SEC - (SIX_RISING_SEC + e)
                                : SIX_COOKING_SEC - e;
    case SIX_PHASE_COLOR_COOKING: return ph - e;
    default:                      return 0;
    }
}

// 显示刷新：按当前 phase/遮罩/暂停刷新页面元素
static void six_cook_apply_display(void)
{
    somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
    if (!sc) return;

    /* 基础显隐 */
    lv_obj_clear_flag(sc->icon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->cookstatus, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->image_10, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->bar_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->stop, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sc->activestatus, LV_OBJ_FLAG_HIDDEN);   /* 恒隐藏 */
    lv_obj_add_flag(sc->container_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sc->text1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sc->text2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->timelabel, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *bl = lv_obj_get_child(sc->stop, 0);

    switch (g_six_phase) {
    case SIX_PHASE_RISING:
        lv_label_set_text(sc->cookstatus, g_six_paused ? "暂停中..." : "发酵中...");
        lv_label_set_text(sc->label_12, "| 面包卷 | 45℃ | 1分钟");
        if (bl) lv_label_set_text(bl, g_six_paused ? "开 始" : "暂 停");
        break;
    case SIX_PHASE_COOKING:
        lv_label_set_text(sc->cookstatus, g_six_paused ? "暂停中..." : "烹饪中...");
        lv_label_set_text(sc->label_12, "| 面包卷 | 1分钟");
        if (bl) lv_label_set_text(bl, g_six_paused ? "开 始" : "暂 停");
        break;
    case SIX_PHASE_ASK:
    case SIX_PHASE_ASK_COLOR:
        lv_obj_add_flag(sc->timelabel, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(sc->cookstatus, "已完成");
        if (g_six_phase == SIX_PHASE_ASK) {
            lv_label_set_text(sc->text1, "请问需要增加");
            lv_label_set_text(sc->text2, "烤色吗!");
        } else {
            lv_label_set_text(sc->text1, "请问还需要增加");
            lv_label_set_text(sc->text2, "烤色吗!");
        }
        lv_obj_clear_flag(sc->text1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->text2, LV_OBJ_FLAG_HIDDEN);
        lv_bar_set_range(sc->bar_1, 0, 100);
        lv_bar_set_value(sc->bar_1, 100, LV_ANIM_OFF);
        if (bl) lv_label_set_text(bl, "需 要");
        break;
    case SIX_PHASE_COLOR_SETUP:
        lv_obj_clear_flag(sc->timelabel, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_fmt(sc->timelabel, "00:%02d:00", g_six_color_min);   /* 所选程度时长 */
        lv_label_set_text(sc->cookstatus, "额外上色");
        lv_label_set_text_fmt(sc->label_12, "| 面包卷 | %d分钟", g_six_color_min);
        if (bl) lv_label_set_text(bl, "开 始");
        lv_bar_set_range(sc->bar_1, 0, 100);
        lv_bar_set_value(sc->bar_1, 3, LV_ANIM_OFF);
        break;
    case SIX_PHASE_COLOR_COOKING:
        lv_label_set_text(sc->cookstatus, g_six_paused ? "暂停中..." : "烹饪中...");
        lv_label_set_text_fmt(sc->label_12, "| 面包卷 | 额外上色 | %d分钟", g_six_color_min);
        if (bl) lv_label_set_text(bl, g_six_paused ? "开 始" : "暂 停");
        break;
    default:
        break;
    }

    /* 倒计时与进度条 */
    if (g_six_phase == SIX_PHASE_RISING || g_six_phase == SIX_PHASE_COOKING ||
        g_six_phase == SIX_PHASE_COLOR_COOKING) {
        int32_t rem = six_remaining_sec();
        lv_label_set_text_fmt(sc->timelabel, "%02d:%02d:%02d",
                              (int)(rem / 3600), (int)((rem % 3600) / 60), (int)(rem % 60));
        /* 进度:发酵+烹饪按总时长,上色按阶段时长 */
        int32_t done, total;
        if (g_six_phase == SIX_PHASE_COLOR_COOKING) {
            done = six_elapsed_sec(); total = six_phase_sec(SIX_PHASE_COLOR_COOKING);
        } else if (g_six_has_rising) {
            done = six_elapsed_sec() + (g_six_phase == SIX_PHASE_COOKING ? SIX_RISING_SEC : 0);
            total = SIX_TOTAL_SEC;
        } else {
            done = six_elapsed_sec(); total = six_phase_sec(SIX_PHASE_COOKING);
        }
        if (done > total) done = total;
        int p = 3 + (int)((int64_t)done * 97 / (total ? total : 1));
        if (p > 100) p = 100;
        lv_bar_set_range(sc->bar_1, 0, 100);
        lv_bar_set_value(sc->bar_1, p, LV_ANIM_OFF);
        g_send.remaining_ms = (int32_t)(rem * 1000);
    } else {
        g_send.remaining_ms = -1;
    }

    /* 遮罩态:最后覆盖(进度条/状态在遮罩期间照常刷新) */
    if (g_six_overlay) {
        lv_obj_add_flag(sc->timelabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->container_1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->text1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->text2, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(sc->text1, "是否结束当前任务");
        lv_label_set_text(sc->text2, "回到主页");
        if (bl) lv_label_set_text(bl, "确 定");
    }
}

// 阶段切换
static void six_cook_set_phase(int phase)
{
    g_six_phase = phase;
    g_six_paused = 0;

    switch (phase) {
    case SIX_PHASE_RISING:
        g_send.cook_mode = MODE_RISING;
        g_send.set_temp = 45;
        g_send.set_temp_lower = 0;
        g_send.iface_status = IFACE_COOKING;
        break;
    case SIX_PHASE_COOKING:
        g_send.cook_mode = MODE_UPDOWN_BBQ;
        g_send.set_temp = 160;
        g_send.set_temp_lower = 160;   /* 上下烧烤:发上下温(一致) */
        g_send.iface_status = IFACE_COOKING;
        break;
    case SIX_PHASE_COLOR_SETUP:
        g_send.cook_mode = MODE_UPDOWN_BBQ;   /* 烤色阶段保持前面模式,不发 color */
        g_send.set_temp = 160;
        g_send.set_temp_lower = 0;
        /* iface_status 不设:保持完成状态(尚未开始烹饪) */
        break;
    case SIX_PHASE_COLOR_COOKING:
        g_send.cook_mode = MODE_UPDOWN_BBQ;
        g_send.set_temp = 160;
        g_send.set_temp_lower = 0;
        g_send.iface_status = IFACE_COOKING;   /* 点"开 始"后才发烹饪状态 */
        break;
    case SIX_PHASE_ASK:
    case SIX_PHASE_ASK_COLOR:
        g_send.iface_status = IFACE_COMPLETE;   /* 完成状态 */
        g_send.remaining_ms = 0;
        g_send.buzzer_req = BUZZER_COOK_DONE;
        break;
    default:
        g_send.cook_mode = MODE_UPDOWN_BBQ;
        break;
    }
    six_cook_apply_display();
    printf("[six_cook] phase -> %d\n", phase);
}

// 1s 定时器:计时 + 阶段切换
static void six_cook_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if (!g_six_running) return;
    if (g_six_paused) return;

    /* 门开自动暂停(运行阶段) */
    if (is_door_open()) {
        cook_elapsed_saved = lv_tick_get() - cook_start_time;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        g_six_paused = 1;
        six_cook_apply_display();
        printf("[six_cook] door open -> auto pause\n");
        return;
    }

    if (g_six_phase == SIX_PHASE_RISING || g_six_phase == SIX_PHASE_COOKING ||
        g_six_phase == SIX_PHASE_COLOR_COOKING) {
        int32_t ph = six_phase_sec(g_six_phase);
        int32_t e = six_elapsed_sec();
        six_cook_apply_display();
        if (e >= ph) {
            /* 阶段到点 */
            if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
            g_six_overlay = 0;   /* 遮罩中到点:退出遮罩,显示完成询问态 */
            if (g_six_phase == SIX_PHASE_RISING) {
                cook_start_time = lv_tick_get();
                cook_elapsed_saved = 0;
                six_cook_set_phase(SIX_PHASE_COOKING);
                cook_timer = lv_timer_create(six_cook_timer_cb, 1000, NULL);
            } else if (g_six_phase == SIX_PHASE_COOKING) {
                six_cook_set_phase(SIX_PHASE_ASK);
            } else {
                six_cook_set_phase(SIX_PHASE_ASK_COLOR);
            }
        }
    }
}

// 运行页 stop 按钮:暂停/开始/需要/确定
static void on_six_stop_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    if (g_six_overlay) {
        six_cook_exit();
        return;
    }
    switch (g_six_phase) {
    case SIX_PHASE_RISING:
    case SIX_PHASE_COOKING:
    case SIX_PHASE_COLOR_COOKING:
        if (!g_six_paused) {
            cook_elapsed_saved = lv_tick_get() - cook_start_time;
            if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
            g_six_paused = 1;
            six_cook_apply_display();
        } else {
            cook_start_time = lv_tick_get() - cook_elapsed_saved;
            cook_elapsed_saved = 0;
            g_six_paused = 0;
            if (cook_timer) lv_timer_del(cook_timer);
            cook_timer = lv_timer_create(six_cook_timer_cb, 1000, NULL);
            six_cook_apply_display();
        }
        break;
    case SIX_PHASE_ASK:
    case SIX_PHASE_ASK_COLOR:
        jump_to_toastcolor();
        break;
    case SIX_PHASE_COLOR_SETUP:
        g_six_paused = 0;
        cook_elapsed_saved = 0;
        cook_start_time = lv_tick_get();
        six_cook_set_phase(SIX_PHASE_COLOR_COOKING);
        if (cook_timer) lv_timer_del(cook_timer);
        cook_timer = lv_timer_create(six_cook_timer_cb, 1000, NULL);
        break;
    default:
        break;
    }
}

// 返回:运行/完成态 → 遮罩;遮罩 → 回源
void six_cook_handle_back(void)
{
    if (g_six_overlay) {
        g_six_overlay = 0;
        six_cook_apply_display();
        return;
    }
    g_six_overlay = 1;
    g_six_from = g_six_phase;
    six_cook_apply_display();
    printf("[six_cook] overlay\n");
}

// 退出:回主菜单
static void six_cook_exit(void)
{
    g_six_running = 0;
    g_six_phase = SIX_PHASE_COOKING;
    g_six_paused = 0;
    g_six_overlay = 0;
    g_six_has_rising = 0;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    g_on_stop_back = 0;
    g_stop_back_complete = NULL;
    cook_elapsed_saved = 0; cook_bar_saved = 0; cook_total_ms = 0;
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0; g_send.set_temp_lower = 0; g_send.remaining_ms = -1;
    /* 退出 → 回第六感主菜单(与 KEY_SIXMENU 入口栈形态一致) */
    depth = 0;
    page_push(PAGE_WAITMENU_24);
    jump_to_sixmenu();
    printf("[six_cook] exit -> sixmenu\n");
}

// 进入运行页(choice=1 有发酵)
void jump_to_six_cooking(void)
{
    page_push(PAGE_SIX_COOKING);
    lv_obj_clean(lv_scr_act());
    somecook_cooking_create(&ui_manager);

    somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
    if (sc) {
        lv_obj_t *btns[] = { sc->stop };
        for (int k = 0; k < 1; k++) lv_group_remove_obj(btns[k]);
        if (g_six_cooking) lv_group_del(g_six_cooking);
        g_six_cooking = group_create_for_page(btns, 1);
        clear_focus_states(btns, 1);
        lv_group_focus_obj(sc->stop);
        lv_obj_add_event_cb(sc->stop, on_six_stop_click, LV_EVENT_CLICKED, NULL);

        /* icon:六感图标,右移 23(115 -> 138) */
        lv_img_set_src(sc->icon, LVGL_IMAGE_PATH(sixicon.png));
        lv_obj_set_pos(sc->icon, 163, 161);
        lv_obj_add_flag(sc->activestatus, LV_OBJ_FLAG_HIDDEN);
    }
    current_group = g_six_cooking;

    g_six_running = 1;
    g_six_overlay = 0;
    g_six_paused = 0;
    g_six_has_rising = (g_rising_choice == 1);
    cook_elapsed_saved = 0;
    cook_start_time = lv_tick_get();
    six_cook_set_phase(g_rising_choice == 1 ? SIX_PHASE_RISING : SIX_PHASE_COOKING);
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(six_cook_timer_cb, 1000, NULL);
    g_send.iface_status = IFACE_COOKING;
    six_cook_apply_display();

    lv_scr_load_anim(somecook_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_cook] jump: cooking (choice=%d)\n", g_rising_choice);
}

// toastcolor next 确认:切到上色准备态(重建后显示)
void six_cook_goto_setup(void)
{
    /* 仅切状态与通讯;烤色页在栈顶时运行页对象已销毁(指针悬空),
       显示刷新由 next 的 page_pop → six_cooking_rebuild → apply_display 完成 */
    g_six_overlay = 0;
    g_six_paused = 0;
    g_six_phase = SIX_PHASE_COLOR_SETUP;
    g_send.cook_mode = MODE_UPDOWN_BBQ;   /* 保持前面模式,不发 color */
    g_send.set_temp = 160;
    g_send.set_temp_lower = 0;
    /* iface_status 不设:保持完成状态,点"开 始"后才发烹饪状态 */
}

// 外部路径(长按关机/探针)清理运行状态
void six_cook_reset(void)
{
    g_six_running = 0;
    g_six_phase = SIX_PHASE_COOKING;
    g_six_paused = 0;
    g_six_overlay = 0;
    g_six_has_rising = 0;
}

// 页面重建:完成询问态(从 toastcolor 返回)或上色准备态
void six_cooking_rebuild(page_id_t child)
{
    if (g_six_cooking) { lv_group_del(g_six_cooking); g_six_cooking = NULL; }
    lv_obj_clean(lv_scr_act());
    somecook_cooking_create(&ui_manager);

    somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
    if (sc) {
        lv_obj_t *btns[] = { sc->stop };
        for (int k = 0; k < 1; k++) lv_group_remove_obj(btns[k]);
        g_six_cooking = group_create_for_page(btns, 1);
        clear_focus_states(btns, 1);
        lv_group_focus_obj(sc->stop);
        lv_obj_add_event_cb(sc->stop, on_six_stop_click, LV_EVENT_CLICKED, NULL);

        lv_img_set_src(sc->icon, LVGL_IMAGE_PATH(sixicon.png));
        lv_obj_set_pos(sc->icon, 163, 161);
        lv_obj_add_flag(sc->activestatus, LV_OBJ_FLAG_HIDDEN);
    }
    current_group = g_six_cooking;

    g_six_overlay = 0;
    g_six_paused = 0;
    six_cook_apply_display();

    lv_scr_load_anim(somecook_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_cook] rebuild: child=%d\n", (int)child);
}
