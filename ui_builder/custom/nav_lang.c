#include "nav_lang.h"
#include "i18n.h"
#include <string.h>
#include <stdio.h>

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

/* ============ 排版微调函数注册表（定义于 nav_lang_tune.c 同事编辑区） ============ */

typedef void (*lang_tune_fn)(void);

extern const struct {
    page_id_t    page;
    lang_tune_fn fn;
    int dx, dy;
} s_tune_tab[];
extern const int s_tune_tab_n;

/* 当前页对应的排版函数（无注册返回占位函数） */
static void tune_placeholder(void) {}

static lang_tune_fn lang_tune_for_page(page_id_t pid)
{
    for (int i = 0; i < s_tune_tab_n; i++)
        if (s_tune_tab[i].page == pid)
            return s_tune_tab[i].fn;
    return tune_placeholder;
}

/* 当前页定时器重写对象的整体平移偏移（注册表 dx/dy；中文模式或未注册返回 0） */
int lang_dyn_dx(void)
{
    if (!is_english() || depth <= 0) return 0;
    page_id_t pid = page_stack[depth - 1];
    for (int i = 0; i < s_tune_tab_n; i++)
        if (s_tune_tab[i].page == pid)
            return s_tune_tab[i].dx;
    return 0;
}

int lang_dyn_dy(void)
{
    if (!is_english() || depth <= 0) return 0;
    page_id_t pid = page_stack[depth - 1];
    for (int i = 0; i < s_tune_tab_n; i++)
        if (s_tune_tab[i].page == pid)
            return s_tune_tab[i].dy;
    return 0;
}


/* ============ 繁體图片替换表(素材在 ui_builder/assets/image/) ============
 * 繁體模式树遍历: 简体图 src 精确命中 → 换 _tw 版;
 * 素材缺失的图(tips/hotcare/hotcleantips/steptext/modebg/waterbg/show)不在表内,
 * 繁體暂显示简体版, 出图后在表里补一行即可。 */
typedef struct { const char *tail; const char *cn_src; const char *tw_src; } tw_img_t;
static const tw_img_t s_tw_imgs[] = {
    { "off.png",              LVGL_IMAGE_PATH(off.png),              LVGL_IMAGE_PATH(off_tw.png) },
    { "on1.png",              LVGL_IMAGE_PATH(on1.png),              LVGL_IMAGE_PATH(on1_tw.png) },
    { "on2.png",              LVGL_IMAGE_PATH(on2.png),              LVGL_IMAGE_PATH(on2_tw.png) },
    { "focusoff.png",         LVGL_IMAGE_PATH(focusoff.png),         LVGL_IMAGE_PATH(focusoff_tw.png) },
    { "clearfrt.png",         LVGL_IMAGE_PATH(clearfrt.png),         LVGL_IMAGE_PATH(clearfrt_tw.png) },
    { "six.png",              LVGL_IMAGE_PATH(six.png),              LVGL_IMAGE_PATH(six_tw.png) },
    { "set_bg_txt.png",       LVGL_IMAGE_PATH(set_bg_txt.png),       LVGL_IMAGE_PATH(set_bg_txt_tw.png) },
    { "set_work_bg_txt.png",  LVGL_IMAGE_PATH(set_work_bg_txt.png),  LVGL_IMAGE_PATH(set_work_bg_txt_tw.png) },
    { "frozencookfr.png",     LVGL_IMAGE_PATH(frozencookfr.png),     LVGL_IMAGE_PATH(frozencookfr_tw.png) },
};
#define TW_IMGS_N (int)(sizeof(s_tw_imgs) / sizeof(s_tw_imgs[0]))

/* 运行时切图统一出口(nav_events 冻结菜单图标等): 繁體返回 _tw 完整路径 */
const char *lang_img_src(const char *cn_fname)
{
    static char s_fb[2][160];   /* 表外文件名兜底路径(环形 2 块) */
    static int  s_fb_idx = 0;
    if (is_trad()) {
        for (int i = 0; i < TW_IMGS_N; i++)
            if (strcmp(cn_fname, s_tw_imgs[i].tail) == 0)
                return s_tw_imgs[i].tw_src;
    }
    for (int i = 0; i < TW_IMGS_N; i++)
        if (strcmp(cn_fname, s_tw_imgs[i].tail) == 0)
            return s_tw_imgs[i].cn_src;
    char *b = s_fb[s_fb_idx];
    s_fb_idx = (s_fb_idx + 1) & 1;
    snprintf(b, sizeof(s_fb[0]), LVGL_DIR "image/%s", cn_fname);
    return b;
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
    /* 单位出口：℉ 模式 ℃/°C→°F 且烙死的数值一并换算(ui_temp_rewrite)；
       ℃ 模式维持 ℃→°C 等长替换（与 tr() 输出一致） */
    if (SET_Data.Set_TempUnit == 1)
        ui_temp_rewrite(buf, buf_len, 1);
    else {
        bp = buf;
        while ((bp = strstr(bp, "\xE2\x84\x83")) != NULL) {
            bp[0] = (char)0xC2; bp[1] = (char)0xB0; bp[2] = 'C';
            bp += 3;
        }
    }
    return 1;
}


static lv_obj_tree_walk_res_t lang_apply_obj(lv_obj_t *obj, void *user_data)
{
    (void)user_data;
    /* 繁體: 简体图 → _tw 图(精确命中 s_tw_imgs 才换, 表外素材缺失保持简体) */
    if (is_trad() && lv_obj_has_class(obj, &lv_image_class)) {
        const void *src = lv_image_get_src(obj);
        if (src && lv_image_src_get_type(src) == LV_IMAGE_SRC_FILE) {
            for (int i = 0; i < TW_IMGS_N; i++) {
                if (strcmp((const char *)src, s_tw_imgs[i].cn_src) == 0) {
                    lv_image_set_src(obj, s_tw_imgs[i].tw_src);
                    break;
                }
            }
        }
        return LV_OBJ_TREE_WALK_NEXT;
    }
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
        return LV_OBJ_TREE_WALK_NEXT;
    }

    /* 只处理 label（button 的子 label 由 LV_OBJ_FLAG_CLICKABLE 区分，直接遍历叶子） */
    if (lv_obj_check_type(obj, &lv_label_class)) {
        /* pass 1: 文本翻译——先精确查表，命中不了再模糊匹配状态条结构
         * 繁體: tr() 出口即词组+字表转换(数字已填充的状态条也能整串转换), 无需模糊匹配 */
        const char *txt = lv_label_get_text(obj);
        if (txt && txt[0]) {
            const char *en = tr(txt);
            if (en != txt && strcmp(en, txt) != 0) {
                lv_label_set_text(obj, en);
            } else if (!is_trad()) {
                char fbuf[256];
                if (lang_fuzzy_status(obj, txt, fbuf, (int)sizeof(fbuf)))
                    lv_label_set_text(obj, fbuf);
            }
        }
        /* pass 2: 字体切换（taiwanpearl → aktivgrotesk，按字号一一映射）
         * 仅英文模式执行; 繁體排版/字体与简体完全一致, 纯数字/单位标签也不切,
         * 否则数字变 Aktiv 变宽、℃ 等符号在 Aktiv 缺字变豆腐, 布局整体偏移 */
        if (is_trad()) return LV_OBJ_TREE_WALK_NEXT;
        const lv_font_t *f = lv_obj_get_style_text_font(obj, 0);
        const char *tp = lv_label_get_text(obj);
        bool has_cjk = false;
        for (const char *q = tp; q && *q; q++) {
            if ((unsigned char)*q >= 0xE4 && (unsigned char)*q <= 0xE9) { has_cjk = true; break; }
        }
        if (has_cjk) return LV_OBJ_TREE_WALK_NEXT;
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
    return LV_OBJ_TREE_WALK_NEXT;
}

/* lv_scr_load_anim 包装：显示新页面后自动翻译+排版（统一出口，覆盖所有跳转/返回/重建路径） */
void lang_scr_load_anim(lv_obj_t *scr, lv_scr_load_anim_t anim_type,
                        uint32_t time, uint32_t delay, bool auto_del)
{
    lv_scr_load_anim(scr, anim_type, time, delay, auto_del);
    lang_on_page_built();
    nav_tempunit_refresh_screen(scr);   /* 温度单位兜底:新屏烙死的 ℃ 标签/占位值重写(中英文都要跑) */
    nav_topflag_demo_sync();   /* 新屏已激活:演示徽标立即重定位(否则500ms内旧位置与新页标题重叠) */
}

void lang_refresh_screen(void)
{
    if (!is_english() && !is_trad()) return;   /* 简体零开销 */
    lv_obj_tree_walk(lv_scr_act(), lang_apply_obj, NULL);
}

void lang_on_page_built(void)
{
    if (depth <= 0) return;

    if (is_english()) {
        /* 英文: 静态标签翻译 + 字体切换 + 排版微调 */
        lang_refresh_screen();
        lang_tune_for_page(page_stack[depth - 1])();
    } else if (is_trad()) {
        /* 繁體: 树遍历翻译(tr→简转繁) + _tw 图片替换;
         * 排版与字体与简体一致(taiwanpearl), 不跑 lang_tune/字体映射 */
        lang_refresh_screen();
    }
}