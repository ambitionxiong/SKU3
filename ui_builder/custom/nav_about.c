/*
 * nav_about.c - 设置页-关于机器页（PAGE_ABOUT，独立屏幕，无按钮纯展示）
 *
 * 结构复用 probetip 的独立屏模式，版式按设计图：左上角标题标签（真实 label，
 * topflag 图标组据此居中）+ 深灰圆角面板 + 5 行左对齐信息（30 号）。
 * 型号/12NC 为固定值；TFT 版本号固定 686030108；按键板版本=接收帧 BUF[16]
 * 高 4 位（BIT4~BIT7，BIT7 为高位）；电源板版本=接收帧 BUF[13]
 * （20260618 协议：电源板→显示板 18 字节帧）。进页时读一次即可（版本静态）。
 * BACK 回设置层，焦点回 GYJQ 行；功能键不拦截。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"
#include "nav_internal.h"
#include "screen_SET.h"       /* 返回设置层后焦点定位 GYJQ_Btn 用 */

#define ABOUT_TFT_VERSION     "686030108"   /* TFT 固件版本号(用户给定) */
#define ABOUT_MODEL           "WCM76P1HB"   /* 型号 */
#define ABOUT_12NC            "858791853530"

typedef struct {
    lv_obj_t *obj;
    lv_obj_t *panel;        /* 深灰圆角面板(与出厂设置页同位 24,118,1232x337) */
    lv_obj_t *Title_Lb;     /* 左上角"关于机器"(24 号) */
    lv_obj_t *Line_Lb[5];   /* 5 行信息(30 号,左对齐 x48,行距≈64) */
    lv_group_t *group;      /* 保留字段:本页无按钮,恒 NULL */
} about_page_t;

static about_page_t s_ab;
static about_page_t *about_get(ui_manager_t *ui) { (void)ui; return &s_ab; }

void about_back_action(void)
{
    page_pop();
    jump_to_screen_set();
    g_send.iface_status = IFACE_SETTING;   /* 离开版本查询态:回设置界面态 */
    uart_print();
    {   /* 焦点回到来源行"关于机器" */
        screen_SET_t *ss = screen_SET_get(&ui_manager);
        if (ss && ss->GYJQ_Btn) lv_group_focus_obj(ss->GYJQ_Btn);
    }
}

void jump_to_about(void)
{
    screen_set_reset();          /* 清设置覆盖层对象/指针 */
    depth--;                     /* 弹掉 PAGE_SCREEN_SET */
    page_push(PAGE_ABOUT);
    lv_obj_clean(lv_scr_act());

    about_page_t *scr = &s_ab;
    scr->group = NULL;

    scr->obj = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(scr->obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(scr->obj, lv_color_hex(0xfcfcfc), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->obj, LVGL_IMAGE_PATH(used/bg1.jpg), LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 深灰圆角面板(与出厂设置页同位 24,118,1232x337) */
    scr->panel = lv_obj_create(scr->obj);
    lv_obj_set_pos(scr->panel, 24, 118);
    lv_obj_set_size(scr->panel, 1232, 337);
    lv_obj_set_scrollbar_mode(scr->panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(scr->panel, lv_color_hex(0x2a2a2a), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->panel, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(scr->panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);   /* 行标签按面板左上绝对定位,清默认内边距 */

    /* 左上角标题:真实标签(topflag nav_topleft_has_text 据此把图标组居中) */
    scr->Title_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Title_Lb, tr("关于机器"));
    lv_obj_set_style_text_font(scr->Title_Lb, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Title_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->Title_Lb, 24, 19);

    /* 5 行信息(30 号,墨迹左缘 x49→标签 x48,墨迹顶 y144/211/272/337/400→标签 y-6) */
    {
        int keypad_ver = (uart_data_receive[16] >> 4) & 0x0F;   /* BUF[16] BIT4~BIT7,BIT7 为高位 */
        int power_ver  = uart_data_receive[Receive_data_Power_Versions];   /* BUF[13] */
        const int line_y[5] = {138, 205, 266, 331, 394};   /* 屏坐标标签顶,面板内再减 118 */
        char buf[5][96];
        snprintf(buf[0], sizeof(buf[0]), "%s: " ABOUT_MODEL, tr("型号"));
        snprintf(buf[1], sizeof(buf[1]), "12NC: " ABOUT_12NC);
        snprintf(buf[2], sizeof(buf[2]), "%s: V" ABOUT_TFT_VERSION, tr("TFT版本号"));
        snprintf(buf[3], sizeof(buf[3]), "%s: V%02d", tr("按键板版本号"), keypad_ver);
        snprintf(buf[4], sizeof(buf[4]), "%s: V%02d", tr("电源板版本号"), power_ver);
        for (int i = 0; i < 5; i++) {
            scr->Line_Lb[i] = lv_label_create(scr->panel);
            lv_label_set_text(scr->Line_Lb[i], buf[i]);
            lv_obj_set_style_text_font(scr->Line_Lb[i], &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(scr->Line_Lb[i], lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_pos(scr->Line_Lb[i], 24, line_y[i] - 118);   /* 面板内相对坐标 */
        }
    }

    /* 本页无按钮:无焦点组,current_group 置空(BACK 由页枚举分支接管) */
    current_group = NULL;
    g_send.iface_status = IFACE_VERSION_QUERY;   /* 界面状态=版本查询(7):通知电源板上报版本 */
    uart_print();
    lang_scr_load_anim(scr->obj, LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
    nav_topflag_demo_sync();    /* 左上角有 Title_Lb:图标组按居中排布 */
    printf("[about] jump\n");
}
