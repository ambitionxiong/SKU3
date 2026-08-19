#include "i18n.h"
#include "nav.h"
#include "custom_defs.h"
#include <string.h>

/* 字体 extern (生成于 ui_builder/font/) */
extern const lv_font_t c_taiwanpearl_regular_30;
extern const lv_font_t c_taiwanpearl_regular_24;
extern const lv_font_t c_aktivgroteskmedium_30;
extern const lv_font_t c_aktivgroteskmedium_24;

/* 中文→英文 翻译表(从 SKU123456 对照表 cook menu 部分) */
typedef struct { const char *zh; const char *en; } i18n_t;
static const i18n_t s_table[] = {
    { "热风",           "Forced Air" },
    { "节能热风",       "Eco Forced Air" },
    { "上下烧烤",       "Conventional" },
    { "底部烧烤",       "Bottom Heat" },
    { "热风烧烤",       "Turbo Grill" },
    { "集中烧烤",       "Half Grill" },
    { "热风对流",       "Convection Bake" },
    { "预热",           "Preheat" },
    { "烹饪功能",       "Cooking Functions" },
    { "顶部烧烤",       "Grill" },
    { "菜单",           "Menu" },
    { "特殊功能",       "Special Functions" },
};
static const int s_table_n = (int)(sizeof(s_table) / sizeof(s_table[0]));

uint8_t g_lang_en = 0;   /* 独立英文标志位: 0=中文 1=英文 */

int is_english(void)
{
    return g_lang_en;
}

const char *tr(const char *zh)
{
    if (!zh) return zh;
    if (is_english()) {
        for (int i = 0; i < s_table_n; i++)
            if (s_table[i].zh[0] == zh[0] && strcmp(s_table[i].zh, zh) == 0)
                return s_table[i].en;
    }
    return zh;   /* 非英文或未查到的中文,原样返回 */
}

void cookmenu_apply_lang(void)
{
    /* 仅 cook menu 页面生效, 其他页面(如 F8 任意页按)直接返回, 避免空指针 */
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COOKMENU) return;
    cookmenu_t *cm = cookmenu_get(&ui_manager);
    if (!cm) return;
    if (!is_english()) return;   /* 非英文保留生成默认 */
    if (!cm->hot_wind_labal || !cm->save_labal) return;   /* 页面未创建完成兜底 */

    lv_label_set_text(cm->hot_wind_labal,     tr("热风"));
    lv_label_set_text(cm->save_labal,         tr("节能热风"));
    lv_label_set_text(cm->up_down_labal,      tr("上下烧烤"));
    lv_label_set_text(cm->bottom_bbq_labal,   tr("底部烧烤"));
    lv_label_set_text(cm->hotwind_bbq_labal,  tr("热风烧烤"));
    lv_label_set_text(cm->central_labal,      tr("集中烧烤"));
    lv_label_set_text(cm->wind_change_labal,  tr("热风对流"));
    lv_label_set_text(cm->preheater_labal,    tr("预热"));
    lv_label_set_text(cm->pengren_labal,      tr("烹饪功能"));
    lv_label_set_text(cm->hot_bbq_labal,      tr("顶部烧烤"));

    /* 字体切换: 各 30 号 labal → AktivGroteskMedium 30, 标题 24 → 24 */
    lv_obj_set_style_text_font(cm->hot_wind_labal,    &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
    lv_obj_set_style_text_font(cm->save_labal,        &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
    lv_obj_set_style_text_font(cm->up_down_labal,     &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
    lv_obj_set_style_text_font(cm->bottom_bbq_labal,  &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
    lv_obj_set_style_text_font(cm->hotwind_bbq_labal, &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
    lv_obj_set_style_text_font(cm->central_labal,     &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
    lv_obj_set_style_text_font(cm->wind_change_labal, &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
    lv_obj_set_style_text_font(cm->preheater_labal,   &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
    lv_obj_set_style_text_font(cm->hot_bbq_labal,     &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
    lv_obj_set_style_text_font(cm->pengren_labal,     &c_aktivgroteskmedium_24, LV_PART_MAIN | 0);
}

void major_menu_apply_lang(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_MAJOR_MENU) return;
    major_menu_t *mm = major_menu_get(&ui_manager);
    if (!mm) return;
    if (!is_english()) return;
    if (!mm->menu_label || !mm->cook_label) return;
    lv_label_set_text(mm->menu_label,    tr("菜单"));
    lv_label_set_text(mm->cook_label,    tr("烹饪功能"));
    lv_label_set_text(mm->cook4_label,   "COOK 4");
    lv_label_set_text(mm->special_label, tr("特殊功能"));
    lv_obj_set_style_text_font(mm->menu_label,    &c_aktivgroteskmedium_24, LV_PART_MAIN | 0);
    lv_obj_set_style_text_font(mm->cook_label,    &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
    lv_obj_set_style_text_font(mm->cook4_label,   &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
    lv_obj_set_style_text_font(mm->special_label, &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
}
