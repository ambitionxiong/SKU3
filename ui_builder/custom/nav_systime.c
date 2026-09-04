/*
 * nav_systime.c - 设置页-日期/时间子页（PAGE_SET_SYSTIME，独立屏幕）
 *
 * 移植自同事 screen_SET_Sys_Time.c：年/月/日/时制(12-24)/时/分 六字段下划线循环
 * 编辑，Yes 确认才写 RTC（sd8568_set_time，成功才保存时制偏好并返回；
 * 失败响错误音光标回"年"），BACK 不写直接回设置层。年份内部 0..99（20xx）。
 * 栏目标题同事用整幅底图（set_SYStime_bg_txt*.png，两包内均无此图），本端以
 * tr 文字标签替代，拿到图后可在 create 内一行换回。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"
#include "nav_internal.h"
#include "screen_SET.h"   /* 返回设置层后焦点定位 Time_Btn 用 */
#ifndef LV_USE_AIC_SIMULATOR
#include "sd8568.h"       /* 真机 RTC 写入（rt_err_t 接口） */
#endif

typedef struct {
    lv_obj_t *obj;
    lv_obj_t *set_time_bg_txt_Img;   /* 栏目标题整幅底图(日期时间/年月日/小时制) */
    lv_obj_t *Yes_Btn;
    lv_obj_t *Set_Year_Lb;
    lv_obj_t *Set_Month_Lb;
    lv_obj_t *Set_Day_Lb;
    lv_obj_t *Set_Time_Type_Lb;
    lv_obj_t *Set_Time_span;
    lv_span_t *Set_Time_span_span_0;
    lv_span_t *Set_Time_span_span_1;
    lv_obj_t *Underline_Btn;
    lv_group_t *group;
} systime_page_t;

static systime_page_t s_st;
static systime_page_t *systime_get(ui_manager_t *ui) { (void)ui; return &s_st; }

static uint8_t s_where_time = 0;   /* 0=Yes 待确认 1=年 2=月 3=日 4=时制 5=时 6=分 */
static int s_year = 0;             /* 0..99,显示 20xx */
static int s_month = 0;
static int s_day = 0;
static int s_timetype = 0;         /* 1=12h 0=24h */
static int s_hour = 0;
static int s_min = 0;

/* 非闰年每月天数 */
static const uint8_t s_month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static bool systime_is_leap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int systime_max_day(int year, int month)
{
    if (month < 1 || month > 12) return 0;
    int days = s_month_days[month - 1];
    if (month == 2 && systime_is_leap(year)) return 29;
    return days;
}

/* 蔡勒公式变体:0=星期日 */
static int systime_weekday(int year, int month, int day)
{
    if (month == 1 || month == 2) {
        month += 12;
        year--;
    }
    return (day + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400 + 1) % 7;
}

/* RTC 写入:真机走 sd8568(2000+year 存 0..99),模拟器 PC 时间不可写视为成功 */
static int systime_rtc_set(int year, int month, int wday, int day, int hour, int min)
{
#ifdef LV_USE_AIC_SIMULATOR
    (void)year; (void)month; (void)wday; (void)day; (void)hour; (void)min;
    return 0;
#else
    return (sd8568_set_time((rt_uint8_t)year, (rt_uint8_t)month, (rt_uint8_t)wday,
                            (rt_uint8_t)day, (rt_uint8_t)hour, (rt_uint8_t)min, 0) == RT_EOK) ? 0 : -1;
#endif
}

/* 下划线摆位:两语言两套(同事坐标原样) */
static void systime_underline_pos(int field)   /* 0..6,0=回 Yes 态(年份位) */
{
    systime_page_t *scr = systime_get(&ui_manager);
    if (!scr->Underline_Btn) return;
    static const int en_x[7] = {60, 60, 260, 457, 722, 994, 1060};
    static const int en_w[7] = {175, 175, 142, 120, 233, 52, 52};
    static const int cn_x[7] = {108, 108, 289, 440, 776, 994, 1060};
    static const int cn_w[7] = {138, 138, 79, 82, 145, 50, 50};
    if (field < 0 || field > 6) field = 0;
    if (is_english()) {
        lv_obj_set_pos(scr->Underline_Btn, en_x[field], 305);
        lv_obj_set_size(scr->Underline_Btn, en_w[field], 30);
    } else {
        lv_obj_set_pos(scr->Underline_Btn, cn_x[field], 305);
        lv_obj_set_size(scr->Underline_Btn, cn_w[field], 30);
    }
}

/* 时间 span 刷新(12h 制用 12 小时表示) */
static void systime_time_span_refresh(void)
{
    systime_page_t *scr = systime_get(&ui_manager);
    if (!scr->Set_Time_span_span_0) return;
    char buf[10];
    if (s_timetype) {
        int h12 = s_hour % 12;
        if (h12 == 0) h12 = 12;
        snprintf(buf, sizeof(buf), "%02d:%02d", h12, s_min);
    } else {
        snprintf(buf, sizeof(buf), "%02d:%02d", s_hour, s_min);
    }
    lv_span_set_text(scr->Set_Time_span_span_0, buf);
}

/* AM/PM span 刷新(24h 制置空) */
static void systime_ampm_refresh(void)
{
    systime_page_t *scr = systime_get(&ui_manager);
    if (!scr->Set_Time_span_span_1) return;
    if (s_timetype)
        lv_span_set_text(scr->Set_Time_span_span_1, tr(s_hour >= 12 ? "下午" : "上午"));
    else
        lv_span_set_text(scr->Set_Time_span_span_1, "");
}

/* 当月天数 clamp(年/月变化后调用) */
static void systime_day_clamp(void)
{
    systime_page_t *scr = systime_get(&ui_manager);
    int maxd = systime_max_day(s_year + 2000, s_month);
    if (s_day > maxd) {
        s_day = maxd;
        if (scr->Set_Day_Lb) lv_label_set_text_fmt(scr->Set_Day_Lb, "%02d", s_day);
    }
}

void encoder_systime_action(uint8_t key)
{
    systime_page_t *scr = systime_get(&ui_manager);
    if (!scr->obj || !lv_obj_is_valid(scr->obj) || !scr->group) return;

    if (key == KEY_ENCODER_CW || key == KEY_ENCODER_CCW) {
        int dir = (key == KEY_ENCODER_CW) ? 1 : -1;
        if (s_where_time == 0) {
            s_where_time = 1;
            lv_group_focus_obj(scr->Underline_Btn);
            systime_underline_pos(1);
            return;
        }
        switch (s_where_time) {
        case 1:
            s_year = (s_year + 100 + dir) % 100;
            lv_label_set_text_fmt(scr->Set_Year_Lb, "20%02d", s_year);
            systime_day_clamp();
            break;
        case 2:
            s_month = (s_month + 12 + dir - 1) % 12 + 1;
            lv_label_set_text_fmt(scr->Set_Month_Lb, "%02d", s_month);
            systime_day_clamp();
            break;
        case 3: {
            int maxd = systime_max_day(s_year + 2000, s_month);
            s_day = (s_day + maxd - 1 + dir) % maxd + 1;
            lv_label_set_text_fmt(scr->Set_Day_Lb, "%02d", s_day);
            break;
        }
        case 4:
            s_timetype = !s_timetype;
            if (scr->Set_Time_Type_Lb)
                lv_label_set_text(scr->Set_Time_Type_Lb, s_timetype ? "12" : "24");
            systime_ampm_refresh();
            break;
        case 5:
            s_hour = (s_hour + 24 + dir) % 24;
            systime_ampm_refresh();
            break;
        case 6:
            s_min = (s_min + 60 + dir) % 60;
            break;
        default:
            break;
        }
        systime_time_span_refresh();
        return;
    }

    /* ---- PRESS ---- */
    if (s_where_time == 0) {
        /* Yes:写 RTC,成功保存时制偏好回设置层,失败错误音光标回"年" */
        if (systime_rtc_set(s_year, s_month, systime_weekday(s_year + 2000, s_month, s_day),
                            s_day, s_hour, s_min) == 0) {
            SET_Data.Set_TimeType = (int8_t)s_timetype;
            uart_print();   /* 立即上报新状态帧(时制位) */
            page_pop();
            jump_to_screen_set();
            {   /* 焦点回到来源项"日期/时间"行 */
                screen_SET_t *ss = screen_SET_get(&ui_manager);
                if (ss && ss->Time_Btn) lv_group_focus_obj(ss->Time_Btn);
            }
            return;
        }
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        s_where_time = 1;
        systime_underline_pos(1);
        return;
    }
    /* 编辑中:字段循环 1..6→0(回 Yes 态) */
    s_where_time = (uint8_t)((s_where_time + 1) % 7);
    systime_underline_pos(s_where_time);
    if (s_where_time == 0)
        lv_group_focus_obj(scr->Yes_Btn);
}

void systime_back_action(void)
{
    page_pop();
    jump_to_screen_set();
    {   /* 焦点回到来源项"日期/时间"行 */
        screen_SET_t *ss = screen_SET_get(&ui_manager);
        if (ss && ss->Time_Btn) lv_group_focus_obj(ss->Time_Btn);
    }
}

lv_group_t *systime_page_group(void) { return s_st.group; }

void jump_to_systime(void)
{
    rtc_time_t t;
    s_where_time = 0;
    if (rtc_get_time(&t) == 0) {
        s_year = t.year % 100;
        s_month = t.month;
        s_day = t.day;
        s_hour = t.hour;
        s_min = t.min;
    }
    s_timetype = SET_Data.Set_TimeType;

    screen_set_reset();          /* 清设置覆盖层对象/指针 */
    depth--;                     /* 弹掉 PAGE_SCREEN_SET */
    page_push(PAGE_SET_SYSTIME);
    lv_obj_clean(lv_scr_act());

    systime_page_t *scr = &s_st;

    scr->obj = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(scr->obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(scr->obj, lv_color_hex(0xfcfcfc), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->obj, LVGL_IMAGE_PATH(used/bg1.jpg), LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 栏目标题整幅底图,按语言三选一(CN/繁體/EN;同事 ui_lang_img 三版本) */
    scr->set_time_bg_txt_Img = lv_img_create(scr->obj);
    if (SET_Data.Set_Language == 0)
        lv_img_set_src(scr->set_time_bg_txt_Img, LVGL_IMAGE_PATH(used/set_SYStime_bg_txt_en.png));
    else if (SET_Data.Set_Language == 1)
        lv_img_set_src(scr->set_time_bg_txt_Img, LVGL_IMAGE_PATH(used/set_SYStime_bg_txt_tw.png));
    else
        lv_img_set_src(scr->set_time_bg_txt_Img, LVGL_IMAGE_PATH(used/set_SYStime_bg_txt.png));
    lv_img_set_pivot(scr->set_time_bg_txt_Img, 50, 50);
    lv_img_set_angle(scr->set_time_bg_txt_Img, 0);
    lv_obj_set_style_img_opa(scr->set_time_bg_txt_Img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->set_time_bg_txt_Img, 24, 24);

    /* Init scr->Yes_Btn */
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

    /* 年/月/日/时制 数值标签 */
    scr->Set_Year_Lb = lv_label_create(scr->obj);
    lv_label_set_text_fmt(scr->Set_Year_Lb, "20%02d", s_year);
    lv_label_set_long_mode(scr->Set_Year_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_size(scr->Set_Year_Lb, 120, 50);
    lv_obj_set_style_text_font(scr->Set_Year_Lb, &c_taiwanpearl_regular_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Set_Year_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->Set_Year_Lb, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);

    scr->Set_Month_Lb = lv_label_create(scr->obj);
    lv_label_set_text_fmt(scr->Set_Month_Lb, "%02d", s_month);
    lv_label_set_long_mode(scr->Set_Month_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_size(scr->Set_Month_Lb, 70, 50);
    lv_obj_set_style_text_font(scr->Set_Month_Lb, &c_taiwanpearl_regular_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Set_Month_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->Set_Month_Lb, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);

    scr->Set_Day_Lb = lv_label_create(scr->obj);
    lv_label_set_text_fmt(scr->Set_Day_Lb, "%02d", s_day);
    lv_label_set_long_mode(scr->Set_Day_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_size(scr->Set_Day_Lb, 70, 50);
    lv_obj_set_style_text_font(scr->Set_Day_Lb, &c_taiwanpearl_regular_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Set_Day_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->Set_Day_Lb, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);

    scr->Set_Time_Type_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Set_Time_Type_Lb, s_timetype ? "12" : "24");
    lv_label_set_long_mode(scr->Set_Time_Type_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_size(scr->Set_Time_Type_Lb, 70, 50);
    lv_obj_set_style_text_font(scr->Set_Time_Type_Lb, &c_taiwanpearl_regular_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Set_Time_Type_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->Set_Time_Type_Lb, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 时间 span(HH:MM + AM/PM) */
    scr->Set_Time_span = lv_spangroup_create(scr->obj);
    lv_spangroup_set_mode(scr->Set_Time_span, LV_SPAN_MODE_FIXED);
    lv_spangroup_set_align(scr->Set_Time_span, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(scr->Set_Time_span, LV_SPAN_OVERFLOW_CLIP);

    scr->Set_Time_span_span_0 = lv_spangroup_new_span(scr->Set_Time_span);
    lv_style_set_text_font(&scr->Set_Time_span_span_0->style, &c_taiwanpearl_regular_48);
    lv_style_set_text_color(&scr->Set_Time_span_span_0->style, lv_color_hex(0xffffff));
    lv_style_set_text_opa(&scr->Set_Time_span_span_0->style, 255);
    lv_style_set_text_decor(&scr->Set_Time_span_span_0->style, LV_TEXT_DECOR_NONE);

    scr->Set_Time_span_span_1 = lv_spangroup_new_span(scr->Set_Time_span);
    lv_style_set_text_font(&scr->Set_Time_span_span_1->style, &c_taiwanpearl_regular_30);
    lv_style_set_text_color(&scr->Set_Time_span_span_1->style, lv_color_hex(0xffffff));
    lv_style_set_text_opa(&scr->Set_Time_span_span_1->style, 255);
    lv_style_set_text_decor(&scr->Set_Time_span_span_1->style, LV_TEXT_DECOR_NONE);

    lv_obj_set_size(scr->Set_Time_span, 184, 50);
    lv_obj_set_style_border_opa(scr->Set_Time_span, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->Set_Time_span, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* Init scr->Underline_Btn */
    scr->Underline_Btn = lv_btn_create(scr->obj);
    lv_obj_t *Underline_Btn_label = lv_label_create(scr->Underline_Btn);
    lv_label_set_text(Underline_Btn_label, "");
    lv_obj_align(Underline_Btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->Underline_Btn, LVGL_IMAGE_PATH(used/HowWellDone_underline.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    /* 字段/下划线坐标(中英两套,同事数值原样) */
    if (is_english()) {
        lv_obj_align(Yes_Btn_label, LV_ALIGN_CENTER, 0, 3);
        lv_obj_set_pos(scr->Set_Year_Lb, 50, 264);
        lv_obj_set_pos(scr->Set_Month_Lb, 243, 264);
        lv_obj_set_pos(scr->Set_Day_Lb, 440, 264);
        lv_obj_set_pos(scr->Set_Time_Type_Lb, 706, 264);
        lv_obj_set_pos(scr->Set_Time_span, 991, 264);
    } else {
        lv_obj_align(Yes_Btn_label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_pos(scr->Set_Year_Lb, 96, 260);
        lv_obj_set_pos(scr->Set_Month_Lb, 272, 260);
        lv_obj_set_pos(scr->Set_Day_Lb, 424, 260);
        lv_obj_set_pos(scr->Set_Time_Type_Lb, 761, 260);
        lv_obj_set_pos(scr->Set_Time_span, 991, 260);
    }
    systime_underline_pos(0);
    systime_time_span_refresh();
    systime_ampm_refresh();

    /* 组:Yes → 下划线 */
    if (scr->group == NULL) {
        scr->group = lv_group_create();
    } else {
        lv_group_del(scr->group);
        scr->group = lv_group_create();
    }
    lv_group_add_obj(scr->group, scr->Yes_Btn);
    lv_group_add_obj(scr->group, scr->Underline_Btn);

    current_group = scr->group;
    if (scr->Yes_Btn) lv_group_focus_obj(scr->Yes_Btn);
    lang_scr_load_anim(scr->obj, LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
    printf("[systime] jump\n");
}
