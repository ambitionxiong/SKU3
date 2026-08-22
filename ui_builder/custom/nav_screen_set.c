/*
 * nav_screen_set.c - 设置页(覆盖层)
 * KEY11 进入的覆盖层设置页(不挂页面栈)。
 * 运行态进入:仅前3个按钮可聚焦+work背景图;非运行态:16按钮。
 * 返回:运行态回原页面,非运行态回待机页。功能键入口防御/跳转在此判定。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"
#include "screen_SET.h"

/* ==============================
 * 设置页（覆盖层）
 * 任何状态（含烹饪中）可按键进入；下层页面/计时器原样保留；
 * BACK 仅弹栈 + 删除覆盖层，恢复原页面焦点组。
 * ============================== */

lv_group_t *g_screen_set = NULL;      /* 设置页焦点组 */
static lv_group_t *s_prev_group = NULL;  /* 进入前焦点组 */
static uint8_t s_was_running = 0;        /* 进入设置页前是否运行态(烹饪/暂停/完成/预约) */

void jump_to_screen_set(void)
{
    if (screen_is_loading(lv_scr_act())) return;
    /* 防重入：已在设置页 */
    if (depth > 0 && page_stack[depth - 1] == PAGE_SCREEN_SET) return;
    /* 记录进入前是否运行态:返回时区分回原页面(运行中)还是回待机页(非运行) */
    s_was_running = (g_send.iface_status == IFACE_COOKING ||
                     g_send.iface_status == IFACE_PAUSE ||
                     g_send.iface_status == IFACE_COMPLETE ||
                     g_send.iface_status == IFACE_DELAY_RESERVE);
    /* 清残留编辑字段注册,防 find_edit_field 指针复用误判 */
    edit_clear();

    /* 覆盖层对象清理（上次返回未删净的防御） */
    if (screen_SET.obj) {
        lv_obj_del(screen_SET.obj);
        screen_SET.obj = NULL;
    }
    screen_SET_create(&ui_manager);

    screen_SET_t *ss = screen_SET_get(&ui_manager);
    if (!ss || !ss->obj) return;

    /* 全屏覆盖层（create 已直接挂当前屏幕,下层页面对象原样保留） */
    lv_obj_set_pos(ss->obj, 0, 0);
    lv_obj_set_size(ss->obj, 1280, 480);
    lv_obj_set_scrollbar_mode(ss->obj, LV_SCROLLBAR_MODE_OFF);

    /* 右上角时间由常驻 topflag 提供，隐藏页面自带时间标签 */
    if (ss->Time_Lb) lv_obj_add_flag(ss->Time_Lb, LV_OBJ_FLAG_HIDDEN);

    /* 焦点组:运行态只留前 3 个按钮(DSQ/TS/ZDBW),非运行态 16 个;
       未入组按钮仍显示但不可聚焦 */
    lv_obj_t *all_btns[] = {
        ss->DSQ_Btn, ss->TS_Btn, ss->ZDBW_Btn, ss->TFLQYX_Btn,
        ss->Six_Btn, ss->LDG_Btn, ss->WDDW_Btn, ss->SYSZ_Btn,
        ss->LDXS_Btn, ss->Time_Btn, ss->DJ_Time_Btn, ss->YY_Btn,
        ss->Power_Btn, ss->Demo_Btn, ss->RESET_Btn, ss->GYJQ_Btn,
    };
    const int n_all = (int)(sizeof(all_btns) / sizeof(all_btns[0]));

    /* ① 先移除默认组(此时按钮还未加入 g_screen_set,
       lv_group_remove_obj 单参数=从所有组移除,若后执行会把按钮从本组也删掉) */
    for (int k = 0; k < n_all; k++)
        lv_group_remove_obj(all_btns[k]);

    /* ② 建组:运行态前 3 个,非运行态全部 */
    lv_obj_t **btns = all_btns;
    int n = n_all;
    if (s_was_running) {
        n = 3;
        /* 运行态:文字框换 set_work_bg_txt */
        lv_img_set_src(ss->TXT_Img, LVGL_IMAGE_PATH(set_work_bg_txt.png));
    }
    if (g_screen_set) lv_group_del(g_screen_set);
    g_screen_set = group_create_for_page(btns, n);
    clear_focus_states(btns, n);

    /* ③ 焦点 */
    s_prev_group = current_group;
    current_group = g_screen_set;
    lv_group_focus_obj(btns[0]);

    page_push(PAGE_SCREEN_SET);
    printf("[screen_set] enter (overlay, running=%d)\n", s_was_running);
}

void screen_set_back(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SCREEN_SET) return;

    /* 删除覆盖层 */
    if (screen_SET.obj) {
        lv_obj_del(screen_SET.obj);
        screen_SET.obj = NULL;
    }
    if (g_screen_set) {
        lv_group_del(g_screen_set);
        g_screen_set = NULL;
    }

    if (s_was_running) {
        /* 运行态(烹饪/暂停/完成/预约):回原页面,下层原样保留 */
        current_group = s_prev_group;
        s_prev_group = NULL;
        depth--;   /* 仅弹栈，不重建（下层页面未被销毁） */
        /* 覆盖期间下层运行页显示未刷新,立即补刷一次,避免返回时短暂显示旧倒计时 */
        cooking_timer_cb(NULL);
        printf("[screen_set] back (restore prev)\n");
    } else {
        /* 非运行态:回待机页 */
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        lv_obj_clean(lv_scr_act());
        waitmenu_24_create(&ui_manager);
        waitmenu_clock_cache_reset();   /* 强制刷新为真实时间 */
        current_group = NULL;
        lang_scr_load_anim(waitmenu_24_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        waitmenu_apply_clock();   /* 立即刷新为真实时间 */
        g_send.iface_status = IFACE_STANDBY;
        printf("[screen_set] back -> waitmenu_24\n");
    }
}

// 外部路径（长按关机/探针插入/功能键弹栈等重置页面）调用：清理覆盖层对象/焦点指针,防悬空 UAF
// 注意:保留 g_screen_set 组不删——current_group 可能仍指向它,删组会悬空;
//       对象删除后组自动清空(空组安全),下次 jump_to_screen_set 会 lv_group_del 清理
void screen_set_reset(void)
{
    if (screen_SET.obj) {
        lv_obj_del(screen_SET.obj);
        screen_SET.obj = NULL;
    }
    s_prev_group = NULL;
}
