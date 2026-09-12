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
    lv_obj_t *Set_Hour_Lb;     /* 时(48 号,独立呼吸) */
    lv_obj_t *Set_Colon_Lb;    /* 冒号(48 号,常亮不呼吸) */
    lv_obj_t *Set_Min_Lb;      /* 分(48 号,独立呼吸) */
    lv_obj_t *Set_AmPm_Lb;     /* 上午/下午(30 号,24h 置空) */
    lv_obj_t *Year_Unit_Lb;    /* 单位"年"(30 号,随数值整组呼吸) */
    lv_obj_t *Month_Unit_Lb;   /* "月" */
    lv_obj_t *Day_Unit_Lb;     /* "日" */
    lv_obj_t *Type_Unit_Lb;    /* "小时制" */
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

/* 下划线摆位:0..6,0=Yes 态(停在年份位)。组墨迹按标注图实测外扩约 2px;
   EN 单位为 Year/Month/Day 较宽,下划线随之加长(数值锚点三语一致) */
static void systime_underline_pos(int field)
{
    systime_page_t *scr = systime_get(&ui_manager);
    if (!scr->Underline_Btn) return;
    static const int en_x[7] = {56, 56, 257, 456, 722, 990, 1056};
    static const int en_w[7] = {179, 179, 146, 124, 233, 58, 58};
    static const int cn_x[7] = {109, 109, 290, 445, 780, 996, 1061};
    static const int cn_w[7] = {136, 136, 76, 77, 139, 49, 50};
    if (field < 0 || field > 6) field = 0;
    /* 线体=纯色矩形(与原 217px 素材居中裁切渲染同位同粗 y318-320);
       素材 bg img 原尺寸绘制不拉伸,条宽超 217 画不满,故弃用 */
    if (is_english()) {
        lv_obj_set_pos(scr->Underline_Btn, en_x[field], 318);
        lv_obj_set_size(scr->Underline_Btn, en_w[field], 3);
    } else {
        lv_obj_set_pos(scr->Underline_Btn, cn_x[field], 318);
        lv_obj_set_size(scr->Underline_Btn, cn_w[field], 3);
    }
}

/* 按当前编辑位(s_where_time)重登记呼吸组:下划线+当前位数值+单位。
   时/分独立标签分开呼吸,冒号常亮 */
static void systime_blink_register(int focused)
{
    systime_page_t *scr = systime_get(&ui_manager);
    if (!scr->Underline_Btn) return;
    lv_obj_t *g[4];
    int n = 0;
    g[n++] = scr->Underline_Btn;
    switch (s_where_time) {
    case 1: g[n++] = scr->Set_Year_Lb;      g[n++] = scr->Year_Unit_Lb;  break;
    case 2: g[n++] = scr->Set_Month_Lb;     g[n++] = scr->Month_Unit_Lb; break;
    case 3: g[n++] = scr->Set_Day_Lb;       g[n++] = scr->Day_Unit_Lb;   break;
    case 4: g[n++] = scr->Set_Time_Type_Lb; g[n++] = scr->Type_Unit_Lb;  break;
    case 5: g[n++] = scr->Set_Hour_Lb;      break;
    case 6: g[n++] = scr->Set_Min_Lb;       break;
    default: return;
    }
    nav_blink_group_register(scr->Underline_Btn, g, n);
    if (focused) nav_blink_refresh(scr->Underline_Btn);
}

/* 时/分标签刷新(12h 制 hour 用 12 小时表示) */
static void systime_time_span_refresh(void)
{
    systime_page_t *scr = systime_get(&ui_manager);
    if (!scr->Set_Hour_Lb) return;
    if (s_timetype) {
        int h12 = s_hour % 12;
        if (h12 == 0) h12 = 12;
        lv_label_set_text_fmt(scr->Set_Hour_Lb, "%02d", h12);
    } else {
        lv_label_set_text_fmt(scr->Set_Hour_Lb, "%02d", s_hour);
    }
    if (scr->Set_Min_Lb)
        lv_label_set_text_fmt(scr->Set_Min_Lb, "%02d", s_min);
}

/* AM/PM 标签刷新(24h 制置空) */
static void systime_ampm_refresh(void)
{
    systime_page_t *scr = systime_get(&ui_manager);
    if (!scr->Set_AmPm_Lb) return;
    if (s_timetype)
        lv_label_set_text(scr->Set_AmPm_Lb, tr(s_hour >= 12 ? "下午" : "上午"));
    else
        lv_label_set_text(scr->Set_AmPm_Lb, "");
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
            systime_underline_pos(1);   /* 线未聚焦不可见:先摆到年位再聚焦,FOCUSED 出现即已就位 */
            lv_group_focus_obj(scr->Underline_Btn);
            systime_blink_register(1);   /* 进入"年"编辑:起呼吸 */
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
    lv_obj_t *focused = lv_group_get_focused(scr->group);
    if (focused == scr->Yes_Btn) {
        /* Yes:写 RTC,成功保存时制偏好回设置层,失败错误音回"年"编辑态 */
        if (systime_rtc_set(s_year, s_month, systime_weekday(s_year + 2000, s_month, s_day),
                            s_day, s_hour, s_min) == 0) {
            SET_Data.Set_TimeType = (int8_t)s_timetype;
            uart_print();   /* 立即上报新状态帧(时制位) */
            nav_topflag_clock_force();   /* 右上角时制排版立即切换(不等 500ms tick) */
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
        if (scr->Underline_Btn) lv_group_focus_obj(scr->Underline_Btn);
        systime_blink_register(1);   /* 回"年"编辑态:起呼吸 */
        return;
    }
    if (!nav_edit_session_active()) {
        nav_edit_session_enter(focused);   /* 浏览态按确认:重新进编辑(起改当前位) */
        return;
    }
    /* 编辑中:字段循环 年→月→日→时制→时→分→确定,确定位再按=写 RTC(与计时器页一致)。
       分→确定不挪线:线失焦即隐身,停在分位;下次 dive 前先摆位再聚焦,杜绝闪影 */
    if (s_where_time >= 6) {
        s_where_time = 0;
        if (scr->Yes_Btn)
            lv_group_focus_obj(scr->Yes_Btn);
        return;
    }
    s_where_time++;
    systime_underline_pos(s_where_time);
    systime_blink_register(1);   /* 换位重登记呼吸组:只闪当前位 */
}

/* ---- 浏览模式:确定→年→月→日→时制→时→分→确定 环游 ----
   数字位以 s_where_time 表示,焦点留在下划线、下划线随位移动 */
static void systime_browse_to_underline(systime_page_t *scr)
{
    if (scr->Underline_Btn) {
        systime_underline_pos(s_where_time);
        systime_blink_register(0);   /* 换位同步呼吸组(浏览态不刷新=不闪,按下确认才起闪) */
        lv_group_focus_obj(scr->Underline_Btn);
    }
}

void systime_browse_action(uint8_t key)
{
    systime_page_t *scr = systime_get(&ui_manager);
    if (!scr->obj || !lv_obj_is_valid(scr->obj) || !scr->group) return;
    lv_obj_t *focused = lv_group_get_focused(scr->group);
    uint8_t fwd = (key == KEY_ENCODER_CW);

    if (focused == scr->Yes_Btn) {
        s_where_time = fwd ? 1 : 6;
        systime_browse_to_underline(scr);   /* 确定→年 / 确定→分 */
    } else {
        if (fwd) {
            if (s_where_time >= 6) { if (scr->Yes_Btn) lv_group_focus_obj(scr->Yes_Btn); }  /* 分→确定 */
            else { s_where_time++; systime_browse_to_underline(scr); }
        } else {
            if (s_where_time <= 1) { if (scr->Yes_Btn) lv_group_focus_obj(scr->Yes_Btn); }  /* 年→确定 */
            else { s_where_time--; systime_browse_to_underline(scr); }
        }
    }
}

/* 浏览模式 PRESS:下划线→进该位编辑;确定→写 RTC */
void systime_browse_press(void)
{
    systime_page_t *scr = systime_get(&ui_manager);
    if (!scr->obj || !lv_obj_is_valid(scr->obj) || !scr->group) return;
    lv_obj_t *focused = lv_group_get_focused(scr->group);
    if (focused == scr->Underline_Btn) {
        nav_edit_session_enter(focused);
        return;
    }
    encoder_systime_action(KEY_ENCODER_PRESS);   /* Yes:写 RTC */
}

void systime_back_action(void)
{
    /* 编辑态 BACK 由 nav_key.c 通用守卫接管(下划线已登记 extras:退浏览,焦点留原地)。
       走到这里即浏览模式:离开页面回设置层 */
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
    s_where_time = 0;   /* 进页停在"确定",会话为编辑态:转旋钮直接进入"年"编辑 */
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
    lv_obj_set_style_bg_img_src(scr->obj, LVGL_IMAGE_PATH(bg.jpg), LV_PART_MAIN | LV_STATE_DEFAULT);

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

    /* 单位标签(年/月/日/小时制,30 号;随数值整组呼吸;坐标=原底图文字实测位置,见定位块) */
    scr->Year_Unit_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Year_Unit_Lb, tr("年"));
    scr->Month_Unit_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Month_Unit_Lb, tr("月"));
    scr->Day_Unit_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Day_Unit_Lb, tr("日"));
    scr->Type_Unit_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Type_Unit_Lb, tr("小时制"));

    /* 时/分独立标签(48 号,分开呼吸)+冒号(常亮)+上午/下午(30 号,24h 置空) */
    scr->Set_Hour_Lb = lv_label_create(scr->obj);
    lv_obj_set_size(scr->Set_Hour_Lb, 58, 50);
    lv_obj_set_style_text_align(scr->Set_Hour_Lb, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    scr->Set_Colon_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Set_Colon_Lb, ":");
    scr->Set_Min_Lb = lv_label_create(scr->obj);
    lv_obj_set_size(scr->Set_Min_Lb, 58, 50);
    lv_obj_set_style_text_align(scr->Set_Min_Lb, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    scr->Set_AmPm_Lb = lv_label_create(scr->obj);

    lv_obj_t *unit_lbs[4] = {scr->Year_Unit_Lb, scr->Month_Unit_Lb, scr->Day_Unit_Lb, scr->Type_Unit_Lb};
    for (int i = 0; i < 4; i++) {
        lv_obj_set_style_text_font(unit_lbs[i], &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(unit_lbs[i], lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    lv_obj_t *time_lbs[4] = {scr->Set_Hour_Lb, scr->Set_Colon_Lb, scr->Set_Min_Lb, scr->Set_AmPm_Lb};
    const lv_font_t *time_fonts[4] = {&c_taiwanpearl_regular_48, &c_taiwanpearl_regular_48,
                                      &c_taiwanpearl_regular_48, &c_taiwanpearl_regular_30};
    for (int i = 0; i < 4; i++) {
        lv_obj_set_style_text_font(time_lbs[i], time_fonts[i], LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(time_lbs[i], lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

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
    lv_obj_set_style_bg_color(scr->Underline_Btn, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(scr->Underline_Btn, 255, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    /* 字段坐标(标注图实测;英文单位 Year/Month/Day/AM-PM 较宽,单独分支便于微调):
       数值右对齐锚点 年217/月340/日494/时制833/时1044/分1110;
       单位/AM-PM 左对齐=数值右缘+4px;y 与原渲染一致 */
    lv_obj_align(Yes_Btn_label, LV_ALIGN_CENTER, 0, 0);
    if (is_english()) {
        lv_obj_set_pos(scr->Set_Year_Lb, 47, 264);      /* 盒宽120,墨迹右缘217 */
        lv_obj_set_pos(scr->Set_Month_Lb, 243, 264);    /* 盒宽70,墨迹右缘340 */
        lv_obj_set_pos(scr->Set_Day_Lb, 439, 264);      /* 盒宽70,墨迹右缘494 */
        lv_obj_set_pos(scr->Set_Time_Type_Lb, 707, 264);/* 盒宽70,墨迹右缘833 */
        lv_obj_set_pos(scr->Set_Hour_Lb, 988, 263);     /* 盒宽58,墨迹右缘1044 */
        lv_obj_set_pos(scr->Set_Colon_Lb, 1046, 262);   /* 冒号墨迹左缘 */
        lv_obj_set_pos(scr->Set_Min_Lb, 1057, 263);     /* 盒宽58,墨迹右缘1110 */
        lv_obj_set_pos(scr->Set_AmPm_Lb, 1116, 277);
        lv_obj_set_pos(scr->Year_Unit_Lb, 169, 277);
        lv_obj_set_pos(scr->Month_Unit_Lb, 314, 277);
        lv_obj_set_pos(scr->Day_Unit_Lb, 510, 277);
        lv_obj_set_pos(scr->Type_Unit_Lb, 778, 277);
    } else {
        lv_obj_set_pos(scr->Set_Year_Lb, 97, 260);      /* 盒宽120,墨迹右缘217 */
        lv_obj_set_pos(scr->Set_Month_Lb, 270, 260);    /* 盒宽70,墨迹右缘340 */
        lv_obj_set_pos(scr->Set_Day_Lb, 424, 260);      /* 盒宽70,墨迹右缘494 */
        lv_obj_set_pos(scr->Set_Time_Type_Lb, 763, 260);/* 盒宽70,墨迹右缘833 */
        lv_obj_set_pos(scr->Set_Hour_Lb, 988, 260);     /* 盒宽58,墨迹右缘1044 */
        lv_obj_set_pos(scr->Set_Colon_Lb, 1046, 260);   /* 冒号墨迹左缘 */
        lv_obj_set_pos(scr->Set_Min_Lb, 1055, 260);     /* 盒宽58,墨迹右缘1110 */
        lv_obj_set_pos(scr->Set_AmPm_Lb, 1114, 276);
        lv_obj_set_pos(scr->Year_Unit_Lb, 216, 276);
        lv_obj_set_pos(scr->Month_Unit_Lb, 343, 276);
        lv_obj_set_pos(scr->Day_Unit_Lb, 497, 276);
        lv_obj_set_pos(scr->Type_Unit_Lb, 831, 276);
    }
    systime_underline_pos(0);   /* 0=Yes 态,下划线隐藏待编辑位选定 */
    systime_time_span_refresh();
    systime_ampm_refresh();

    /* 组:Yes → 下划线;group_create_for_page 统一挂 blink 焦点回调
       (裸 lv_group_add_obj 不挂回调,焦点到 Yes 无人停闪,分钟会一直呼吸) */
    lv_obj_t *st_btns[2] = { scr->Yes_Btn, scr->Underline_Btn };
    if (scr->group == NULL) {
        scr->group = group_create_for_page(st_btns, 2);
    } else {
        lv_group_del(scr->group);
        scr->group = group_create_for_page(st_btns, 2);
    }
    nav_editable_extra_register(scr->Underline_Btn);   /* 两态:下划线=可编辑对象(page_push 已清表,此处补登记) */

    current_group = scr->group;
    if (scr->Yes_Btn) lv_group_focus_obj(scr->Yes_Btn);   /* 进页聚焦确定;会话=编辑态,转旋钮直接进"年"编辑 */
    lang_scr_load_anim(scr->obj, LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
    printf("[systime] jump\n");
}
