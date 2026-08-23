/*
 * nav_stepset.c - 多段烹饪步骤设置页
 * 每个步骤的模式/温度/时间设置，返回多段主页面。
 */
#include "nav.h"
#include "protocol.h"

/* stepset 模式范围表（温度/小时范围 + 默认温度，与各模式菜单一致） */
typedef struct {
    uint8_t mode;
    int tmin, tmax;
    int hmax;
    int tdef;
} stepset_range_t;

static const stepset_range_t g_stepset_ranges[] = {
    { MODE_HOTWIND_BBQ,    30, 300, 4, 180 },
    { MODE_SAVE_BBQ,      125, 250, 4, 180 },
    { MODE_UPDOWN_BBQ,     30, 300, 4, 180 },
    { MODE_BOTTOM_BBQ,     30, 250, 4, 180 },
    { MODE_HOT_BBQ,        30, 300, 4, 180 },
    { MODE_CENTRAL_BBQ,    30, 300, 4, 300 },
    { MODE_WINDCHANGE_BBQ, 30, 300, 4, 180 },
    { MODE_TOP_BBQ,        30, 300, 4, 300 },
    { MODE_AIR,            30, 300, 4, 230 },
    { MODE_RISING,         30,  50, 7,  35 },
    { MODE_SLOWCOOK,       70, 120, 7,  80 },
    { MODE_UNFROZEN,       30,  60, 4,  40 },
};

/* 烹饪功能 8 项（roller_main 第 0 项） */
static const char *g_modes_cook[] = {
    "上下烧烤", "顶部烧烤", "热风烧烤", "热风",
    "节能热风", "底部烧烤", "集中烧烤", "热风对流",
};
static const uint8_t g_modes_cook_id[] = {
    MODE_UPDOWN_BBQ, MODE_TOP_BBQ, MODE_HOT_BBQ, MODE_HOTWIND_BBQ,
    MODE_SAVE_BBQ, MODE_BOTTOM_BBQ, MODE_CENTRAL_BBQ, MODE_WINDCHANGE_BBQ,
};

/* 特殊功能（步骤一 4 项；步骤 2/3 去掉发酵/解冻 → 前 2 项） */
static const char *g_modes_special[] = { "空气炸", "慢煮", "解冻", "发酵" };
static const uint8_t g_modes_special_id[] = {
    MODE_AIR, MODE_SLOWCOOK, MODE_UNFROZEN, MODE_RISING,
};

// 当前 roller 选中项 → MODE
static uint8_t stepset_sel_mode(stepset_t *scr)
{
    uint32_t main_sel = lv_roller_get_selected(scr->roller_main);
    uint32_t sub_sel = lv_roller_get_selected(scr->roller_mode);
    if (main_sel == 0)
        return (sub_sel < 8) ? g_modes_cook_id[sub_sel] : MODE_NONE;
    if (g_cur_step == 0)
        return (sub_sel < 4) ? g_modes_special_id[sub_sel] : MODE_NONE;
    /* 步骤 2/3:特殊功能仅 空气炸/慢煮(索引与选项列表一一对应) */
    static const uint8_t sub_23[] = { MODE_AIR, MODE_SLOWCOOK };
    return (sub_sel < 2) ? sub_23[sub_sel] : MODE_NONE;
}

// 按模式查找范围
static const stepset_range_t *stepset_find_range(uint8_t mode)
{
    for (int i = 0; i < (int)(sizeof(g_stepset_ranges) / sizeof(g_stepset_ranges[0])); i++) {
        if (g_stepset_ranges[i].mode == mode)
            return &g_stepset_ranges[i];
    }
    return &g_stepset_ranges[0];
}

// 应用当前选中模式：cook_mode + 范围重注册 + 温度/时间设置
// restore=true(进入 stepset)：已设置步骤回填已保存值；restore=false(切换模式)：切为该模式默认
void stepset_apply_sel_mode(bool restore)
{
    stepset_t *scr = stepset_get(&ui_manager);
    if (!scr) return;
    uint8_t mode = stepset_sel_mode(scr);
    g_send.cook_mode = mode;
    const stepset_range_t *r = stepset_find_range(mode);

    if (restore && g_cur_step >= 0 && g_cur_step <= 2 && g_steps[g_cur_step].set) {
        /* 进入编辑已设置步骤：回填已保存值 */
        set_temp = g_steps[g_cur_step].temp;
        set_hour = g_steps[g_cur_step].hour;
        set_min  = g_steps[g_cur_step].min;
    } else {
        /* 新设置 或 切换模式：该模式默认 */
        set_temp = r->tdef;
        set_hour = 0;
        set_min  = 30;
    }

    edit_clear();
    edit_register(scr->temp, scr->templine2, scr->templine3,
                  &set_temp, r->tmin, r->tmax, 5, "%d");
    edit_register(scr->hour, scr->hourline, NULL,
                  &set_hour, 0, r->hmax, 1, "%02d");
    edit_register(scr->min, scr->minline, NULL,
                  &set_min, 0, 59, 1, "%02d");

    lv_label_set_text_fmt(scr->temp, "%d", set_temp);
    lv_label_set_text_fmt(scr->hour, "%02d", set_hour);
    lv_label_set_text_fmt(scr->min, "%02d", set_min);
    /* line 显隐统一由 stepset_on_focus 按当前焦点控制 */
    stepset_on_focus(NULL);
}

// 回填已设置步骤：按保存的模式定位 roller（大类 + 子类 + 选项列表），再应用
void stepset_restore_mode(uint8_t mode)
{
    stepset_t *scr = stepset_get(&ui_manager);
    if (!scr) return;
    int main_sel = -1, sub_sel = 0;
    for (int i = 0; i < 8; i++)
        if (g_modes_cook_id[i] == mode) { main_sel = 0; sub_sel = i; break; }
    if (main_sel < 0) {
        for (int i = 0; i < 4; i++)
            if (g_modes_special_id[i] == mode) { main_sel = 1; sub_sel = i; break; }
    }
    if (main_sel < 0) return;

    lv_roller_set_selected(scr->roller_main, main_sel, LV_ANIM_OFF);
    if (main_sel == 0)
        lv_roller_set_options(scr->roller_mode,
                              tr("上下烧烤\n顶部烧烤\n热风烧烤\n热风\n节能热风\n底部烧烤\n集中烧烤\n热风对流"),
                              LV_ROLLER_MODE_NORMAL);
    else if (g_cur_step == 0)
        lv_roller_set_options(scr->roller_mode, tr("空气炸\n慢煮\n解冻\n发酵"), LV_ROLLER_MODE_NORMAL);
    else
        lv_roller_set_options(scr->roller_mode, tr("空气炸\n慢煮"), LV_ROLLER_MODE_NORMAL);
    /* 步骤 2/3: 4 项表前两项(空气炸/慢煮)与 2 项表一一对应,无需映射 */
    lv_roller_set_selected(scr->roller_mode, sub_sel, LV_ANIM_OFF);
    stepset_apply_sel_mode(true);
}

// roller_mode 选中项变化：更新 line（按汉字个数选线）+ 应用模式
static void stepset_on_mode_change(lv_event_t *e)
{
    stepset_t *scr = lv_event_get_user_data(e);
    if (!scr) return;
    stepset_apply_sel_mode(false);
    stepset_on_focus(NULL);   /* 刷新 line 显示 */
}

// 焦点变化：显示对应选项的 line
void stepset_on_focus(lv_event_t *e)
{
    stepset_t *scr = stepset_get(&ui_manager);
    if (!scr) return;
    lv_obj_t *f = e ? lv_event_get_target(e)
                    : (current_group ? lv_group_get_focused(current_group) : NULL);

    /* 全部隐藏 */
    lv_obj_add_flag(scr->mainline, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->modeline4, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->modeline2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->modeline3, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->templine3, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->templine2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->hourline, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->minline, LV_OBJ_FLAG_HIDDEN);

    if (f == scr->roller_main) {
        lv_obj_clear_flag(scr->mainline, LV_OBJ_FLAG_HIDDEN);
    } else if (f == scr->roller_mode) {
        /* 按选中项宽度选线（中文按汉字数 2/3/4 → line2/3/4；
           英文按字符数近似，避免 UTF-8 统计失效导致 line 不显示） */
        char buf[32];
        lv_roller_get_selected_str(scr->roller_mode, buf, sizeof(buf));
        int n = 0;   /* 显示宽度单位：汉字=2、ASCII=1（英文约半字宽） */
        for (char *p = buf; *p; p++) {
            if ((unsigned char)*p >= 0x80) n += 2;
            else n += 1;
        }
        n = n / 2;
        if (n <= 2)        lv_obj_clear_flag(scr->modeline2, LV_OBJ_FLAG_HIDDEN);
        else if (n <= 4)   lv_obj_clear_flag(scr->modeline3, LV_OBJ_FLAG_HIDDEN);
        else               lv_obj_clear_flag(scr->modeline4, LV_OBJ_FLAG_HIDDEN);
    } else if (f == scr->temp) {
        if (set_temp < 100)
            lv_obj_clear_flag(scr->templine2, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_clear_flag(scr->templine3, LV_OBJ_FLAG_HIDDEN);
    } else if (f == scr->hour) {
        lv_obj_clear_flag(scr->hourline, LV_OBJ_FLAG_HIDDEN);
    } else if (f == scr->min) {
        lv_obj_clear_flag(scr->minline, LV_OBJ_FLAG_HIDDEN);
    }
    /* next：全部隐藏 */

    /* 整屏强制重绘，彻底清除真机显示缓冲区残留 */
    if (lv_scr_act())
        lv_obj_invalidate(lv_scr_act());
}

// 大类切换：更新子类选项（步骤 2/3 去掉发酵/解冻）+ 应用模式
static void stepset_on_main_change(lv_event_t *e)
{
    stepset_t *scr = lv_event_get_user_data(e);
    if (!scr) return;
    uint32_t sel = lv_roller_get_selected(scr->roller_main);
    if (sel == 0) {
        lv_roller_set_options(scr->roller_mode,
                              tr("上下烧烤\n顶部烧烤\n热风烧烤\n热风\n节能热风\n底部烧烤\n集中烧烤\n热风对流"),
                              LV_ROLLER_MODE_NORMAL);
    } else {
        if (g_cur_step == 0)
            lv_roller_set_options(scr->roller_mode, tr("空气炸\n慢煮\n解冻\n发酵"), LV_ROLLER_MODE_NORMAL);
        else
            lv_roller_set_options(scr->roller_mode, tr("空气炸\n慢煮"), LV_ROLLER_MODE_NORMAL);
    }
    lv_roller_set_selected(scr->roller_mode, 0, LV_ANIM_OFF);
    stepset_apply_sel_mode(false);
    stepset_on_focus(NULL);
}

void stepset_bind_events(void)
{
    stepset_t *scr = stepset_get(&ui_manager);
    if (!scr || !scr->roller_main) return;
    /* 先移除旧回调防重复累积(stepset_bind_events 每次进入页面都会调用) */
    lv_obj_remove_event_cb(scr->roller_main, stepset_on_main_change);
    lv_obj_remove_event_cb(scr->roller_mode, stepset_on_mode_change);
    lv_obj_remove_event_cb(scr->roller_main, stepset_on_focus);
    lv_obj_remove_event_cb(scr->roller_mode, stepset_on_focus);
    lv_obj_remove_event_cb(scr->temp, stepset_on_focus);
    lv_obj_remove_event_cb(scr->hour, stepset_on_focus);
    lv_obj_remove_event_cb(scr->min, stepset_on_focus);
    lv_obj_remove_event_cb(scr->next, stepset_on_focus);

    lv_obj_add_event_cb(scr->roller_main, stepset_on_main_change,
                        LV_EVENT_VALUE_CHANGED, scr);
    lv_obj_add_event_cb(scr->roller_mode, stepset_on_mode_change,
                        LV_EVENT_VALUE_CHANGED, scr);
    /* 焦点切换时刷新 line 显隐(与 updown_bbq_menu 一致) */
    lv_obj_add_event_cb(scr->roller_main, stepset_on_focus, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(scr->roller_mode, stepset_on_focus, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(scr->temp, stepset_on_focus, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(scr->hour, stepset_on_focus, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(scr->min, stepset_on_focus, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(scr->next, stepset_on_focus, LV_EVENT_FOCUSED, NULL);
}
