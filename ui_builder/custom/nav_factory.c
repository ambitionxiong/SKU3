/*
 * nav_factory.c - 设置页-出厂设置确认页（PAGE_FACTORY_RESET，独立屏幕）
 *
 * 结构复用 probetip 的独立屏模式，版式按设计图：左上角标题标签（真实 label，
 * topflag 图标组据此居中）+ 右上确定胶囊（与日期页同位）+ 深灰圆角面板 +
 * 居中大标题（48 号）与说明（30 号，中 2 行/英 3 行）。
 * 确认 = SET_Data 全字段回默认（与 nav_screen_set.c 初始化器一致）+ 双收藏集
 * 清空 + 语言回默认简体，uart_print 上报；随后本页就地切"恢复出厂设置中"进度页
 * （进度条 5s 走满，期间按键无效），走满等同第一次上电：清栈进语言选择页→时间设置→系统。
 * BACK 不复位回设置层，焦点回 RESET 行。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"
#include "nav_internal.h"
#include "screen_SET.h"       /* 返回设置层后焦点定位 RESET_Btn 用 */
#include "nav_favorites.h"    /* Fav_Cur/双收藏集/空模板/收藏计数 */
#ifndef LV_USE_AIC_SIMULATOR
#include "test_data.h"        /* config_save/settings_mirror_rebuild(SDK 掉电保存,模拟器无持久化) */
#endif

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

/* 确认后的 5s 进度页(2026-10-14 用户定稿):大标题换"恢复出厂设置中"+进度条走满,
 * 走满回主菜单;期间按键无效(确认/BACK 双守卫) */
static lv_obj_t *s_prog_bar;
static lv_timer_t *s_prog_timer;
static uint8_t s_prog_run = 0;

static void factory_progress_done_cb(lv_timer_t *t)
{
    (void)t;
    s_prog_timer = NULL;
    s_prog_run = 0;
    s_prog_bar = NULL;
    if (!s_fr.obj || !lv_obj_is_valid(s_fr.obj)) return;   /* 5s 内页面被清场(关机等):只回状态不导航 */
    /* 复位=第一次上电:清栈后走完整首设链 语言页→时间页→进系统(g_langpick_done 已在 apply 清零) */
    depth = 0;
    page_push(PAGE_WAITMENU_24);   /* 与开机路径同构:wait 页垫底,随即被语言页盖住 */
    langpick_enter_from_standby();
    nav_topflag_demo_sync();    /* demo 徽标图分中英文,复位后立即重排 */
    nav_topflag_clock_force();  /* 时制回 24h:右上角时钟立即切换 */
}

/* 进度条动画执行(lv_anim 数值→bar 值) */
static void factory_bar_anim_cb(void *var, int32_t v)
{
    lv_bar_set_value((lv_obj_t *)var, v, LV_ANIM_OFF);
}

/* 进度页运行中(5s):process_key 全键吞/nav_keyio 电源动作吞/nav_alarm 警报延后/
 * count_down 超时不抢屏,各打断源守卫共用 */
int factory_prog_running(void)
{
    return s_prog_run;
}

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
    SET_Data.Set_VolumeKey = 4;   /* 默认 4 档(2026-10-09 用户定稿,与初始化器同步,原 7) */
    SET_Data.Set_VolumeWelcome = 1;
    SET_Data.Set_Brightness = 7;
    SET_Data.Set_TimeType = 0;
    SET_Data.Set_StandbyTime = 0;
    SET_Data.Set_Language = 2;
    SET_Data.Set_Power = 1;
    SET_Data.Set_DemoMode = 0;
    g_langpick_done = 0;   /* 恢复出厂后下次上电重新出现语言选择页(完整首设) */
    g_systime_boot_mode = 0;   /* 卫生位:开机日期页模式随首设标志一并作废 */

    /* 收藏双集合清空(普通+探针;Fav_Cur 停回普通集) */
    Fav_Cur = &Func_favorites_Value;
    *Fav_Cur = Func_favorites_Value_NULL;
    Fav_Cur = &Func_favorites_Value_Probe;
    *Fav_Cur = Func_favorites_Value_NULL;
    Fav_Cur = &Func_favorites_Value;
    favorites_how_many = 0;

    /* 多段参数全局载体(persist 的 multi= 键)一并清零:首次上电 RAM 默认即全零,
       不清则上一用户的半截多段编辑在恢复出厂后仍从收藏/多段页带出 */
    memset(&Favorites_Value, 0, sizeof(Favorites_Value));

    /* 后台倒计时整体停表清零(表/超时层/抢屏与下层记录):运行中恢复出厂,
       定时器不该再存活——否则首次设置链中到期会重建计时页抢屏拽走用户 */
    count_down_poweroff_reset();

    nav_brightness_apply();   /* 默认亮度档立即生效(背光随后续调整才变不合理) */

#ifndef LV_USE_AIC_SIMULATOR
    settings_mirror_rebuild();   /* 重建 BUF[15..17] 设置字节镜像:复位后当前会话立即按复位值上报 */
    config_save();               /* 立即全量落盘清掉旧内容,不等 persist 线程 1s 轮询 */
#endif

    g_lang_en = (SET_Data.Set_Language == 0);   /* 默认简体:同步 tr 语言标志 */
    uart_print();               /* 立即上报复位后的状态帧 */
    /* 语言已回默认:导航回主菜单延后到 5s 进度页走满(factory_progress_done_cb) */
}

/* 确定胶囊点击(框架 PRESS=聚焦按钮 click,与设置行回调同机制):
 * 数据立即复位落盘,页面就地切进度页,5s 后回主菜单 */
static void on_factory_ok(lv_event_t *e)
{
    (void)e;
    factory_page_t *scr = &s_fr;
    lv_anim_t a;
    const char *ttl;
    if (s_prog_run) return;   /* 进度 5s 内再按无效 */
    s_prog_run = 1;
    g_send.buzzer_req = BUZZER_KEY_VALID;
    ttl = tr("恢复出厂设置中");   /* 先取词:apply 会把语言重置回简体,之后 tr 只会返回中文 */
    factory_reset_apply();

    /* 进度页化:说明/确认胶囊退场,大标题换词(位置不动,设计图标题中心 ≈238=确认页原位),面板内加进度条 5s 线性走满 */
    lv_obj_add_flag(scr->Yes_Btn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->Desc_Lb, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(scr->Reset_Lb, ttl);

    s_prog_bar = lv_bar_create(scr->panel);
    lv_obj_set_size(s_prog_bar, 772, 32);      /* 设计图实测:外圈 772x32 */
    lv_obj_align(s_prog_bar, LV_ALIGN_TOP_MID, 0, 156);   /* 面板内水平自动居中;上移贴近标题(隙≈12px) */
    lv_obj_set_style_pad_all(s_prog_bar, 4, LV_PART_MAIN);   /* 可见间隙=pad-描边宽:4-2=2px,内条高 32-8=24(对齐标准图) */
    lv_obj_set_style_radius(s_prog_bar, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_prog_bar, lv_color_hex(0x2a2a2a), LV_PART_MAIN);
    lv_obj_set_style_border_color(s_prog_bar, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_border_width(s_prog_bar, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(s_prog_bar, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(s_prog_bar, lv_color_hex(0xffffff), LV_PART_INDICATOR);
    lv_bar_set_range(s_prog_bar, 0, 100);
    lv_bar_set_value(s_prog_bar, 0, LV_ANIM_OFF);
    lv_anim_init(&a);
    lv_anim_set_var(&a, s_prog_bar);
    lv_anim_set_exec_cb(&a, factory_bar_anim_cb);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_time(&a, 5000);                /* 进度条 5s 走满 */
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_start(&a);

    s_prog_timer = lv_timer_create(factory_progress_done_cb, 5000, NULL);
    lv_timer_set_repeat_count(s_prog_timer, 1);   /* 一次性:走满自删 */
}

void factory_back_action(void)
{
    if (s_prog_run) return;   /* 进度 5s 内 BACK 无效 */
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
    if (s_prog_timer) { lv_timer_del(s_prog_timer); s_prog_timer = NULL; }   /* 卫生位:清残留进度计时 */
    s_prog_run = 0;
    s_prog_bar = NULL;
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
