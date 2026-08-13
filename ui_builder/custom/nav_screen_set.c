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

void jump_to_screen_set(void)
{
    if (screen_is_loading(lv_scr_act())) return;
    /* 防重入：已在设置页 */
    if (depth > 0 && page_stack[depth - 1] == PAGE_SCREEN_SET) return;
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

    /* 焦点组：16 个设置按钮（生成顺序） */
    lv_obj_t *btns[] = {
        ss->DSQ_Btn, ss->TS_Btn, ss->ZDBW_Btn, ss->TFLQYX_Btn,
        ss->Six_Btn, ss->LDG_Btn, ss->WDDW_Btn, ss->SYSZ_Btn,
        ss->LDXS_Btn, ss->Time_Btn, ss->DJ_Time_Btn, ss->YY_Btn,
        ss->Power_Btn, ss->Demo_Btn, ss->RESET_Btn, ss->GYJQ_Btn,
    };
    const int n = (int)(sizeof(btns) / sizeof(btns[0]));

    /* ① 先移除默认组（此时按钮还未加入 g_screen_set,
       lv_group_remove_obj 单参数=从所有组移除,若后执行会把按钮从本组也删掉） */
    for (int k = 0; k < n; k++)
        lv_group_remove_obj(btns[k]);

    /* ② 再建组加入 */
    if (g_screen_set) lv_group_del(g_screen_set);
    g_screen_set = group_create_for_page(btns, n);
    clear_focus_states(btns, n);

    /* ③ 焦点 */
    s_prev_group = current_group;
    current_group = g_screen_set;
    lv_group_focus_obj(btns[0]);

    page_push(PAGE_SCREEN_SET);
    printf("[screen_set] enter (overlay)\n");
}

void screen_set_back(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SCREEN_SET) return;

    /* 删除覆盖层（下层页面原样保留） */
    if (screen_SET.obj) {
        lv_obj_del(screen_SET.obj);
        screen_SET.obj = NULL;
    }
    if (g_screen_set) {
        lv_group_del(g_screen_set);
        g_screen_set = NULL;
    }
    current_group = s_prev_group;
    s_prev_group = NULL;

    depth--;   /* 仅弹栈，不重建（下层页面未被销毁） */

    /* 覆盖期间下层运行页显示未刷新,立即补刷一次,避免返回时短暂显示旧倒计时 */
    cooking_timer_cb(NULL);
    printf("[screen_set] back (restore prev)\n");
}

// 外部路径（长按关机/探针插入等重置页面）调用：清理覆盖层对象/组/焦点指针,防悬空 UAF
void screen_set_reset(void)
{
    if (screen_SET.obj) {
        lv_obj_del(screen_SET.obj);
        screen_SET.obj = NULL;
    }
    if (g_screen_set) {
        lv_group_del(g_screen_set);
        g_screen_set = NULL;
    }
    s_prev_group = NULL;
}
