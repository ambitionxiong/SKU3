/*
 * nav_count_down.c - 设置页-计时器子页（PAGE_SET_COUNT，独立屏幕）
 *
 * 移植自同事 screen_SET_count_down.c：时:分:秒 下划线编辑 + 圆弧/指针倒计时 +
 * 超时全屏层（5 分钟无操作自动退出）。纯本地功能：不写 SET_Data、不发串口。
 *
 * 与同事版的差异（架构适配）：
 *   - 同事靠外部 Tock_1s 秒节拍对 171ms 定时器去抖；本端直接用 1s lv_timer 等价。
 *   - 同事离开页面不销毁屏幕（后台计时继续）；本端屏幕会被 auto_del 销毁，
 *     故定时器独立于屏幕存活，所有控件访问带 lv_obj_is_valid 守卫，
 *     屏幕销毁时经 DELETE 回调整体置空控件指针。
 *   - 在别的页到期：重建计时页并以 auto_del=false 加载（保留原屏），
 *     超时层退出时恢复原屏；无原屏记录（在本页到期/刚从本页离开）回设置层。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"
#include "nav_internal.h"

typedef struct {
    lv_obj_t *obj;
    lv_obj_t *Title_Lb;
    lv_obj_t *Yes_Btn;
    lv_obj_t *Count_down_Cont;
    lv_obj_t *Count_Down_Pointer_icon;
    lv_obj_t *Count_Down_Arc;
    lv_obj_t *hour_CH_Lb;
    lv_obj_t *min_CH_Lb;
    lv_obj_t *second_CH_Lb;
    lv_obj_t *Clock_Num_Lb;
    lv_obj_t *Hour_Lb;
    lv_obj_t *Min_Lb;
    lv_obj_t *Second_Lb;
    lv_obj_t *Underline_Btn;
    lv_obj_t *Reset_icon_Btn;
    lv_obj_t *Overtime_Cont;
    lv_obj_t *BBGG_Img;
    lv_obj_t *Over_Time_Lb;
    lv_obj_t *Overtime_txt_Lb;
    lv_group_t *group;
} count_down_page_t;

static count_down_page_t s_cd;
static count_down_page_t *count_down_get(ui_manager_t *ui) { (void)ui; return &s_cd; }

/* ---- 倒计时状态（独立于屏幕存活，后台继续计时） ---- */
static uint32_t s_cd_total = 0;       /* 总秒数(圆弧/指针基准) */
static uint32_t s_cd_left = 0;        /* 剩余秒数 */
static uint8_t  s_dhour = 0, s_dmin = 0, s_dsec = 0;   /* 编辑中的时/分/秒 */
static uint8_t  s_set_where = 0;      /* 下划线字段 1=时 2=分 3=秒 */
static uint8_t  s_run = 0;            /* 倒计时进行中 */
static uint8_t  s_overtime = 0;       /* 超时层显示中 */
static uint16_t s_overtime_sec = 1;
static uint8_t  s_reset_flag = 0;     /* Reset 清零后抑制到期弹层 */
static uint8_t  s_get_out_scr = 1;    /* 同事语义:跑过一次倒计时后为 0 */
static lv_obj_t *s_return_scr = NULL; /* 抢屏前所在屏幕(auto_del=false 保留,可恢复) */
static lv_timer_t *s_cd_timer = NULL;

static void count_down_create(ui_manager_t *ui);
void screen_overtime_cont_create(void);

/* 屏幕销毁:删组+控件指针整体置空(定时器仍在跑,靠空指针守卫跳过显示刷新) */
static void count_down_del_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    if (obj != s_cd.obj) return;   /* 子控件删除不处理,只认屏幕本身 */
    if (s_cd.group) { lv_group_del(s_cd.group); }
    memset(&s_cd, 0, sizeof(s_cd));
}

static void count_down_timer_stop(void)
{
    if (s_cd_timer) { lv_timer_del(s_cd_timer); s_cd_timer = NULL; }
}

/* 恢复/刷新运行态可视(圆弧/指针/时钟),控件无效时跳过 */
static void count_down_running_visual(void)
{
    count_down_page_t *scr = count_down_get(&ui_manager);
    if (!scr->obj || !lv_obj_is_valid(scr->obj)) return;
    if (scr->Count_Down_Arc && s_cd_total > 0) {
        lv_arc_set_value(scr->Count_Down_Arc, (int32_t)s_cd_left);
        int32_t angle = (int32_t)((s_cd_total - s_cd_left) * 3600 / s_cd_total);
        lv_image_set_rotation(scr->Count_Down_Pointer_icon, -angle);
    }
    if (scr->Clock_Num_Lb) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
                 (int)(s_cd_left / 3600), (int)((s_cd_left % 3600) / 60), (int)(s_cd_left % 60));
        lv_label_set_text(scr->Clock_Num_Lb, buf);
    }
}

static void count_down_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    count_down_page_t *scr = count_down_get(&ui_manager);

    if (s_cd_left > 0) {
        s_reset_flag = 0;
        s_get_out_scr = 0;
        s_run = 1;
        s_cd_left--;
        count_down_running_visual();
        return;
    }

    if (s_overtime) {
        /* 超时层:5 分钟无操作自动退出 */
        s_overtime_sec++;
        if (s_overtime_sec >= 300) {
            s_overtime_sec = 1;
            s_overtime = 0;
            count_down_timer_stop();
            if (s_return_scr && lv_obj_is_valid(s_return_scr)) {
                lv_obj_t *back = s_return_scr;
                s_return_scr = NULL;
                lv_scr_load_anim(back, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);   /* 删计时屏恢复原屏 */
            } else {
                s_return_scr = NULL;
                page_pop();              /* 弹掉 COUNT,按栈顶重建下层页面 */
                jump_to_screen_set();    /* 回设置覆盖层(同 KEY_SET 入口) */
            }
        } else if (scr->Over_Time_Lb && scr->obj && lv_obj_is_valid(scr->Over_Time_Lb)) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%02d:%02d", (int)((s_overtime_sec % 3600) / 60), (int)(s_overtime_sec % 60));
            lv_label_set_text(scr->Over_Time_Lb, buf);
        }
        return;
    }

    /* 刚归零:本页弹超时层;已离开页面则重建抢屏(auto_del=false 保留原屏) */
    s_run = 0;
    if (s_get_out_scr == 0 && s_reset_flag == 0) {
        if (scr->obj && lv_obj_is_valid(scr->obj) && scr->obj == lv_scr_act()) {
            s_return_scr = NULL;                      /* 在本页到期 */
            screen_overtime_cont_create();
        } else {
            s_return_scr = lv_scr_act();              /* 在别的页到期:记住原屏 */
            if (!scr->obj || !lv_obj_is_valid(scr->obj))
                count_down_create(&ui_manager);       /* 屏幕已被 auto_del:重建 */
            if (scr->obj) {
                current_group = scr->group;           /* 抢屏后按键由本页消化(超时层分支) */
                lv_scr_load_anim(scr->obj, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);   /* false:保留原屏 */
                screen_overtime_cont_create();
            }
        }
    }
}

/* 超时层创建(全屏,盖住计时页) */
void screen_overtime_cont_create(void)
{
    count_down_page_t *scr = count_down_get(&ui_manager);
    if (!scr->obj || !lv_obj_is_valid(scr->obj)) return;

    s_overtime_sec = 1;
    s_overtime = 1;

    scr->Overtime_Cont = lv_obj_create(scr->obj);
    lv_obj_set_pos(scr->Overtime_Cont, 0, 0);
    lv_obj_set_size(scr->Overtime_Cont, 1280, 480);
    lv_obj_set_scrollbar_mode(scr->Overtime_Cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(scr->Overtime_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->Overtime_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->Overtime_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->Overtime_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->Overtime_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->Overtime_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->Overtime_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->Overtime_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->Overtime_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    scr->BBGG_Img = lv_img_create(scr->Overtime_Cont);
    lv_img_set_src(scr->BBGG_Img, LVGL_IMAGE_PATH(used/Over_Time_Bg.png));
    lv_img_set_pivot(scr->BBGG_Img, 50, 50);
    lv_img_set_angle(scr->BBGG_Img, 0);
    lv_obj_set_style_img_opa(scr->BBGG_Img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->BBGG_Img, 0, 0);

    scr->Over_Time_Lb = lv_label_create(scr->Overtime_Cont);
    lv_label_set_text(scr->Over_Time_Lb, "00:01");
    lv_label_set_long_mode(scr->Over_Time_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->Over_Time_Lb, 393, 246);
    lv_obj_set_size(scr->Over_Time_Lb, 210, 75);
    lv_obj_set_style_text_font(scr->Over_Time_Lb, &c_taiwanpearl_regular_72, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Over_Time_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->Over_Time_Lb, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->Over_Time_Lb, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->Over_Time_Lb, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    scr->Overtime_txt_Lb = lv_label_create(scr->Overtime_Cont);
    lv_label_set_text(scr->Overtime_txt_Lb, tr("计时器超时"));
    lv_label_set_long_mode(scr->Overtime_txt_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->Overtime_txt_Lb, 618, 267);
    lv_obj_set_size(scr->Overtime_txt_Lb, 360, 55);
    lv_obj_set_style_text_font(scr->Overtime_txt_Lb, &c_taiwanpearl_regular_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Overtime_txt_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->Overtime_txt_Lb, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->Overtime_txt_Lb, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->Overtime_txt_Lb, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

/* 超时层退出:恢复抢屏前的屏幕(无记录回设置层)。PRESS/BACK 共用 */
static void overtime_dismiss(void)
{
    s_overtime_sec = 1;
    s_overtime = 0;
    count_down_timer_stop();   /* 已到期,定时器使命结束 */
    if (s_return_scr && lv_obj_is_valid(s_return_scr)) {
        lv_obj_t *back = s_return_scr;
        s_return_scr = NULL;
        lv_obj_clean(lv_scr_act());   /* 清掉计时页内容(含超时层) */
        lv_scr_load_anim(back, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
    } else {
        s_return_scr = NULL;
        page_pop();
        jump_to_screen_set();
    }
}

/* 编辑态可视复位(Reset/启动后数字区淡出共用) */
static void count_down_edit_visual(int running)
{
    count_down_page_t *scr = count_down_get(&ui_manager);
    if (!scr->obj || !lv_obj_is_valid(scr->obj)) return;
    lv_opa_t edit_opa = running ? LV_OPA_30 : LV_OPA_100;
    lv_obj_set_style_text_opa(scr->Clock_Num_Lb, running ? LV_OPA_100 : LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(scr->Count_Down_Pointer_icon, running ? LV_OPA_100 : LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(scr->hour_CH_Lb, edit_opa, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(scr->min_CH_Lb, edit_opa, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(scr->second_CH_Lb, edit_opa, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(scr->Hour_Lb, edit_opa, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(scr->Min_Lb, edit_opa, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(scr->Second_Lb, edit_opa, LV_PART_MAIN | LV_STATE_DEFAULT);
}

/* 编码器处理(CW/CCW/PRESS 由 nav_key.c 按组分流)
 * 拒绝路径(24h 禁改分秒/0 秒启动/超时层只响一声)内部改无效/提示音 */
void encoder_count_down_action(uint8_t key)
{
    count_down_page_t *scr = count_down_get(&ui_manager);
    if (!scr->obj || !lv_obj_is_valid(scr->obj) || !scr->group) return;

    if (key == KEY_ENCODER_CW || key == KEY_ENCODER_CCW) {
        if (s_overtime) {
            g_send.buzzer_req = BUZZER_KEY_VALID;   /* 超时层:只响一声不做事 */
            return;
        }
        lv_obj_t *focused = lv_group_get_focused(scr->group);
        if (focused == scr->Yes_Btn) {
            if (s_cd_left > 0)
                lv_group_focus_obj(scr->Reset_icon_Btn);   /* 运行中:转去 Reset */
            else {
                lv_group_focus_obj(scr->Underline_Btn);    /* 待编辑:转去时长 */
                s_set_where = 1;
            }
        } else if (focused == scr->Underline_Btn) {
            if (s_set_where == 1) {
                s_dhour = (uint8_t)((s_dhour + (key == KEY_ENCODER_CW ? 1 : 24)) % 25);
                if (s_dhour == 24) {   /* 上限 24:00:00,时到 24 强制分秒清零 */
                    s_dmin = s_dsec = 0;
                    lv_label_set_text_fmt(scr->Min_Lb, "%02d", s_dmin);
                    lv_label_set_text_fmt(scr->Second_Lb, "%02d", s_dsec);
                }
                lv_label_set_text_fmt(scr->Hour_Lb, "%02d", s_dhour);
            } else if (s_set_where == 2) {
                if (s_dhour == 24) { g_send.buzzer_req = BUZZER_KEY_INVALID; return; }
                s_dmin = (uint8_t)((s_dmin + (key == KEY_ENCODER_CW ? 1 : 59)) % 60);
                lv_label_set_text_fmt(scr->Min_Lb, "%02d", s_dmin);
            } else if (s_set_where == 3) {
                if (s_dhour == 24) { g_send.buzzer_req = BUZZER_KEY_INVALID; return; }
                s_dsec = (uint8_t)((s_dsec + (key == KEY_ENCODER_CW ? 1 : 59)) % 60);
                lv_label_set_text_fmt(scr->Second_Lb, "%02d", s_dsec);
            }
        } else if (focused == scr->Reset_icon_Btn) {
            lv_group_focus_obj(scr->Yes_Btn);
        }
        return;
    }

    /* ---- PRESS ---- */
    if (s_overtime) {
        overtime_dismiss();
        return;
    }
    lv_obj_t *focused = lv_group_get_focused(scr->group);
    if (focused == scr->Yes_Btn) {
        if (s_run) {
            /* 运行中确认:离开页面,计时器后台继续(定时器不停) */
            page_pop();
            jump_to_screen_set();
            return;
        }
        s_cd_total = s_cd_left = (uint32_t)s_dhour * 3600 + (uint32_t)s_dmin * 60 + s_dsec;
        if (s_cd_total > 0) {
            lv_arc_set_range(scr->Count_Down_Arc, 0, (int32_t)s_cd_total);
            lv_arc_set_value(scr->Count_Down_Arc, (int32_t)s_cd_total);
            count_down_running_visual();
            count_down_edit_visual(1);
        } else {
            g_send.buzzer_req = BUZZER_KEY_INVALID;   /* 0 秒不可启动 */
        }
    } else if (focused == scr->Underline_Btn) {
        /* 下划线在 时→分→秒 间循环,秒后再按回 Yes */
        s_set_where = (uint8_t)((s_set_where % 3) + 1);
        if (s_set_where == 1) lv_obj_set_pos(scr->Underline_Btn, 588, 306);
        else if (s_set_where == 2) lv_obj_set_pos(scr->Underline_Btn, 726, 306);
        else {
            lv_obj_set_pos(scr->Underline_Btn, 862, 306);
            lv_group_focus_obj(scr->Yes_Btn);
        }
    } else if (focused == scr->Reset_icon_Btn) {
        s_reset_flag = 1;
        lv_group_focus_obj(scr->Underline_Btn);
        s_cd_total = s_cd_left = 0;
        s_dhour = s_dmin = s_dsec = 0;
        s_run = 0;
        lv_arc_set_value(scr->Count_Down_Arc, 0);
        lv_image_set_rotation(scr->Count_Down_Pointer_icon, 0);
        lv_label_set_text(scr->Clock_Num_Lb, "00:00:00");
        lv_label_set_text_fmt(scr->Hour_Lb, "00");
        lv_label_set_text_fmt(scr->Min_Lb, "00");
        lv_label_set_text_fmt(scr->Second_Lb, "00");
        count_down_edit_visual(0);
    }
}

/* BACK:回设置层(运行中计时器后台继续)。同事版还处理超时层,本端超时层
 * 的退出在 encoder PRESS/定时器自动退出里完成,BACK 与 PRESS 同路径 */
void count_down_back_action(void)
{
    if (s_overtime) {
        overtime_dismiss();
        return;
    }
    if (s_cd_left == 0 && !s_run) count_down_timer_stop();   /* 无后台任务才停表 */
    page_pop();
    jump_to_screen_set();
}

lv_group_t *count_down_page_group(void) { return s_cd.group; }

void jump_to_count_down(void)
{
    /* 后台正在跑:重建页面并恢复运行可视(不重置参数);否则全新编辑态 */
    int restore_run = (s_run && s_cd_left > 0);
    if (!restore_run) {
        s_cd_total = s_cd_left = 0;
        s_dhour = s_dmin = s_dsec = 0;
        s_set_where = 0;
        s_run = 0;
        s_get_out_scr = 1;
        s_return_scr = NULL;
    }
    screen_set_reset();          /* 清设置覆盖层对象/指针 */
    depth--;                     /* 弹掉 PAGE_SCREEN_SET */
    page_push(PAGE_SET_COUNT);
    lv_obj_clean(lv_scr_act());
    count_down_create(&ui_manager);
    if (restore_run) {
        lv_arc_set_range(s_cd.Count_Down_Arc, 0, (int32_t)s_cd_total);
        count_down_running_visual();
        count_down_edit_visual(1);
    }
    current_group = s_cd.group;
    if (s_cd.Yes_Btn) lv_group_focus_obj(s_cd.Yes_Btn);
    lang_scr_load_anim(s_cd.obj, LV_SCR_LOAD_ANIM_NONE, 0, 0, ui_manager.auto_del);
    if (s_cd_timer == NULL)
        s_cd_timer = lv_timer_create(count_down_timer_cb, 1000, NULL);   /* 秒节拍(离开页面仍跑) */
    printf("[count_down] jump (run=%d left=%u)\n", restore_run, (unsigned)s_cd_left);
}

static void count_down_create(ui_manager_t *ui)
{
    count_down_page_t *scr = count_down_get(ui);

    /* Init scr->obj */
    scr->obj = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(scr->obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(scr->obj, lv_color_hex(0xfcfcfc), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->obj, LVGL_IMAGE_PATH(used/bg1.jpg), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(scr->obj, count_down_del_cb, LV_EVENT_DELETE, NULL);

    /* Init scr->Title_Lb */
    scr->Title_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Title_Lb, tr("计时器"));
    lv_label_set_long_mode(scr->Title_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->Title_Lb, 24, 24);
    lv_obj_set_size(scr->Title_Lb, 110, 32);
    lv_obj_set_style_text_font(scr->Title_Lb, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Title_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    /* Init scr->Yes_Btn */
    scr->Yes_Btn = lv_btn_create(scr->obj);
    lv_obj_t *Yes_Btn_label = lv_label_create(scr->Yes_Btn);
    lv_label_set_text(Yes_Btn_label, tr("确 定"));
    lv_obj_set_pos(scr->Yes_Btn, 976, 22);
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

    /* Init scr->Count_down_Cont */
    scr->Count_down_Cont = lv_obj_create(scr->obj);
    lv_obj_set_pos(scr->Count_down_Cont, 252, 173);
    lv_obj_set_size(scr->Count_down_Cont, 162, 162);
    lv_obj_set_scrollbar_mode(scr->Count_down_Cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(scr->Count_down_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->Count_down_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->Count_down_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->Count_down_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->Count_down_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->Count_down_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->Count_down_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->Count_down_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->Count_down_Cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* Init scr->Count_Down_Pointer_icon */
    scr->Count_Down_Pointer_icon = lv_img_create(scr->Count_down_Cont);
    lv_image_set_src(scr->Count_Down_Pointer_icon, LVGL_IMAGE_PATH(used/Pointer_icon.png));
    lv_image_set_pivot(scr->Count_Down_Pointer_icon, 9, 80);
    lv_image_set_rotation(scr->Count_Down_Pointer_icon, 0);
    lv_obj_set_style_image_opa(scr->Count_Down_Pointer_icon, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->Count_Down_Pointer_icon, 71, 0);

    /* Init scr->Count_Down_Arc */
    scr->Count_Down_Arc = lv_arc_create(scr->Count_down_Cont);
    lv_arc_set_mode(scr->Count_Down_Arc, LV_ARC_MODE_NORMAL);
    lv_arc_set_range(scr->Count_Down_Arc, 0, 100);
    lv_arc_set_angles(scr->Count_Down_Arc, 0, 0);
    lv_arc_set_rotation(scr->Count_Down_Arc, 270);
    lv_obj_set_pos(scr->Count_Down_Arc, 0, 0);
    lv_obj_set_size(scr->Count_Down_Arc, 162, 162);
    lv_obj_set_style_shadow_opa(scr->Count_Down_Arc, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(scr->Count_Down_Arc, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(scr->Count_Down_Arc, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_color(scr->Count_Down_Arc, lv_color_hex(0xffffff), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(scr->Count_Down_Arc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(scr->Count_Down_Arc, false, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(scr->Count_Down_Arc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    /* 时/分/秒 单位标签 */
    scr->hour_CH_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->hour_CH_Lb, tr("小时"));
    lv_label_set_long_mode(scr->hour_CH_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->hour_CH_Lb, 659, 281);
    lv_obj_set_size(scr->hour_CH_Lb, 40, 35);
    lv_obj_set_style_text_font(scr->hour_CH_Lb, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->hour_CH_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    scr->min_CH_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->min_CH_Lb, tr("分钟"));
    lv_label_set_long_mode(scr->min_CH_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->min_CH_Lb, 796, 281);
    lv_obj_set_size(scr->min_CH_Lb, 40, 35);
    lv_obj_set_style_text_font(scr->min_CH_Lb, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->min_CH_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    scr->second_CH_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->second_CH_Lb, tr("秒"));
    lv_label_set_long_mode(scr->second_CH_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->second_CH_Lb, 933, 281);
    lv_obj_set_size(scr->second_CH_Lb, 40, 35);
    lv_obj_set_style_text_font(scr->second_CH_Lb, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->second_CH_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    /* Init scr->Clock_Num_Lb */
    scr->Clock_Num_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Clock_Num_Lb, "00:00:00");
    lv_label_set_long_mode(scr->Clock_Num_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->Clock_Num_Lb, 173, 356);
    lv_obj_set_size(scr->Clock_Num_Lb, 315, 65);
    lv_obj_set_style_text_font(scr->Clock_Num_Lb, &c_taiwanpearl_regular_60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Clock_Num_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(scr->Clock_Num_Lb, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->Clock_Num_Lb, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(scr->Clock_Num_Lb, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->Clock_Num_Lb, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->Clock_Num_Lb, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* Init 时/分/秒 数字标签 */
    scr->Hour_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Hour_Lb, "00");
    lv_label_set_long_mode(scr->Hour_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_size(scr->Hour_Lb, 80, 65);
    lv_obj_set_style_text_font(scr->Hour_Lb, &c_taiwanpearl_regular_60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Hour_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->Hour_Lb, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);

    scr->Min_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Min_Lb, "00");
    lv_label_set_long_mode(scr->Min_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_size(scr->Min_Lb, 80, 65);
    lv_obj_set_style_text_font(scr->Min_Lb, &c_taiwanpearl_regular_60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Min_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->Min_Lb, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);

    scr->Second_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->Second_Lb, "00");
    lv_label_set_long_mode(scr->Second_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_size(scr->Second_Lb, 80, 65);
    lv_obj_set_style_text_font(scr->Second_Lb, &c_taiwanpearl_regular_60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->Second_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->Second_Lb, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* Init scr->Underline_Btn */
    scr->Underline_Btn = lv_btn_create(scr->obj);
    lv_obj_t *Underline_Btn_label = lv_label_create(scr->Underline_Btn);
    lv_label_set_text(Underline_Btn_label, "");
    lv_obj_align(Underline_Btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->Underline_Btn, 588, 306);
    lv_obj_set_size(scr->Underline_Btn, 102, 30);
    lv_obj_set_style_bg_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->Underline_Btn, LVGL_IMAGE_PATH(used/HowWellDone_underline.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->Underline_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    /* Init scr->Reset_icon_Btn */
    scr->Reset_icon_Btn = lv_btn_create(scr->obj);
    lv_obj_t *Reset_icon_Btn_label = lv_label_create(scr->Reset_icon_Btn);
    lv_label_set_text(Reset_icon_Btn_label, "");
    lv_obj_align(Reset_icon_Btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->Reset_icon_Btn, 1024, 261);
    lv_obj_set_size(scr->Reset_icon_Btn, 52, 52);
    lv_obj_set_style_bg_opa(scr->Reset_icon_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->Reset_icon_Btn, LVGL_IMAGE_PATH(used/count_down_reset_icon_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->Reset_icon_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->Reset_icon_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->Reset_icon_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->Reset_icon_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->Reset_icon_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->Reset_icon_Btn, LVGL_IMAGE_PATH(used/count_down_reset_icon_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->Reset_icon_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->Reset_icon_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    /* 英文排版 */
    if (is_english()) {
        lv_obj_align(Yes_Btn_label, LV_ALIGN_CENTER, 0, 3);
        lv_obj_set_pos(scr->Hour_Lb, 578, 259);
        lv_obj_set_pos(scr->Min_Lb, 715, 259);
        lv_obj_set_pos(scr->Second_Lb, 851, 259);
    } else {
        lv_obj_align(Yes_Btn_label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_pos(scr->Hour_Lb, 578, 255);
        lv_obj_set_pos(scr->Min_Lb, 715, 255);
        lv_obj_set_pos(scr->Second_Lb, 851, 255);
    }

    /* 组:Yes → 下划线 → Reset */
    if (scr->group == NULL) {
        scr->group = lv_group_create();
    } else {
        lv_group_del(scr->group);
        scr->group = lv_group_create();
    }
    lv_group_add_obj(scr->group, scr->Yes_Btn);
    lv_group_add_obj(scr->group, scr->Underline_Btn);
    lv_group_add_obj(scr->group, scr->Reset_icon_Btn);
}
