/*
 * nav_screen_set.c - 设置页(覆盖层)
 * KEY11 进入的覆盖层设置页(不挂页面栈)。
 * 运行态进入:仅前3个按钮可聚焦+work背景图;非运行态:16按钮。
 * 返回:运行态回原页面,非运行态回待机页。功能键入口防御/跳转在此判定。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"
#include "nav_internal.h"
#include "screen_SET.h"

/* ==============================
 * 设置页（覆盖层）
 * 任何状态（含烹饪中）可按键进入；下层页面/计时器原样保留；
 * BACK 仅弹栈 + 删除覆盖层，恢复原页面焦点组。
 * ============================== */

/* 机器设置数据(同事 SET_Data,custom_defs.h extern 声明,工程内唯一实例,默认值同同事) */
volatile Setting_Data SET_Data = {
    .Set_Lock = 0, .Set_KeepWarm = 0, .Set_FanCooling = 0, .Set_6th = 0,
    .Set_Light = 0, .Set_TempUnit = 0, .Set_VolumeFlag = 1, .Set_VolumeHintTime = 0,
    .Set_VolumeKey = 7, .Set_VolumeWelcome = 1, .Set_Brightness = 7,
    .Set_TimeType = 0, .Set_StandbyTime = 0, .Set_Language = 2, .Set_Power = 0,
    .Set_DemoMode = 0,
};

lv_group_t *g_screen_set = NULL;      /* 设置页焦点组 */
static lv_group_t *s_prev_group = NULL;  /* 进入前焦点组 */
static uint8_t s_was_running = 0;        /* 进入设置页前是否运行态(烹饪/暂停/完成/预约) */

/* 覆盖层对象被任意 lv_obj_clean 销毁时同步置空指针(防悬空,同 nav_hint.c hint_del_cb。
 * 否则下次 jump_to_screen_set 的防御性 lv_obj_del 是对已释放内存的二次释放) */
static void screen_set_del_cb(lv_event_t *e)
{
    if (lv_event_get_target(e) == screen_SET.obj) screen_SET.obj = NULL;
}

/* ===== 选项弹窗(移植同事 SET_Select_N2/N4:风扇二选一/炉灯四选一/功率二选一) =====
 * 全屏半透明遮罩 + lock_select_bg 面板 + 标题 + 选项行(lock_select01/02 图标标示选中);
 * 弹窗激活时按键全部由弹窗消化:编码器切选项、PRESS 确认、BACK 取消 */
#define SEL_WHERE_FAN    1
#define SEL_WHERE_LIGHT  2
#define SEL_WHERE_POWER  3
#define SEL_WHERE_TS     4
#define SEL_WHERE_DJ     5
#define SEL_WHERE_DEMO   6
#define SEL_WHERE_LANG   7
static lv_obj_t *s_sel_cont = NULL;   /* 全屏遮罩容器(filter_Cont) */
static lv_obj_t *s_sel_icons[4];     /* 选项选中图标(02=选中) */
static lv_obj_t *s_sel_labels[4];    /* 选项文本 */
static int s_sel_where = 0;          /* 0=关闭 1=风扇 2=炉灯 3=功率 */
static int s_sel_flag = 0;           /* 弹窗内临时选择 */
static int s_sel_n = 2;              /* 选项数 */
static uint8_t s_sel_notrans = 0;    /* 选项文本不过翻译表(语言弹窗:语言名保持原文) */
/* 语言弹窗选项/YY_Lb 回显名(下标与 SET_Data.Set_Language 0英语 1繁体 2简体 对应) */
static const char *const s_lang_names[3] = { "English", "繁體中文", "简体中文" };
/* YY_Lb 显示名:English 在 125 宽值标签里按设计稿折成 Eng-/lish 两行(与弹窗选项分开) */
static const char *const s_lang_yy_names[3] = { "Eng-\nlish", "繁體中文", "简体中文" };

static void sel_icons_refresh(void)
{
    for (int i = 0; i < s_sel_n; i++) {
        lv_obj_t *icon = s_sel_icons[i];
        if (!icon) continue;
        /* LVGL_IMAGE_PATH 是编译期字符串化宏,不能塞三元表达式,按分支展开(同同事写法) */
        if (i == s_sel_flag)
            lv_img_set_src(icon, LVGL_IMAGE_PATH(used/lock_select02.png));
        else
            lv_img_set_src(icon, LVGL_IMAGE_PATH(used/lock_select01.png));
    }
}

static void sel_popup_close(void)
{
    if (s_sel_cont) { lv_obj_del(s_sel_cont); s_sel_cont = NULL; }
    for (int i = 0; i < 4; i++) { s_sel_icons[i] = NULL; s_sel_labels[i] = NULL; }
    s_sel_where = 0;
}

/* ---- 每个弹窗的标准几何(照搬同事 SET_Select_N2/N4_Cont_Create 的精确坐标) ---- */
typedef struct {
    int px, py, pw, ph;             /* 面板位置尺寸 */
    const char *bg;                 /* 面板背景图文件名(assets/image/used/ 下) */
    int tx[4], ty[4];               /* 选项文本位置(尺寸 200x32,L3 450x32) */
    int ix[4], iy[4];               /* 选中图标位置(pivot 中心) */
} sel_layout_t;

/* N2:lock_select_bg 面板(390,120)501x241;文本(51,105)/(50,166);图标(429,109)/(430,170) */
static const sel_layout_t s_sel_n2 = {
    390, 120, 501, 241, "used/lock_select_bg.png",
    { 51, 50, 0, 0 }, { 105, 166, 0, 0 },
    { 429, 430, 0, 0 }, { 109, 170, 0, 0 },
};
/* N4:Set_4seletc_bg_IMG 面板(390,87)501x361;文本(50,105)/(50,166)/(50,226)/(50,286);
 * 图标(429,110)/(430,171)/(429,231)/(430,291) */
static const sel_layout_t s_sel_n4 = {
    390, 87, 501, 361, "used/Set_4seletc_bg_IMG.png",
    { 50, 50, 50, 50 }, { 105, 166, 226, 286 },
    { 429, 430, 429, 430 }, { 110, 171, 231, 291 },
};
/* N3:Set_3seletc_bg_IMG 面板(390,111)501x315;文本(50,111)/(50,171)/(50,231)200x32;
 * 图标(429,115)/(429,176)/(429,236) */
static const sel_layout_t s_sel_n3 = {
    390, 111, 501, 315, "used/Set_3seletc_bg_IMG.png",
    { 50, 50, 50 }, { 111, 171, 231 },
    { 429, 429, 429 }, { 115, 176, 236 },
};

static void sel_popup_create(int where, int flag, int n, const char *title_text, const char *const *opts)
{
    screen_SET_t *ss = screen_SET_get(&ui_manager);
    if (!ss || !ss->obj) return;
    sel_popup_close();

    const sel_layout_t *L = (n >= 4) ? &s_sel_n4 : (n == 3) ? &s_sel_n3 : &s_sel_n2;

    s_sel_cont = lv_obj_create(ss->obj);   /* 全屏遮罩(同同事 filter_Cont) */
    lv_obj_set_pos(s_sel_cont, 0, 0);
    lv_obj_set_size(s_sel_cont, 1280, 480);
    lv_obj_set_scrollbar_mode(s_sel_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(s_sel_cont, lv_color_hex(0x070404), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(s_sel_cont, 115, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(s_sel_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 面板背景:编译期字面量二选一(同事即此写法);运行时 snprintf 拼路径会被
     * 96 字节缓冲截断成 "…assets/image/used" 导致图片加载失败 */
    const char *bg_path = (n >= 4) ? LVGL_IMAGE_PATH(used/Set_4seletc_bg_IMG.png)
                                   : LVGL_IMAGE_PATH(used/lock_select_bg.png);
    lv_obj_t *panel = lv_obj_create(s_sel_cont);
    lv_obj_set_pos(panel, L->px, L->py);
    lv_obj_set_size(panel, L->pw, L->ph);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_img_src(panel, bg_path, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title, title_text);
    lv_obj_set_pos(title, 0, 46);
    lv_obj_set_size(title, 502, 32);
    lv_obj_set_style_text_font(title, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    s_sel_n = n;
    for (int i = 0; i < n; i++) {
        s_sel_icons[i] = lv_img_create(panel);
        lv_img_set_src(s_sel_icons[i], LVGL_IMAGE_PATH(used/lock_select01.png));
        lv_img_set_pivot(s_sel_icons[i], 50, 50);   /* 同事:set_pos 即图标中心 */
        lv_obj_set_pos(s_sel_icons[i], L->ix[i], L->iy[i]);
        s_sel_labels[i] = lv_label_create(panel);
        lv_label_set_text(s_sel_labels[i], s_sel_notrans ? opts[i] : tr(opts[i]));
        lv_obj_set_pos(s_sel_labels[i], L->tx[i], L->ty[i]);
        lv_obj_set_size(s_sel_labels[i], (i == 2 && n == 4) ? 450 : 200, 32);
        lv_obj_set_style_text_font(s_sel_labels[i], &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(s_sel_labels[i], lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    s_sel_flag = flag;
    s_sel_where = where;
    sel_icons_refresh();
}
static void sel_popup_apply(void)
{
    screen_SET_t *ss = screen_SET_get(&ui_manager);
    switch (s_sel_where) {
    case SEL_WHERE_FAN:
        SET_Data.Set_FanCooling = s_sel_flag;
        if (s_sel_flag) MSt_CoolingFanRun_of_TimeControlled; else MSt_CoolingFanRun_of_TempControlled;
        break;
    case SEL_WHERE_LIGHT:
        SET_Data.Set_Light = s_sel_flag;
        switch (s_sel_flag) {
        case 0: MSt_LightState_ON;     break;
        case 1: MSt_LightState_ON_15S; break;
        case 2: MSt_LightState_DOOR;   break;
        default: MSt_LightState_OFF;   break;
        }
        break;
    case SEL_WHERE_POWER:
        SET_Data.Set_Power = s_sel_flag;
        if (s_sel_flag) MSt_Power_16A; else MSt_Power_13A;
        if (ss && ss->Power_Lb) lv_label_set_text(ss->Power_Lb, s_sel_flag ? "16A" : "13A");
        break;
    case SEL_WHERE_TS:
        /* 选项行序 开(row0)/关(row1):row0=开锁(row1 是关) */
        SET_Data.Set_Lock = (s_sel_flag == 0) ? 1 : 0;
        if (ss && ss->TS_Lb) lv_label_set_text(ss->TS_Lb, tr(SET_Data.Set_Lock ? "开" : "关"));
        break;
    case SEL_WHERE_DJ:
        SET_Data.Set_StandbyTime = s_sel_flag;
        break;
    case SEL_WHERE_DEMO:
        SET_Data.Set_DemoMode = s_sel_flag;
        if (ss && ss->Demo_Lb) lv_label_set_text(ss->Demo_Lb, tr(s_sel_flag ? "开" : "关"));
        break;
    case SEL_WHERE_LANG:
        /* s_sel_flag: 0=English 1=繁體 2=简体(与 Set_Language 字段注释一致);繁體暂无词表,显示同简体 */
        SET_Data.Set_Language = s_sel_flag;
        g_lang_en = (s_sel_flag == 0) ? 1 : 0;
        if (g_lang_en) {
            lang_on_page_built();   /* 切英文:树遍历翻译+英文排版 */
        } else {
            /* 切中文:lang_on_page_built 对中文早退,重建覆盖层立即恢复中文布局
               (弹窗是覆盖层子对象一并销毁;reset 清悬挂指针,函数尾部 close 变无害) */
            if (screen_SET.obj) {
                lv_obj_del(screen_SET.obj);
                screen_SET.obj = NULL;
            }
            screen_set_rebuild();
            screen_set_popup_reset();
            if (ss && ss->YY_Btn) lv_group_focus_obj(ss->YY_Btn);   /* 焦点回语言按钮(与切英文行为一致) */
        }
        if (ss && ss->YY_Lb) lv_label_set_text(ss->YY_Lb, tr(s_lang_yy_names[s_sel_flag]));
        break;
    default: break;
    }
    sel_popup_close();
    uart_print();   /* 立即上报新状态帧 */
}

/* 弹窗激活时按键全部由弹窗消化;返回 1=已处理 */
int screen_set_popup_key(uint8_t key)
{
    if (!s_sel_where) return 0;
    switch (key) {
    case KEY_ENCODER_CW:  s_sel_flag = (s_sel_flag + 1) % s_sel_n; sel_icons_refresh(); g_send.buzzer_req = BUZZER_KEY_VALID; break;
    case KEY_ENCODER_CCW: s_sel_flag = (s_sel_flag + s_sel_n - 1) % s_sel_n; sel_icons_refresh(); g_send.buzzer_req = BUZZER_KEY_VALID; break;
    case KEY_ENCODER_PRESS: sel_popup_apply(); break;
    case KEY_BACK: sel_popup_close(); break;
    default: g_send.buzzer_req = BUZZER_KEY_INVALID; break;   /* 其余键无效音,弹窗保持 */
    }
    uart_print();
    return 1;
}

int screen_set_popup_active(void)
{
    return s_sel_where != 0;
}

/* 覆盖层被整屏清理(功能键跳离等)时同步复位弹窗状态(对象随父销毁,只需清指针) */
void screen_set_popup_reset(void)
{
    s_sel_cont = NULL;
    for (int i = 0; i < 4; i++) { s_sel_icons[i] = NULL; s_sel_labels[i] = NULL; }
    s_sel_where = 0;
}

/* ===== 设置覆盖层基础设置项:状态字节即通信帧 BUF[15]/17,PRESS 循环切换 ===== */
static void on_set_zdbw_click(lv_event_t *e)
{
    (void)e;
    int on = !(Machine_Set_num & Send_MachineState_AutoKeepWarm);
    SET_Data.Set_KeepWarm = on;
    if (on) MSt_AutoKeepWarm_EN; else MSt_AutoKeepWarm_UN;
    screen_SET_t *ss = screen_SET_get(&ui_manager);
    if (ss && ss->ZDBW_Lb) lv_label_set_text(ss->ZDBW_Lb, tr(on ? "开" : "关"));
    uart_print();   /* 立即上报新状态帧 */
}

static void on_set_tflqyx_click(lv_event_t *e)
{
    (void)e;
    int flag = (Machine_Set_num & Send_MachineState_CoolingFanRun) ? 1 : 0;   /* 0=温度控制(BIT清) 1=时间控制(BIT置),与选项行序一致 */
    static const char *const opts[2] = { "温度控制", "时间控制" };
    sel_popup_create(SEL_WHERE_FAN, flag, 2, "通风冷却", opts);
}

static void on_set_ldg_click(lv_event_t *e)
{
    (void)e;
    int flag = (int)((Machine_Set_num >> 4) & 0x03);
    static const char *const opts[4] = { "常亮", "亮15秒", "开门亮", "炉灯关" };
    sel_popup_create(SEL_WHERE_LIGHT, flag, 4, "炉灯", opts);
}

static void on_set_power_click(lv_event_t *e)
{
    (void)e;
    int flag = (Machine_Set_num & Send_MachineState_Power_xxA) ? 1 : 0;   /* 0=13A 1=16A */
    static const char *const opts[2] = { "13A", "16A" };
    sel_popup_create(SEL_WHERE_POWER, flag, 2, "功率", opts);
}

static void on_set_ts_click(lv_event_t *e)
{
    (void)e;
    static const char *const opts[2] = { "开", "关" };
    /* 选项行序 开(row0)/关(row1):选中行 = 1-锁状态(锁开→高亮"开") */
    sel_popup_create(SEL_WHERE_TS, SET_Data.Set_Lock ? 0 : 1, 2, "童锁", opts);
}

static void on_set_six_click(lv_event_t *e)
{
    (void)e;
    SET_Data.Set_6th = !SET_Data.Set_6th;
    screen_SET_t *ss = screen_SET_get(&ui_manager);
    if (ss && ss->Six_Lb) lv_label_set_text(ss->Six_Lb, tr(SET_Data.Set_6th ? "含猪肉" : "全部"));
}

static void on_set_wddw_click(lv_event_t *e)
{
    (void)e;
    SET_Data.Set_TempUnit = !SET_Data.Set_TempUnit;
    screen_SET_t *ss = screen_SET_get(&ui_manager);
    if (ss && ss->WDDW_Lb) lv_label_set_text(ss->WDDW_Lb, tr(SET_Data.Set_TempUnit ? "°F" : "°C"));
}

static void on_set_djtime_click(lv_event_t *e)
{
    (void)e;
    static const char *const opts[3] = { "开", "关", "夜间模式" };
    sel_popup_create(SEL_WHERE_DJ, SET_Data.Set_StandbyTime, 3, "待机显示", opts);
}

/* 语言按钮:弹出语言三选项弹窗(English/繁體中文/简体中文),语言名保持原文不过翻译表 */
static void on_set_yy_click(lv_event_t *e)
{
    (void)e;
    s_sel_notrans = 1;
    sel_popup_create(SEL_WHERE_LANG, SET_Data.Set_Language, 3, tr("语言"), s_lang_names);
    s_sel_notrans = 0;
}

static void on_set_demo_click(lv_event_t *e)
{
    (void)e;
    static const char *const opts[2] = { "关", "开" };
    sel_popup_create(SEL_WHERE_DEMO, SET_Data.Set_DemoMode, 2, "演示模式", opts);
}
/* SYSZ 声音设置行：进入声音设置子页（nav_loudness.c） */
static void on_set_sysz_click(lv_event_t *e)
{
    (void)e;
    jump_to_loudness();
}

void screen_set_rebuild(void)
{
    /* 防重入：覆盖层对象仍存活（jump 重复进入）。从子页（声音设置等）返回时
     * 栈顶同样是 PAGE_SCREEN_SET，但跳转子页时对象已被 screen_set_reset 清掉，
     * 需正常重建——故按对象判断而非栈顶页 */
    if (screen_SET.obj) return;
    /* 记录进入前是否运行态:返回时区分回原页面(运行中)还是回待机页(非运行) */
    s_was_running = (g_send.iface_status == IFACE_COOKING ||
                     g_send.iface_status == IFACE_PAUSE ||
                     g_send.iface_status == IFACE_COMPLETE ||
                     g_send.iface_status == IFACE_DELAY_RESERVE);
    /* 不 edit_clear:下层页面在覆盖层之下仍存活,其编辑注册表需保留
     * (退出覆盖层后编码器才能继续调参);功能键跳离时的残表由
     * find_edit_field 的 lv_obj_is_valid 校验兜底 */

    /* 覆盖层对象清理（上次返回未删净的防御） */
    if (screen_SET.obj) {
        lv_obj_del(screen_SET.obj);
        screen_SET.obj = NULL;
    }
    screen_SET_create(&ui_manager);

    screen_SET_t *ss = screen_SET_get(&ui_manager);
    if (!ss || !ss->obj) return;
    lv_obj_add_event_cb(ss->obj, screen_set_del_cb, LV_EVENT_DELETE, NULL);

    /* 全屏覆盖层（create 已直接挂当前屏幕,下层页面对象原样保留） */
    lv_obj_set_pos(ss->obj, 0, 0);
    lv_obj_set_size(ss->obj, 1280, 480);
    lv_obj_set_scrollbar_mode(ss->obj, LV_SCROLLBAR_MODE_OFF);

    /* 右上角时间由常驻 topflag 提供，隐藏页面自带时间标签 */
    if (ss->Time_Lb) lv_obj_add_flag(ss->Time_Lb, LV_OBJ_FLAG_HIDDEN);

    /* 焦点组:运行态只留前 3 个按钮(DSQ/TS/ZDBW),非运行态 16 个;
       未入组按钮仍显示但不可聚焦 */
    lv_obj_t *all_btns[] = {
        ss->DSQ_Btn, ss->TS_Btn, ss->ZDBW_Btn, ss->TFLQYX_Btn,
        ss->Six_Btn, ss->LDG_Btn, ss->WDDW_Btn, ss->SYSZ_Btn,
        ss->LDXS_Btn, ss->Time_Btn, ss->DJ_Time_Btn, ss->YY_Btn,
        ss->Power_Btn, ss->Demo_Btn, ss->RESET_Btn, ss->GYJQ_Btn,
    };
    const int n_all = (int)(sizeof(all_btns) / sizeof(all_btns[0]));

    /* ① 先移除默认组(此时按钮还未加入 g_screen_set,
       lv_group_remove_obj 单参数=从所有组移除,若后执行会把按钮从本组也删掉) */
    for (int k = 0; k < n_all; k++)
        lv_group_remove_obj(all_btns[k]);

    /* ② 建组:运行态前 3 个,非运行态全部 */
    lv_obj_t **btns = all_btns;
    int n = n_all;
    if (s_was_running) {
        n = 3;
        /* 运行态:文字框换 set_work_bg_txt */
        lv_img_set_src(ss->TXT_Img, LVGL_IMAGE_PATH(set_work_bg_txt.png));
    }
    if (g_screen_set) lv_group_del(g_screen_set);
    g_screen_set = group_create_for_page(btns, n);
    clear_focus_states(btns, n);


    /* 基础设置项:状态字节 → 标签回显 + PRESS 切换回调(运行态 3 键组仅含 ZDBW) */
    if (ss->ZDBW_Lb) lv_label_set_text(ss->ZDBW_Lb, tr((Machine_Set_num & Send_MachineState_AutoKeepWarm) ? "关" : "开"));
    if (ss->Power_Lb) lv_label_set_text(ss->Power_Lb, (Machine_Set_num & Send_MachineState_Power_xxA) ? "16A" : "13A");
    if (ss->TS_Lb) lv_label_set_text(ss->TS_Lb, tr(SET_Data.Set_Lock ? "开" : "关"));
    if (ss->Six_Lb) lv_label_set_text(ss->Six_Lb, tr(SET_Data.Set_6th ? "含猪肉" : "全部"));
    if (ss->WDDW_Lb) lv_label_set_text(ss->WDDW_Lb, tr(SET_Data.Set_TempUnit ? "°F" : "°C"));
    if (ss->Demo_Lb) lv_label_set_text(ss->Demo_Lb, tr(SET_Data.Set_DemoMode ? "开" : "关"));
    screen_set_yy_lb_sync();   /* YY_Lb 语言值回显(按 Set_Language 显示语言原名) */
    lv_obj_add_event_cb(ss->ZDBW_Btn, on_set_zdbw_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ss->TFLQYX_Btn, on_set_tflqyx_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ss->LDG_Btn, on_set_ldg_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ss->Power_Btn, on_set_power_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ss->TS_Btn, on_set_ts_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ss->Six_Btn, on_set_six_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ss->WDDW_Btn, on_set_wddw_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ss->DJ_Time_Btn, on_set_djtime_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ss->Demo_Btn, on_set_demo_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ss->SYSZ_Btn, on_set_sysz_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ss->YY_Btn, on_set_yy_click, LV_EVENT_CLICKED, NULL);   /* 语言设置弹窗 */
    /* ③ 焦点 */
    s_prev_group = current_group;
    current_group = g_screen_set;
    lv_group_focus_obj(btns[0]);

}

/* 运行态判定:进入设置页前是否烹饪/暂停/完成/预约(英文排版底图分支用) */
int screen_set_was_running(void)
{
    return s_was_running;
}

/* YY_Lb 语言值回显:按 Set_Language 显示语言原名(tr:EN 模式下"简体中文"译为 Simplified Chinese) */
void screen_set_yy_lb_sync(void)
{
    screen_SET_t *ss = screen_SET_get(&ui_manager);
    int lang = SET_Data.Set_Language;
    if (lang < 0 || lang > 2) lang = 2;
    if (ss && ss->YY_Lb) lv_label_set_text(ss->YY_Lb, tr(s_lang_yy_names[lang]));
}

void jump_to_screen_set(void)
{
    screen_set_rebuild();   /* 含防重入/运行态记录/覆盖层构建 */
    page_push(PAGE_SCREEN_SET);
    lang_on_page_built();   /* 覆盖层挂当前屏、无 screen load:进页补跑翻译+英文排版 */
    printf("[screen_set] enter (overlay, running=%d)\n", s_was_running);
}

void screen_set_back(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SCREEN_SET) return;

    /* 删除覆盖层 */
    if (screen_SET.obj) {
        lv_obj_del(screen_SET.obj);
        screen_SET.obj = NULL;
    }
    if (g_screen_set) {
        lv_group_del(g_screen_set);
        g_screen_set = NULL;
    }

    if (s_was_running) {
        /* 运行态(烹饪/暂停/完成/预约):回原页面,下层原样保留 */
        current_group = s_prev_group;
        s_prev_group = NULL;
        depth--;   /* 仅弹栈，不重建（下层页面未被销毁） */
        /* 覆盖期间下层运行页显示未刷新,立即补刷一次,避免返回时短暂显示旧倒计时 */
        cooking_timer_cb(NULL);
        printf("[screen_set] back (restore prev)\n");
    } else {
        /* 非运行态:回待机页 */
        depth = 0;
        page_push(PAGE_WAITMENU_24);
        lv_obj_clean(lv_scr_act());
        waitmenu_24_create(&ui_manager);
        waitmenu_clock_cache_reset();   /* 强制刷新为真实时间 */
        current_group = NULL;
        lang_scr_load_anim(waitmenu_24_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0,
                         ui_manager.auto_del);
        waitmenu_apply_clock();   /* 立即刷新为真实时间 */
        g_send.iface_status = IFACE_STANDBY;
        printf("[screen_set] back -> waitmenu_24\n");
    }
}

// 外部路径（长按关机/探针插入/功能键弹栈等重置页面）调用：清理覆盖层对象/焦点指针,防悬空 UAF
// 注意:保留 g_screen_set 组不删——current_group 可能仍指向它,删组会悬空;
//       对象删除后组自动清空(空组安全),下次 jump_to_screen_set 会 lv_group_del 清理
void screen_set_reset(void)
{
    if (screen_SET.obj) {
        lv_obj_del(screen_SET.obj);
        screen_SET.obj = NULL;
    }
    s_prev_group = NULL;
    screen_set_popup_reset();   /* 覆盖层销毁时弹窗对象随之消亡,复位状态 */
}
