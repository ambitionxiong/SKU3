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


/* 取 opts 中 s 起 len 字节的子串查表（临时缓冲） */
static const char *tr_line(const char *s, size_t len)
{
    static char tmp[128];
    if (len >= sizeof(tmp)) return s;
    memcpy(tmp, s, len);
    tmp[len] = '\0';
    return tr(tmp);
}

/* ============ 树遍历双 pass ============ */

/* ============ 模糊匹配：状态条 "| 模式 | 数值℃ | 时间" ============
 * 根部生成文件里状态条是"数值已填充"的完整文本（如 "| 披萨 | 180℃ | 1小时20分钟"），
 * 精确匹配翻译表命中不了。这里解析结构：
 *   取第一个 | 与第二个 | 之间的模式名 → 查表 → 替换
 *   剩余部分做单位转换：小时→h、分钟→min（℃ 保留）
 * 返回 1 表示已模糊翻译，0 表示非状态条结构。 */
static int lang_fuzzy_status(lv_obj_t *obj, const char *txt, char *buf, int buf_len)
{
    const char *p1 = strchr(txt, '|');
    if (!p1) return 0;
    const char *p2 = strchr(p1 + 1, '|');
    if (!p2) return 0;

    /* 提取模式名段（去首尾空格） */
    char mode[64];
    int mlen = (int)(p2 - p1 - 1);
    if (mlen <= 0 || mlen >= (int)sizeof(mode)) return 0;
    int s = 0, e = mlen - 1;
    while (s <= e && p1[1 + s] == ' ') s++;
    while (e >= s && p1[1 + e] == ' ') e--;
    if (e < s) return 0;   /* 空模式名 */
    memcpy(mode, p1 + 1 + s, (size_t)(e - s + 1));
    mode[e - s + 1] = '\0';

    /* 模式名查表（tr 查不到说明非状态条） */
    const char *en_mode = tr(mode);
    if (en_mode == mode) return 0;

    /* 重组：| 英文模式名 + 尾部（单位转换） */
    snprintf(buf, (size_t)buf_len, "| %s %s", en_mode, p2);

    /* 尾部单位转换：小时→h、分钟→min（先小时后分钟）
     * "小时"=6字节→" h "=3字节；"分钟"=6字节→" min"=4字节。
     * memmove 源从偏移6取（保留尾串），替换后指针前进避免死循环。 */
    char *bp = buf;
    while ((bp = strstr(bp, "小时")) != NULL) {
        *bp = ' '; bp[1] = 'h';
        memmove(bp + 2, bp + 6, strlen(bp + 6) + 1);
        bp += 3;
    }
    bp = buf;
    while ((bp = strstr(bp, "分钟")) != NULL) {
        *bp = ' '; bp[1] = 'm'; bp[2] = 'i'; bp[3] = 'n';
        memmove(bp + 4, bp + 6, strlen(bp + 6) + 1);
        bp += 4;
    }
    return 1;
}


static void lang_apply_obj(lv_obj_t *obj)
{
    /* roller 选项翻译：逐行查表替换（树遍历能看到当前选中项，但需覆盖全部选项） */
    if (lv_obj_check_type(obj, &lv_roller_class)) {
        const char *opts = lv_roller_get_options(obj);
        if (opts && opts[0] && strchr(opts, '\n')) {
            char nb[256];
            const char *s = opts;
            size_t pos = 0;
            while (pos < sizeof(nb) - 1) {
                const char *e = strchr(s, '\n');
                size_t len = e ? (size_t)(e - s) : strlen(s);
                const char *en = tr_line(s, len);
                size_t enlen = strlen(en);
                if (pos + enlen + (e ? 1 : 0) >= sizeof(nb)) break;
                memcpy(nb + pos, en, enlen);
                pos += enlen;
                if (e) { nb[pos++] = '\n'; s = e + 1; }
                else break;
            }
            nb[pos] = '\0';
            if (strcmp(nb, opts) != 0) {
                uint32_t sel = lv_roller_get_selected(obj);
                lv_roller_set_options(obj, nb, LV_ROLLER_MODE_NORMAL);
                lv_roller_set_selected(obj, sel, LV_ANIM_OFF);
            }
        }
        return;
    }

    /* 只处理 label（button 的子 label 由 LV_OBJ_FLAG_CLICKABLE 区分，直接遍历叶子） */
    if (lv_obj_check_type(obj, &lv_label_class)) {
        /* pass 1: 文本翻译——先精确查表，命中不了再模糊匹配状态条结构 */
        const char *txt = lv_label_get_text(obj);
        if (txt && txt[0]) {
            const char *en = tr(txt);
            if (en != txt) {
                lv_label_set_text(obj, en);
            } else {
                char fbuf[128];
                if (lang_fuzzy_status(obj, txt, fbuf, (int)sizeof(fbuf)))
                    lv_label_set_text(obj, fbuf);
            }
        }
        /* pass 2: 字体切换（taiwanpearl → aktivgrotesk，按字号一一映射）
         * 仅当文本全为 ASCII 才切换——aktivgrotesk 只含 0x20-0x7E，
         * 含 ℃/°↑↓/中文等符号的文本保留 taiwanpearl（全字形）避免方块 */
        const lv_font_t *f = lv_obj_get_style_text_font(obj, 0);
        const char *tp = lv_label_get_text(obj);
        bool ascii_only = true;
        for (const char *q = tp; q && *q; q++)
            if ((unsigned char)*q > 0x7E) { ascii_only = false; break; }
        if (!ascii_only) return;
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

/* lv_scr_load_anim 包装：显示新页面后自动翻译+排版（统一出口，覆盖所有跳转/返回/重建路径） */
void lang_scr_load_anim(lv_obj_t *scr, lv_scr_load_anim_t anim_type,
                        uint32_t time, uint32_t delay, bool auto_del)
{
    lv_scr_load_anim(scr, anim_type, time, delay, auto_del);
    lang_on_page_built();
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