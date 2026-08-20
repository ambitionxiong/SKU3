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
lv_group_t *g_probeneedtip = NULL;
int g_six_probe_temp = 80;   /* 烤全鸡所选探针目标温度（浅75/中80/深85℃），默认中 */

/* 烤鸡菜名（后续按菜谱扩展为配置表） */
const char *six_chick_name(void) { return "烤全鸡"; }
/* 当前六感菜名：烤鸡走独立名，其余走面包/蛋糕共用表 */
const char *six_current_name(void)
{
    return (g_six_bread_type == SIX_CHICK_WHOLE) ? six_chick_name() : six_bread_name();
}

static void on_chick6menu_chicken_click(lv_event_t *e);
static void on_chickenmenu_whole_click(lv_event_t *e);
static void on_probeneedtip_sure_click(lv_event_t *e);

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

        /* 菜谱事件：烤全鸡已接入（探针判断 → 提示/烤色选择）；其余菜谱后续步骤接入 */
        if (cm->wholechicken) {
            lv_obj_add_event_cb(cm->wholechicken, on_chickenmenu_whole_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(cm->wholechicken);
        }
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

        /* 菜谱事件：烤全鸡已接入（探针判断 → 提示/烤色选择）；其余菜谱后续步骤接入 */
        if (cm->wholechicken) {
            lv_obj_add_event_cb(cm->wholechicken, on_chickenmenu_whole_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(cm->wholechicken);
        }
    }
    current_group = g_chickenmenu;

    lv_scr_load_anim(chickenmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] rebuild: chickenmenu (child=%d)\n", (int)child);
}

/* 烤全鸡：探针判断 → 未插探针进提示页，已插入进烤色/程度选择页 */
static void on_chickenmenu_whole_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = SIX_CHICK_WHOLE;
    if (is_probe_inserted())
        jump_to_toastcolor();
    else
        jump_to_probeneedtip();
}

/* ================= probeneedtip（烤鸡探针提示页） =================
 * 未插探针时的提示页：仅 sure 进焦点组；点 sure 或 BACK 都回到 chickenmenu。 */

void jump_to_probeneedtip(void)
{
    page_push(PAGE_PROBENEEDTIP);
    lv_obj_clean(lv_scr_act());
    probeneedtip_create(&ui_manager);

    probeneedtip_t *pt = probeneedtip_get(&ui_manager);
    if (pt) {
        lv_obj_t *btns[] = { pt->sure };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_probeneedtip) lv_group_del(g_probeneedtip);
        g_probeneedtip = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (pt->sure) {
            lv_obj_add_event_cb(pt->sure, on_probeneedtip_sure_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(pt->sure);
        }
    }
    current_group = g_probeneedtip;

    lv_scr_load_anim(probeneedtip_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] jump: probeneedtip\n");
}

/* sure：弹出提示页，回到 chickenmenu（与 BACK 行为一致） */
static void on_probeneedtip_sure_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    page_pop();
    printf("[six_chicken] probeneedtip sure -> back to chickenmenu\n");
}

/* ================= 烤全鸡烹饪页（chickencooking UI） =================
 * 探针温度标记完成（无倒计时）：
 *   进度条 = 实时探针温度 → 设置探针温度的进度，bar 从 13 起、bartemp 跟随，
 *   与 preheatcooking 的 preheat_progress/preheat_update_bar 逻辑一致（13+87×推进）。
 * BACK → 遮罩确认（text1/2 + 按钮变"确 定"）；完成 → text1/2 高温防烫提示 + 隐藏按钮。 */

lv_group_t *g_chick_cooking = NULL;
static uint8_t s_chick_running = 0;   /* 运行中 */
static uint8_t s_chick_paused = 0;    /* 暂停中 */
static uint8_t s_chick_overlay = 0;   /* 遮罩确认态 */
static uint8_t s_chick_done = 0;      /* 完成态（高温防烫提示，按钮隐藏） */
static int s_chick_start_probe = 0;   /* 进入时起始探针温度（进度条 13 基准） */

static void six_chick_timer_cb(lv_timer_t *timer);
static void six_chick_apply_display(void);
static void six_chick_exit_to_menu(void);

static void on_chick_cooking_stop_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    if (s_chick_done && !s_chick_overlay) return; /* 完成页(无遮罩)按钮已隐藏，不会触发 */
    if (s_chick_overlay) {                    /* 遮罩/完成确认 → 退出回鸡菜菜单 */
        six_chick_exit_to_menu();
        return;
    }
    if (!s_chick_paused) {
        /* 暂停 */
        if (is_door_open()) { g_send.buzzer_req = BUZZER_KEY_INVALID; return; }
        cook_elapsed_saved = lv_tick_get() - cook_start_time;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        s_chick_paused = 1;
        g_send.iface_status = IFACE_PAUSE;
    } else {
        /* 恢复 */
        if (is_door_open()) { g_send.buzzer_req = BUZZER_KEY_INVALID; return; }
        cook_start_time = lv_tick_get() - cook_elapsed_saved;
        s_chick_paused = 0;
        s_chick_running = 1;
        g_send.iface_status = IFACE_COOKING;
        if (cook_timer) lv_timer_del(cook_timer);
        cook_timer = lv_timer_create(six_chick_timer_cb, 1000, NULL);
    }
    six_chick_apply_display();
}

void jump_to_chick_cooking(void)
{
    page_push(PAGE_CHICKENCOOKING);
    lv_obj_clean(lv_scr_act());
    chickencooking_create(&ui_manager);

    chickencooking_t *ck = chickencooking_get(&ui_manager);
    if (ck) {
        lv_obj_t *btns[] = { ck->stop };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_chick_cooking) lv_group_del(g_chick_cooking);
        g_chick_cooking = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (ck->stop) {
            lv_obj_add_event_cb(ck->stop, on_chick_cooking_stop_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(ck->stop);
        }
        /* icon：六感图标 */
        if (ck->icon) lv_img_set_src(ck->icon, LVGL_IMAGE_PATH(sixicon.png));
    }
    current_group = g_chick_cooking;

    s_chick_running = 1;
    s_chick_paused = 0;
    s_chick_overlay = 0;
    s_chick_done = 0;
    s_chick_start_probe = get_probe_temp();   /* 进度条起点：起始探针温度 */
    cook_elapsed_saved = 0;
    cook_start_time = lv_tick_get();
    cook_total_ms = 80 * 60 * 1000;   /* 探针异常时兜底，非倒计时 */

    g_send.iface_status = IFACE_COOKING;
    g_send.cook_mode = MODE_WINDCHANGE_BBQ;   /* 烤全鸡：热风对流 */
    g_send.set_temp = 230;
    g_send.set_temp_lower = 0;
    g_send.cook_flag = 2;                     /* 探针模式 */
    g_send.probe_temp = (uint8_t)g_six_probe_temp;
    g_send.remaining_ms = 0;   /* 探针驱动：无倒计时 */

    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(six_chick_timer_cb, 1000, NULL);
    six_chick_apply_display();

    lv_scr_load_anim(chickencooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] jump: chick cooking (probe target=%d)\n", g_six_probe_temp);
}

/* 进度条：与 preheatcooking 一致——起始为 13，按探针温度推进到 100 */
static int six_chick_bar_value(int cur)
{
    int range = g_six_probe_temp - s_chick_start_probe;
    if (range <= 0) return 100;
    int p = 13 + (int)((int64_t)87 * (cur - s_chick_start_probe) / range);
    if (p < 13) p = 13;
    if (p > 100) p = 100;
    return p;
}

/* 烤色程度文字：按所选档位（浅75/中80/深85℃）显示 */
static const char *six_chick_degree_text(void)
{
    return g_six_probe_temp <= 75 ? "浅色" :
           g_six_probe_temp >= 85 ? "深色" : "中等色";
}

/* 烹饪页显示刷新 */
static void six_chick_apply_display(void)
{
    chickencooking_t *ck = chickencooking_get(&ui_manager);
    if (!ck) return;

    /* 探针实时温度（无效/未插按 0，封顶到目标温度） */
    int cur = get_probe_temp();
    if (cur < 0) cur = 0;
    if (cur > g_six_probe_temp) cur = g_six_probe_temp;

    /* status:菜名+烤色程度（浅色/中等色/深色） */
    if (ck->status)
        lv_label_set_text_fmt(ck->status, "| %s | %s |", six_chick_name(), six_chick_degree_text());

    /* cookstatus:烹饪中/暂停中/已完成（遮罩确认态不改 cookstatus） */
    if (ck->cookstatus) {
        if (s_chick_done)           lv_label_set_text(ck->cookstatus, "已完成");
        else if (s_chick_paused)    lv_label_set_text(ck->cookstatus, "暂停中...");
        else                        lv_label_set_text(ck->cookstatus, "烹饪中...");
    }

    /* 按钮:遮罩→"确 定";暂停→"开 始";运行→"暂 停";完成页(无遮罩)→隐藏 */
    if (ck->stop) {
        lv_obj_t *bl = lv_obj_get_child(ck->stop, 0);
        if (s_chick_done && !s_chick_overlay) {
            lv_obj_add_flag(ck->stop, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(ck->stop, LV_OBJ_FLAG_HIDDEN);
            if (bl) lv_label_set_text(bl, s_chick_overlay ? "确 定" :
                                             s_chick_paused  ? "开 始" : "暂 停");
        }
    }

    /* 遮罩容器 + 右侧 text1/text2：
     遮罩容器仅返回确定确认态显示（完成页不遮罩）；
     text1/text2 在确认界面（遮罩/完成页）显示，运行与暂停时隐藏 */
    int show_tip = (s_chick_overlay || s_chick_done);
    if (ck->container_1) {
        if (s_chick_overlay) lv_obj_clear_flag(ck->container_1, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(ck->container_1, LV_OBJ_FLAG_HIDDEN);
    }
    if (ck->text1) {
        if (show_tip) {
            lv_obj_clear_flag(ck->text1, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ck->text1, (s_chick_done && !s_chick_overlay) ?
                              "高温防烫" : "是否结束当前任务");
        } else {
            lv_obj_add_flag(ck->text1, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (ck->text2) {
        if (show_tip) {
            lv_obj_clear_flag(ck->text2, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ck->text2, (s_chick_done && !s_chick_overlay) ?
                              "请缓慢打开门体！" : "回到主页");
        } else {
            lv_obj_add_flag(ck->text2, LV_OBJ_FLAG_HIDDEN);
        }
    }

    /* 进度条 + bartemp：探针温度 起始→目标，bartemp 跟随进度条移动（preheat 同款算法） */
    if (ck->bar_2) {
        int p = six_chick_bar_value(cur);
        lv_bar_set_range(ck->bar_2, 0, 100);
        lv_bar_set_value(ck->bar_2, p, LV_ANIM_OFF);
        if (ck->bartemp) {
            lv_label_set_text_fmt(ck->bartemp, "%d℃", cur);
            int bx = 122 + (637 * p) / 100 - 80;
            lv_obj_set_pos(ck->bartemp, bx, 323);
        }
    }
}

/* 1s 定时器：探针温度标记完成（无倒计时；最长时长仅兜底） */
static void six_chick_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if (!s_chick_running || s_chick_paused) return;

    /* 门开自动暂停 */
    if (is_door_open()) {
        cook_elapsed_saved = lv_tick_get() - cook_start_time;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        s_chick_paused = 1;
        g_send.iface_status = IFACE_PAUSE;
        six_chick_apply_display();
        return;
    }

    uint32_t elapsed = lv_tick_get() - cook_start_time;
    /* 完成：探针达到目标温度；或超最长安全时间（探针异常兜底） */
    if (get_probe_temp() >= g_six_probe_temp || elapsed >= (uint32_t)cook_total_ms) {
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        s_chick_running = 0;
        s_chick_done = 1;
        s_chick_overlay = 0;   /* 完成页:高温防烫提示、按钮隐藏、无遮罩；BACK 再进返回确定页 */
        g_send.iface_status = IFACE_COMPLETE;
        g_send.cook_flag = 0;               /* 退出探针模式标记 */
        g_send.remaining_ms = 0;
        g_send.buzzer_req = BUZZER_COOK_DONE;
        six_chick_apply_display();
        return;
    }
    g_send.remaining_ms = 0;   /* 探针驱动：无剩余时间 */
    six_chick_apply_display();
}

/* 烤全鸡烹饪页 BACK */
void six_chick_handle_back(void)
{
    if (s_chick_done) {
        /* 完成页:BACK → 返回确定确认页（再按 BACK 回完成页；确定→退出） */
        s_chick_overlay = !s_chick_overlay;
        six_chick_apply_display();
        printf("[six_chicken] done back -> overlay=%d\n", s_chick_overlay);
        return;
    }
    if (s_chick_overlay) {        /* 已在遮罩：取消遮罩继续烹饪 */
        s_chick_overlay = 0;
        six_chick_apply_display();
        return;
    }
    s_chick_overlay = 1;          /* 显示遮罩确认（text1/2 + 按钮变"确 定"） */
    six_chick_apply_display();
    printf("[six_chicken] overlay\n");
}

/* 退出：回到鸡菜菜单（弹掉 cooking + toastcolor，不残留栈） */
static void six_chick_exit_to_menu(void)
{
    s_chick_running = 0;
    s_chick_paused = 0;
    s_chick_overlay = 0;
    s_chick_done = 0;
    s_chick_start_probe = 0;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    cook_elapsed_saved = 0; cook_bar_saved = 0; cook_total_ms = 0;
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.cook_flag = 0;
    g_send.set_temp = 0; g_send.set_temp_lower = 0; g_send.remaining_ms = -1;
    g_send.probe_temp = 0;
    depth = 0;
    page_push(PAGE_WAITMENU_24);
    jump_to_chickenmenu();
    printf("[six_chicken] exit -> chickenmenu\n");
}