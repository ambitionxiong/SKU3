/*
 * 首次上电语言选择页（PAGE_LANG_PICK，独立屏幕）
 *
 * 首次上电（g_langpick_done == 0）由 nav_init() 进入本页代替主菜单：
 * 全页底图 bg.png，中央面板 preheatbg.png（与预热页同图同位），左上固定双语标题，
 * 面板内 English/繁體中文/简体中文 三项滚轮（lv_roller，样式与 stepset 滚轮一致：
 * 未选中 24 号灰 0x89817d / 选中 36 号白，行距 84px，默认选中 English），编码器
 * 滚动（到端点无效音）、按下确认后写语言并进入首次上电日期设置页（nav_systime.c
 * 复用，date_mode 分支）；日期页 OK 写 RTC + firstboot=1 落盘后进 waitmenu 待机页。
 * 选中项带呼吸闪烁（全 UI 统一 blink 系统）：滚轮进闪烁组后选中行走 SELECTED
 * text_opa 通道、下划线图走 img_opa 通道，同相位；由焦点组 FOCUSED 事件驱动。
 * 下划线固定在中央行下方（roller 选中恒居中，线不随动）。
 *
 * 三项文字为固定原文，任何语言下都不过翻译树遍历（页级豁免见 nav_lang.c
 * lang_on_page_built；"简体中文" 同时是 i18n 英文表词条键，误入遍历会被改写）。
 *
 * 持久化：SDK 侧 g_langpick_done 经 test_data.c config.txt 的 firstboot= 键掉电
 * 保存，语义为"语言+日期全流程已完成"——语言确认时只存语言选择（firstboot= 仍 0），
 * 日期页 OK 才置 1 落盘，故流程中途任意时刻断电，重上电都会重新进设置；恢复出厂
 * （nav_factory.c）清标志同理。模拟器无持久化，每次启动都进一遍设置链路。
 */

#include "nav.h"
#include "protocol.h"     /* 蜂鸣常量/g_send */
#include "custom_defs.h"  /* SET_Data */
#include "nav_internal.h" /* group_create_for_page/nav_blink_* */
#include "nav_idle.h"     /* nav_goto_major_menu: 确认后进主菜单 */
#include "i18n.h"         /* g_lang_en */
#ifndef LV_USE_AIC_SIMULATOR
#include "test_data.h"    /* config_save(SDK 掉电保存,模拟器无此文件) */
#endif

/* ===== 页面结构体(本文件自持,不进 ui_objects) ===== */
typedef struct {
    lv_obj_t *obj;
    lv_obj_t *Title_Lb;
    lv_obj_t *Panel_Img;
    lv_obj_t *Roller;          /* 三语言滚轮(样式同 stepset roller) */
    lv_obj_t *Line;            /* 选中项下划线(选中恒居中,固定位置,随选中文字一起呼吸) */
    lv_group_t *group;
} screen_langpick_page_t;
static screen_langpick_page_t s_pick_page;

static screen_langpick_page_t *screen_Langpick_get(ui_manager_t *ui) { (void)ui; return &s_pick_page; }

uint8_t g_langpick_done = 0;    /* 首次设置流程(语言+日期)已完成;日期页 OK 时才置 1+落盘(SDK firstboot= 键) */
uint8_t g_langpick_date_mode = 0;   /* 首次上电链路:语言已确认进入日期页(RAM 态,断电即失) */

/* 三项固定原文,下标即 SET_Data.Set_Language 编码(0英语 1繁体 2简体,与设置页语言弹窗一致) */
static const char *const s_lang_names[3] = { "English", "繁體中文", "简体中文" };

/* 滚轮几何:行距 84 = 24 号行高 33 + 行距 51,可见 3 行高 252,在面板(24,118,1233x339)内垂直居中
 * 下划线在中央行文字下方(中央行 band 245..329,36 号文字底 ≈305,线 y=311 居中 x) */
#define LANGPICK_ROLLER_X   440
#define LANGPICK_ROLLER_Y   161
#define LANGPICK_ROLLER_W   400
#define LANGPICK_ROLLER_H   252
#define LANGPICK_LINE_W     135
#define LANGPICK_LINE_X     ((1280 - LANGPICK_LINE_W) / 2)
#define LANGPICK_LINE_Y     311

/* 页面销毁时释放焦点组(挂 Title_Lb 的 LV_EVENT_DELETE,照 nav_loudness Scr_Group_Delete) */
static void langpick_group_del_cb(lv_event_t *e)
{
    (void)e;
    if (s_pick_page.group) {
        lv_group_del(s_pick_page.group);
        s_pick_page.group = NULL;
    }
}

lv_group_t *langpick_page_group(void)
{
    return s_pick_page.group;
}

void screen_Langpick_create(ui_manager_t *ui)
{
    screen_langpick_page_t *scr = screen_Langpick_get(ui);

    if (!ui->auto_del && scr->obj) {
        return;
    }

    // Init scr->obj
    scr->obj = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(scr->obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(scr->obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->obj, LVGL_IMAGE_PATH(bg.png), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->Title_Lb  固定双语原文,不过翻译表
    scr->Title_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Title_Lb, "Set Language | 設定語言");
    lv_obj_set_pos(scr->Title_Lb, 24, 24);
    lv_obj_set_size(scr->Title_Lb, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_text_font(scr->Title_Lb, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Title_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->Panel_Img  与预热页同图同位
    scr->Panel_Img = lv_image_create(scr->obj);
    lv_image_set_src(scr->Panel_Img, LVGL_IMAGE_PATH(preheatbg.png));
    lv_obj_set_pos(scr->Panel_Img, 24, 118);

    // Init scr->Roller  三语言滚轮,样式照抄 stepset roller(未选中 24 灰/选中 36 白)
    scr->Roller = lv_roller_create(scr->obj);
    lv_roller_set_options(scr->Roller, "English\n繁體中文\n简体中文", LV_ROLLER_MODE_NORMAL);
    lv_roller_set_visible_row_count(scr->Roller, 3);
    lv_obj_set_pos(scr->Roller, LANGPICK_ROLLER_X, LANGPICK_ROLLER_Y);
    lv_obj_set_size(scr->Roller, LANGPICK_ROLLER_W, LANGPICK_ROLLER_H);
    lv_obj_set_style_bg_opa(scr->Roller, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->Roller, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Roller, lv_color_hex(0x89817d), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->Roller, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->Roller, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->Roller, 0, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->Roller, &c_taiwanpearl_regular_36, LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Roller, lv_color_hex(0xffffff), LV_PART_SELECTED | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->Roller, 51, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->Line  选中下划线(roller 选中恒居中,固定位置)
    scr->Line = lv_image_create(scr->obj);
    lv_image_set_src(scr->Line, LVGL_IMAGE_PATH(underline_135x4.png));
    lv_obj_set_pos(scr->Line, LANGPICK_LINE_X, LANGPICK_LINE_Y);

    /* 进页默认选中 English(0):首次上电/恢复出厂/中途断电重来都从英语起 */
    lv_roller_set_selected(scr->Roller, 0, LV_ANIM_OFF);

    /* 呼吸闪烁(照 stepset 滚轮注册,nav_somecook.c jump_to_stepset):
     * roller 进组→选中行 SELECTED text_opa 呼吸,线进组→img_opa 呼吸,同相位;
     * nav_editable_extra_register 放行两态门槛(boot 后 nav_blink_forget 已清表) */
    lv_obj_t *blink_g[2] = { scr->Roller, scr->Line };
    nav_blink_group_register(scr->Roller, blink_g, 2);
    nav_editable_extra_register(scr->Roller);

    /* 焦点组:blink 由 FOCUSED 事件驱动(nav_blink_focus_cb 挂在组上);
     * 键路由走 nav_key.c 栈顶分流,不经过组,组只承载焦点/呼吸 */
    if (scr->group == NULL) {
        lv_obj_t *btns[1] = { scr->Roller };
        scr->group = group_create_for_page(btns, 1);
        lv_obj_add_event_cb(scr->Title_Lb, langpick_group_del_cb, LV_EVENT_DELETE, NULL);
    }
    if (scr->group) lv_group_focus_obj(scr->Roller);
}

lv_obj_t *screen_Langpick_obj(void)
{
    return s_pick_page.obj;
}

/* 日期页 BACK 返回语言页(首次上电链路内往返):整屏重建。
 * 语言页对象在上次确认时已被 lv_obj_clean 清掉子对象(焦点组也随之释放),
 * 先弃旧屏指针/删残留屏再重建;page_push 自带 nav_blink_forget 清旧注册,
 * create 内重建闪烁组/焦点组,默认选中 English */
void langpick_reenter(void)
{
    if (s_pick_page.obj && lv_obj_is_valid(s_pick_page.obj) &&
        s_pick_page.obj != lv_scr_act()) {
        lv_obj_del(s_pick_page.obj);   /* auto_del=0 时旧屏仍挂着:删掉防泄漏 */
    }
    s_pick_page.obj = NULL;
    s_pick_page.group = NULL;

    depth--;                           /* 弹掉日期页栈槽 */
    page_push(PAGE_LANG_PICK);
    lv_obj_clean(lv_scr_act());
    screen_Langpick_create(&ui_manager);
    current_group = langpick_page_group();
    lang_scr_load_anim(screen_Langpick_obj(),
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[langpick] reenter from systime\n");
}

/* 首设未完成时从待机页(waitmenu)改道重进语言设置页——wait 页任意非关机键
 * (nav_key.c 守卫)与 KEY1 长按唤醒(nav_keyio.c)共用入口。
 * 注意不能 lv_obj_clean(lv_scr_act()):此时当前屏是 waitmenu,而 waitmenu_24_create
 * 靠旧 obj+子对象复用(auto_del=0 早退守卫),clean 掉子对象下次进待机会白屏;
 * 语言页在 idle 拽走时(nav_enter_standby)只被 clean 了子对象,屏对象残留在
 * s_pick_page.obj,这里先删掉再全新重建。栈压成 [WAITMENU_24, LANG_PICK] 与
 * 开机 nav_init 同构 */
void langpick_enter_from_standby(void)
{
    if (s_pick_page.obj && lv_obj_is_valid(s_pick_page.obj) &&
        s_pick_page.obj != lv_scr_act()) {
        lv_obj_del(s_pick_page.obj);   /* auto_del=0 时旧屏仍挂着:删掉防泄漏 */
    }
    s_pick_page.obj = NULL;
    s_pick_page.group = NULL;
    g_langpick_date_mode = 0;   /* 回到链路起点:语言确认后才会再置 1 */

    page_push(PAGE_LANG_PICK);
    screen_Langpick_create(&ui_manager);
    current_group = langpick_page_group();
    lang_scr_load_anim(screen_Langpick_obj(),
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    g_send.iface_status = IFACE_SETTING;
    topflag_update_visibility();   /* 设置链两页:顶层状态条隐藏 */
    printf("[langpick] reenter from waitmenu\n");
}

/* 编码器滚动滚轮:到端点给无效音,可动给编码器音(与 stepset roller 同语义) */
void langpick_encoder_action(char key)
{
    if (!s_pick_page.Roller) return;
    uint32_t sel = lv_roller_get_selected(s_pick_page.Roller);

    if (key == KEY_ENCODER_CW) {
        if (sel >= 2) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            return;
        }
        lv_roller_set_selected(s_pick_page.Roller, sel + 1, LV_ANIM_ON);
    } else if (key == KEY_ENCODER_CCW) {
        if (sel == 0) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            return;
        }
        lv_roller_set_selected(s_pick_page.Roller, sel - 1, LV_ANIM_ON);
    }
    g_send.buzzer_req = BUZZER_ENCODER;
}

/* 编码器按下确认:写语言状态并进入首次上电日期设置页(蜂鸣由 nav_key.c 分支给 BUZZER_KEY_VALID)。
 * g_langpick_done 此处不置 1:flash 完成标志代表"语言+日期全流程完成",只在日期页 OK 时落盘——
 * 语言确认后、日期 OK 前断电重上电,firstboot= 仍为 0,会重新进设置 */
void langpick_confirm(void)
{
    int sel = (int)lv_roller_get_selected(s_pick_page.Roller);

    SET_Data.Set_Language = (int8_t)sel;
    g_lang_en = (sel == 0) ? 1 : 0;
#ifndef LV_USE_AIC_SIMULATOR
    config_save();   /* 保存语言选择(firstboot= 仍写 0) */
#endif
    printf("[langpick] confirm sel=%d lang=%d en=%u -> systime\n", sel, SET_Data.Set_Language, g_lang_en);
    g_langpick_date_mode = 1;
    jump_to_systime();   /* 复用日期设置页(nav_systime.c):date_mode 分支免弹设置覆盖层 */
}
