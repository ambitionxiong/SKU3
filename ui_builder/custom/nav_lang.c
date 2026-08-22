#include "nav_lang.h"
#include "i18n.h"

/* =====================================================================
 * 英文排版调整层（nav_lang.c）
 * 树遍历双 pass：
 *   pass 1 文本：label 当前文本命中翻译表 zh → 替换为英文
 *   pass 2 字体：label 当前字体是 taiwanpearl_XX → 切换为 aktivgrotesk_XX
 * 排版微调：当前页注册过 lang_tune 函数 → 调用（同事编辑区）
 * 中文模式全部早退。
 * ===================================================================== */

/* 字体 extern（生成于 ui_builder/font/，当前仅 24/30 两档在用） */
extern const lv_font_t c_taiwanpearl_regular_24;
extern const lv_font_t c_taiwanpearl_regular_30;
extern const lv_font_t c_taiwanpearl_regular_36;
extern const lv_font_t c_taiwanpearl_regular_48;
extern const lv_font_t c_taiwanpearl_regular_60;
extern const lv_font_t c_taiwanpearl_regular_72;
extern const lv_font_t c_taiwanpearl_regular_128;
extern const lv_font_t c_aktivgroteskmedium_24;
extern const lv_font_t c_aktivgroteskmedium_30;
extern const lv_font_t c_aktivgroteskmedium_36;
extern const lv_font_t c_aktivgroteskmedium_48;
extern const lv_font_t c_aktivgroteskmedium_60;
extern const lv_font_t c_aktivgroteskmedium_72;
extern const lv_font_t c_aktivgroteskmedium_128;

/* tr() 查表入口（i18n.c 提供，这里做首字节哈希加速可后续优化） */
extern const char *tr(const char *zh);

/* ============ 排版微调函数注册表（同事编辑区） ============ */

typedef void (*lang_tune_fn)(void);

static void tune_placeholder(void) {}

/* 有排版问题的页面在此注册；未注册页面零成本 */
static const struct {
    page_id_t    page;
    lang_tune_fn fn;
} s_tune_tab[] = {
    /* 示例：{ PAGE_AIR_MENU, air_menu_lang_tune }, */
};

/* 当前页对应的排版函数（无注册返回占位函数） */
static lang_tune_fn lang_tune_for_page(page_id_t pid)
{
    for (int i = 0; i < (int)(sizeof(s_tune_tab) / sizeof(s_tune_tab[0])); i++)
        if (s_tune_tab[i].page == pid)
            return s_tune_tab[i].fn;
    return tune_placeholder;
}

/* ============ 树遍历双 pass ============ */

static void lang_apply_obj(lv_obj_t *obj)
{
    /* 只处理 label（button 的子 label 由 LV_OBJ_FLAG_CLICKABLE 区分，直接遍历叶子） */
    if (lv_obj_check_type(obj, &lv_label_class)) {
        /* pass 1: 文本精确匹配翻译表 */
        const char *txt = lv_label_get_text(obj);
        if (txt && txt[0]) {
            const char *en = tr(txt);
            if (en != txt)
                lv_label_set_text(obj, en);
        }
        /* pass 2: 字体切换（taiwanpearl → aktivgrotesk，按字号一一映射） */
        const lv_font_t *f = lv_obj_get_style_text_font(obj, 0);
        if (f == &c_taiwanpearl_regular_128)
            lv_obj_set_style_text_font(obj, &c_aktivgroteskmedium_128, LV_PART_MAIN | 0);
        else if (f == &c_taiwanpearl_regular_72)
            lv_obj_set_style_text_font(obj, &c_aktivgroteskmedium_72, LV_PART_MAIN | 0);
        else if (f == &c_taiwanpearl_regular_60)
            lv_obj_set_style_text_font(obj, &c_aktivgroteskmedium_60, LV_PART_MAIN | 0);
        else if (f == &c_taiwanpearl_regular_48)
            lv_obj_set_style_text_font(obj, &c_aktivgroteskmedium_48, LV_PART_MAIN | 0);
        else if (f == &c_taiwanpearl_regular_36)
            lv_obj_set_style_text_font(obj, &c_aktivgroteskmedium_36, LV_PART_MAIN | 0);
        else if (f == &c_taiwanpearl_regular_30)
            lv_obj_set_style_text_font(obj, &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
        else if (f == &c_taiwanpearl_regular_24)
            lv_obj_set_style_text_font(obj, &c_aktivgroteskmedium_24, LV_PART_MAIN | 0);
    }
}

void lang_refresh_screen(void)
{
    if (!is_english()) return;
    lv_obj_tree_walk(lv_scr_act(), lang_apply_obj, NULL);
}

void lang_on_page_built(void)
{
    if (!is_english()) return;
    if (depth <= 0) return;

    /* 静态标签翻译 + 字体切换 */
    lang_refresh_screen();

    /* 当前页排版微调（同事注册的函数） */
    lang_tune_for_page(page_stack[depth - 1])();
}