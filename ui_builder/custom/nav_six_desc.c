/*
 * nav_six_desc.c - 第六感-烹饪描述页
 * 显示配方描述/温度时间，确认进入六感运行页。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"
#include <string.h>

/* ==============================
 * 第六感-烹饪说明页（descriptionmenu）
 * 菜单流程: risingpage → descriptionmenu → 开始/延迟
 * ============================== */

lv_group_t *g_descriptionmenu = NULL;

static void on_description_start_click(lv_event_t *e);
static void on_description_delay_click(lv_event_t *e);

/* ==============================
 * 说明页文本重排（简/繁/英通用）
 * ------------------------------
 * LVGL 自动折行只看盒宽，还把 . - , 当断行字符，于是会出现几种“不好看”的断法：
 * 行首孤零零一个“。”（2026-11-17 繁中炸排骨）、1.5-/tablespoon 与 2.5cm 被拆开
 * （2026-11-17 英文曲奇）、末行只剩一两个字（2026-11-18 用户要求整体微调）。
 * 这里在版面定稿后自己断行：
 *   ① 按作者写的 \n 分段；段内取“词”——中文/全角一字一词（字间可断），
 *      英文/数字连成一词（. - , 等都在词内，绝不拆开）；
 *   ② 贪心装满一行，行尾优先落在标点处（中文允许回退 2 个字），其次落在空格处；
 *   ③ 行首若还是句末标点，先按实际像素把标签加宽一点（≤40px，不越卡片内容宽；
 *      +40 后右缘≤982，不碰右下延迟钮 x983），加不动了才把上一行末词推下来；
 *   ④ 末行太短（< 盒宽 1/3）时，把上一行末尾的词挪下来，两行长短拉开得更均匀。
 * 只插入 \n、不增删字符；行数超过 LVGL 自动折行数就整体放弃（绝不把标签顶高）。
 * ============================== */
#define DESC_TOK_MAX     384
#define DESC_BUF_SIZE    2048
#define DESC_WIDEN_MAX   40     /* 为“行首标点”最多加宽 px */
#define DESC_ORPHAN_DIV  3      /* 末行 < 盒宽/3 视为孤行 */

typedef struct {
    int16_t  off;      /* 段内字节偏移 */
    int16_t  len;      /* 字节长度 */
    int16_t  w;        /* 像素宽 */
    int16_t  sp_w;     /* 与前一词之间的空格宽 */
    uint32_t first;    /* 首码点 */
    uint32_t last;     /* 末码点 */
    uint8_t  punct;    /* 末字符是标点 → 可在此断行(优先) */
    uint8_t  space;    /* 词后跟空格 → 中文短语边界(次优先) */
} desc_tok_t;

static desc_tok_t s_tok[DESC_TOK_MAX];
static int16_t    s_lb[DESC_TOK_MAX];   /* 每行起止词号 */
static int16_t    s_le[DESC_TOK_MAX];
static char       s_reflow[DESC_BUF_SIZE];

/* 中文/全角:一字一词,字间可断行 */
static bool desc_cp_wide(uint32_t cp)
{
    return (cp >= 0x2E80 && cp <= 0x9FFF) ||   /* CJK 部首/表意/标点 */
           (cp >= 0xAC00 && cp <= 0xD7AF) ||   /* 谚文 */
           (cp >= 0xF900 && cp <= 0xFAFF) ||   /* 兼容表意 */
           (cp >= 0xFF00 && cp <= 0xFFEF);     /* 全角 ASCII */
}

/* 句末/收尾标点:行首禁止出现,行尾优先在此断行 */
static bool desc_cp_close(uint32_t cp)
{
    switch (cp) {
    case 0x3002:   /* 。 */
    case 0xFF0C:   /* ， */
    case 0x3001:   /* 、 */
    case 0xFF1B:   /* ； */
    case 0xFF1A:   /* ： */
    case 0xFF1F:   /* ？ */
    case 0xFF01:   /* ！ */
    case 0xFF09:   /* ） */
    case 0xFF3D:   /* ］ */
    case 0x300D:   /* 」 */
    case 0x300F:   /* 』 */
    case 0x3011:   /* 】 */
    case 0x300B:   /* 》 */
    case 0x2026:   /* … */
    case '.':
    case ',':
    case ';':
    case ':':
    case '!':
    case '?':
    case ')':
    case ']':
    case '}':
        return true;
    default:
        return false;
    }
}

/* UTF-8 解一个码点,len 回填字节数 */
static uint32_t desc_utf8_next(const char *s, int *len)
{
    const unsigned char *p = (const unsigned char *)s;
    uint32_t cp;
    int n;
    if (p[0] < 0x80) { *len = 1; return p[0]; }
    if ((p[0] & 0xE0) == 0xC0)      { n = 2; cp = p[0] & 0x1F; }
    else if ((p[0] & 0xF0) == 0xE0) { n = 3; cp = p[0] & 0x0F; }
    else if ((p[0] & 0xF8) == 0xF0) { n = 4; cp = p[0] & 0x07; }
    else { *len = 1; return p[0]; }
    for (int i = 1; i < n; i++) {
        if ((p[i] & 0xC0) != 0x80) { *len = 1; return p[0]; }
        cp = (cp << 6) | (uint32_t)(p[i] & 0x3F);
    }
    *len = n;
    return cp;
}

/* 整串是否含中文(决定用中文字间断行还是英文整词断行) */
static bool desc_has_wide(const char *s)
{
    while (*s) {
        int cl = 0;
        if (desc_cp_wide(desc_utf8_next(s, &cl))) return true;
        s += cl;
    }
    return false;
}

/* 把一段 [ps,pe) 切成词,返回词数(-1=词表溢出) */
static int desc_tokenize(const char *p, int s, int e, const lv_font_t *font, int32_t ls,
                         int32_t space_w, desc_tok_t *tk, int max_tok)
{
    int n = 0, i = s, gap = 0;
    while (i < e) {
        int cl = 0;
        uint32_t cp = desc_utf8_next(p + i, &cl);
        if (cp == ' ') { gap += cl; i += cl; continue; }
        if (n >= max_tok) return -1;
        int st = i, len = 0;
        uint32_t last_cp = cp;
        if (desc_cp_wide(cp)) {
            len = cl;
            i += cl;
        } else {                       /* 拉丁/数字连成一词: 1.5-tablespoon、2.5cm、recipe. */
            while (i < e) {
                int cl2 = 0;
                uint32_t c2 = desc_utf8_next(p + i, &cl2);
                if (c2 == ' ' || desc_cp_wide(c2)) break;
                last_cp = c2;
                len += cl2;
                i += cl2;
            }
        }
        tk[n].off   = (int16_t)(st - s);
        tk[n].len   = (int16_t)len;
        tk[n].w     = (int16_t)lv_text_get_width(p + st, (uint32_t)len, font, ls);
        tk[n].sp_w  = (int16_t)(gap * space_w);
        tk[n].first = cp;
        tk[n].last  = last_cp;
        tk[n].punct = desc_cp_close(last_cp) ? 1 : 0;
        tk[n].space = 0;
        if (gap > 0 && n > 0) tk[n - 1].space = 1;
        gap = 0;
        n++;
    }
    return n;
}

static int32_t desc_line_w(const desc_tok_t *tk, int b, int e, int32_t ls)
{
    int32_t w = tk[b].w;
    for (int k = b + 1; k <= e; k++) w += tk[k].sp_w + tk[k].w + (ls > 0 ? ls : 0);
    return w;
}

/* 贪心断行:标点优先 → 空格优先 → 装满
 * allow = 为落到标点/空格最多回退几个词(中文 2/1/0 逐级降,英文恒 0 不丢词)
 * need_out 回填“要消掉行首标点还差多少像素”(0=没有行首标点)
 * push=true 时不加宽,直接把上一行末词推下来保证行首不是标点 */
static int desc_pack(const desc_tok_t *tk, int ntok, int32_t max_w, int32_t ls, bool cjk,
                     int allow, int16_t *lb, int16_t *le, int32_t *need_out, bool push)
{
    int i = 0, L = 0;
    int32_t need = 0;
    if (!cjk) allow = 0;               /* 英文按整词断,不回退丢词 */
    while (i < ntok && L < DESC_TOK_MAX) {
        int32_t w = tk[i].w;
        int fit = i;
        for (int k = i + 1; k < ntok; k++) {
            int32_t add = tk[k].sp_w + tk[k].w + (ls > 0 ? ls : 0);
            if (w + add > max_w) break;
            w += add;
            fit = k;
        }
        int end = fit;
        for (int k = fit; k > i && (fit - k) <= allow; k--) {
            if (tk[k].punct) { end = k; break; }
        }
        if (end == fit && cjk && allow > 0) {   /* 其次:空格处(如“第 3 層”这类短语边界) */
            for (int k = fit; k > i && (fit - k) <= allow; k--) {
                if (tk[k].space) { end = k; break; }
            }
        }
        if (end + 1 < ntok && desc_cp_close(tk[end + 1].first)) {
            int32_t full = desc_line_w(tk, i, end, ls) + tk[end + 1].w + (ls > 0 ? ls : 0);
            if (full - max_w > need) need = full - max_w;
            if (push) {
                while (end > i && desc_cp_close(tk[end + 1].first)) end--;
            }
        }
        /* 别让下一行只剩“粉。”这种两字残句:把前一个字也挪下去,保住“糖粉。” */
        if (allow > 0 && end > i && end + 2 < ntok &&
            desc_cp_wide(tk[end + 1].first) && !desc_cp_close(tk[end + 1].first) &&
            tk[end + 1].space == 0 && desc_cp_close(tk[end + 2].first)) {
            end--;
            if (end + 1 < ntok && desc_cp_close(tk[end + 1].first)) end--;   /* 别把标点留行首 */
            if (end < i) end = i;
        }
        lb[L] = (int16_t)i;
        le[L] = (int16_t)end;
        L++;
        i = end + 1;
    }
    *need_out = need;
    return L;
}

/* 当前盒宽下 LVGL 自动折行的行数(作为“不许变高”的预算) */
static int desc_lvgl_lines(const char *txt, const lv_font_t *font, int32_t ls, int32_t max_w)
{
    int n = 0;
    const char *p = txt;
    while (*p && n < 250) {
        int32_t used = 0;
        uint32_t adv = _lv_text_get_next_line(p, font, ls, max_w, &used, LV_TEXT_FLAG_NONE);
        if (adv == 0) break;
        p += adv;
        n++;
    }
    return n;
}

/* 按 width 把整串重排进 buf,返回总行数(-1=溢出) */
static int desc_build(const char *txt, const lv_font_t *font, int32_t ls, int32_t space_w,
                      int32_t width, bool cjk, int allow, bool push, char *buf, int bufsz,
                      int32_t *need_out)
{
    int out = 0, total = 0;
    int32_t need_max = 0;
    int i = 0;
    int n = (int)strlen(txt);
    if (n + 2 >= bufsz) return -1;

    while (1) {
        int ps = i;
        while (i < n && txt[i] != '\n') i++;
        int pe = i;

        int ntok = desc_tokenize(txt, ps, pe, font, ls, space_w, s_tok, DESC_TOK_MAX);
        if (ntok < 0) return -1;

        if (ntok <= 1) {                     /* 空行/单字:原样搬 */
            int len = pe - ps;
            if (out + len + 1 >= bufsz) return -1;
            memcpy(buf + out, txt + ps, (size_t)len);
            out += len;
            total += 1;
        } else {
            int32_t need = 0;
            int L = desc_pack(s_tok, ntok, width, ls, cjk, allow, s_lb, s_le, &need, push);
            if (need > need_max) need_max = need;

            /* 孤行:末行太短就把上一行末尾的词挪下来(不能把标点挪到行首) */
            if (L >= 2 && s_le[L - 2] - s_lb[L - 2] >= 2) {
                int32_t wl = desc_line_w(s_tok, s_lb[L - 1], s_le[L - 1], ls);
                int first = s_lb[L - 1];
                for (int moved = 0; moved < 2 && wl * DESC_ORPHAN_DIV < width; moved++) {
                    int cand = s_le[L - 2];
                    if (cand <= s_lb[L - 2]) break;          /* 上一行至少留一个词 */
                    if (s_tok[cand].punct) break;            /* 标点不许落行首 */
                    int32_t add = s_tok[first].sp_w + s_tok[cand].w + (ls > 0 ? ls : 0);
                    if (wl + add > width) break;
                    wl += add;
                    s_le[L - 2]--;
                    s_lb[L - 1] = (int16_t)cand;
                }
            }

            for (int k = 0; k < L; k++) {
                int a = ps + s_tok[s_lb[k]].off;
                int b = ps + s_tok[s_le[k]].off + s_tok[s_le[k]].len;
                if (out + (b - a) + 2 >= bufsz) return -1;
                memcpy(buf + out, txt + a, (size_t)(b - a));
                out += b - a;
                if (k + 1 < L) buf[out++] = '\n';
            }
            total += L;
        }

        if (i < n) { buf[out++] = '\n'; i++; }
        else break;
    }
    buf[out] = '\0';
    *need_out = need_max;
    return total;
}

/* 核心:在 [base_w, wlimit] 内把 txt 排好,返回行数;*out_w 回填最终盒宽(-1=放弃)
 * allow 从 2 降到 0:优先保住“不变高”,再尽量让行尾落在标点处 */
static int desc_layout(const char *txt, const lv_font_t *font, int32_t ls,
                       int32_t base_w, int32_t wlimit, bool cjk,
                       char *buf, int bufsz, int32_t *out_w)
{
    int32_t space_w = lv_text_get_width(" ", 1, font, ls);
    int last_total = -1;
    int32_t last_w = base_w;
    for (int allow = 2; allow >= 0; allow--) {
        int32_t width = base_w, need = 0;
        int total = desc_build(txt, font, ls, space_w, width, cjk, allow, false,
                               buf, bufsz, &need);
        if (total <= 0) return -1;
        if (need > 0 && width + need + 1 <= wlimit) {   /* 行首标点:先加宽一点再排 */
            width += need + 1;
            total = desc_build(txt, font, ls, space_w, width, cjk, allow, false,
                               buf, bufsz, &need);
        }
        if (total > 0 && need > 0) {                    /* 加不动了:把上一行末词推下来 */
            total = desc_build(txt, font, ls, space_w, width, cjk, allow, true,
                               buf, bufsz, &need);
        }
        if (total <= 0) return -1;
        last_total = total;
        last_w = width;
        if (total <= desc_lvgl_lines(txt, font, ls, width)) {   /* 不变高:采用 */
            *out_w = width;
            return total;
        }
    }
    *out_w = last_w;
    return last_total;
}

static void desc_reflow(lv_obj_t *lbl)
{
    if (!lbl) return;
    if (lv_label_get_long_mode(lbl) != LV_LABEL_LONG_WRAP) return;

    const char *txt = lv_label_get_text(lbl);
    if (!txt || txt[0] == '\0') return;

    const lv_font_t *font = lv_obj_get_style_text_font(lbl, LV_PART_MAIN);
    int32_t ls = lv_obj_get_style_text_letter_space(lbl, LV_PART_MAIN);
    lv_obj_update_layout(lbl);            /* 先落一次版面,宽度才是生成稿的 894 */
    int32_t base_w = lv_obj_get_width(lbl);
    if (base_w <= 0) return;

    /* 英文说明页最终盒宽由 lang_tune 定(宽菜 1100,其余 930),这里先用最终值排,
     * 否则按 894 排好再被 tune 放宽,会变成一排短行 */
    int32_t width = is_english() ? (six_desc_is_wide_dish() ? 1100 : 930) : base_w;
    int32_t wlimit = width + DESC_WIDEN_MAX;
    lv_obj_t *par = lv_obj_get_parent(lbl);
    if (par) {                            /* 不越过卡片内容宽 */
        int32_t pw = lv_obj_get_content_width(par);
        if (pw > 0 && pw < wlimit) wlimit = pw;
    }

    bool cjk = desc_has_wide(txt);
    int32_t out_w = width;
    int total = desc_layout(txt, font, ls, width, wlimit, cjk,
                            s_reflow, DESC_BUF_SIZE, &out_w);
    if (total <= 0) return;

    int auto_n = desc_lvgl_lines(txt, font, ls, out_w);
    if (total > auto_n) return;           /* 会变高:整体放弃,保持 LVGL 自动折行 */

    if (out_w != base_w) lv_obj_set_width(lbl, out_w);
    if (strcmp(s_reflow, txt) == 0) return;
    lv_label_set_text(lbl, s_reflow);
    lv_obj_update_layout(lbl);
}

static void descriptionmenu_layout(descriptionmenu_t *dm)
{
    if (!dm || !dm->container_1) return;

    /* 左上角菜名(六感薄饼繁体"薄餅"改由词典统一,2026-11-05) */
    if (dm->label_19) {
        lv_label_set_text(dm->label_19, six_current_name());
    }

    /* 摘要：探针菜/程度→时间显示烧烤程度；烤鸡翅类显示份量；面包有发酵显示发酵小结；其余隐藏 */
    if (dm->summary) {
        if (six_chick_is_probe()) {
            lv_obj_clear_flag(dm->summary, LV_OBJ_FLAG_HIDDEN);
            if (six_chick_is_matdeg())
                lv_label_set_text_fmt(dm->summary, tr("小结：\n熟度：%s\n烧烤程度：%s\n"),
                                      six_2d_mat_text(), six_2d_deg_text());   /* 牛肉/羊腿/羊排:熟度+程度 */
            else if (g_six_bread_type == SIX_MEAT_GRILL_STEAK)
                lv_label_set_text_fmt(dm->summary, tr("小结：\n熟度：%s\n"),
                                      six_chick_degree_short());                     /* 烤牛排:熟度 */
            else
                lv_label_set_text_fmt(dm->summary, tr("小结：\n烧烤程度：%s\n"), six_chick_degree_short());   /* 烤全鸡/鸭 */
        } else if (six_chick_is_degree_time()) {
            int d = toastcolor_degree_value();
            const char *dt = (d == 1) ? tr("浅") : (d == 3) ? tr("深") : tr("中等");
            lv_obj_clear_flag(dm->summary, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->summary, tr("小结：\n烧烤程度：%s\n"), dt);
        } else if (six_chick_is_kind()) {
            int w = toastcolor_weight_value();
            if (w < 0) w = 800;   /* 兜底 */
            lv_obj_clear_flag(dm->summary, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->summary, tr("小结：\n份量：%d%s\n"), w, toastcolor_weight_unit_n(w));
        } else if (six_chick_is_pizza()) {
            lv_obj_clear_flag(dm->summary, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(dm->summary, tr("小结：\n份量：1份\n"));   /* 披萨:固定1份 */
        } else if (six_chick_is_2d()) {
            lv_obj_clear_flag(dm->summary, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->summary, tr("小结：\n份量：%dg\n烧烤程度：%s\n"),
                                  six_2d_weight(), six_2d_deg_text());   /* 二维菜:份量+程度 */
        } else if (six_bread_has_rising()) {
            lv_label_set_text(dm->summary, g_rising_choice == 1 ?
                              tr("小结：\n有发酵阶段\n") : tr("小结：\n没有发酵阶段\n"));
        } else {
            lv_obj_add_flag(dm->summary, LV_OBJ_FLAG_HIDDEN);
        }
    }

    /* 烹饪时间：探针菜不显示；程度→时间/份量按各自时间显示；其余原逻辑 */
    if (dm->cooktime) {
        if (six_chick_is_probe()) {
            lv_obj_add_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
        } else if (six_chick_is_degree_time()) {
            int d = toastcolor_degree_value();
            if (d < 1 || d > 3) d = 2;
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), six_chick_degree_min(d));
        } else if (six_chick_is_kind()) {
            int w = toastcolor_weight_value();
            if (w < 0) w = 800;
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), six_chick_cook_min(w));
        } else if (six_chick_is_seafood()) {
            const seafood_dish_t *sd = seafood_dish_cfg();
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), sd ? sd->cook_min : 18);

        } else if (six_chick_is_veg()) {
            const seafood_dish_t *sd = veg_fixed_cfg();
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), sd ? sd->cook_min : 22);
        } else if (six_chick_is_pizza()) {
            const seafood_dish_t *sd = pizza_fixed_cfg();
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), sd ? sd->cook_min : 23);

        } else if (six_chick_is_2d()) {
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), six_2d_cook_min());   /* 二维菜 */
        } else {
            lv_obj_clear_flag(dm->cooktime, LV_OBJ_FLAG_HIDDEN);
            /* 选了发酵:发酵分钟+烹饪分钟分开显示 */
            if (six_bread_has_rising() && g_rising_choice == 1)
                lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：发酵%d分钟+烹饪%d分钟"),
                                      six_rising_min(), six_bread_cook_min());
            else
                lv_label_set_text_fmt(dm->cooktime, tr("预计烹饪时间：%d分钟"), six_bread_cook_min());
        }
    }
    if (dm->cookdescriptin) {
        lv_label_set_text(dm->cookdescriptin, six_current_desc());
    }

    /* 容器 Flex 竖排：左/上边距 24，垂直间距 16 */
    lv_obj_set_flex_flow(dm->container_1, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_left(dm->container_1, 24, 0);
    lv_obj_set_style_pad_top(dm->container_1, 24, 0);
    lv_obj_set_style_pad_row(dm->container_1, 16, 0);

    /* Flex 按子对象顺序排列:调整顺序为 cooktime → cookdescriptin → summary */
    if (dm->cooktime) lv_obj_move_to_index(dm->cooktime, 0);
    if (dm->cookdescriptin) lv_obj_move_to_index(dm->cookdescriptin, 1);
    /* summary 自然落位 idx 2,不动 */

    /* 三标签：高度按内容自适应 + 行高对齐字号(30 号字体 line_height=35,负 5 对齐上位机) */
    lv_obj_t *labels[] = { dm->cooktime, dm->cookdescriptin, dm->summary };
    for (int k = 0; k < 3; k++) {
        if (!labels[k]) continue;
        lv_obj_set_height(labels[k], LV_SIZE_CONTENT);
        lv_obj_set_style_text_line_space(labels[k], -5, 0);
    }
    /* 二维菜(烤带皮土豆/千层面/卡内罗尼)说明文本最长:生成宽 894 折 5 行把小结顶到框底,
     * 说明标签放宽到容器内容宽(1233-24 左距=1209),折行减少给小结底部留距 */
    if (six_chick_is_2d() && dm->cookdescriptin)
        lv_obj_set_width(dm->cookdescriptin, 1209);
    /* 说明页断行重排:断在标点/词边界、不拆英文词与数字、末行不留孤字
     * (简/繁/英通用;行数超过自动折行就整体放弃,不会把标签顶高) */
    desc_reflow(dm->cookdescriptin);
    /* 探针菜:隐藏时间后，说明与小结整体上移顶对齐（隐藏项不占 flex 位） */
    if (six_chick_is_probe()) {
        if (dm->cookdescriptin) lv_obj_move_to_index(dm->cookdescriptin, 0);
        if (dm->summary) lv_obj_move_to_index(dm->summary, 1);
    }
    lv_obj_update_layout(dm->container_1);
}

void jump_to_descriptionmenu(void)
{
    page_push(PAGE_DESCRIPTIONMENU);
    lv_obj_clean(lv_scr_act());
    descriptionmenu_create(&ui_manager);

    descriptionmenu_t *dm = descriptionmenu_get(&ui_manager);
    if (dm) {
        descriptionmenu_layout(dm);

        lv_obj_t *btns[] = { dm->start, dm->delay };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_descriptionmenu) lv_group_del(g_descriptionmenu);
        g_descriptionmenu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (dm->start) {
            lv_obj_add_event_cb(dm->start, on_description_start_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(dm->start);
        }
        if (dm->delay) {
            lv_obj_add_event_cb(dm->delay, on_description_delay_click, LV_EVENT_CLICKED, NULL);
        }
    }
    current_group = g_descriptionmenu;

    lang_scr_load_anim(descriptionmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_desc] jump: descriptionmenu (choice=%d)\n", g_rising_choice);
}

void descriptionmenu_rebuild(page_id_t child)
{
    if (g_descriptionmenu) { lv_group_del(g_descriptionmenu); g_descriptionmenu = NULL; }
    lv_obj_clean(lv_scr_act());
    descriptionmenu_create(&ui_manager);

    descriptionmenu_t *dm = descriptionmenu_get(&ui_manager);
    if (dm) {
        descriptionmenu_layout(dm);

        lv_obj_t *btns[] = { dm->start, dm->delay };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_descriptionmenu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (dm->start) {
            lv_obj_add_event_cb(dm->start, on_description_start_click, LV_EVENT_CLICKED, NULL);
        }
        if (dm->delay) {
            lv_obj_add_event_cb(dm->delay, on_description_delay_click, LV_EVENT_CLICKED, NULL);
        }
        /* 从延迟设置返回时恢复焦点到“延迟”，否则默认“开始” */
        if (child == PAGE_DELAYSET && dm->delay)
            lv_group_focus_obj(dm->delay);
        else if (dm->start)
            lv_group_focus_obj(dm->start);
    }
    current_group = g_descriptionmenu;

    lang_scr_load_anim(descriptionmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_desc] rebuild: descriptionmenu\n");
}

static void on_description_start_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    if (six_chick_is_probe())
        jump_to_chick_cooking();   /* 探针菜(烤全鸡/烤全鸭):探针温度驱动烹饪页 */
    else
        jump_to_six_cooking();
}

static void on_description_delay_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_delayset();   /* 来源页自动记录为 PAGE_DESCRIPTIONMENU */
}
