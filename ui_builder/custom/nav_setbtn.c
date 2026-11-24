/*
 * nav_setbtn.c - 烹饪族页面"设置"齿轮钮横向贴靠
 *
 * 2026-11-24 用户:这颗钮的位置改成动态——齿轮图标离左侧标题文字 48px，
 * 所有带这颗钮的页面都按同一条规则贴。
 *
 * 生成层把这颗钮钉死在 (609,170)(50x43，底图 little.png 33x34 居中)：
 * 标题一换(烹饪中/暂停中/已完成/保温中/预约中，EN 还有 Cooking.../Paused.../
 * Completed/Keep Warm.../Delayed Start...)文字宽度从 180 到 400+ 不等，
 * 视觉间距就从 20 飘到 100+。这里不再依赖固定 x，进页(含语言热切)、
 * 以及完成页"已完成↔保温中"这种常驻时改标题的场合，都按当前标题实测重算:
 *
 *   钮 x = 标题文字墨迹右缘 + 48 - 底图左留白
 *   底图左留白 = 钮宽/2 - 图宽/2
 *     (LVGL 背景图在对象内 LV_ALIGN_CENTER，见 lv_draw_rect.c 的 lv_area_align，
 *      50 宽钮内 33 宽图 → 左留白 9px；这 9px 补掉，屏幕上量到的才是 48)
 *
 * "文字墨迹右缘"按字形盒算(max(笔位 + ofs_x + box_w))，不用 advance 宽度:
 * 标题多以 "..." 收尾，advance 里带着末字的右侧留白，直接用会多出 3-4px。
 *
 * 一屏五态的页(多段烹饪 烹饪/暂停/确认/完成/编辑)标题在页内改，进页那一次贴完
 * 还得跟着改——nav_setbtn_refit_on(屏) 供这类页在状态落地后自己重贴。
 *
 * 判定不看生成层成员名(各页 little / littlebutton / little_button / button_43 …
 * 五花八门)，只看结构特征：按钮 + DEFAULT 态底图 little.png + 50x43；
 * 标题取"同一条水平带上、钮左侧、墨迹右缘最靠右"的标签——全工程 85 屏
 * (烹饪/暂停/返回确认/完成 四类 × 21 种模式 + 探针变体)都只命中一个。
 *
 * 顺带：标题盒装不下一整行时按内容放开宽度(WRAP 会在空格处折行、第二行被 60 高盒裁掉,
 * "Keep Warm..."/"Delayed Start..." 这类带空格的 EN 长标题都靠这一步保一行)。
 */
#include "protocol.h"
#include "nav.h"
#include <string.h>
#include <stdio.h>

#define SETBTN_GAP        48          /* 标题文字墨迹右缘 → 齿轮图标墨迹左缘 */
#define SETBTN_IMG_TAIL   "little.png"

/* UTF-8 取一个字符(只认首字节定长，够用: 界面文本都是合法 UTF-8) */
static uint32_t setbtn_utf8_next(const char *s, size_t len, size_t *i)
{
    const unsigned char *p = (const unsigned char *)s;
    uint32_t cp;
    size_t n;

    if (p[*i] < 0x80)             { cp = p[*i];         n = 1; }
    else if ((p[*i] & 0xE0) == 0xC0) { cp = p[*i] & 0x1F; n = 2; }
    else if ((p[*i] & 0xF0) == 0xE0) { cp = p[*i] & 0x0F; n = 3; }
    else                          { cp = p[*i] & 0x07; n = 4; }
    if (*i + n > len) n = 1;

    for (size_t k = 1; k < n; k++)
        cp = (cp << 6) | (p[*i + k] & 0x3F);
    *i += n;
    return cp;
}

/* 一行: 返回墨迹右缘(相对行首)，adv_out 返回 advance 宽度(与 lv_txt_get_width 同口径) */
static int32_t setbtn_line_ink(const char *s, size_t len, const lv_font_t *f,
                               int32_t ls, int32_t *adv_out)
{
    size_t i = 0;
    int32_t pen = 0, ink = 0, adv = 0;

    while (i < len) {
        size_t idx = i;
        uint32_t letter = setbtn_utf8_next(s, len, &idx);
        uint32_t letter_next = 0;
        if (idx < len) {
            size_t j = idx;
            letter_next = setbtn_utf8_next(s, len, &j);
        }

        lv_font_glyph_dsc_t g;
        if (lv_font_get_glyph_dsc(f, &g, letter, letter_next)) {
            int32_t r = pen + g.ofs_x + (int32_t)g.box_w;
            if (r > ink) ink = r;
        }
        adv = pen + (int32_t)lv_font_get_glyph_width(f, letter, letter_next);
        pen = adv + ls;
        i = idx;
    }

    if (adv_out) *adv_out = adv;
    return ink;
}

/* 标签按行取最宽一行: 返回墨迹宽，*adv_out 返回该行 advance 宽(居中/右对齐用) */
static int32_t setbtn_text_ink(lv_obj_t *lbl, int32_t *adv_out)
{
    const char *t = lv_label_get_text(lbl);
    if (adv_out) *adv_out = 0;
    if (!t || !t[0]) return 0;

    const lv_font_t *f = lv_obj_get_style_text_font(lbl, LV_PART_MAIN);
    int32_t ls = lv_obj_get_style_text_letter_space(lbl, LV_PART_MAIN);
    int32_t best_ink = 0, best_adv = 0;
    const char *p = t;

    for (;;) {
        const char *e = strchr(p, '\n');
        size_t len = e ? (size_t)(e - p) : strlen(p);
        int32_t adv = 0;
        int32_t ink = setbtn_line_ink(p, len, f, ls, &adv);
        if (ink > best_ink) { best_ink = ink; best_adv = adv; }
        if (!e) break;
        p = e + 1;
    }

    if (adv_out) *adv_out = best_adv;
    return best_ink;
}

/* 标签文字墨迹右缘(盒坐标 + 实测墨迹宽，含居中/右对齐按 advance 宽算的偏移) */
static int32_t setbtn_text_right(lv_obj_t *lbl)
{
    int32_t x  = lv_obj_get_x(lbl);
    int32_t bw = lv_obj_get_width(lbl);
    int32_t adv = 0;
    int32_t ink = setbtn_text_ink(lbl, &adv);
    if (ink <= 0) return x;

    lv_text_align_t al = lv_obj_get_style_text_align(lbl, LV_PART_MAIN);
    if (al == LV_TEXT_ALIGN_CENTER)     x += (bw - adv) / 2;
    else if (al == LV_TEXT_ALIGN_RIGHT) x += bw - adv;
    return x + ink;
}

/* 找齿轮钮：按钮 + DEFAULT 态底图 little.png(读本地样式，不受当前聚焦态影响) */
static lv_obj_t *setbtn_find(lv_obj_t *scr)
{
    uint32_t n = lv_obj_get_child_count(scr);
    size_t tl = strlen(SETBTN_IMG_TAIL);

    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *c = lv_obj_get_child(scr, i);
        if (!c || !lv_obj_check_type(c, &lv_button_class)) continue;
        if (lv_obj_has_flag(c, LV_OBJ_FLAG_HIDDEN)) continue;   /* 延迟取消等场合会藏掉 */

        lv_style_value_t v;
        if (lv_obj_get_local_style_prop(c, LV_STYLE_BG_IMAGE_SRC, &v,
                                        LV_PART_MAIN | LV_STATE_DEFAULT) != LV_STYLE_RES_FOUND)
            continue;
        if (!v.ptr || lv_image_src_get_type(v.ptr) != LV_IMAGE_SRC_FILE) continue;
        const char *s = (const char *)v.ptr;
        size_t sl = strlen(s);
        if (sl < tl || strcmp(s + sl - tl, SETBTN_IMG_TAIL) != 0) continue;
        return c;
    }
    return NULL;
}

/* 标题：同一条水平带上、钮左侧、墨迹右缘最靠右的那个标签 */
static lv_obj_t *setbtn_find_title(lv_obj_t *scr, lv_obj_t *btn)
{
    int32_t bx = lv_obj_get_x(btn);
    int32_t by = lv_obj_get_y(btn);
    int32_t bh = lv_obj_get_height(btn);
    lv_obj_t *best = NULL;
    int32_t best_r = -100000;
    uint32_t n = lv_obj_get_child_count(scr);

    for (uint32_t i = 0; i < n; i++) {
        lv_obj_t *c = lv_obj_get_child(scr, i);
        if (!c || !lv_obj_check_type(c, &lv_label_class)) continue;
        if (lv_obj_has_flag(c, LV_OBJ_FLAG_HIDDEN)) continue;
        if (lv_obj_get_x(c) >= bx) continue;                  /* 只看钮左边的文字 */

        int32_t cy = lv_obj_get_y(c), ch = lv_obj_get_height(c);
        if (cy + ch <= by || cy >= by + bh) continue;         /* 竖直不搭界(状态行在下面) */

        int32_t r = setbtn_text_right(c);
        if (r > best_r) { best_r = r; best = c; }
    }
    return best;
}

/* 底图在钮内居中 → 左留白(与 LVGL bg image 的 LV_ALIGN_CENTER 同口径) */
static int32_t setbtn_img_pad(lv_obj_t *btn)
{
    lv_style_value_t v;
    if (lv_obj_get_local_style_prop(btn, LV_STYLE_BG_IMAGE_SRC, &v,
                                    LV_PART_MAIN | LV_STATE_DEFAULT) != LV_STYLE_RES_FOUND ||
        !v.ptr || lv_image_src_get_type(v.ptr) != LV_IMAGE_SRC_FILE)
        return 0;

    lv_image_header_t hdr;
    if (lv_image_decoder_get_info(v.ptr, &hdr) != LV_RESULT_OK)
        return 0;

    int32_t pad = lv_obj_get_width(btn) / 2 - (int32_t)hdr.w / 2;
    return pad > 0 ? pad : 0;
}

/* 指定屏贴一次(幂等；没有这颗钮的页面直接返回)。
 * 不取 lv_scr_act():同页多态页(多段烹饪 五态)标题在页内改,改完要按自己那张屏算,
 * 这时屏可能还没激活/或者算的是别的屏。 */
void nav_setbtn_refit_on(lv_obj_t *scr)
{
    if (!scr) return;

    lv_obj_t *btn = setbtn_find(scr);
    if (!btn) return;

    lv_obj_t *title = setbtn_find_title(scr, btn);
    if (!title) return;

    /* 标题盒放不下整行就按内容放开:WRAP 模式会在空格处折行,第二行被 60 高盒裁掉
       (2026-11-24:EN 标题补上省略号后 "Keep Warm..."≈365px 撑破 330 宽的盒,
       渲染成 "Keep"/"Warm..." 两行;带空格的 "Delayed Start..." 同理)。
       只对单行标题生效(含 \n 的不动),宽度放开后齿轮仍按文字墨迹算,位置不受影响。 */
    if (lv_label_get_long_mode(title) == LV_LABEL_LONG_WRAP) {
        const char *t = lv_label_get_text(title);
        int32_t adv = 0;
        (void)setbtn_text_ink(title, &adv);
        if (t && !strchr(t, '\n') && adv > lv_obj_get_width(title))
            lv_obj_set_width(title, LV_SIZE_CONTENT);
    }

    int32_t x = setbtn_text_right(title) + SETBTN_GAP - setbtn_img_pad(btn);
    if (x == lv_obj_get_x(btn)) return;

    printf("[setbtn] \"%s\" ink-right=%d -> btn x %d => %d\n",
           lv_label_get_text(title), (int)setbtn_text_right(title),
           (int)lv_obj_get_x(btn), (int)x);
    lv_obj_set_x(btn, x);
}

/* 当前页贴一次(进页/语言热切走 lang_on_page_built) */
void nav_setbtn_refit(void)
{
    nav_setbtn_refit_on(lv_scr_act());
}
