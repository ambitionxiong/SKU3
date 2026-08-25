/*
 * nav_six_tz.c - 第六感-探针版菜单（sixmenutz / chickmenutz）
 *
 * 探针模式下按第六感功能键进入 sixmenutz（探针版第六感菜单）；
 * sixmenutz 的 chick → chickmenutz（探针版家禽菜单）；
 * chickmenutz 的 wholechicken/wholeduck → 走烤全鸡/烤全鸭完整探针流程
 *   （toastcolor → descriptionmenu → chickencooking）。
 * 而非探针模式的 chickenmenu/duckmenu 里，烤全鸡/烤全鸭仅跳探针提示页。
 */
#include "protocol.h"
#include "nav.h"

lv_group_t *g_sixmenutz = NULL;
lv_group_t *g_chickmenutz = NULL;

static void on_sixmenutz_chick_click(lv_event_t *e);
static void on_sixmenutz_meat_click(lv_event_t *e);
static void on_chickmenutz_wholechicken_click(lv_event_t *e);
static void on_chickmenutz_wholeduck_click(lv_event_t *e);

static uint8_t s_meat_tz_mode = 0;   /* 0=家禽 1=牛肉 2=羊肉 3=猪肉 二级菜单(复用 chickmenutz) */

/* ================= sixmenutz（探针版第六感菜单） ================= */

void jump_to_sixmenutz(void)
{
    page_push(PAGE_SIXMENUTZ);
    lv_obj_clean(lv_scr_act());
    sixmenutz_create(&ui_manager);

    sixmenutz_t *mz = sixmenutz_get(&ui_manager);
    if (mz) {
        lv_obj_t *btns[] = { mz->chick, mz->meat };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_sixmenutz) lv_group_del(g_sixmenutz);
        g_sixmenutz = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (mz->chick) {
            lv_obj_add_event_cb(mz->chick, on_sixmenutz_chick_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(mz->chick);
        }
        if (mz->meat) {
            lv_obj_add_event_cb(mz->meat, on_sixmenutz_meat_click, LV_EVENT_CLICKED, NULL);
        }
    }
    current_group = g_sixmenutz;

    lang_scr_load_anim(sixmenutz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_tz] jump: sixmenutz\n");
}

void sixmenutz_rebuild(page_id_t child)
{
    if (g_sixmenutz) { lv_group_del(g_sixmenutz); g_sixmenutz = NULL; }
    lv_obj_clean(lv_scr_act());
    sixmenutz_create(&ui_manager);

    sixmenutz_t *mz = sixmenutz_get(&ui_manager);
    if (mz) {
        lv_obj_t *btns[] = { mz->chick, mz->meat };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_sixmenutz = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (mz->chick) {
            lv_obj_add_event_cb(mz->chick, on_sixmenutz_chick_click, LV_EVENT_CLICKED, NULL);
        }
        if (mz->meat) {
            lv_obj_add_event_cb(mz->meat, on_sixmenutz_meat_click, LV_EVENT_CLICKED, NULL);
        }
        /* 焦点恢复:从肉菜单/牛肉菜单返回 → meat; 家禽返回 → chick */
        if (s_meat_tz_mode && child == PAGE_CHICKMENUTZ && mz->meat)
            lv_group_focus_obj(mz->meat);
        else if (child == PAGE_SIXOP3PAGE && mz->meat)
            lv_group_focus_obj(mz->meat);
        else if (mz->chick)
            lv_group_focus_obj(mz->chick);
    }
    current_group = g_sixmenutz;

    lang_scr_load_anim(sixmenutz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_tz] rebuild: sixmenutz\n");
}

static void on_sixmenutz_chick_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_chickmenutz();
}

/* 肉:探针版六选项3页(牛肉/羊肉/猪肉,op/probe 隐藏) */
static void on_sixmenutz_meat_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    s_meat_tz_mode = 0;   /* 清牛肉二级模式(本次进入肉菜单) */
    jump_to_sixop3page_tz(tr("肉"), tr("牛肉"), tr("羊肉"), tr("猪肉"));
}

/* ================= chickmenutz（探针版家禽菜单 / 牛肉二级菜单复用） ================= */

/* 肉二级菜单:复用 chickmenutz(1=牛肉 2=羊肉 3=猪肉) */
static void jump_to_tzmenutz(int kind)
{
    static const char *tz_label4[4] = { NULL, "牛肉", "羊肉", "猪肉" };
    static const char *tz_label5[4] = { NULL, "烤牛排", "烤羊腿", "烤猪里脊肉" };
    static const char *tz_label6[4] = { NULL, "烤牛肉", "烤羊排", "烤五花肉" };
    s_meat_tz_mode = (uint8_t)kind;
    page_push(PAGE_CHICKMENUTZ);
    lv_obj_clean(lv_scr_act());
    chickmenutz_create(&ui_manager);

    chickmenutz_t *cm = chickmenutz_get(&ui_manager);
    if (cm) {
        if (cm->label_4) lv_label_set_text(cm->label_4, tr(tz_label4[kind]));
        if (cm->label_5) lv_label_set_text(cm->label_5, tr(tz_label5[kind]));
        if (cm->label_6) lv_label_set_text(cm->label_6, tr(tz_label6[kind]));

        lv_obj_t *btns[] = { cm->wholechicken, cm->wholeduck };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_chickmenutz) { lv_group_del(g_chickmenutz); g_chickmenutz = NULL; }
        g_chickmenutz = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (cm->wholechicken) {
            lv_obj_add_event_cb(cm->wholechicken, on_chickmenutz_wholechicken_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(cm->wholechicken);
        }
        if (cm->wholeduck) {
            lv_obj_add_event_cb(cm->wholeduck, on_chickmenutz_wholeduck_click, LV_EVENT_CLICKED, NULL);
        }
    }
    current_group = g_chickmenutz;

    lang_scr_load_anim(chickmenutz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_tz] jump: meatmenutz kind=%d\n", kind);
}

void jump_to_beefmenutz(void)  { jump_to_tzmenutz(1); }   /* 牛肉(兼容 sixop3page) */
void jump_to_muttonmenutz(void) { jump_to_tzmenutz(2); }  /* 羊肉 */
void jump_to_porkmenutz(void)   { jump_to_tzmenutz(3); }  /* 猪肉 */
void jump_to_meatmenutz(int kind)
{
    if (kind >= 1 && kind <= 3) jump_to_tzmenutz(kind);
}

void jump_to_chickmenutz(void)
{
    s_meat_tz_mode = 0;
    page_push(PAGE_CHICKMENUTZ);
    lv_obj_clean(lv_scr_act());
    chickmenutz_create(&ui_manager);

    chickmenutz_t *cm = chickmenutz_get(&ui_manager);
    if (cm) {
        lv_obj_t *btns[] = { cm->wholechicken, cm->wholeduck };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_chickmenutz) lv_group_del(g_chickmenutz);
        g_chickmenutz = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (cm->wholechicken) {
            lv_obj_add_event_cb(cm->wholechicken, on_chickmenutz_wholechicken_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(cm->wholechicken);
        }
        if (cm->wholeduck) {
            lv_obj_add_event_cb(cm->wholeduck, on_chickmenutz_wholeduck_click, LV_EVENT_CLICKED, NULL);
        }
    }
    current_group = g_chickmenutz;

    lang_scr_load_anim(chickmenutz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_tz] jump: chickmenutz\n");
}

void chickmenutz_rebuild(page_id_t child)
{
    if (g_chickmenutz) { lv_group_del(g_chickmenutz); g_chickmenutz = NULL; }
    lv_obj_clean(lv_scr_act());
    chickmenutz_create(&ui_manager);

    chickmenutz_t *cm = chickmenutz_get(&ui_manager);
    if (cm) {
        if (s_meat_tz_mode) {
            /* 肉二级菜单:恢复标签(1=牛肉 2=羊肉 3=猪肉) */
            static const char *tz_label4[4] = { NULL, "牛肉", "羊肉", "猪肉" };
            static const char *tz_label5[4] = { NULL, "烤牛排", "烤羊腿", "烤猪里脊肉" };
            static const char *tz_label6[4] = { NULL, "烤牛肉", "烤羊排", "烤五花肉" };
            if (cm->label_4) lv_label_set_text(cm->label_4, tr(tz_label4[s_meat_tz_mode]));
            if (cm->label_5) lv_label_set_text(cm->label_5, tr(tz_label5[s_meat_tz_mode]));
            if (cm->label_6) lv_label_set_text(cm->label_6, tr(tz_label6[s_meat_tz_mode]));
        }

        lv_obj_t *btns[] = { cm->wholechicken, cm->wholeduck };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_chickmenutz = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (cm->wholechicken)
            lv_obj_add_event_cb(cm->wholechicken, on_chickmenutz_wholechicken_click, LV_EVENT_CLICKED, NULL);
        if (cm->wholeduck)
            lv_obj_add_event_cb(cm->wholeduck, on_chickmenutz_wholeduck_click, LV_EVENT_CLICKED, NULL);

        /* 返回时恢复进入时的菜焦点（探针流程会设置 g_six_bread_type） */
        if (s_meat_tz_mode && cm->wholeduck &&
            (g_six_bread_type == SIX_MEAT_GRILL_BEEF || g_six_bread_type == SIX_MEAT_GRILL_LAMBS
             || g_six_bread_type == SIX_MEAT_GRILL_BELLY))
            lv_group_focus_obj(cm->wholeduck);                 /* 肉菜单:从右侧菜返回 */
        else if (s_meat_tz_mode && cm->wholechicken)
            lv_group_focus_obj(cm->wholechicken);              /* 肉菜单:从左侧菜返回 */
        else if (g_six_bread_type == SIX_CHICK_DUCK_WHOLE && cm->wholeduck)
            lv_group_focus_obj(cm->wholeduck);
        else if (cm->wholechicken)
            lv_group_focus_obj(cm->wholechicken);
    }
    current_group = g_chickmenutz;

    lang_scr_load_anim(chickmenutz_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_tz] rebuild: chickmenutz\n");
}

/* 烤全鸡：探针模式必有探针 → 直接进 toastcolor(degree 组) 走完整流程 */
static void on_chickmenutz_wholechicken_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    if (s_meat_tz_mode) {
        /* 肉二级菜单左侧菜: 牛肉→烤牛排(成熟度) 羊肉→烤羊腿(二维) 猪肉→烤猪里脊肉(程度) */
        switch (s_meat_tz_mode) {
        case 1:
            g_six_bread_type = SIX_MEAT_GRILL_STEAK;
            g_toast_mode = TOAST_MODE_MATURITY;
            jump_to_toastcolor();
            return;
        case 2:
            g_six_bread_type = SIX_MEAT_GRILL_LEG;
            jump_to_sixset2();
            return;
        case 3:
            g_six_bread_type = SIX_MEAT_GRILL_TENDERLOIN;
            g_toast_mode = TOAST_MODE_DEGREE;
            jump_to_toastcolor();
            return;
        }
    }
    g_six_bread_type = SIX_CHICK_WHOLE;
    g_toast_mode = TOAST_MODE_DEGREE;
    jump_to_toastcolor();
}

/* 烤全鸭：同上 */
static void on_chickmenutz_wholeduck_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    if (s_meat_tz_mode) {
        /* 肉二级菜单右侧菜: 牛肉→烤牛肉(二维) 羊肉→烤羊排(二维) 猪肉→烤五花肉(程度) */
        switch (s_meat_tz_mode) {
        case 1:
            g_six_bread_type = SIX_MEAT_GRILL_BEEF;
            jump_to_sixset2();
            return;
        case 2:
            g_six_bread_type = SIX_MEAT_GRILL_LAMBS;
            jump_to_sixset2();
            return;
        case 3:
            g_six_bread_type = SIX_MEAT_GRILL_BELLY;
            g_toast_mode = TOAST_MODE_DEGREE;
            jump_to_toastcolor();
            return;
        }
    }
    g_six_bread_type = SIX_CHICK_DUCK_WHOLE;
    g_toast_mode = TOAST_MODE_DEGREE;
    jump_to_toastcolor();
}
