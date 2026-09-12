/*
 * nav_factory.c - 设置页-出厂设置确认页（PAGE_FACTORY_RESET，独立屏幕）
 *
 * 结构复用 probetip 的独立屏模式，版式按设计图：左上角标题标签（真实 label，
 * topflag 图标组据此居中）+ 右上确定胶囊（与日期页同位）+ 深灰圆角面板 +
 * 居中大标题（48 号）与说明（30 号，中 2 行/英 3 行）。
 * 确认 = SET_Data 全字段回默认（与 nav_screen_set.c 初始化器一致）+ 双收藏集
 * 清空 + 语言回默认简体，uart_print 上报后整页栈重置回主菜单（重建即时生效）。
 * BACK 不复位回设置层，焦点回 RESET 行。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"
#include "nav_internal.h"
#include "screen_SET.h"       /* 返回设置层后焦点定位 RESET_Btn 用 */
#include "nav_favorites.h"    /* Fav_Cur/双收藏集/空模板/收藏计数 */

typedef struct {
    lv_obj_t *obj;
    lv_obj_t *panel;        /* 深灰圆角面板(设计图实测 24,118,1232x337) */
    lv_obj_t *Title_Lb;     /* 左上角"出厂设置"(30 号) */
    lv_obj_t *Reset_Lb;     /* "恢复出厂设置"大标题(48 号,居中) */
    lv_obj_t *Desc_Lb;      /* 说明(30 号,居中,中 2 行/英 3 行) */
    lv_obj_t *Yes_Btn;
    lv_group_t *group;
} factory_page_t;

static factory_page_t s_fr;
static factory_page_t *factory_get(ui_manager_t *ui) { (void)ui; return &s_fr; }

/* 复位执行:确认键回调与后续扩展共用 */
static void factory_reset_apply(void)
{
    /* SET_Data 全字段回默认(逐字段与 nav_screen_set.c 初始化器保持一致) */
    SET_Data.Set_Lock = 0;
    SET_Data.Set_KeepWarm = 0;
    SET_Data.Set_FanCooling = 0;
    SET_Data.Set_6th = 0;
    SET_Data.Set_Light = 0;
    SET_Data.Set_TempUnit = 0;
    SET_Data.Set_VolumeFlag = 1;
    SET_Data.Set_VolumeHintTime = 0;
    SET_Data.Set_VolumeKey = 7;
    SET_Data.Set_VolumeWelcome = 1;
    SET_Data.Set_Brightness = 7;
    SET_Data.Set_TimeType = 0;
    SET_Data.Set_StandbyTime = 0;
    SET_Data.Set_Language = 2;
    SET_Data.Set_Power = 0;
    SET_Data.Set_DemoMode = 0;

    /* 收藏双集合清空(普通+探针;Fav_Cur 停回普通集) */
    Fav_Cur = &Func_favorites_Value;
    *Fav_Cur = Func_favorites_Value_NULL;
    Fav_Cur = &Func_favorites_Value_Probe;
    *Fav_Cur = Func_favorites_Value_NULL;
    Fav_Cur = &Func_favorites_Value;
    favorites_how_many = 0;

    g_lang_en = (SET_Data.Set_Language == 0);   /* 默认简体:同步 tr 语言标志 */
    uart_print();               /* 立即上报复位后的状态帧 */

    /* 语言已回默认:整页栈重置回主菜单,新语言随整页重建即时生效 */
    nav_goto_major_menu();
    nav_topflag_demo_sync();    /* demo 徽标图分中英文,复位后立即重排 */
    nav_topflag_clock_force();  /* 时制回 24h:右上角时钟立即切换 */
    g_send.buzzer_req = BUZZER_KEY_VALID;
}

/* 确定胶囊点击(框架 PRESS=聚焦按钮 click,与设置行回调同机制) */
static void on_factory_ok(lv_event_t *e)
{
    (void)e;
    factory_reset_apply();
}

void factory_back_action(void)
{
    page_pop();
    jump_to_screen_set();
    {   /* 焦点回到来源行"出厂设置" */
        screen_SET_t *ss = screen_SET_get(&ui_manager);
        if (ss && ss->RESET_Btn) lv_group_focus_obj(ss->RESET_Btn);
    }
}

void jump_to_factory(void)
{
    screen_set_reset();          /* 清设置覆盖层对象/指针 */
    depth--;                     /* 弹掉 PAGE_SCREEN_SET */
    page_push(PAGE_FACTORY_RESET);
    lv_obj_clean(lv_scr_act());

    factory_page_t *scr = &s_fr;

    scr->obj = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(scr->obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(scr->obj, lv_color_hex(0xfcfcfc), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->obj, LVGL_IMAGE_PATH(used/bg1.jpg), LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 深灰圆角面板(设计图实测 24,118,1232x337) */
    scr->panel = lv_obj_create(scr->obj);
    lv_obj_set_pos(scr->panel, 24, 118);
    lv_obj_set_size(scr->panel, 1232, 337);
    lv_obj_set_scrollbar_mode(scr->panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(scr->panel, lv_color_hex(0x2a2a2a), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->panel, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 左上角标题:真实标签(topflag nav_topleft_has_text 据此把图标组居中) */
    scr->Title_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Title_Lb, tr("出厂设置"));
    lv_obj_set_style_text_font(scr->Title_Lb, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Title_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->Title_Lb, 24, 19);

    /* 大标题(48 号,整行居中;中英墨迹中心 y 239/243,标签 y210) */
    scr->Reset_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Reset_Lb, tr("恢复出厂设置"));
    lv_obj_set_width(scr->Reset_Lb, 1280);
    lv_obj_set_style_text_font(scr->Reset_Lb, &c_taiwanpearl_regular_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Reset_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->Reset_Lb, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->Reset_Lb, 0, 210);

    /* 说明(30 号,整行居中;词条内带 \n:中 2 行/英 3 行;标签 y283,行距 37) */
    scr->Desc_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Desc_Lb, tr("所有自定义配置都将丢失，重置完成后，\n这些设置将无法恢复，您确定要重置吗？"));
    lv_obj_set_width(scr->Desc_Lb, 1280);
    lv_obj_set_style_text_font(scr->Desc_Lb, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Desc_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->Desc_Lb, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->Desc_Lb, 0, 283);

    /* 确定胶囊(与日期页同位 975,22,150x70) */
    scr->Yes_Btn = lv_btn_create(scr->obj);
    lv_obj_t *Yes_Btn_label = lv_label_create(scr->Yes_Btn);
    lv_label_set_text(Yes_Btn_label, tr("确 定"));
    lv_obj_set_pos(scr->Yes_Btn, 975, 22);
    lv_obj_set_size(scr->Yes_Btn, 150, 70);
    lv_obj_set_style_bg_opa(scr->Yes_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->Yes_Btn, LVGL_IMAGE_PATH(used/Btn_bg_N.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->Yes_Btn, &c_taiwanpearl_regular_36, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->Yes_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->Yes_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->Yes_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->Yes_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->Yes_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->Yes_Btn, LVGL_IMAGE_PATH(used/Btn_bg_Y.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->Yes_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->Yes_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_width(scr->Yes_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_align(Yes_Btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(scr->Yes_Btn, on_factory_ok, LV_EVENT_CLICKED, NULL);

    /* 组:仅确定胶囊(本页无编辑字段,确认=复位,BACK=回设置层) */
    if (scr->group == NULL) {
        scr->group = group_create_for_page(&scr->Yes_Btn, 1);
    } else {
        lv_group_del(scr->group);
        scr->group = group_create_for_page(&scr->Yes_Btn, 1);
    }
    current_group = scr->group;
    lv_group_focus_obj(scr->Yes_Btn);
    lang_scr_load_anim(scr->obj, LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
    nav_topflag_demo_sync();    /* 左上角有 Title_Lb:图标组按居中排布 */
    printf("[factory] jump\n");
}
