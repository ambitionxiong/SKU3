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

/* 说明页盒宽；lang_tune 通过 six_desc_en_width() 取同一个值——"切行宽度 = 显示宽度"。
 * 2026-11-19 用户定：英文不再自动选宽，固定用卡片内容宽，折行交 LVGL，断点手写进词条；
 * 中/繁沿用 930(生成稿 894 需要时按行宽加宽)。 */
#define DESC_EN_W_MAX    1209   /* 卡片内容宽:容器 1233 - 左内距 24(英文说明页固定用) */
#define DESC_EN_W_FALL   930    /* 中/繁说明页盒宽(lang_tune 套用) */
static int32_t s_en_w = DESC_EN_W_FALL;

int32_t six_desc_en_width(void)
{
    return s_en_w;
}

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
static char       s_src[DESC_BUF_SIZE]; /* 原文副本:试排会改写标签文本,原指针会失效(2026-11-19) */

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

/* 一级断点(句末):。！？ . ! ?；其余标点(，、；：,;:)算二级断点 */
static bool desc_cp_period(uint32_t cp)
{
    return cp == 0x3002 || cp == 0xFF01 || cp == 0xFF1F ||
           cp == '.' || cp == '!' || cp == '?';
}

/* 断行:一律断在标点处,能整句就整句
 *  ① 基本宽 max_w 内最后一个句号(且本行不太空) → 断
 *  ② 其次基本宽内最后一个标点(逗号/顿号/分号…)
 *  ③ 基本宽内一个标点都没有 → 加宽到框内(wlimit)第一个标点
 *  ④ 框内也没有标点 → 按基本宽铺满(极少数超长无标点句)
 * plain=true 时退化成纯铺满(LVGL 自动折行同款),行数超标时兜底用。
 * maxw_out 回填最长行宽,调用方据此把标签加宽(只要不超过框)。 */
static int desc_pack(const desc_tok_t *tk, int ntok, int32_t max_w, int32_t wlimit,
                     int32_t ls, bool plain, int merge, int16_t *lb, int16_t *le,
                     int32_t *maxw_out)
{
    int i = 0, L = 0;
    int32_t maxw = 0;
    int merge_left = merge;            /* 只在第一行生效:前 merge 个句号不换行 */
    while (i < ntok && L < DESC_TOK_MAX) {
        int32_t w = tk[i].w;
        int fit = i;
        int punct_in = -1, punct_beyond = -1;
        int first_per = -1;
        int per_seen = 0;
        for (int k = i + 1; k < ntok; k++) {
            int32_t add = tk[k].sp_w + tk[k].w + (ls > 0 ? ls : 0);
            int32_t nw = w + add;
            bool merging = (merge_left > 0);        /* 合并行:越过基本宽也要把第 N+1 句读进来 */
            if (nw > wlimit && !merging) break;
            if (nw <= max_w) {
                fit = k;
                if (tk[k].punct) punct_in = k;
            } else if (!merging && punct_beyond < 0 && tk[k].punct) {
                punct_beyond = k;
            }
            /* 中文(plain=false)按句断:扫到第 (merge+1) 个句号就停,整句一行。
             * 英文(plain=true)不参与:否则扫描被句号提前截断,"Coat" 这种放得下的词
             * 也不再装进本行,行数凭空多一行(2026-11-18 炸鸡中翅 Summary 被顶出面板) */
            if (!plain && tk[k].punct && desc_cp_period(tk[k].last)) {
                per_seen++;
                if (per_seen == merge_left + 1) { first_per = k; break; }
            }
            w = nw;
        }
        int end = fit;
        if (!plain) {
            /* ① 一句一行:框内第 (merge+1) 个句号(前 merge 句并到同一行) */
            if (first_per >= 0) end = first_per;
            /* ② 这一句长到框里放不下:退到句内最后一个逗号,下一行接着这句 */
            else if (punct_in >= 0) end = punct_in;
            /* ③ 基本宽内没标点:加宽到框内第一个标点 */
            else if (punct_beyond >= 0) end = punct_beyond;
        }
        if (end < i) end = i;
        int32_t lw = desc_line_w(tk, i, end, ls);
        if (lw > maxw) maxw = lw;
        merge_left = 0;                /* 只有第一行合并 */
        lb[L] = (int16_t)i;
        le[L] = (int16_t)end;
        L++;
        i = end + 1;
    }
    *maxw_out = maxw;
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

/* 按 max_w 断行、允许加宽到 wlimit;返回总行数(-1=溢出) */
static int desc_build(const char *txt, const lv_font_t *font, int32_t ls, int32_t space_w,
                      int32_t max_w, int32_t wlimit, bool plain, int merge,
                      char *buf, int bufsz, int32_t *maxw_out)
{
    int out = 0, total = 0;
    int32_t maxw = 0;
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
            if (out + len + 2 >= bufsz) return -1;
            memcpy(buf + out, txt + ps, (size_t)len);
            out += len;
            total += 1;
            if (len > 0) {
                int32_t lw = lv_text_get_width(txt + ps, (uint32_t)len, font, ls);
                if (lw > maxw) maxw = lw;
            }
        } else {
            int32_t pmax = 0;
            int L = desc_pack(s_tok, ntok, max_w, wlimit, ls, plain, merge, s_lb, s_le, &pmax);
            if (pmax > maxw) maxw = pmax;
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
    *maxw_out = maxw;
    return total;
}

/* 说明标签在卡片里还剩多少可用高度(用于判断多折一行会不会顶到框底) */
static int32_t desc_avail_h(lv_obj_t *lbl)
{
    lv_obj_t *par = lv_obj_get_parent(lbl);
    if (!par) return 0;
    lv_obj_update_layout(par);
    int32_t ch = lv_obj_get_content_height(par);
    int32_t pad_row = lv_obj_get_style_pad_row(par, LV_PART_MAIN);
    int32_t used = 0;
    int vis = 0;
    uint32_t n = lv_obj_get_child_count(par);
    for (uint32_t k = 0; k < n; k++) {
        lv_obj_t *c = lv_obj_get_child(par, k);
        if (!c || c == lbl) continue;
        if (lv_obj_has_flag(c, LV_OBJ_FLAG_HIDDEN)) continue;
        used += lv_obj_get_height(c);
        vis++;
    }
    return ch - used - pad_row * vis;
}

/* ==============================
 * 逐页断行微调（2026-11-18 用户逐页截图）
 * ------------------------------
 * key    = 该页文本里一段唯一子串
 * merge  = 该页第一段前 merge 个句号不换行（把几句并成一行）
 * 命中后若这一行放不进卡片框，会逐级收紧字距（-1/-2）再试。
 * ============================== */
typedef struct {
    const char *key;
    int merge;
} desc_merge_t;

static const desc_merge_t s_desc_merge[] = {
    { "在焗盤上擠出約2吋大小的圓形",        2 },   /* 泡芙：前 3 句并一行 */
    { "將麵團分成等份小球",                 1 },   /* 蛋撻：前 2 句并一行 */
    { "將酥皮放上焗盤並灑上糖霜",           1 },   /* 千層酥：前 2 句并一行 */
    { "平鋪在氣炸盤上，並在表面輕輕噴上一層煮食油", 1 },   /* 炸排骨：前 2 句并一行 */
    { "按個人喜好灑上鹽和燒烤調味料",       2 },   /* 燜薯片：前 3 句并一行 */
    { "按個人喜好加入蒜蓉醬、鹽和油調味",   1 },   /* 烤茄子：前 2 句并一行 */
    { "將忌廉均勻倒在表面，並灑上芝士",     1 },   /* 法式燜薯仔：前 2 句并一行 */
    { "把鸡翅裹上蛋液，然后裹上面包屑",     1 },   /* 炸雞翼（简中词条）：前 2 句并一行 */
    { "把鸡腿裹上蛋液，然后裹上面包屑",     1 },   /* 炸雞髀（简中词条）：前 2 句并一行 */
    { "將雞翼沾上蛋液，然後裹上麵包糠",     1 },   /* 炸雞翼（繁中词条）：前 2 句并一行(2026-11-19) */
    { "将2英寸的面团放到烤盘上",                 2 },   /* 泡芙（简中）：前 3 句并一行 */
    { "把面团分成相等的球",                       1 },   /* 蛋挞（简中）：前 2 句并一行 */
    { "将酥皮放在烤盘上，撒上糖粉",               1 },   /* 千层酥（简中）：前 2 句并一行 */
    { "根据喜好用蒜蓉酱、盐和油调味",             1 },   /* 烤茄子（简中）：前 2 句并一行 */
    { "去掉玉米的皮和须",                         2 },   /* 烤玉米（简中）：前 3 句并一行 */
    { "在放入烤箱之前，用油刷皮",                 1 },   /* 烤带皮土豆（简中）：前 2 句并一行 */
    { "把奶油均匀地倒在上面，撒上奶酪",           1 },   /* 奶油焗土豆（简中）：前 2 句并一行 */
    { "顶部浇上白浆和奶酪碎",                     1 },   /* 千层面/卡内罗尼（简中）：前 2 句并一行 */
    /* 2026-11-19 用户「把简体中文模式做了合并行的地方同步到繁体中文」：
     * 简中已合并行的 10 页里,泡芙/蛋撻/千層酥/烤茄子/法式燜薯仔 5 页的繁中键本来就有,
     * 炸雞翼 当天单独补过,剩下这 4 页的繁中键补齐(合并句数与简中一一对应)。 */
    { "將雞髀沾上蛋液，然後裹上麵包糠",           1 },   /* 炸雞髀（繁中）：前 2 句并一行 */
    { "去除粟米葉和粟米鬚",                       2 },   /* 烤玉米（繁中）：前 3 句并一行 */
    { "用叉在薯仔表面戳",                         1 },   /* 烤帶皮土豆（繁中）：前 2 句并一行 */
    { "在表面淋上白汁並灑上芝士",                 1 },   /* 千層面/卡內羅尼（繁中）：前 2 句并一行 */
};

static int desc_merge_of(const char *txt)
{
    for (unsigned i = 0; i < sizeof(s_desc_merge) / sizeof(s_desc_merge[0]); i++)
        if (strstr(txt, s_desc_merge[i].key)) return s_desc_merge[i].merge;
    return 0;
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

    /* 2026-11-19 用户定：英文说明页不再由程序自己排版——只把盒子放宽到卡片内容宽(1209)，
     * 折行交给 LVGL 自动折行；哪里换行由用户逐页指定，直接写进 i18n 英文词条的 \n。
     * 比算法断行准(算法在英文上总有"末行只剩一个词""句子从中间被切开"的观感)，
     * 也顺带避开试排改写标签文本带来的副作用(见 11.19.1)。中/繁仍走下面的按页合并+断行。 */
    if (is_english()) {
        s_en_w = DESC_EN_W_MAX;
        lv_obj_set_width(lbl, DESC_EN_W_MAX);
        lv_obj_update_layout(lbl);
        return;
    }

    /* 简体/繁体：量宽字体就是渲染字体(lang_refresh_screen 只在英文模式换 aktiv) */
    const lv_font_t *mfont = font;

    /* 2026-11-19 原文副本：下面虽然不再改写标签文本，仍统一用副本量宽，
     * 标签被谁改写过都不影响这次切行的输入(11.19.1 的悬空指针问题就此绝迹)。 */
    if (strlen(txt) + 1 > sizeof(s_src)) return;   /* 超长(理论上不会):放弃重排,保持自动折行 */
    strcpy(s_src, txt);
    txt = s_src;

    s_en_w = DESC_EN_W_FALL;              /* 英文在上面早退时改成 1209；中繁沿用 930 */

    bool cjk = desc_has_wide(txt);
    int32_t space_w = lv_text_get_width(" ", 1, mfont, ls);
    int merge = desc_merge_of(txt);

    /* 中文保持生成稿宽(894)，需要时按行宽加宽，但不超过卡片内容宽 */
    int32_t width = base_w;
    int32_t wlimit = base_w;
    {
        lv_obj_t *par = lv_obj_get_parent(lbl);
        if (par) {
            int32_t pw = lv_obj_get_content_width(par);
            if (pw > wlimit) wlimit = pw;
        }
    }

    int32_t maxw = 0;
    bool punct_mode = cjk;                 /* 中文按标点断;英文保持整词铺满 */
    int32_t ls_use = ls;
    int total = desc_build(txt, mfont, ls_use, space_w, width, wlimit, !punct_mode, merge,
                           s_reflow, DESC_BUF_SIZE, &maxw);
    while (merge > 0 && maxw > wlimit && ls_use - ls > -3) {  /* 合并后一行放不下:收紧字距 */
        ls_use--;
        total = desc_build(txt, mfont, ls_use, space_w, width, wlimit, !punct_mode, merge,
                           s_reflow, DESC_BUF_SIZE, &maxw);
    }
    if (ls_use != ls) lv_obj_set_style_text_letter_space(lbl, ls_use, 0);
    if (total <= 0) return;
    int32_t new_w = (maxw > width) ? maxw : width;
    if (new_w > wlimit) new_w = wlimit;

    int auto_n = desc_lvgl_lines(txt, mfont, ls, new_w);
    if (total > auto_n) {                 /* 折行变多:框里还有余量就留,没有就退回铺满 */
        int32_t avail = desc_avail_h(lbl);
        int32_t lh = lv_font_get_line_height(mfont) +
                     lv_obj_get_style_text_line_space(lbl, LV_PART_MAIN);
        if (avail > 0 && total * lh > avail && total <= auto_n + 2) avail = total * lh;  /* 最多容许多 2 行 */
        if (avail <= 0 || total * lh > avail) {
            total = desc_build(txt, mfont, ls_use, space_w, width, width, true, 0,
                               s_reflow, DESC_BUF_SIZE, &maxw);
            if (total <= 0) return;
            new_w = width;
        }
    }

    lv_obj_set_width(lbl, new_w);
    /* 2026-11-19:要和"标签此刻的文本"比,不能和原文比——试排已经把标签换成某一档的切法了,
     * 若最终文本恰好等于原文(短页常见)就会跳过写回,标签留着试排那一档的切法配新宽度。 */
    {
        const char *cur = lv_label_get_text(lbl);
        if (!cur || strcmp(s_reflow, cur) != 0) lv_label_set_text(lbl, s_reflow);
    }
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
