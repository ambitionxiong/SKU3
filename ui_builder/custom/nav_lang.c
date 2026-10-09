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
 * 素材缺失的图(show)不在表内, 繁體暂显示简体版, 出图后在表里补一行即可。 */
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
    { "steptext.png",         LVGL_IMAGE_PATH(steptext.png),         LVGL_IMAGE_PATH(steptext_tw.png) },
    { "hotcare.png",          LVGL_IMAGE_PATH(hotcare.png),          LVGL_IMAGE_PATH(hotcare_tw.png) },
    { "modebg.png",           LVGL_IMAGE_PATH(modebg.png),           LVGL_IMAGE_PATH(modebg_tw.png) },
    { "tips.png",             LVGL_IMAGE_PATH(tips.png),             LVGL_IMAGE_PATH(tips_tw.png) },
    { "hotcleantips.png",     LVGL_IMAGE_PATH(hotcleantips.png),     LVGL_IMAGE_PATH(hotcleantips_tw.png) },
    { "waterbg.png",          LVGL_IMAGE_PATH(waterbg.png),          LVGL_IMAGE_PATH(waterbg_tw.png) },
    { "delaytext.png",        LVGL_IMAGE_PATH(delaytext.png),        LVGL_IMAGE_PATH(delaytext_tw.png) },
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
     * "小时"=6字节→" h"=2字节；"分钟"=6字节→"min"=3字节（数字紧贴无空格）。
     * memmove 源从偏移取（保留尾串），替换后指针前进避免死循环。 */
    char *bp = buf;
    while ((bp = strstr(bp, "小时")) != NULL) {
        *bp = ' '; bp[1] = 'h';
        memmove(bp + 2, bp + 6, strlen(bp + 6) + 1);
        bp += 3;
    }
    bp = buf;
    while ((bp = strstr(bp, "分钟")) != NULL) {
        *bp = 'm'; bp[1] = 'i'; bp[2] = 'n';
        memmove(bp + 3, bp + 6, strlen(bp + 6) + 1);
        bp += 3;
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
    /* 繁體: lv_btn 状态背景图(开关按钮 off/focusoff/on1/on2 等) → _tw 图
     * 生成层用 lv_obj_set_style_bg_img_src 把图烙在 DEFAULT/FOCUSED 两个状态,
     * 运行时只切显隐不改图, 这里逐状态读出比对 s_tw_imgs, 命中才覆写本地样式;
     * 表外图(如 switchbg30/80 无 _tw 素材)查不中自动原样保留。
     * 读必须走 lv_obj_get_local_style_prop(精确 selector 只读本地样式):
     * 普通 getter 按对象活状态解析(lv_obj_style.c: selector=part|obj->state,
     * 传入状态位被丢弃), 从 delayset 返回时先恢复焦点再进遍历, 按钮正聚焦,
     * 读 DEFAULT 会拿到 FOCUSED 的聚焦图并误写进 DEFAULT 态 → 焦点移走后
     * 聚焦框残留; 未聚焦按钮则反向把 DEFAULT 图写进 FOCUSED 态(聚焦框丢失) */
    if (is_trad() && lv_obj_has_class(obj, &lv_button_class)) {
        const lv_state_t sts[2] = { LV_STATE_DEFAULT, LV_STATE_FOCUSED };
        for (int k = 0; k < 2; k++) {
            lv_style_value_t v;
            if (lv_obj_get_local_style_prop(obj, LV_STYLE_BG_IMAGE_SRC, &v,
                                            LV_PART_MAIN | sts[k]) != LV_STYLE_RES_FOUND)
                continue;
            const void *bsrc = v.ptr;
            if (!bsrc || lv_image_src_get_type(bsrc) != LV_IMAGE_SRC_FILE) continue;
            for (int i = 0; i < TW_IMGS_N; i++) {
                if (strcmp((const char *)bsrc, s_tw_imgs[i].cn_src) == 0) {
                    lv_obj_set_style_bg_img_src(obj, s_tw_imgs[i].tw_src,
                                                LV_PART_MAIN | sts[k]);
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

/* EN 单位标签宽度兜底:树遍历把"分"译成 "min" 后,生成层窄单位标签(27-64px,73 页
 * menu/set/setting 同构)装不下三字符,折行被标签高度裁成 "m"/"mi"。整串恰为 "min"
 * 的只有独立单位标签(状态条/正文不可能是这个形态)。放开为内容宽、位置/高度不动。
 * 必须在 tune 之后跑:部分页 tune 会把单位标签压回窄宽(updown_bbq_set fen 42/30px)。
 * 注:数字与单位的间距补偿(pad/translate/x 平移)经多轮实测用户仍不满意,已整体回退,
 * 间距维持各页原状,后续如需另立方案再议。 */
static lv_obj_tree_walk_res_t lang_fit_min_unit_cb(lv_obj_t *obj, void *user_data)
{
    (void)user_data;
    if (!lv_obj_check_type(obj, &lv_label_class)) return LV_OBJ_TREE_WALK_NEXT;
    const char *txt = lv_label_get_text(obj);
    if (!txt || strcmp(txt, "min") != 0) return LV_OBJ_TREE_WALK_NEXT;
    lv_obj_set_width(obj, LV_SIZE_CONTENT);
    return LV_OBJ_TREE_WALK_NEXT;
}

static void lang_fit_unit_labels(void)
{
    lv_obj_tree_walk(lv_scr_act(), lang_fit_min_unit_cb, NULL);
}

/* ==============================
 * EN 按钮换肤(2026-10-09 客户新要求):运行/暂停/stopback/演示预约页右下按钮
 * 底图 stopbk1/2 换 pause/resume/stopcooking/cancel 专属图并改文案(仅英文)。
 * 按钮识别=bg 图名字符串,不依赖生成层成员名;页面 id 表由 nav.h 枚举后缀脚本
 * 生成,覆盖 38/35/35 个同构页(含 probe 变体)。setting/set 页的确定钮用同名
 * 底图但 id 不在表内,不受影响;i18n 词典零改动;简繁零改动(is_english 门禁)。
 * LVGL8/9 背景图拉伸铺满按钮:按新图原生尺寸设按钮(169x64/stopcooking 264x64)
 * 零形变,水平中心与上方时间/文案列中线对齐(用户 2026-10-09 二轮修正)+垂直中心
 * 保持;focus 图 166x61 同盒拉伸(与旧 stopbk2 同规则)。
 * ============================== */
static const page_id_t s_bskin_cooking[] = {
    PAGE_AIR_COOKING, PAGE_BOTTOM_BBQ_COOKING, PAGE_BOTTOM_BBQ_COOKING_PROBE,
    PAGE_BREAD_COOKING, PAGE_CENTRAL_BBQ_COOKING, PAGE_CHIP_COOKING,
    PAGE_CHICKENCOOKING, PAGE_COLOR_COOKING, PAGE_COOKIE_COOKING,
    PAGE_CORN_COOKING, PAGE_CUSTOM_COOKING, PAGE_HEATCONTAIN_COOKING,
    PAGE_HOTCLEANHIGH_COOKING, PAGE_HOTCLEANMIDDLE_COOKING, PAGE_HOTCLEANSAVE_COOKING,
    PAGE_HOTWIND_BBQ_COOKING, PAGE_HOT_BBQ_COOKING, PAGE_HOT_BBQ_COOKING_PROBE,
    PAGE_LASAGNA_COOKING, PAGE_MENU_COOK_COOKING, PAGE_PIZZA3_COOKING,
    PAGE_PIZZA_2_COOKING, PAGE_PIZZA_COOKING, PAGE_PREHEAT_COOKING,
    PAGE_RISING_COOKING, PAGE_SAVE_BBQ_COOKING, PAGE_SIX_COOKING,
    PAGE_SLOWCOOK_COOKING, PAGE_SLOWCOOK_COOKING_PROBE, PAGE_SOMECOOK_COOKING,
    PAGE_STRUDEL_COOKING, PAGE_TOP_BBQ_COOKING, PAGE_UNFROZEN_COOKING,
    PAGE_UPDOWN_BBQ_COOKING, PAGE_UPDOWN_BBQ_COOKING_PROBE, PAGE_WATER_CLEAN_COOKING,
    PAGE_WEST_COOKING, PAGE_WINDCHANGE_BBQ_COOKING,
};
static const page_id_t s_bskin_stop[] = {
    PAGE_AIR_STOP, PAGE_BOTTOM_BBQ_STOP, PAGE_BOTTOM_BBQ_STOP_PROBE,
    PAGE_BREAD_STOP, PAGE_CENTRAL_BBQ_STOP, PAGE_CHIP_STOP,
    PAGE_COLOR_STOP, PAGE_COOKIE_STOP, PAGE_CORN_STOP,
    PAGE_CUSTOM_STOP, PAGE_HEATCONTAIN_STOP, PAGE_HOTCLEANHIGH_STOP,
    PAGE_HOTCLEANMIDDLE_STOP, PAGE_HOTCLEANSAVE_STOP, PAGE_HOTWIND_BBQ_STOP,
    PAGE_HOT_BBQ_STOP, PAGE_HOT_BBQ_STOP_PROBE, PAGE_LASAGNA_STOP,
    PAGE_MENU_COOK_STOP, PAGE_PIZZA3_STOP, PAGE_PIZZA_2_STOP,
    PAGE_PIZZA_STOP, PAGE_PREHEAT_STOP, PAGE_RISING_STOP,
    PAGE_SAVE_BBQ_STOP, PAGE_SLOWCOOK_STOP, PAGE_SLOWCOOK_STOP_PROBE,
    PAGE_STRUDEL_STOP, PAGE_TOP_BBQ_STOP, PAGE_UNFROZEN_STOP,
    PAGE_UPDOWN_BBQ_STOP, PAGE_UPDOWN_BBQ_STOP_PROBE, PAGE_WATER_CLEAN_STOP,
    PAGE_WEST_STOP, PAGE_WINDCHANGE_BBQ_STOP,
};
static const page_id_t s_bskin_stopback[] = {
    PAGE_AIR_STOP_BACK, PAGE_BOTTOM_BBQ_STOP_BACK, PAGE_BOTTOM_BBQ_STOP_BACK_PROBE,
    PAGE_BREAD_STOP_BACK, PAGE_CENTRAL_BBQ_STOP_BACK, PAGE_CHIP_STOP_BACK,
    PAGE_COLOR_STOP_BACK, PAGE_COOKIE_STOP_BACK, PAGE_CORN_STOP_BACK,
    PAGE_CUSTOM_STOP_BACK, PAGE_HEATCONTAIN_STOP_BACK, PAGE_HOTCLEANHIGH_STOP_BACK,
    PAGE_HOTCLEANMIDDLE_STOP_BACK, PAGE_HOTCLEANSAVE_STOP_BACK, PAGE_HOTWIND_BBQ_STOP_BACK,
    PAGE_HOT_BBQ_STOP_BACK, PAGE_HOT_BBQ_STOP_BACK_PROBE, PAGE_LASAGNA_STOP_BACK,
    PAGE_MENU_COOK_STOP_BACK, PAGE_PIZZA3_STOP_BACK, PAGE_PIZZA_2_STOP_BACK,
    PAGE_PIZZA_STOP_BACK, PAGE_PREHEAT_STOP_BACK, PAGE_RISING_STOP_BACK,
    PAGE_SAVE_BBQ_STOP_BACK, PAGE_SLOWCOOK_STOP_BACK, PAGE_SLOWCOOK_STOP_BACK_PROBE,
    PAGE_STRUDEL_STOP_BACK, PAGE_TOP_BBQ_STOP_BACK, PAGE_UNFROZEN_STOP_BACK,
    PAGE_UPDOWN_BBQ_STOP_BACK, PAGE_UPDOWN_BBQ_STOP_BACK_PROBE, PAGE_WATER_CLEAN_STOP_BACK,
    PAGE_WEST_STOP_BACK, PAGE_WINDCHANGE_BBQ_STOP_BACK,
};

typedef struct {
    const char *src, *src_focus;   /* DEFAULT/FOCUSED 新底图 */
    lv_coord_t w, h;               /* 新按钮尺寸(=底图原生) */
    int cls;                       /* 0=仅换图 1=暂停页文案 2=stopback 文案 */
} bskin_arg_t;

static int bskin_id_in(const page_id_t *tab, int n, page_id_t id)
{
    int i;
    for (i = 0; i < n; i++)
        if (tab[i] == id) return 1;
    return 0;
}

static lv_obj_tree_walk_res_t bskin_walk_cb(lv_obj_t *obj, void *user_data)
{
    bskin_arg_t *a = user_data;
    const char *bg = lv_obj_get_style_bg_img_src(obj, LV_PART_MAIN);

    /* 不做 lv_btn_class 判断(SIM 是 LVGL9 类名 lv_button_class,设备端版本未必一致):
     * stopbk1/2 底图只有目标按钮在用,按 bg 图名匹配即天然唯一 */
    if (bg && (strcmp(bg, LVGL_IMAGE_PATH(stopbk1.png)) == 0 ||
               strcmp(bg, LVGL_IMAGE_PATH(stopbk2.png)) == 0)) {
        lv_coord_t x = lv_obj_get_x(obj), y = lv_obj_get_y(obj);
        /* 水平中心保持(用户:按钮要与上方时间/文案列中线对齐,原右缘保持会左偏 ~20px) */
        lv_obj_set_pos(obj, x + lv_obj_get_width(obj) / 2 - a->w / 2,
                       y + (lv_obj_get_height(obj) - a->h) / 2);
        lv_obj_set_size(obj, a->w, a->h);
        lv_obj_set_style_bg_img_src(obj, a->src, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_img_src(obj, a->src_focus, LV_PART_MAIN | LV_STATE_FOCUSED);
        return LV_OBJ_TREE_WALK_NEXT;
    }
    if (!a->cls || !lv_obj_check_type(obj, &lv_label_class)) return LV_OBJ_TREE_WALK_NEXT;
    {
        const char *t = lv_label_get_text(obj);
        if (!t) return LV_OBJ_TREE_WALK_NEXT;
        if (a->cls == 1) {   /* 暂停页:按钮 Start→Resume,标题按新要求对齐 mockup */
            if (strcmp(t, "Start") == 0) lv_label_set_text(obj, "Resume");
            else if (strcmp(t, "Paused...") == 0) lv_label_set_text(obj, "Pause...");
        } else {             /* stopback 页 */
            if (strcmp(t, "Start") == 0) {
                lv_label_set_text(obj, "Stop Cooking");
            } else if (strcmp(t, "Paused...") == 0) {
                lv_label_set_text(obj, "Cooking...");
            } else if (strcmp(t, "Cancel the") == 0 || strcmp(t, "current program?") == 0) {
                /* 两行提示语换新文案(词典词条仅 stopback 在用):按新要求 mockup
                 * 走 Aktiv Grotesk Medium 30 号+470 宽固定盒双行居中(盒中心 1005,
                 * 实测 mockup 墨迹轴≈1005);y 不动(墨迹顶 163/201 与 mockup 164/200 吻合) */
                lv_label_set_text(obj, strcmp(t, "Cancel the") == 0 ?
                                      "Cooking is currently in progress." : "Stop cooking and exit?");
                lv_obj_set_width(obj, 470);
                lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
                lv_obj_set_style_text_font(obj, &c_aktivgroteskmedium_30, LV_PART_MAIN);
                lv_obj_set_pos(obj, 770, lv_obj_get_y(obj));
            }
        }
    }
    return LV_OBJ_TREE_WALK_NEXT;
}

void nav_btnskin_refit(void)
{
    bskin_arg_t a;
    page_id_t id;

    if (depth <= 0 || !is_english()) return;
    id = page_stack[depth - 1];
    if (bskin_id_in(s_bskin_cooking, (int)(sizeof(s_bskin_cooking) / sizeof(s_bskin_cooking[0])), id)) {
        a.src = LVGL_IMAGE_PATH(pause.png); a.src_focus = LVGL_IMAGE_PATH(pause_focus.png);
        a.w = 169; a.h = 64; a.cls = 0;
    } else if (bskin_id_in(s_bskin_stop, (int)(sizeof(s_bskin_stop) / sizeof(s_bskin_stop[0])), id)) {
        a.src = LVGL_IMAGE_PATH(resume.png); a.src_focus = LVGL_IMAGE_PATH(resume_focus.png);
        a.w = 169; a.h = 64; a.cls = 1;
    } else if (bskin_id_in(s_bskin_stopback, (int)(sizeof(s_bskin_stopback) / sizeof(s_bskin_stopback[0])), id)) {
        a.src = LVGL_IMAGE_PATH(stopcooking.png); a.src_focus = LVGL_IMAGE_PATH(stopcooking_focus.png);
        a.w = 264; a.h = 64; a.cls = 2;
    } else if (id == PAGE_DELAYCOOKING) {
        a.src = LVGL_IMAGE_PATH(cancel.png); a.src_focus = LVGL_IMAGE_PATH(cancel_focus.png);
        a.w = 169; a.h = 64; a.cls = 0;
    } else {
        return;
    }
    lv_obj_tree_walk(lv_scr_act(), bskin_walk_cb, &a);
}

void lang_on_page_built(void)
{
    if (depth <= 0) return;

    /* 语言选择页:三项/标题为固定原文,任何语言下都不过翻译/字体树遍历
     * ("简体中文"是 i18n 英文表词条键,遍历会把它改写成英译/繁体化) */
    if (page_stack[depth - 1] == PAGE_LANG_PICK) return;

    if (is_english()) {
        /* 英文: 静态标签翻译 + 字体切换 + 排版微调 */
        lang_refresh_screen();
        lang_tune_for_page(page_stack[depth - 1])();
        lang_fit_unit_labels();   /* 单位标签宽度兜底:tune 压窄后放开装不下 "min" 的 */
    } else if (is_trad()) {
        /* 繁體: 树遍历翻译(tr→简转繁) + _tw 图片替换;
         * 排版与字体与简体一致(taiwanpearl), 不跑 lang_tune/字体映射 */
        lang_refresh_screen();
    }

    nav_lockicon_refit();   /* lockicon 换 54x54 大图:全语言统一重适配锁图标按钮(EN 须在 tune 之后,tune 里有 50x43 复位) */
    nav_dirimg_refit();     /* menu_top/low+setting 箭头换图+下划线加长:全语言统一(EN 须在 tune 之后覆盖 tune 的 dir/line 行) */
    nav_btnskin_refit();    /* EN 运行/暂停/stopback/预约按钮换 pause/resume/stopcooking/cancel 底图+文案(2026-10-09) */
}