/*
 * 首次上电功率(电源管理)选择页（PAGE_POWER_PICK，独立屏幕）
 *
 * 首次上电链路（g_langpick_done == 0）：
 *   语言选择页(langpick_confirm 写语言) → 本页(确认功率) → 日期设置页
 *   (nav_systime.c, date_mode 分支) OK 写 RTC + firstboot=1 落盘 → 主菜单。
 *   中途任意时刻断电重上电，firstboot 仍为 0，从语言页重新走一遍。
 *
 * 页面结构照语言选择页(nav_langpick.c)：全页底图 bg.png，中央面板 preheatbg.png
 * (与预热页同图同位)，左上标题，面板内 16A/13A 两项滚轮(样式同 stepset roller：
 * 未选中 30 号灰 0x89817d / 选中 48 号白，行距 84px)，选中项带呼吸闪烁与下划线。
 * 标题随上一页已选语言(EN 为英文/繁中/简中各一套原文，本页不过翻译树遍历，
 * 页级豁免见 nav_lang.c lang_on_page_built)——与需求“根据上一页所设置语言”一致。
 *
 * 选项语义与设置页 电源管理 弹窗同源：16A = Set_Power 1 / 13A = 0，
 * 同时改 Machine_Set_num 功率位(协议 SEND_SETTINGS2 上报电源板)；
 * 默认光标按当前 Set_Power(出厂 16A)。确认时落盘 config_save。
 * 编码器滚动(到端点无效音)、按下确认进日期页；BACK 回语言页(链路内往返)。
 */

#include "nav.h"
#include "protocol.h"     /* 蜂鸣常量/g_send/MSt_Power_* */
#include "custom_defs.h"  /* SET_Data */
#include "nav_internal.h" /* group_create_for_page/nav_blink_* */
#include "i18n.h"         /* is_english/g_lang_en */
#ifndef LV_USE_AIC_SIMULATOR
#include "test_data.h"    /* config_save(SDK 掉电保存,模拟器无此文件) */
#endif

/* ===== 页面结构体(本文件自持,不进 ui_objects) ===== */
typedef struct {
    lv_obj_t *obj;
    lv_obj_t *Title_Lb;
    lv_obj_t *Panel_Img;
    lv_obj_t *Roller;          /* 16A/13A 两项滚轮 */
    lv_obj_t *Line;            /* 选中项下划线(选中恒居中,随选中文字一起呼吸) */
    lv_group_t *group;
} screen_powerpick_page_t;
static screen_powerpick_page_t s_pwr_page;

static screen_powerpick_page_t *screen_Powerpick_get(ui_manager_t *ui) { (void)ui; return &s_pwr_page; }

/* 滚轮几何/下划线:与语言页完全同参,两页切换无跳变 */
#define POWERPICK_ROLLER_X 440
#define POWERPICK_ROLLER_Y 158
#define POWERPICK_ROLLER_W 400
#define POWERPICK_ROLLER_H 258
#define POWERPICK_LINE_W   135
#define POWERPICK_LINE_X   ((1280 - POWERPICK_LINE_W) / 2)
#define POWERPICK_LINE_Y   317

/* 页面销毁时释放焦点组(挂 Title_Lb 的 LV_EVENT_DELETE,同语言页) */
static void powerpick_group_del_cb(lv_event_t *e)
{
    (void)e;
    if (s_pwr_page.group) {
        lv_group_del(s_pwr_page.group);
        s_pwr_page.group = NULL;
    }
}

lv_group_t *powerpick_page_group(void)
{
    return s_pwr_page.group;
}

void screen_Powerpick_create(ui_manager_t *ui)
{
    screen_powerpick_page_t *scr = screen_Powerpick_get(ui);

    if (!ui->auto_del && scr->obj) {
        return;
    }

    // Init scr->obj
    scr->obj = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(scr->obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(scr->obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->obj, LVGL_IMAGE_PATH(bg.png), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->Title_Lb  随上一页所选语言,固定原文,不过翻译表
    scr->Title_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Title_Lb, is_english() ? "Set Power Management" :
                                     ((SET_Data.Set_Language == 1) ? "設定電源管理" : "设置电源管理"));
    lv_obj_set_pos(scr->Title_Lb, 24, 24);
    lv_obj_set_size(scr->Title_Lb, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(scr->Title_Lb, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Title_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->Panel_Img  与预热页/语言页同图同位
    scr->Panel_Img = lv_image_create(scr->obj);
    lv_image_set_src(scr->Panel_Img, LVGL_IMAGE_PATH(preheatbg.png));
    lv_obj_set_pos(scr->Panel_Img, 24, 118);

    // Init scr->Roller  16A 在上 / 13A 在下(与设置页弹窗 13A→16A 行序相反:本页为选中大项在中间)
    scr->Roller = lv_roller_create(scr->obj);
    lv_roller_set_options(scr->Roller, "16A\n13A", LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(scr->Roller, 3);
    lv_obj_set_pos(scr->Roller, POWERPICK_ROLLER_X, POWERPICK_ROLLER_Y);
    lv_obj_set_size(scr->Roller, POWERPICK_ROLLER_W, POWERPICK_ROLLER_H);
    lv_obj_set_style_bg_opa(scr->Roller, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->Roller, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Roller, lv_color_hex(0x89817d), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->Roller, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->Roller, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->Roller, 0, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->Roller, &c_taiwanpearl_regular_48, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Roller, lv_color_hex(0xffffff), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->Roller, 51, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->Line  选中下划线:按选项文字宽度收窄(2026-11-11 用户)
    // "16A/13A" 48 号实测墨迹 78px(602..679),线取 90px(2×45),两侧各余 6px;
    // 语言页那条 190px 是按 "English" 墨迹配的,本页两项都短,沿用会明显长一截
    scr->Line = lv_image_create(scr->obj);
    lv_image_set_src(scr->Line, LVGL_IMAGE_PATH(underline_135x4.png));
    lv_obj_set_pos(scr->Line, POWERPICK_LINE_X, POWERPICK_LINE_Y);
    lv_image_set_scale_x(scr->Line, 171);   /* 135*171/256≈90,缩放绕中心,线仍居中 */

    /* 默认选中当前设置:Set_Power 1=16A(出厂默认) / 0=13A */
    lv_roller_set_selected(scr->Roller, SET_Data.Set_Power ? 0 : 1, LV_ANIM_OFF);

    /* 呼吸闪烁:照语言页注册(roller 选中行 text_opa + 下划线 img_opa,同相位) */
    lv_obj_t *blink_g[2] = { scr->Roller, scr->Line };
    nav_blink_group_register(scr->Roller, blink_g, 2);
    nav_editable_extra_register(scr->Roller);

    /* 焦点组:blink 由 FOCUSED 事件驱动,键路由走 nav_key.c 栈顶分流 */
    if (scr->group == NULL) {
        lv_obj_t *btns[1] = { scr->Roller };
        scr->group = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(scr->Title_Lb, powerpick_group_del_cb, LV_EVENT_DELETE, NULL);
    }
    if (scr->group) lv_group_focus_obj(scr->Roller);
}

lv_obj_t *screen_Powerpick_obj(void)
{
    return s_pwr_page.obj;
}

/* 进功率页(语言页确认后):清语言页子对象、重建本页并显示 */
void jump_to_power_pick(void)
{
    if (s_pwr_page.obj && lv_obj_is_valid(s_pwr_page.obj) &&
        s_pwr_page.obj != lv_scr_act()) {
        lv_obj_del(s_pwr_page.obj);   /* BACK 往返残留的空屏对象:删掉防泄漏 */
    }
    s_pwr_page.obj = NULL;
    s_pwr_page.group = NULL;

    page_push(PAGE_POWER_PICK);
    lv_obj_clean(lv_scr_act());
    screen_Powerpick_create(&ui_manager);
    current_group = powerpick_page_group();
    lang_scr_load_anim(screen_Powerpick_obj(),
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[powerpick] enter from langpick\n");
}

/* 日期页 BACK 回功率页(首次上电链路内往返):整屏重建。
 * 日期页已把本页子对象清掉,旧屏对象残留在 s_pwr_page.obj,先删再建 */
void powerpick_reenter(void)
{
    if (s_pwr_page.obj && lv_obj_is_valid(s_pwr_page.obj) &&
        s_pwr_page.obj != lv_scr_act()) {
        lv_obj_del(s_pwr_page.obj);
    }
    s_pwr_page.obj = NULL;
    s_pwr_page.group = NULL;
    g_langpick_date_mode = 0;   /* 回到链路中间:日期页 flag 失效,确认本页后重新置 1 */

    depth--;                    /* 弹掉日期页栈槽 */
    page_push(PAGE_POWER_PICK);
    lv_obj_clean(lv_scr_act());
    screen_Powerpick_create(&ui_manager);
    current_group = powerpick_page_group();
    lang_scr_load_anim(screen_Powerpick_obj(),
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[powerpick] reenter from systime\n");
}

/* 编码器滚动:到端点给无效音,可动给编码器音(与语言页同语义) */
void powerpick_encoder_action(char key)
{
    if (!s_pwr_page.Roller) return;
    uint32_t sel = lv_roller_get_selected(s_pwr_page.Roller);

    if (key == KEY_ENCODER_CW) {
        if (sel >= 1) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            return;
        }
        lv_roller_set_selected(s_pwr_page.Roller, sel + 1, LV_ANIM_ON);
    } else if (key == KEY_ENCODER_CCW) {
        if (sel == 0) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            return;
        }
        lv_roller_set_selected(s_pwr_page.Roller, sel - 1, LV_ANIM_ON);
    }
    g_send.buzzer_req = BUZZER_ENCODER;
}

/* 编码器按下确认:写功率状态(Set_Power + 协议位)并进入日期设置页。
 * firstboot 完成标志仍只在日期页 OK 落盘(中途断电从语言页重来) */
void powerpick_confirm(void)
{
    int is16 = (lv_roller_get_selected(s_pwr_page.Roller) == 0);

    SET_Data.Set_Power = is16 ? 1 : 0;
    if (is16) MSt_Power_16A; else MSt_Power_13A;
#ifndef LV_USE_AIC_SIMULATOR
    config_save();   /* 保存功率选择(firstboot= 仍写 0) */
#endif
    printf("[powerpick] confirm %s -> systime\n", is16 ? "16A" : "13A");
    g_langpick_date_mode = 1;
    jump_to_systime();   /* 复用日期设置页(nav_systime.c):date_mode 分支免弹设置覆盖层 */
}
