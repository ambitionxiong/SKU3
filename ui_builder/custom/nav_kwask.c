/*
 * nav_kwask.c - 完成页保温中「要继续保温吗？」询问框(2026-11-20 用户设计)
 *
 * 触发(用户定稿):完成页保温中(g_keepwarm_active)开门、再关门,关门那一下
 *   保温还没结束 → 弹本框;其他时候一律不弹(15 分钟到点结束不弹、手动保温不弹)。
 *   开门这一下只记标志(nav_system.c system_timer_cb 的门边沿),关门边沿才弹。
 *
 * 两个键(编码器旋到哪个按哪个):
 *   取消 / BACK :结束保温(BUF[17]=0)→ 回主目录(nav_goto_major_menu)
 *   继续        :收框,保温原样继续(计时不重置、BUF[17] 保持 1)
 * 本框零操作 1 分钟、或保温本身满 15 分钟:结束保温,收框,停在完成页(已完成)。
 *   —— 1 分钟那条在 nav_idle.c 里抢在"完成页每秒刷新活动时钟"之前判;
 *      15 分钟那条由 nav_cooktimer.c 的正常结束支顺手收框。
 *
 * 外观照现有 topflag 弹层:全屏遮罩 0x060505/113 + 右中提示列 + 两个 129x83
 * 按钮(取消默认选中,底图 stopbk1/stopbk2 与各页"确定"同款)。
 */
#include "nav.h"
#include "nav_internal.h"
#include "nav_idle.h"
#include "protocol.h"

#define KWASK_W        129
#define KWASK_H        83
#define KWASK_GAP      32
#define KWASK_CX       1022   /* 与 topflag 提示列同一中线(tip1 797+450/2) */
#define KWASK_Y        295

static lv_obj_t *s_mask = NULL;
static lv_obj_t *s_title = NULL;
static lv_obj_t *s_btn[2] = { NULL, NULL };   /* 0=取消 1=继续 */
static lv_obj_t *s_lab[2] = { NULL, NULL };
static int s_sel = 0;
static uint8_t s_active = 0;
static uint32_t s_touch_ms = 0;

static void kwask_focus(void)
{
    for (int i = 0; i < 2; i++) {
        if (!s_btn[i]) continue;
        if (i == s_sel) lv_obj_add_state(s_btn[i], LV_STATE_FOCUSED);
        else            lv_obj_clear_state(s_btn[i], LV_STATE_FOCUSED);
    }
}

static void kwask_build(void)
{
    if (s_mask) return;
    s_mask = lv_obj_create(lv_layer_top());
    if (!s_mask) return;
    lv_obj_set_pos(s_mask, 0, 0);
    lv_obj_set_size(s_mask, 1280, 480);
    lv_obj_set_scrollbar_mode(s_mask, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(s_mask, lv_color_hex(0x060505), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(s_mask, 113, LV_PART_MAIN | LV_STATE_DEFAULT);   /* 同 topflag container_1 遮罩 */
    lv_obj_set_style_border_width(s_mask, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(s_mask, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(s_mask, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(s_mask, LV_OBJ_FLAG_HIDDEN);

    s_title = lv_label_create(s_mask);
    lv_label_set_text(s_title, "要继续保温吗？");
    lv_label_set_long_mode(s_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(s_title, 797, 161);          /* 与 topflag tip1 同轴(中心 x=1022) */
    lv_obj_set_size(s_title, 450, 36);
    lv_obj_set_style_text_font(s_title, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(s_title, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(s_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    for (int i = 0; i < 2; i++) {
        s_btn[i] = lv_btn_create(s_mask);
        lv_obj_set_pos(s_btn[i], KWASK_CX - KWASK_W - KWASK_GAP / 2 + i * (KWASK_W + KWASK_GAP), KWASK_Y);
        lv_obj_set_size(s_btn[i], KWASK_W, KWASK_H);
        lv_obj_set_style_bg_opa(s_btn[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_img_src(s_btn[i], LVGL_IMAGE_PATH(stopbk1.png),
                                    LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(s_btn[i], 0, LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_bg_img_src(s_btn[i], LVGL_IMAGE_PATH(stopbk2.png),
                                    LV_PART_MAIN | LV_STATE_FOCUSED);
        lv_obj_set_style_border_width(s_btn[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(s_btn[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(s_btn[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        s_lab[i] = lv_label_create(s_btn[i]);
        lv_label_set_text(s_lab[i], i == 0 ? "取消" : "继续");
        lv_obj_align(s_lab[i], LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_font(s_lab[i], &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

void nav_kwask_show(void)
{
    if (s_active) {               /* 已弹着又开关一次门:门动作算用户操作,1 分钟重新计 */
        s_touch_ms = lv_tick_get();
        return;
    }
    kwask_build();
    if (!s_mask) return;

    nav_topflag_popups_yield();   /* 别的弹层让位(无效提示/收藏确认/关机确认),免叠印 */
    s_active = 1;
    s_sel = 1;                    /* 设计稿:继续为默认选中(实心),取消是空心态 */
    s_touch_ms = lv_tick_get();

    const lv_font_t *f = is_english() ? &c_aktivgroteskmedium_30 : &c_taiwanpearl_regular_30;
    /* 按钮文字:中文 30;英文 24(Continue 在 129 宽的按钮图里用 aktiv30 会贴边,
       与各页 EN 小字规则一致走 24) */
    const lv_font_t *fl = is_english() ? &c_aktivgroteskmedium_24 : &c_taiwanpearl_regular_30;
    lv_obj_set_style_text_font(s_title, f, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(s_title, tr("要继续保温吗？"));
    lv_label_set_text(s_lab[0], tr("取消"));
    lv_label_set_text(s_lab[1], is_english() ? "Continue" : tr("继续"));
    for (int i = 0; i < 2; i++)
        lv_obj_set_style_text_font(s_lab[i], fl, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_clear_flag(s_mask, LV_OBJ_FLAG_HIDDEN);
    nav_favtip_collect_hide();   /* 完成页右侧防烫/额外上色提示与提示列同位:先藏,收起时还原 */
    kwask_focus();
    printf("[keepwarm] ask show\n");
}

/* 只收界面,不动保温(继续/超时/自然结束共用) */
void nav_kwask_hide(void)
{
    if (!s_mask) return;
    s_active = 0;
    lv_obj_add_flag(s_mask, LV_OBJ_FLAG_HIDDEN);
    for (int i = 0; i < 2; i++)
        if (s_btn[i]) lv_obj_clear_state(s_btn[i], LV_STATE_FOCUSED);
    nav_favtip_restore_elems();   /* 还原 show 时藏掉的完成页右侧组件 */
}

int nav_kwask_active(void)
{
    return s_active;
}

uint32_t nav_kwask_touch_ms(void)
{
    return s_touch_ms;
}

/* 继续:保温原样继续(不重置计时,不重发 BUF[17]) */
void nav_kwask_resume(void)
{
    if (!s_active) return;
    nav_kwask_hide();
    printf("[keepwarm] ask resume -> keep warm continues\n");
}

/* 取消/BACK:结束保温并回主目录 */
void nav_kwask_cancel(void)
{
    if (!s_active) return;
    nav_kwask_hide();
    g_keepwarm_active = 0;        /* BUF[17] 随本标志下发 */
    g_keepwarm_sec = 0;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    nav_goto_major_menu();
    printf("[keepwarm] ask cancel -> major menu\n");
}

/* 1 分钟无操作 / 保温满 15 分钟:结束保温,收框,停在完成页(已完成) */
void nav_kwask_timeout(void)
{
    if (!s_active) return;
    nav_kwask_hide();
    nav_keepwarm_finish();        /* 下一拍 cooking_timer_cb 走正常结束支:文案回已完成+删定时器 */
    printf("[keepwarm] ask timeout -> 已完成\n");
}

void nav_kwask_key(uint8_t key)
{
    if (!s_active) return;
    s_touch_ms = lv_tick_get();   /* 任意按键重新计 1 分钟 */
    switch (key) {
    case KEY_ENCODER_CW:
    case KEY_ENCODER_CCW:
        s_sel ^= 1;
        kwask_focus();
        g_send.buzzer_req = BUZZER_KEY_VALID;
        break;
    case KEY_ENCODER_PRESS:
        g_send.buzzer_req = BUZZER_KEY_VALID;
        if (s_sel == 0) nav_kwask_cancel();
        else            nav_kwask_resume();
        break;
    case KEY_BACK:
        g_send.buzzer_req = BUZZER_KEY_VALID;
        nav_kwask_cancel();
        break;
    default:
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        break;
    }
    uart_print();
}
