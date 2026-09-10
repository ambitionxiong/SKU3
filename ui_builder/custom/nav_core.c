/*
 * nav_core.c - 导航核心：全局状态 + 基础工具
 *
 * 职责：
 *   1. 全局变量定义（页面栈、温度/时间设置值、各页面焦点组、运行状态标志）
 *   2. 可编辑字段注册表（edit_*：支持编码器加减数值的页面字段）
 *   3. 公共 UI 工具（时间/进度/开关/焦点态设置）
 *   4. 页面栈操作 page_push（跳转子页前入栈）
 *   5. 焦点组创建 groups_create（主菜单焦点组，开机/返回时重建）
 *
 * 页面栈约定：
 *   page_stack[0] = 根页(WAITMENU)，depth-1 = 当前页；
 *   跳转 = page_push + 重建 UI + 切 current_group；返回 = page_pop。
 */

#include "nav.h"
#include "nav_lang.h"
#include "nav_internal.h"
#include <stdarg.h>

// === 页面栈 ===
#define MAX_STACK 16
page_id_t page_stack[MAX_STACK];  // 栈数组，stack[0]=根页，stack[depth-1]=当前页
int depth = 0;                     // 栈高度，depth=1 时只有根页

int set_temp_up = 180;
int set_temp_down = 180;
int cook_bar_saved = 0;
int probe_target_temp = 80;
int preheat_start_cavity = 0;
int preheat_wait_door = 0;
int g_complete_to_stop_back = 0;
int g_cooling_to_stop_back = 0;
int g_extra_color_to_stop_back = 0;
int g_color_from_probe = 0;   // 进入额外上色时是否为探针模式（color 返回时跳探针主菜单）

#ifdef LV_USE_AIC_SIMULATOR

void uart_print(void)
{
    uart_send_fill();
    printf("[UART]");
    for (int i = 0; i < 24; i++) printf(" %02X", uart_data_send[i]);
    static const char *st[] = {"stdby","set","cook","pause","done","sleep"};
    static const char *md[] = {"none","","hotwind","updown","top","bottom","pizza2","","hot","","","heatcontain","rising","air","","","","","","","waterclean","","","","","windchange","central","save","preheat","","","","","cook4","frozen","slowcook","corn","","color","","unfrozen","hotclean"};
     printf("\n[UART] decoded: st=%s(%d) mode=%s(%d) flag=%d t=%d t_lo=%d time=%02d:%02d:%02d buz=%d\n",
            st[g_send.iface_status<=5?g_send.iface_status:0], g_send.iface_status,
            md[g_send.cook_mode<=41?g_send.cook_mode:0], g_send.cook_mode,
            g_send.cook_flag,
            g_send.set_temp, g_send.set_temp_lower,
           uart_data_send[SEND_TIME_HOUR],
           uart_data_send[SEND_TIME_MIN],
           uart_data_send[SEND_TIME_SEC],
           uart_data_send[SEND_BUZZER]);
}
#else
/* 真机:必须保持空函数!帧的填充+发送由 custom.c 的 handle_uart_send 定时器
   (uart_send_fill + send_uart_param_array,100ms)统一完成;
   此处绝不能再调 uart_send_fill——那是"填充即消费 buzzer_req"的语义,
   提前消费会把按键音请求清零,导致实机无声(20260902 修过的坑) */
void uart_print(void)
{
}
#endif

// === 各页面焦点组（NULL=未创建）===
lv_group_t *g_major_menu;
lv_group_t *g_cookmenu;
lv_group_t *g_probetip;
lv_group_t *g_major_menu_tz;
lv_group_t *g_cook_menu_tz;
lv_group_t *g_special_menu_tz;
lv_group_t *g_special_menu;
lv_group_t *g_updown_bbq_menu;
lv_group_t *g_updown_bbq_set;
lv_group_t *g_delayset;
lv_group_t *g_delaycooking;
uint8_t g_delay_cancel_to_stop_back;
uint8_t g_delay_cancel_btn;         /* 仅 delaycooking 点"取消"按钮进入的 stop_back（区别于 BACK 键） */
uint8_t g_keepwarm_active;
int g_keepwarm_sec;
page_id_t g_delay_source_page = PAGE_WAITMENU_24;
int64_t g_delay_target = -1;   /* -1=无有效预约目标（哨兵，到点检测不触发） */
lv_group_t *g_updown_bbq_cooking;
lv_group_t *g_updown_bbq_complete;

lv_group_t *g_updown_bbq_menu_top;
lv_group_t *g_updown_bbq_menu_low;
lv_group_t *g_updown_bbq_menu_probe;
lv_group_t *g_updown_bbq_set_probe;
lv_group_t *g_updown_bbq_cooking_probe;
lv_group_t *g_updown_bbq_stop_probe;
lv_group_t *g_updown_bbq_stop_back_probe;
lv_group_t *g_updown_bbq_complete_probe;
lv_group_t *g_hot_bbq_menu_probe;
lv_group_t *g_hot_bbq_set_probe;
lv_group_t *g_hot_bbq_cooking_probe;
lv_group_t *g_hot_bbq_stop_probe;
lv_group_t *g_hot_bbq_stop_back_probe;
lv_group_t *g_hot_bbq_complete_probe;
lv_group_t *g_bottom_bbq_menu_probe;
lv_group_t *g_bottom_bbq_set_probe;
lv_group_t *g_bottom_bbq_cooking_probe;
lv_group_t *g_bottom_bbq_stop_probe;
lv_group_t *g_bottom_bbq_stop_back_probe;
lv_group_t *g_bottom_bbq_complete_probe;
lv_group_t *g_slowcook_menu_probe;
lv_group_t *g_slowcook_set_probe;
lv_group_t *g_slowcook_cooking_probe;
lv_group_t *g_slowcook_stop_probe;
lv_group_t *g_slowcook_stop_back_probe;
lv_group_t *g_slowcook_complete_probe;
lv_group_t *g_preheat_menu;

lv_group_t *g_preheat_cooking;

lv_group_t *g_preheat_stop;

lv_group_t *g_preheat_stop_back;

lv_group_t *g_preheat_complete;
lv_group_t *g_cook4_menu;
lv_group_t *g_cookie_menu;
lv_group_t *g_cookie_set;
lv_group_t *g_cookie_cooking;
lv_group_t *g_cookie_setting;
lv_group_t *g_cookie_stop;
lv_group_t *g_cookie_stop_back;
lv_group_t *g_cookie_complete;
lv_group_t *g_west_menu;
lv_group_t *g_west_set;
lv_group_t *g_west_cooking;
lv_group_t *g_west_setting;
lv_group_t *g_west_stop;
lv_group_t *g_west_stop_back;
lv_group_t *g_west_complete;
lv_group_t *g_pizza_menu;
lv_group_t *g_pizza_set;
lv_group_t *g_pizza_cooking;
lv_group_t *g_pizza_setting;
lv_group_t *g_pizza_stop;
lv_group_t *g_pizza_stop_back;
lv_group_t *g_pizza_complete;
lv_group_t *g_menu_cook_menu;
lv_group_t *g_menu_cook_set;
lv_group_t *g_menu_cook_cooking;
lv_group_t *g_menu_cook_setting;
lv_group_t *g_menu_cook_stop;
lv_group_t *g_menu_cook_stop_back;
lv_group_t *g_menu_cook_complete;
lv_group_t *g_air_menu;
lv_group_t *g_air_set;
lv_group_t *g_air_cooking;
lv_group_t *g_air_setting;
lv_group_t *g_air_stop;
lv_group_t *g_air_stop_back;
lv_group_t *g_air_complete;
lv_group_t *g_pizza_2_menu;
lv_group_t *g_pizza_2_set;
lv_group_t *g_pizza_2_cooking;
lv_group_t *g_pizza_2_setting;
lv_group_t *g_pizza_2_stop;
lv_group_t *g_pizza_2_stop_back;
lv_group_t *g_pizza_2_complete;
lv_group_t *g_slowcook_menu;
lv_group_t *g_slowcook_set;
lv_group_t *g_slowcook_cooking;
lv_group_t *g_slowcook_setting;
lv_group_t *g_slowcook_stop;
lv_group_t *g_slowcook_stop_back;
lv_group_t *g_slowcook_complete;
lv_group_t *g_unfrozen_menu;
lv_group_t *g_unfrozen_set;
lv_group_t *g_unfrozen_cooking;
lv_group_t *g_unfrozen_setting;
lv_group_t *g_unfrozen_stop;
lv_group_t *g_unfrozen_stop_back;
lv_group_t *g_unfrozen_complete;
lv_group_t *g_rising_menu;
lv_group_t *g_rising_set;
lv_group_t *g_rising_cooking;
lv_group_t *g_rising_setting;
lv_group_t *g_rising_stop;
lv_group_t *g_rising_stop_back;
lv_group_t *g_rising_complete;
lv_group_t *g_corn_menu;
lv_group_t *g_corn_set;
lv_group_t *g_corn_cooking;
lv_group_t *g_corn_setting;
lv_group_t *g_corn_stop;
lv_group_t *g_corn_stop_back;
lv_group_t *g_corn_complete;
lv_group_t *g_heatcontain_menu;
lv_group_t *g_heatcontain_set;
lv_group_t *g_heatcontain_cooking;
lv_group_t *g_heatcontain_setting;
lv_group_t *g_heatcontain_stop;
lv_group_t *g_heatcontain_stop_back;
lv_group_t *g_heatcontain_complete;
lv_group_t *g_frozen_cook;
lv_group_t *g_favorites;
lv_group_t *g_lasagna_menu;
lv_group_t *g_lasagna_set;
lv_group_t *g_lasagna_cooking;
lv_group_t *g_lasagna_setting;
lv_group_t *g_lasagna_stop;
lv_group_t *g_lasagna_stop_back;
lv_group_t *g_lasagna_complete;
lv_group_t *g_strudel_menu;
lv_group_t *g_strudel_set;
lv_group_t *g_strudel_cooking;
lv_group_t *g_strudel_setting;
lv_group_t *g_strudel_stop;
lv_group_t *g_strudel_stop_back;
lv_group_t *g_strudel_complete;
lv_group_t *g_bread_menu;
lv_group_t *g_bread_set;
lv_group_t *g_bread_cooking;
lv_group_t *g_bread_setting;
lv_group_t *g_bread_stop;
lv_group_t *g_bread_stop_back;
lv_group_t *g_bread_complete;
lv_group_t *g_pizza3_menu;
lv_group_t *g_pizza3_set;
lv_group_t *g_pizza3_cooking;
lv_group_t *g_pizza3_setting;
lv_group_t *g_pizza3_stop;
lv_group_t *g_pizza3_stop_back;
lv_group_t *g_pizza3_complete;
lv_group_t *g_chip_menu;
lv_group_t *g_chip_set;
lv_group_t *g_chip_cooking;
lv_group_t *g_chip_setting;
lv_group_t *g_chip_stop;
lv_group_t *g_chip_stop_back;
lv_group_t *g_chip_complete;
lv_group_t *g_custom_menu;
lv_group_t *g_custom_set;
lv_group_t *g_custom_cooking;
lv_group_t *g_custom_setting;
lv_group_t *g_custom_stop;
lv_group_t *g_custom_stop_back;
lv_group_t *g_custom_complete;
lv_group_t *g_clean_menu;
lv_group_t *g_wc_set;
lv_group_t *g_wc_cooking;
lv_group_t *g_wc_stop;
lv_group_t *g_wc_stop_back;
lv_group_t *g_wc_complete;
lv_group_t *g_hotclean_menu;
lv_group_t *g_hcs_set;
lv_group_t *g_hcs_cooking;
lv_group_t *g_hcs_stop;
lv_group_t *g_hcs_stop_back;
lv_group_t *g_hcs_cooling;
lv_group_t *g_hcs_complete;
lv_group_t *g_hcm_set;
lv_group_t *g_hcm_cooking;
lv_group_t *g_hcm_stop;
lv_group_t *g_hcm_stop_back;
lv_group_t *g_hcm_cooling;
lv_group_t *g_hcm_complete;
lv_group_t *g_hch_set;
lv_group_t *g_hch_cooking;
lv_group_t *g_hch_stop;
lv_group_t *g_hch_stop_back;
lv_group_t *g_hch_cooling;
lv_group_t *g_hch_complete;
lv_group_t *g_cook4_menu;

int updown_menu_top_saved, updown_menu_low_saved;

uint8_t g_on_stop_back = 0;
void (*g_stop_back_complete)(void) = NULL;

lv_group_t *g_cookie_menu;

lv_group_t *g_cookie_set;

lv_group_t *g_cookie_cooking;

lv_group_t *g_cookie_setting;

lv_group_t *g_cookie_stop;

lv_group_t *g_cookie_stop_back;

lv_group_t *g_cookie_complete;

lv_group_t *g_west_menu;

lv_group_t *g_west_set;

lv_group_t *g_west_cooking;

lv_group_t *g_west_setting;

lv_group_t *g_west_stop;

lv_group_t *g_west_stop_back;

lv_group_t *g_west_complete;

lv_group_t *g_pizza_menu;

lv_group_t *g_pizza_set;

lv_group_t *g_pizza_cooking;

lv_group_t *g_pizza_setting;

lv_group_t *g_pizza_stop;

lv_group_t *g_pizza_stop_back;

lv_group_t *g_pizza_complete;

lv_group_t *g_menu_cook_menu;

lv_group_t *g_menu_cook_set;

lv_group_t *g_menu_cook_cooking;

lv_group_t *g_menu_cook_setting;

lv_group_t *g_menu_cook_stop;

lv_group_t *g_menu_cook_stop_back;

lv_group_t *g_menu_cook_complete;


#ifdef LV_USE_AIC_SIMULATOR

uint16_t g_sim_cavity_temp = 25;

#endif


uint16_t get_cavity_temp(void)

{

#ifdef LV_USE_AIC_SIMULATOR

    return g_sim_cavity_temp;

#else

    return ((uint16_t)uart_data_receive[Receive_data_QiangTi_Temp_H] << 8)

           | uart_data_receive[Receive_data_QiangTi_Temp_L];

#endif

}
lv_group_t *g_extra_color;
lv_group_t *g_color_cookoing;
lv_group_t *g_color_stop;
lv_group_t *g_color_stop_back;
lv_group_t *g_updown_bbq_setting;
lv_group_t *g_updown_bbq_stop;
lv_group_t *g_updown_bbq_stop_back;

lv_group_t *g_top_bbq_menu;
lv_group_t *g_top_bbq_set;
lv_group_t *g_top_bbq_cooking;
lv_group_t *g_top_bbq_setting;
lv_group_t *g_top_bbq_stop;
lv_group_t *g_top_bbq_stop_back;
lv_group_t *g_top_bbq_complete;

lv_group_t *g_bottom_bbq_menu;
lv_group_t *g_bottom_bbq_set;
lv_group_t *g_bottom_bbq_cooking;
lv_group_t *g_bottom_bbq_setting;
lv_group_t *g_bottom_bbq_stop;
lv_group_t *g_bottom_bbq_stop_back;
lv_group_t *g_bottom_bbq_complete;

lv_group_t *g_hot_bbq_menu;
lv_group_t *g_hot_bbq_set;
lv_group_t *g_hot_bbq_cooking;
lv_group_t *g_hot_bbq_setting;
lv_group_t *g_hot_bbq_stop;
lv_group_t *g_hot_bbq_stop_back;
lv_group_t *g_hot_bbq_complete;

lv_group_t *g_hotwind_bbq_menu;
lv_group_t *g_hotwind_bbq_set;
lv_group_t *g_hotwind_bbq_cooking;
lv_group_t *g_hotwind_bbq_setting;
lv_group_t *g_hotwind_bbq_stop;
lv_group_t *g_hotwind_bbq_stop_back;
lv_group_t *g_hotwind_bbq_complete;

lv_group_t *g_save_bbq_menu;
lv_group_t *g_save_bbq_set;
lv_group_t *g_save_bbq_cooking;
lv_group_t *g_save_bbq_setting;
lv_group_t *g_save_bbq_stop;
lv_group_t *g_save_bbq_stop_back;
lv_group_t *g_save_bbq_complete;

lv_group_t *g_central_bbq_menu;
lv_group_t *g_central_bbq_set;
lv_group_t *g_central_bbq_cooking;
lv_group_t *g_central_bbq_setting;
lv_group_t *g_central_bbq_stop;
lv_group_t *g_central_bbq_stop_back;
lv_group_t *g_central_bbq_complete;

lv_group_t *g_windchange_bbq_menu;
lv_group_t *g_windchange_bbq_set;
lv_group_t *g_windchange_bbq_cooking;
lv_group_t *g_windchange_bbq_setting;
lv_group_t *g_windchange_bbq_stop;
lv_group_t *g_windchange_bbq_stop_back;
lv_group_t *g_windchange_bbq_complete;

lv_group_t *current_group = NULL;  // 当前活跃的焦点组，nav_handle_key 操作的就是这个组
/* updown_bbq setting 页进入时保存原始值，BACK 返回时恢复 */
int updown_setting_saved_temp_up, updown_setting_saved_temp_down;
int updown_setting_saved_hour, updown_setting_saved_min;
// ==============================
// 可编辑字段注册表
// ==============================

#define MAX_EDIT_FIELDS 8
edit_field_t edit_fields[MAX_EDIT_FIELDS];
int edit_count = 0;

/* 清空可编辑字段注册表（切页前调用，防止 find_edit_field 指针复用误判） */
void edit_clear(void)
{
    edit_count = 0;
}

/* ==================== 温度显示单位（℉）====================
 * 内部值/编码器范围/协议恒为摄氏，仅显示层换算。总注见 nav.h */
/* 摄氏 → 显示值：℉ 模式四舍五入(71℃→160℉，截断会丢 0.8)，℃ 模式原样 */
int temp_disp_c(int c)
{
    if (SET_Data.Set_TempUnit != 1) return c;
    return (c * 9 + 2) / 5 + 32;
}
/* 显示值 → 摄氏（遍历兜底回切 ℃ 时用）：86℉→30℃ */
static int temp_inv_c(int f)
{
    if (f < 32) return f;
    return ((f - 32) * 5 + 4) / 9;
}
/* 原地把 buf 内 "紧邻数字+单位符号" 重写为当前显示单位。
   to_f=1: ℃/°C→°F；=0: °F→℃。纯符号(无数字)只换符号；
   已是目标单位的跳过——遍历可重复执行，幂等 */
void ui_temp_rewrite(char *buf, int cap, int to_f)
{
    char *p = buf;
    while (*p) {
        int mark = 0;
        if (strncmp(p, "\xE2\x84\x83", 3) == 0 ||      /* ℃ */
            strncmp(p, "\xC2\xB0" "C", 3) == 0 ||      /* °C */
            strncmp(p, "\xC2\xB0" "F", 3) == 0)        /* °F */
            mark = 3;
        if (!mark) { p++; continue; }
        int cur_f = (p[2] == 'F');
        if (to_f == cur_f) { p += 3; continue; }
        const char *sym = to_f ? "\xC2\xB0" "F" : "\xE2\x84\x83";
        char *d = p;
        while (d > buf && d[-1] >= '0' && d[-1] <= '9') d--;
        if (d == p) {                       /* 纯单位标签(生成层 icon):只换符号 */
            memcpy(d, sym, 3);
            p = d + 3;
            continue;
        }
        int val = 0;
        for (char *q = d; q < p; q++) val = val * 10 + (*q - '0');
        char num[12];
        int nlen = snprintf(num, sizeof(num), "%d", to_f ? temp_disp_c(val) : temp_inv_c(val));
        int delta = nlen - (int)(p - d);
        char *tail = p + 3;                 /* 旧符号(3字节)之后 */
        if (delta != 0) {
            int tlen = (int)strlen(tail) + 1;
            if ((int)(tail - buf) + tlen + delta > cap) return;   /* 放不下:放弃本次重写 */
            memmove(p + 3 + delta, tail, (size_t)tlen);
        }
        memcpy(d, num, (size_t)nlen);
        memcpy(d + nlen, sym, 3);
        p = d + nlen + 3;
    }
}
/* lv_label_set_text_fmt 出口（nav.h 宏重定向到这）：fmt 含 ℃/°C/°F 标记时，
   "紧跟标记的整数参数"按显示单位换算后输出，其余照常格式化（数字换算在此收口，
   各页面写点无需逐处包换算） */
void ui_label_fmt_impl(lv_obj_t *obj, const char *fmt, ...)
{
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    if (fmt && (strstr(fmt, "\xE2\x84\x83") || strstr(fmt, "\xC2\xB0"))) {
        char *o = buf;
        char *oend = buf + (int)sizeof(buf) - 1;
        const char *p = fmt;
        while (*p && o <= oend - 24) {
            if (*p != '%') { *o++ = *p++; continue; }
            const char *spec = p;
            p++;                                    /* 跳过 '%' */
            while (*p && strchr("-+ #0123456789.hl", *p)) p++;
            if (!*p) break;
            char conv = *p++;
            int conv_temp = 0;
            if ((conv == 'd' || conv == 'i') &&
                (strncmp(p, "\xE2\x84\x83", 3) == 0 ||
                 strncmp(p, "\xC2\xB0" "C", 3) == 0 ||
                 strncmp(p, "\xC2\xB0" "F", 3) == 0))
                conv_temp = 1;
            char sp[24];
            size_t slen = (size_t)(p - spec);
            if (slen >= sizeof(sp)) break;
            memcpy(sp, spec, slen);
            sp[slen] = '\0';
            if (conv_temp) {
                int v = temp_disp_c(va_arg(ap, int));
                sp[slen - 1] = 'd';
                o += snprintf(o, (size_t)(oend - o), sp, v);
            } else {
                switch (conv) {
                case 'd': case 'i': o += snprintf(o, (size_t)(oend - o), sp, va_arg(ap, int)); break;
                case 'u': o += snprintf(o, (size_t)(oend - o), sp, va_arg(ap, unsigned int)); break;
                case 's': o += snprintf(o, (size_t)(oend - o), sp, va_arg(ap, const char *)); break;
                case 'c': o += snprintf(o, (size_t)(oend - o), sp, va_arg(ap, int)); break;
                case '%': *o++ = '%'; break;
                default: break;   /* 项目 fmt 未用到其它转换符 */
                }
            }
        }
        *o = '\0';
    } else {
        vsnprintf(buf, sizeof(buf), fmt ? fmt : "", ap);
    }
    va_end(ap);
    lv_label_set_text(obj, buf);
}
/* 遍历兜底：把树里生成层烙死的温度文本（单位标签 "℃"、占位 "180℃"）重写为
   当前单位。页面构建出口（lang_scr_load_anim）与设置页切单位时调用；
   运行时写入已由出口宏/编辑字段实时换算，本遍历幂等可重复跑 */
static lv_obj_tree_walk_res_t tempunit_walk_cb(lv_obj_t *obj, void *user_data)
{
    (void)user_data;
    if (lv_obj_check_type(obj, &lv_label_class)) {
        const char *txt = lv_label_get_text(obj);
        if (txt && (strstr(txt, "\xE2\x84\x83") || strstr(txt, "\xC2\xB0"))) {
            char buf[256];
            strncpy(buf, txt, sizeof(buf) - 1);   /* 标签最长为状态条 ~128 字节，256 够 */
            buf[sizeof(buf) - 1] = '\0';
            ui_temp_rewrite(buf, (int)sizeof(buf), SET_Data.Set_TempUnit == 1);
            int pure = (strcmp(txt, "\xE2\x84\x83") == 0 ||
                        strcmp(txt, "\xC2\xB0" "C") == 0 ||
                        strcmp(txt, "\xC2\xB0" "F") == 0);
            if (strcmp(buf, txt) != 0)
                lv_label_set_text(obj, buf);
            /* 生成层的单位小盒只有 26px 级固定宽(updown set/stepset 等):WRAP 模式下
               "°F" 在 ° 后折行、F 被盒子高度裁没(℃ 单字形无断行点故没事)。
               ℉ 下宽度不够就按实际宽度加宽——生成层标签都是左上角 set_pos 锚定,
               向右长几 px 无副作用;宽盒(如设置页 WDDW_Lb)自动跳过;重复遍历幂等 */
            if (pure && SET_Data.Set_TempUnit == 1) {
                lv_coord_t w = lv_obj_get_style_width(obj, 0);
                int need = lv_txt_get_width("°F", 3, lv_obj_get_style_text_font(obj, 0),
                                            lv_obj_get_style_text_letter_space(obj, 0)) + 2;
                if ((int)w != LV_SIZE_CONTENT && (int)w < need)
                    lv_obj_set_width(obj, (lv_coord_t)need);
            }
        }
    }
    return LV_OBJ_TREE_WALK_NEXT;
}
void nav_tempunit_refresh_screen(lv_obj_t *root)
{
    if (root)
        lv_obj_tree_walk(root, tempunit_walk_cb, NULL);
}

static void blink_group_add(lv_obj_t *trigger, lv_obj_t **objs, int n);   /* 前置:edit_register 自动登记闪烁组 */
/* 注册一个可编辑字段：label 为显示标签，ind_s/ind_l 为温度<100/≥100 的指示条，
   value 指向实际存储值，min/max/step 为循环范围，fmt 为显示格式（"%d"/"%02d"） */
void edit_register(lv_obj_t *label, lv_obj_t *ind_s, lv_obj_t *ind_l,
                   int *value, int min, int max, int step, const char *fmt)
{
    if (edit_count < MAX_EDIT_FIELDS) {
        edit_fields[edit_count].label = label;
        edit_fields[edit_count].ind_short = ind_s;
        edit_fields[edit_count].ind_long = ind_l;
        edit_fields[edit_count].value = value;
        edit_fields[edit_count].min = min;
        edit_fields[edit_count].max = max;
        edit_fields[edit_count].step = step;
        edit_fields[edit_count].fmt = fmt;
        edit_fields[edit_count].is_temp = 0;   /* 槽位复用必须清残留:上页温度字段的
                                                  is_temp=1 若不清,本页时分字段会被
                                                  当温度渲染(edit_register_temp 再置 1) */
        edit_count++;
    }
    /* 自动登记呼吸闪烁组:值 + 长短两根指示线(隐藏的闪了不可见,页面显哪根哪根闪)。
       单位/方向箭头等配套对象由页面在 build 时追加 nav_blink_extra(label, obj) */
    lv_obj_t *grp[3] = { label, ind_s, ind_l };
    blink_group_add(label, grp, 3);
}
/* 温度字段注册：同 edit_register，另置 is_temp——渲染值与长短指示线阈值按
   显示单位(℉)换算，内部值恒摄氏；漏置标记则编码器调温时显示回摄氏 */
void edit_register_temp(lv_obj_t *label, lv_obj_t *ind_s, lv_obj_t *ind_l,
                        int *value, int min, int max, int step, const char *fmt)
{
    edit_register(label, ind_s, ind_l, value, min, max, step, fmt);
    if (edit_count > 0)
        edit_fields[edit_count - 1].is_temp = 1;
}
/* 按 label 查找已注册的可编辑字段（编码器加减时定位字段）。
 * lv_obj_is_valid 校验:残条目(所属页已销毁,如功能键跳离编辑页未清注册)的
 * label 地址失效即跳过并就地置空——从根上杜绝"地址复用误命中→悬空 label
 * 调参"的 UAF 类问题,不依赖每条跳转路径都记得 edit_clear */
edit_field_t *find_edit_field(lv_obj_t *obj)
{
    for (int i = 0; i < edit_count; i++) {
        if (edit_fields[i].label == obj) {
            if (!lv_obj_is_valid(edit_fields[i].label)) {
                edit_fields[i].label = NULL;   /* 失效条目就地失效 */
                continue;
            }
            return &edit_fields[i];
        }
    }
    return NULL;
}
/* 编辑字段渲染值：温度字段按显示单位(℉)，时间字段原样。
   供 adjust_value 与 on_edit_focus(nav_events.c，聚焦瞬间选线)共用 */
int edit_disp(const edit_field_t *f, int v)
{
    return f->is_temp ? temp_disp_c(v) : v;
}
/* 编码器加减：循环调整数值、刷新标签与温度指示线、
   执行各设置页的温差约束(上下≤20℃)与 dir/icon 即时更新 */
void adjust_value(edit_field_t *f, int delta)
{
    int old_val = *f->value;
    int new_val = old_val + f->step * delta;

    /* 循环 */
    if (new_val > f->max) new_val = f->min;
    if (new_val < f->min) new_val = f->max;

    *f->value = new_val;
    lv_label_set_text_fmt(f->label, f->fmt, edit_disp(f, new_val));

    /* 温度线切换(按显示值位数) */
    if (f->ind_short && f->ind_long) {
        lv_obj_add_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
        if (edit_disp(f, new_val) < 100)
            lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
        lv_obj_invalidate(lv_scr_act());
    }

    /* dir 方向图标切换（上下烧烤菜单页） */
    validate_constraints();

    /* 设置页：上下温差 ≤ 20°C，超限则回弹 */
    if (current_group == g_updown_bbq_setting) {
        int diff = set_temp_up - set_temp_down;
        if (diff < 0) diff = -diff;
        updown_bbq_setting_t *set = updown_bbq_setting_get(&ui_manager);
        if (diff > 20) {
            int new_v;
            if (f->value == &set_temp_up)
                new_v = set_temp_up > set_temp_down ? set_temp_down - 20 : set_temp_down + 20;
            else
                new_v = set_temp_down > set_temp_up ? set_temp_up - 20 : set_temp_up + 20;
            if (new_v > 300) new_v = 300;
            if (new_v < 30) new_v = 30;
            *f->value = new_v;
            lv_label_set_text_fmt(f->label, f->fmt, edit_disp(f, new_v));
            if (f->ind_short && f->ind_long) {
                lv_obj_add_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
                if (edit_disp(f, new_v) < 100)
                    lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
            }
        }
        update_setting_dir_icon(set);
    }
    /* 上层温度页：上下温差 ≤ 20°C，超限则循环 */
    if (current_group == g_updown_bbq_menu_top) {
        int diff = set_temp_up - set_temp_down;
        if (diff < 0) diff = -diff;
        if (diff > 20) {
            int new_v = set_temp_up > set_temp_down ? set_temp_down - 20 : set_temp_down + 20;
            if (new_v > 300) new_v = 300;
            if (new_v < 30) new_v = 30;
            *f->value = new_v;
            lv_label_set_text_fmt(f->label, f->fmt, edit_disp(f, new_v));
            if (f->ind_short && f->ind_long) {
                lv_obj_add_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
                if (edit_disp(f, new_v) < 100)
                    lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
            }
            {
                updown_bbq_menu_top_t *m = updown_bbq_menu_top_get(&ui_manager);
                if (m) {
                    if (m->dir3) lv_obj_add_flag(m->dir3, LV_OBJ_FLAG_HIDDEN);
                    if (m->dir2) lv_obj_add_flag(m->dir2, LV_OBJ_FLAG_HIDDEN);
                    if (edit_disp(f, new_v) < 100)
                        if (m->dir2) lv_obj_clear_flag(m->dir2, LV_OBJ_FLAG_HIDDEN);
                    else
                        if (m->dir3) lv_obj_clear_flag(m->dir3, LV_OBJ_FLAG_HIDDEN);
                }
            }
            lv_obj_invalidate(lv_scr_act());
        }
        {
            updown_bbq_menu_top_t *m2 = updown_bbq_menu_top_get(&ui_manager);
            if (m2) {
                if (m2->dir3) lv_obj_add_flag(m2->dir3, LV_OBJ_FLAG_HIDDEN);
                if (m2->dir2) lv_obj_add_flag(m2->dir2, LV_OBJ_FLAG_HIDDEN);
                if (temp_disp_c(set_temp_up) < 100) { if (m2->dir2) lv_obj_clear_flag(m2->dir2, LV_OBJ_FLAG_HIDDEN); }
                else { if (m2->dir3) lv_obj_clear_flag(m2->dir3, LV_OBJ_FLAG_HIDDEN); }
            }
            lv_obj_invalidate(lv_scr_act());
        }
    }
    /* 下层温度页：上下温差 ≤ 20°C，超限则循环 */
    if (current_group == g_updown_bbq_menu_low) {
        int diff = set_temp_up - set_temp_down;
        if (diff < 0) diff = -diff;
        if (diff > 20) {
            int new_v = set_temp_down > set_temp_up ? set_temp_up - 20 : set_temp_up + 20;
            if (new_v > 300) new_v = 300;
            if (new_v < 30) new_v = 30;
            *f->value = new_v;
            lv_label_set_text_fmt(f->label, f->fmt, edit_disp(f, new_v));
            if (f->ind_short && f->ind_long) {
                lv_obj_add_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
                if (edit_disp(f, new_v) < 100)
                    lv_obj_clear_flag(f->ind_short, LV_OBJ_FLAG_HIDDEN);
                else
                    lv_obj_clear_flag(f->ind_long, LV_OBJ_FLAG_HIDDEN);
            }
            {
                updown_bbq_menu_low_t *m = updown_bbq_menu_low_get(&ui_manager);
                if (m) {
                    if (m->dir3) lv_obj_add_flag(m->dir3, LV_OBJ_FLAG_HIDDEN);
                    if (m->dir2) lv_obj_add_flag(m->dir2, LV_OBJ_FLAG_HIDDEN);
                    if (edit_disp(f, new_v) < 100)
                        if (m->dir2) lv_obj_clear_flag(m->dir2, LV_OBJ_FLAG_HIDDEN);
                    else
                        if (m->dir3) lv_obj_clear_flag(m->dir3, LV_OBJ_FLAG_HIDDEN);
                }
            }
            lv_obj_invalidate(lv_scr_act());
        }
        {
            updown_bbq_menu_low_t *m2 = updown_bbq_menu_low_get(&ui_manager);
            if (m2) {
                if (m2->dir3) lv_obj_add_flag(m2->dir3, LV_OBJ_FLAG_HIDDEN);
                if (m2->dir2) lv_obj_add_flag(m2->dir2, LV_OBJ_FLAG_HIDDEN);
                if (temp_disp_c(set_temp_down) < 100) { if (m2->dir2) lv_obj_clear_flag(m2->dir2, LV_OBJ_FLAG_HIDDEN); }
                else { if (m2->dir3) lv_obj_clear_flag(m2->dir3, LV_OBJ_FLAG_HIDDEN); }
            }
            lv_obj_invalidate(lv_scr_act());
        }
    }
    /* 顶部烧烤设置页 dir/icon 即时更新 */
    if (current_group == g_top_bbq_setting) {
        top_bbq_setting_t *set = top_bbq_setting_get(&ui_manager);
        update_top_bbq_dir_icon(set);
    }

    /* 底部烧烤设置页 dir/icon 即时更新 */
    if (current_group == g_bottom_bbq_setting) {
        bottom_bbq_setting_t *set = bottom_bbq_setting_get(&ui_manager);
        update_bottom_bbq_dir_icon(set);
    }

    /* 热风烧烤设置页 dir/icon 即时更新 */
    if (current_group == g_hot_bbq_setting) {
        hot_bbq_setting_t *set = hot_bbq_setting_get(&ui_manager);
        update_hot_bbq_dir_icon(set);
    }
    if (current_group == g_hotwind_bbq_setting) {
        hotwind_bbq_setting_t *set = hotwind_bbq_setting_get(&ui_manager);
        update_hotwind_bbq_dir_icon(set);
    }
    if (current_group == g_save_bbq_setting) {
        save_bbq_setting_t *set = save_bbq_setting_get(&ui_manager);
        update_save_bbq_dir_icon(set);
    }
    if (current_group == g_central_bbq_setting) {
        central_bbq_setting_t *set = central_bbq_setting_get(&ui_manager);
        update_central_bbq_dir_icon(set);
    }
    if (current_group == g_windchange_bbq_setting) {
        windchange_bbq_setting_t *set = windchange_bbq_setting_get(&ui_manager);
        update_windchange_bbq_dir_icon(set);
    }
    if (current_group == g_cookie_setting) {
        cookie_setting_t *set = cookie_setting_get(&ui_manager);
        update_cookie_dir_icon(set);
    }
    if (current_group == g_west_setting) {
        west_setting_t *set = west_setting_get(&ui_manager);
        update_west_dir_icon(set);
    }
    if (current_group == g_pizza_setting) {
        pizza_setting_t *set = pizza_setting_get(&ui_manager);
        update_pizza_dir_icon(set);
    }
    if (current_group == g_menu_cook_setting) {
        menu_setting_t *set = menu_setting_get(&ui_manager);
        update_menu_dir_icon(set);
    }

    /* 空气炸设置页 dir/icon 即时更新 */
    if (current_group == g_air_setting) {
        air_setting_t *set = air_setting_get(&ui_manager);
        update_air_dir_icon(set);
    }

    /* 披萨设置页 dir/icon 即时更新 */
    if (current_group == g_pizza_2_setting) {
        pizza_2_setting_t *set = pizza_2_setting_get(&ui_manager);
        update_pizza_2_dir_icon(set);
    }

    /* 慢煮设置页 dir/icon 即时更新 */
    if (current_group == g_slowcook_setting) {
        slowcook_setting_t *set = slowcook_setting_get(&ui_manager);
        update_slowcook_dir_icon(set);
    }

    /* 解冻设置页 dir/icon 即时更新 */
    if (current_group == g_unfrozen_setting) {
        unfrozen_setting_t *set = unfrozen_setting_get(&ui_manager);
        update_unfrozen_dir_icon(set);
    }

    /* 发酵设置页 dir/icon 即时更新 */
    if (current_group == g_rising_setting) {
        rising_setting_t *set = rising_setting_get(&ui_manager);
        update_rising_dir_icon(set);
    }

    /* 干果设置页 dir/icon 即时更新 */
    if (current_group == g_corn_setting) {
        corn_setting_t *set = corn_setting_get(&ui_manager);
        update_corn_dir_icon(set);
    }

    /* 保温设置页 dir/icon 即时更新 */
    if (current_group == g_heatcontain_setting) {
        heatcontain_setting_t *set = heatcontain_setting_get(&ui_manager);
        update_heatcontain_dir_icon(set);
    }
}
void validate_constraints(void)
{
    /* 找到 minute 字段 */
    edit_field_t *min_field = NULL;
    for (int i = 0; i < edit_count; i++) {
        if (edit_fields[i].value == &set_min) {
            min_field = &edit_fields[i];
            break;
        }
    }
    if (!min_field) return;

    /* 找到 hour 字段，取其 max 作为动态最大小时 */
    int max_h = 4;
    for (int i = 0; i < edit_count; i++) {
        if (edit_fields[i].value == &set_hour) {
            max_h = edit_fields[i].max;
            break;
        }
    }

    /* 根据 hour 动态调整 minute 的循环范围 */
    if (set_hour == 0) {
        min_field->min = 5;    // hour=0时，最少5分钟
        min_field->max = 59;
    } else if (set_hour == max_h) {
        min_field->min = 0;    // 最大小时时 minute 不可调
        min_field->max = 0;
    } else {
        min_field->min = 0;    // 0-59 正常循环
        min_field->max = 59;
    }

    /* 纠正 minute 值（如果当前值超出新范围） */
    if (set_hour == 0 && set_min < 5) {
        set_min = 5;   /* hour=0 时最少 5 分钟(与 min_field->min 一致,对齐 SDK) */
        lv_label_set_text_fmt(min_field->label, min_field->fmt, set_min);
    } else if (set_hour == max_h && set_min != 0) {
        set_min = 0;
        lv_label_set_text_fmt(min_field->label, min_field->fmt, set_min);
    }
}
// ==============================
// 公共 helper
// ==============================
static void set_status_label(lv_obj_t *label, int temp, int hour, int min)
{
    if (hour == 0)
        lv_label_set_text_fmt(label, tr("| 上下烧烤 | %d℃ | %02d分钟"), temp, min);
    else
        lv_label_set_text_fmt(label, tr("| 上下烧烤 | %d℃ | %d小时%02d分钟"), temp, hour, min);
}
void set_status_label_min(lv_obj_t *label, int temp_up, int temp_down, int hour, int min)
{
    int t = temp_up < temp_down ? temp_up : temp_down;
    set_status_label(label, t, hour, min);
}
void set_time_label(lv_obj_t *label, int remaining_ms)
{
    if (remaining_ms < 0) remaining_ms = 0;
    int total_sec = remaining_ms / 1000;
    int h = total_sec / 3600;
    int m = (total_sec % 3600) / 60;
    int s = total_sec % 60;
    lv_label_set_text_fmt(label, "%02d:%02d:%02d", h, m, s);
}
void set_bar_progress(lv_obj_t *bar, int64_t elapsed_ms, int total_ms)
{
    if (total_ms <= 0) return;
    int progress = (int)(elapsed_ms * 100 / total_ms);
    if (progress > 100) progress = 100;
    if (progress < 0) progress = 0;
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, progress, LV_ANIM_OFF);
}
void apply_toggle_state(lv_obj_t *btn_off, lv_obj_t *btn_on, int on)
{
    if (on) {
        lv_obj_add_flag(btn_off, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(btn_on, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(btn_off, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_on, LV_OBJ_FLAG_HIDDEN);
    }
}
// 清除按钮数组的 FOCUSED 状态
void clear_focus_states(lv_obj_t **btns, int count)
{
    for (int i = 0; i < count; i++)
        if (btns[i])
            lv_obj_clear_state(btns[i], LV_STATE_FOCUSED);
}
// updown_bbq_set 温度组件显隐（2 位 / 3 位自动切换）
void setup_set_temp_display(updown_bbq_set_t *set)
{
    lv_obj_add_flag(set->up2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->up2_dir_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->up2_icon_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->down2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->down2_dir_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->down2_icon_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->up3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->up3_dir_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->up3_icon_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->down3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->down3_dir_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(set->down3_icon_label, LV_OBJ_FLAG_HIDDEN);

    if (temp_disp_c(set_temp_up) < 100) {
        lv_label_set_text_fmt(set->up2_tempnum_label, "%d", temp_disp_c(set_temp_up));
        lv_obj_clear_flag(set->up2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->up2_dir_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->up2_icon_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text_fmt(set->up3_tempnum_label, "%d", temp_disp_c(set_temp_up));
        lv_obj_clear_flag(set->up3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->up3_dir_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->up3_icon_label, LV_OBJ_FLAG_HIDDEN);
    }
    if (temp_disp_c(set_temp_down) < 100) {
        lv_label_set_text_fmt(set->down2_tempnum_label, "%d", temp_disp_c(set_temp_down));
        lv_obj_clear_flag(set->down2_tempnum_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->down2_dir_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->down2_icon_label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_label_set_text_fmt(set->down3_tempnum_label, "%d", temp_disp_c(set_temp_down));
        lv_obj_clear_flag(set->down3_tempnum_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->down3_dir_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(set->down3_icon_label, LV_OBJ_FLAG_HIDDEN);
    }
}
// ==============================
// 页面栈操作
// ==============================

// 跳转子页前调用，记录"当前页"到栈顶
void page_push(page_id_t id)
{
    nav_blink_forget();   /* 旧页对象即将销毁:先遗忘闪烁组(防悬空,详见函数注释) */
    if (depth < MAX_STACK) {
        page_stack[depth++] = id;  // 写入栈顶，depth 自增
        printf("[nav] page push: depth=%d id=%d\n", depth, id);
    } else {
        printf("[nav] ERROR: page_stack overflow! depth=%d id=%d\n", depth, id);
    }
    topflag_update_visibility();
}

// ==============================
// 焦点组创建
// ==============================

// 创建 g_major_menu（只在初始化时调一次，返回时也会调）
void groups_create(void)
{
    major_menu_t *major = major_menu_get(&ui_manager);
    if (!major) {
        printf("[nav] major_menu struct is NULL\n");
        return;
    }

    if (g_major_menu) lv_group_del(g_major_menu);
    g_major_menu = lv_group_create();
    if (!g_major_menu) {
        printf("[nav] failed to create major_menu group\n");
        return;
    }

    /* 按焦点顺序依次加入按钮 */
    if (major->cook_button)
        lv_group_add_obj(g_major_menu, major->cook_button);
    else
        printf("[nav] major->cook_button is NULL\n");

    if (major->cook4_button)
        lv_group_add_obj(g_major_menu, major->cook4_button);
    else
        printf("[nav] major->cook4_button is NULL\n");

    if (major->special_button)
        lv_group_add_obj(g_major_menu, major->special_button);
    else
        printf("[nav] major->special_button is NULL\n");

    printf("[nav] major_menu group created\n");
}

/* 设置页"自动保温"(SET_Data.Set_KeepWarm)是总默认值:各模式 set 页进入时
   保温开关(contain_on)初值跟随它;页内仍可手动改,仅本次烹饪会话有效。
   停止/初始化清理路径仍复位为 0,下次进 set 页再取默认 */
int contain_default(void)
{
    return SET_Data.Set_KeepWarm ? 1 : 0;
}

/* ==============================
 * 选中项呼吸闪烁（数值+下划线+配套单位同步渐变显隐）
 * 原理:每个对象一条 lv_anim 写 style opa(255→0 去程+playback 回程,无限重复);
 * 同组对象同一时刻 start、参数一致→相位永远同步。
 * 字段组成来自权威注册(不做任何几何猜测/素材名过滤):
 *   1) edit_register(label, ind_s, ind_l, ...) 自动登记 {label, 长线, 短线}
 *      ——全部设置页(34 文件 305 处)零改动自动生效;
 *   2) 字段另有单位/箭头等配套对象时,build 时调 nav_blink_extra(label, obj) 追加;
 *   3) 非 edit 体系页面(sixset2/toastcolor)调 nav_blink_group_register 显式登记。
 * 隐藏对象闪了不可见,页面显哪套/哪根线,哪套就"在闪"——无需识别位数换套逻辑。
 * 焦点命中 trigger → 整组同步闪;按钮/未登记对象 → 停。
 * 生命周期:页面对象销毁路径(page_push/pop/screen_set_reset)调 nav_blink_forget,
 *   组表与动画注册表一并遗忘(动画随对象销毁自动消亡,不触碰任何指针)。
 * ============================== */
#define BLINK_HALF_MS   700    /* 半程时长:255→去程 / 回程,一个呼吸周期 = 2x */
#define BLINK_MIN_OPA   0      /* 最低透明度(0=完全隐藏;嫌闪得太狠可改 60~100) */
#define BLINK_STEP_OPA  24     /* 呼吸量化步长:量化值不变不写样式不失效,刷屏 ~60Hz→~15Hz(治实机卡顿) */
#define BLINK_MAX_OBJS  8      /* 单组上限:值+长短线+单位+箭头 */
#define BLINK_MAX_GROUPS 48    /* 全页组数上限 */

typedef struct {
    lv_obj_t *trigger;                    /* 焦点落到此对象上→整组闪 */
    lv_obj_t *objs[BLINK_MAX_OBJS];
    int n;
} blink_group_t;

static blink_group_t s_blink_groups[BLINK_MAX_GROUPS];
static int s_blink_group_n = 0;
static lv_obj_t *s_blink_objs[BLINK_MAX_OBJS];
static int s_blink_n = 0;
/* 每对象存档:起闪前的原始透明度(收线按存档恢复,不再硬编码全显);
   last_v 为上次写入的量化值(-1=未写),量化值不变就不写样式不失效——刷屏降 4 倍 */
static int32_t s_blink_opa[BLINK_MAX_OBJS];
static int32_t s_blink_img_opa[BLINK_MAX_OBJS];
static int32_t s_blink_sel_opa[BLINK_MAX_OBJS];
static int32_t s_blink_last_v[BLINK_MAX_OBJS];

static int blink_idx(const void *var)
{
    for (int i = 0; i < s_blink_n; i++)
        if (s_blink_objs[i] == var) return i;
    return -1;
}

static int32_t blink_quantize(int32_t v)
{
    v += BLINK_STEP_OPA / 2;                       /* 四舍五入到台阶,端点 0/255 保留 */
    int32_t q = (v / BLINK_STEP_OPA) * BLINK_STEP_OPA;
    if (q > LV_OPA_COVER) q = LV_OPA_COVER;
    if (q < BLINK_MIN_OPA) q = BLINK_MIN_OPA;
    return q;
}

static void blink_exec_cb(void *var, int32_t v)
{
    int i = blink_idx(var);
    int32_t q = blink_quantize(v);
    if (i < 0 || q == s_blink_last_v[i]) return;   /* 台阶没变:不写样式不触发重绘 */
    s_blink_last_v[i] = q;
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_style_opa(obj, q, 0);
    if (lv_obj_has_class(obj, &lv_image_class))    /* 线是 lv_img:设备上 img_opa 通道才真正呼吸 */
        lv_obj_set_style_img_opa(obj, q, 0);
}

/* roller 专用:呼吸 LV_PART_SELECTED 的 text_opa——roller 选中行在 DRAW_POST 用
   SELECTED part 的 label 描述重绘(lv_roller.c),只闪选中值;整轮 opa 会全部选项一起呼吸 */
static void blink_exec_sel_cb(void *var, int32_t v)
{
    int i = blink_idx(var);
    int32_t q = blink_quantize(v);
    if (i < 0 || q == s_blink_last_v[i]) return;
    s_blink_last_v[i] = q;
    lv_obj_set_style_text_opa((lv_obj_t *)var, q, LV_PART_SELECTED);
}

static void blink_stop(void);
static void blink_obj_add(lv_obj_t *obj);

/* 遗忘全部闪烁注册(组表+动画组)。在页面对象销毁路径的入口调用:
   page_push/page_pop/screen_set_reset——先停活动画(防孤儿动画在对象死后继续刷屏),
   再清注册表;不做任何树遍历/有效性检查 */
void nav_blink_forget(void)
{
    blink_stop();
    s_blink_n = 0;
    s_blink_group_n = 0;
}

static void blink_stop(void)
{
    for (int i = 0; i < s_blink_n; i++) {
        lv_obj_t *obj = s_blink_objs[i];
        if (!obj) continue;
        lv_anim_del(obj, blink_exec_cb);
        lv_anim_del(obj, blink_exec_sel_cb);
        lv_obj_set_style_opa(obj, s_blink_opa[i], 0);                      /* 按存档恢复 */
        if (lv_obj_has_class(obj, &lv_image_class))
            lv_obj_set_style_img_opa(obj, s_blink_img_opa[i], 0);
        lv_obj_set_style_text_opa(obj, s_blink_sel_opa[i], LV_PART_SELECTED);   /* roller 选中行 */
    }
    s_blink_n = 0;
}

static void blink_obj_add(lv_obj_t *obj)
{
    for (int i = 0; i < s_blink_n; i++)
        if (s_blink_objs[i] == obj) return;   /* 已在组内(值+线同对象不可能,防重) */
    if (s_blink_n >= BLINK_MAX_OBJS) return;
    int i = s_blink_n;
    /* 起闪前存档原透明度,收线时按存档恢复 */
    s_blink_opa[i] = lv_obj_get_style_opa(obj, 0);
    s_blink_img_opa[i] = lv_obj_get_style_img_opa(obj, 0);
    s_blink_sel_opa[i] = lv_obj_get_style_text_opa(obj, LV_PART_SELECTED);
    s_blink_last_v[i] = -1;                   /* 强制首帧写入 */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, lv_obj_has_class(obj, &lv_roller_class) ? blink_exec_sel_cb : blink_exec_cb);
    lv_anim_set_values(&a, LV_OPA_COVER, BLINK_MIN_OPA);
    lv_anim_set_time(&a, BLINK_HALF_MS);
    lv_anim_set_playback_time(&a, BLINK_HALF_MS);
    lv_anim_set_repeat_delay(&a, 200);        /* 周期之间 200ms 停顿:呼吸间隙降平均负载 */
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_start(&a);
    s_blink_objs[i] = obj;
    s_blink_n = i + 1;
}

/* ---- 闪烁组注册表 ---- */
// 登记一个闪烁组(trigger 焦点命中→objs 整组同步闪;重复登记同一 trigger 覆盖旧组)
static void blink_group_add(lv_obj_t *trigger, lv_obj_t **objs, int n)
{
    if (!trigger) return;
    blink_group_t *g = NULL;
    for (int i = 0; i < s_blink_group_n; i++)
        if (s_blink_groups[i].trigger == trigger) { g = &s_blink_groups[i]; break; }   /* 重复登记:原位覆盖 */
    if (!g) {
        if (s_blink_group_n >= BLINK_MAX_GROUPS) return;
        g = &s_blink_groups[s_blink_group_n++];
        g->trigger = trigger;
    }
    g->n = 0;
    for (int i = 0; i < n && g->n < BLINK_MAX_OBJS; i++)
        if (objs[i]) g->objs[g->n++] = objs[i];
}

// 给已登记字段追加配套对象(单位/方向箭头等);无组则静默
void nav_blink_extra(lv_obj_t *label, lv_obj_t *extra)
{
    if (!label || !extra) return;
    for (int i = 0; i < s_blink_group_n; i++) {
        if (s_blink_groups[i].trigger == label) {
            if (s_blink_groups[i].n < BLINK_MAX_OBJS)
                s_blink_groups[i].objs[s_blink_groups[i].n++] = extra;
            return;
        }
    }
}

// 非 edit 体系页面(sixset2/toastcolor)build 时显式登记闪烁组(nav.h 导出)
void nav_blink_group_register(lv_obj_t *trigger, lv_obj_t **objs, int n)
{
    blink_group_add(trigger, objs, n);
}

// 焦点命中查表:有登记组→整组同步闪;无→停闪
static blink_group_t *blink_group_find(lv_obj_t *trigger)
{
    for (int i = 0; i < s_blink_group_n; i++)
        if (s_blink_groups[i].trigger == trigger)
            return &s_blink_groups[i];
    return NULL;
}

static void blink_evaluate(lv_obj_t *focused)
{
    blink_group_t *g = blink_group_find(focused);
    if (g) {
        blink_stop();       /* 换字段:停旧组起新组,重启即重新同步 */
        for (int j = 0; j < g->n; j++)
            blink_obj_add(g->objs[j]);
        return;
    }
    blink_stop();   /* 焦点对象未登记闪烁组:停闪 */
}

/* 焦点落在新对象:查闪烁组登记表(按钮/未登记对象→停闪)。
   非 label 触发器(roller)已登记组同样起闪——roller 选中值经 SELECTED text_opa 呼吸 */
static void nav_blink_focus_cb(lv_event_t *e)
{
    lv_obj_t *focused = lv_event_get_target(e);
    if (!focused) {
        blink_stop();
        return;
    }
    if (!lv_obj_check_type(focused, &lv_label_class) && !blink_group_find(focused)) {
        blink_stop();   /* 焦点到按钮/图片/未登记的 roller:停闪 */
        return;
    }
    blink_evaluate(focused);
}

// 将 buttons 数组中的非 NULL 对象全部加入 group（统一 NULL 检查）
static void group_add_all_btns(lv_group_t *g, lv_obj_t **btns, int count)
{
    for (int i = 0; i < count; i++) {
        if (btns[i]) {
            lv_obj_add_event_cb(btns[i], nav_blink_focus_cb, LV_EVENT_FOCUSED, NULL);
            lv_group_add_obj(g, btns[i]);
        }
    }
}
// 创建 group 并加入所有按钮（跳转子页时调用）
lv_group_t *group_create_for_page(lv_obj_t **btns, int count)
{
    lv_group_t *g = lv_group_create();
    if (g)
        group_add_all_btns(g, btns, count);
    return g;
}

