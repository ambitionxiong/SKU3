/*
 * nav_hotcleandoor.c - 高温清洁"开门检测"等待页（PAGE_HOTCLEAN_DOORWAIT，三档共用）
 *
 * set 描述页[开始]后不再直接开始清洁，先进本页：设计要求先开门取出腔体内
 * 所有附件、再关闭机门，检测到关门才自动开始清洁烹饪。
 *
 * 门检测复用预热完成页的全局机制（nav_system.c system_timer_cb 500ms 门边沿
 * 检测）：hc_wait_door 标志 + 栈顶守卫，关门边沿触发 hc_doorwait_start_cooking()。
 * 门一直关着不产生边沿，"先开门再关门"的顺序天然满足；本页自身零定时器，
 * 任意路径离页（BACK/待机/关机/清洁键）残留标志由栈顶守卫兜住（别的页开关门
 * 只清标志不动作）。
 *
 * 标题"请打开门..."呼吸闪烁（lv_anim opa 通道，节奏常量与 nav_core blink 体系
 * 一致；label 随屏销毁时 LVGL 自动回收其动画）。页面保持 IFACE_SETTING +
 * MODE_HOT_CLEAN（set 页口径），关门跳 jump_to_h*_cooking 后才发 COOKING 帧，
 * 与"进烹饪前门开拦截"语义一致。空闲策略同 set 页（IFACE_SETTING，5 分钟待机）。
 */

#include "nav.h"
#include "protocol.h"     /* g_send/MODE_HOT_CLEAN */
#include "nav_internal.h" /* group_create_for_page/current_group */
#include "i18n.h"         /* tr */

/* ===== 页面结构体(本文件自持,不进 ui_objects,同 nav_langpick 先例) ===== */
typedef struct {
    lv_obj_t *obj;
    lv_obj_t *icon;        /* 档位图标 */
    lv_obj_t *title;       /* "请打开门..." 呼吸闪烁 */
    lv_obj_t *status;      /* "| 热解自清洁 | 2小时00分钟" */
    lv_obj_t *timelabel;   /* 右侧 "02:00:00" */
    lv_obj_t *label_3;     /* 底部提示 */
    lv_group_t *group;
} hc_doorwait_page_t;
static hc_doorwait_page_t s_doorwait_page;

/* hc_wait_door 定义在 nav_core.c(preheat_wait_door 旁),nav.h extern */

/* 档位:0 节能 1 中 2 高(只区分图标;时长文字按 set_hour/set_min,进 set 页已设) */
static uint8_t s_doorwait_tier = 2;

/* 标题呼吸闪烁:常量同 nav_core blink 体系(700ms 半程,200ms 间隔) */
#define DOORWAIT_BLINK_HALF_MS 700
#define DOORWAIT_BLINK_GAP_MS  200

static void doorwait_title_opa_cb(void *var, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)var, (lv_opa_t)v, LV_PART_MAIN);
}

static void doorwait_title_blink(lv_obj_t *label)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, label);
    lv_anim_set_exec_cb(&a, doorwait_title_opa_cb);
    lv_anim_set_values(&a, LV_OPA_COVER, 0);
    lv_anim_set_time(&a, DOORWAIT_BLINK_HALF_MS);
    lv_anim_set_playback_time(&a, DOORWAIT_BLINK_HALF_MS);
    lv_anim_set_repeat_delay(&a, DOORWAIT_BLINK_GAP_MS);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
}

/* 页面构建:几何/字体/配色同 hotcleanhigh_set 生成屏,时间标签同 hotclean*_cooking */
static void hc_doorwait_build(void)
{
    hc_doorwait_page_t *scr = &s_doorwait_page;

    scr->obj = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(scr->obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(scr->obj, lv_color_hex(0xfcfcfc), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->obj, LVGL_IMAGE_PATH(bg.jpg), LV_PART_MAIN | LV_STATE_DEFAULT);

    scr->icon = lv_img_create(scr->obj);
    switch (s_doorwait_tier) {
    case 0:  lv_img_set_src(scr->icon, LVGL_IMAGE_PATH(hotcleansaveicon.png));   break;
    case 1:  lv_img_set_src(scr->icon, LVGL_IMAGE_PATH(hotcleanmiddleicon.png)); break;
    default: lv_img_set_src(scr->icon, LVGL_IMAGE_PATH(hotcleanhighicon.png));   break;
    }
    lv_obj_set_pos(scr->icon, 115, 161);   /* 同 hotclean*_cooking image_17,切页零偏移 */

    scr->title = lv_label_create(scr->obj);
    lv_label_set_text(scr->title, "请打开门...");
    lv_label_set_long_mode(scr->title, LV_LABEL_LONG_WRAP);
    /* 起点/字号同 cooking label_21"清洁中..."(273,157,font60);宽 512 因中文
     * ≈285px 装不下 cooking 的 235 盒,左对齐起点一致即无偏移 */
    lv_obj_set_pos(scr->title, 273, 157);
    lv_obj_set_size(scr->title, 512, 60);
    lv_obj_set_style_text_font(scr->title, &c_taiwanpearl_regular_60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->title, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    doorwait_title_blink(scr->title);

    scr->status = lv_label_create(scr->obj);
    lv_label_set_long_mode(scr->status, LV_LABEL_LONG_WRAP);
    lv_label_set_text_fmt(scr->status, tr("| 热解自清洁 | %d小时%02d分钟"), set_hour, set_min);
    lv_obj_set_pos(scr->status, 274, 232);
    lv_obj_set_size(scr->status, 490, 39);
    lv_obj_set_style_text_font(scr->status, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->status, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    scr->timelabel = lv_label_create(scr->obj);
    lv_label_set_text_fmt(scr->timelabel, "%02d:%02d:00", set_hour, set_min);
    lv_label_set_long_mode(scr->timelabel, LV_LABEL_LONG_WRAP);
    /* 与 hotclean*_cooking timelabel 完全同几何(907,157,235x60,左对齐),切页零偏移 */
    lv_obj_set_pos(scr->timelabel, 907, 157);
    lv_obj_set_size(scr->timelabel, 235, 60);
    lv_obj_set_style_text_font(scr->timelabel, &c_taiwanpearl_regular_60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->timelabel, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);

    scr->label_3 = lv_label_create(scr->obj);
    lv_label_set_text(scr->label_3, "请打开门取出腔体内所有附件，并关闭机门");
    lv_label_set_long_mode(scr->label_3, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->label_3, 120, 320);
    lv_obj_set_size(scr->label_3, 600, 39);
    lv_obj_set_style_text_font(scr->label_3, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->label_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    /* 左对齐:起点与图标(x117)同列,设计稿口径 */
    lv_obj_set_style_text_align(scr->label_3, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 无可交互对象:空组(PRESS/编码器走通用尾无效音),组生命周期同其它页 */
    if (s_doorwait_page.group) lv_group_del(s_doorwait_page.group);
    s_doorwait_page.group = group_create_for_page(NULL, 0);
    current_group = s_doorwait_page.group;
}

/* set 描述页[开始] → 本页(三档入口,只差档位图标) */
static void doorwait_enter(uint8_t tier)
{
    edit_clear();
    s_doorwait_tier = tier;
    hc_wait_door = 1;   /* 武装:关门边沿自动开始(nav_system.c 消费) */
    page_push(PAGE_HOTCLEAN_DOORWAIT);
    lv_obj_clean(lv_scr_act());
    hc_doorwait_build();
    lang_scr_load_anim(s_doorwait_page.obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    /* IFACE_SETTING/MODE_HOT_CLEAN 维持 set 页口径,关门后才发 COOKING 帧 */
    printf("[hcs] jump: set -> doorwait (tier=%d)\n", tier);
}

void jump_to_hcs_doorwait(void) { doorwait_enter(0); }
void jump_to_hcm_doorwait(void) { doorwait_enter(1); }
void jump_to_hch_doorwait(void) { doorwait_enter(2); }

/* 关门边沿命中(nav_system.c 调用):按档进入清洁烹饪 */
void hc_doorwait_start_cooking(void)
{
    switch (s_doorwait_tier) {
    case 0:  jump_to_hcs_cooking(); break;
    case 1:  jump_to_hcm_cooking(); break;
    default: jump_to_hch_cooking(); break;
    }
}

/* pop 回本页重建(防御分派,正常流无子页压栈) */
void hc_doorwait_rebuild(void)
{
    hc_wait_door = 1;   /* 与进页同口径重新武装 */
    hc_doorwait_build();
    lang_scr_load_anim(s_doorwait_page.obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[hcs] back to doorwait\n");
}

/* 英文排版(仅英文模式经 lang_on_page_built 调用,无需内部语言判断):
 * 四共有组件坐标/字号逐项镜像 hotcleanhigh_cooking_lang_tune 的 EN 段——
 * 切页零偏移;标题不设字号覆盖,随树遍历自然映射 aktiv 60 同"清洁中"英文态;
 * label_3 为本页独有(cooking 该位置是进度条),650 宽装不下折两行 */
void hc_doorwait_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEAN_DOORWAIT) return;
    hc_doorwait_page_t *scr = &s_doorwait_page;

    lv_obj_set_pos(scr->title, 272, 161);
    lv_obj_set_size(scr->title, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_obj_set_pos(scr->status, 275, 232);
    lv_obj_set_size(scr->status, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_obj_set_pos(scr->timelabel, 902, 162);
    lv_obj_set_size(scr->timelabel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    /* 底部标语:本页独有(cooking 该位置是进度条),但需与 set 确认页 EN 对齐——
     * 逐行照抄 set EN tune 的定位(650 宽+中心偏移+行距 -1),固定 pos 会偏低 */
    lv_obj_set_style_text_align(scr->label_3, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_long_mode(scr->label_3, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_line_space(scr->label_3, -1, 0);
    lv_obj_set_size(scr->label_3, 650, LV_SIZE_CONTENT);
    lv_obj_align(scr->label_3, LV_ALIGN_CENTER, -193, 95);
}
