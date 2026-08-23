/* =====================================================================
 * 英文排版调整层：同事编辑区（nav_lang_tune.c）
 *
 * 用法：
 *   1. 在本文件写 void xxx_lang_tune(void) 函数
 *   2. 在 nav_lang.c 的 s_tune_tab[] 注册一行 { PAGE_XXX, xxx_lang_tune }
 *   3. 函数在英文模式页面显示后自动调用（在树遍历翻译之后执行，
 *      可覆盖 文本/坐标/字号，优先级最高）
 *
 * 中文模式：本文件所有函数不会被调用，零影响。
 * 详见 docs/I18N_GUIDE_zh.md
 * ===================================================================== */
#include "nav_lang.h"
#include "i18n.h"

/* ================= specialmenu 英文布局示例 =================
 * 生成文件 special_menu.c 基准坐标：
 *   标题 special_label (24,24) 字号24 | 第一行按钮文字 y≈287 字号30
 * ============================================================ */
void special_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SPECIAL_MENU) return;
    special_menu_t *sp = special_menu_get(&ui_manager);
    if (!sp) return;

    /* ① 页面标题：覆盖英文 + 加宽容器 + 升字号 */
    lv_label_set_text(sp->special_label, "SPECIAL FUNCTIONS");
    lv_obj_set_size(sp->special_label, 220, 30);
    lv_obj_set_pos(sp->special_label, 24, 22);
    lv_obj_set_style_text_font(sp->special_label, &c_aktivgroteskmedium_30,
                               LV_PART_MAIN | 0);

    /* ② 按钮文字：覆盖英文（长文本降字号防溢出，演示排版微调） */
    lv_label_set_text(sp->air_label, "AIR FRY");
    lv_obj_set_size(sp->air_label, 150, 30);
    lv_obj_set_pos(sp->air_label, 69, 290);
    lv_obj_set_style_text_font(sp->air_label, &c_aktivgroteskmedium_24,
                               LV_PART_MAIN | 0);

    lv_label_set_text(sp->piza_label, "PIZZA");
    lv_obj_set_size(sp->piza_label, 150, 30);
    lv_obj_set_style_text_font(sp->piza_label, &c_aktivgroteskmedium_24,
                               LV_PART_MAIN | 0);

    lv_label_set_text(sp->frozen_cook_label, "FROZEN");
    lv_obj_set_size(sp->frozen_cook_label, 150, 30);
    lv_obj_set_style_text_font(sp->frozen_cook_label, &c_aktivgroteskmedium_24,
                               LV_PART_MAIN | 0);

    lv_label_set_text(sp->slow_cook_label, "SLOW COOK");
    lv_obj_set_size(sp->slow_cook_label, 150, 30);
    lv_obj_set_style_text_font(sp->slow_cook_label, &c_aktivgroteskmedium_24,
                               LV_PART_MAIN | 0);

    lv_label_set_text(sp->unfrozen_label, "DEFROST");
    lv_obj_set_size(sp->unfrozen_label, 150, 30);
    lv_obj_set_style_text_font(sp->unfrozen_label, &c_aktivgroteskmedium_24,
                               LV_PART_MAIN | 0);

    lv_label_set_text(sp->fajiao_label, "FERMENT");
    lv_obj_set_size(sp->fajiao_label, 150, 30);
    lv_obj_set_style_text_font(sp->fajiao_label, &c_aktivgroteskmedium_24,
                               LV_PART_MAIN | 0);

    lv_label_set_text(sp->corn_label, "DRIED FRUITS");
    lv_obj_set_size(sp->corn_label, 150, 30);
    lv_obj_set_style_text_font(sp->corn_label, &c_aktivgroteskmedium_24,
                               LV_PART_MAIN | 0);

    lv_label_set_text(sp->heat_contain_label, "KEEP WARM");
    lv_obj_set_size(sp->heat_contain_label, 150, 30);
    lv_obj_set_style_text_font(sp->heat_contain_label, &c_aktivgroteskmedium_24,
                               LV_PART_MAIN | 0);

    lv_label_set_text(sp->some_cook_label, "MULTI-STEP COOK");
    lv_obj_set_size(sp->some_cook_label, 150, 30);
    lv_obj_set_style_text_font(sp->some_cook_label, &c_aktivgroteskmedium_24,
                               LV_PART_MAIN | 0);
}