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
 * 中文折行禁则：行首不留句末标点
 * ------------------------------
 * LVGL 对 CJK 是“一字一词”贪心断行，句子末尾恰好差半个字宽时，
 * “。”会被单独挤到下一行(用户 2026-11-17 报繁中炸排骨说明页：
 *  “…一層煮食油” / “。” / “將食物放入第 3 層。”)。
 * 版面定稿后逐行量一遍：行首若是句末标点，就按“上一行+该标点”的实际
 * 像素宽把标签加宽(最多 +40px，且不越过卡片内容宽；+40 后右缘≤982，
 * 仍不侵入右下延迟钮 x983)，把这一个标点收回上一行。
 * 加宽到上限仍收不回(2D 菜已是满宽)时，退一步收紧字距 1~2px。
 * 简中句短不触发；英文按整词折行不会出现行首标点，同样不受影响。
 * ============================== */
#define DESC_FIT_MAX_EXTRA   40   /* 加宽上限(px)，约 1.5 个字宽 */
#define DESC_FIT_MAX_TRY     3

/* 折行禁则里的“句末标点”:行首出现即回拉(严格说不含开引号/开括号) */
static bool desc_tail_punct(uint32_t cp)
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

/* 当前宽/字距下，为把行首标点收回上一行还需多少像素(0=无需加宽)
 * 行号用 lv_label_get_letter_pos 的 y 变化判定，与渲染的折行完全一致 */
static int32_t desc_wrap_need(lv_obj_t *lbl, const char *txt,
                              const lv_font_t *font, int32_t ls)
{
    const int32_t cw = lv_obj_get_content_width(lbl);
    int32_t need = 0;
    int32_t line_start = 0;    /* 当前行行首字节偏移 */
    int32_t last_y = 0;
    uint32_t li = 0;           /* 字序号(lv_label_get_letter_pos 用) */
    int32_t bo = 0;            /* 字节偏移 */
    while (txt[bo] != '\0') {
        lv_point_t pos;
        int clen = 0;
        uint32_t cp = desc_utf8_next(txt + bo, &clen);
        lv_label_get_letter_pos(lbl, li, &pos);
        if (li > 0 && pos.y != last_y) {
            /* 新行行首：自动折行(上一字节不是换行符)且是句末标点才处理。
             * 上一行 = 刚结束的这一行，行首即 line_start */
            if (txt[bo - 1] != '\n' && txt[bo - 1] != '\r' && desc_tail_punct(cp)) {
                int32_t w = lv_text_get_width(txt + line_start,
                                              (uint32_t)(bo + clen - line_start),
                                              font, ls);
                if (w - cw > need) need = w - cw;
            }
            line_start = bo;
        }
        last_y = pos.y;
        li++;
        bo += clen;
    }
    return need;
}

static void desc_kinsoku_fit(lv_obj_t *lbl)
{
    if (!lbl) return;
    if (lv_label_get_long_mode(lbl) != LV_LABEL_LONG_WRAP) return;

    const char *txt = lv_label_get_text(lbl);
    if (!txt || txt[0] == '\0') return;

    const lv_font_t *font = lv_obj_get_style_text_font(lbl, LV_PART_MAIN);
    int32_t ls = lv_obj_get_style_text_letter_space(lbl, LV_PART_MAIN);
    lv_obj_update_layout(lbl);            /* 先落一次版面，lv_obj_get_width 才是生成稿的 894 */
    int32_t base_w = lv_obj_get_width(lbl);
    if (base_w <= 0) return;

    /* 快路径：整串没有换行且一行放得下 → 不会折行，省掉逐字定位 */
    if (strchr(txt, '\n') == NULL &&
        lv_text_get_width(txt, (uint32_t)strlen(txt), font, ls) <= base_w) return;

    int32_t limit = base_w + DESC_FIT_MAX_EXTRA;

    /* 不越过卡片内容宽(容器 pad_left 已算在内容区外) */
    lv_obj_t *par = lv_obj_get_parent(lbl);
    if (par) {
        int32_t pw = lv_obj_get_content_width(par);
        if (pw > 0 && pw < limit) limit = pw;
    }

    /* 第一轮：加宽 */
    int32_t w = base_w;
    for (int i = 0; i < DESC_FIT_MAX_TRY; i++) {
        lv_obj_set_width(lbl, w);
        lv_obj_update_layout(lbl);
        int32_t need = desc_wrap_need(lbl, txt, font, ls);
        if (need <= 0) return;
        int32_t nw = w + need + 1;      /* +1: 贪心判定用 > max_w */
        if (nw > limit) nw = limit;
        if (nw <= w) break;             /* 已到上限 */
        w = nw;
    }

    /* 第二轮：满宽仍收不回，收紧字距(每字 1px，一个标点约需一个字宽) */
    lv_obj_set_width(lbl, w);
    lv_obj_update_layout(lbl);
    if (desc_wrap_need(lbl, txt, font, ls) > 0) {
        bool ok = false;
        for (int k = 1; k <= 2 && !ok; k++) {
            lv_obj_set_style_text_letter_space(lbl, ls - k, 0);
            lv_obj_update_layout(lbl);
            ok = (desc_wrap_need(lbl, txt, font, ls - k) <= 0);
        }
        if (!ok) lv_obj_set_style_text_letter_space(lbl, ls, 0);   /* 收不回落回原字距 */
        lv_obj_update_layout(lbl);
    }
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
    /* 繁中长句折行后句末“。”可能被挤到行首单独成行(2026-11-17 炸排骨页)，
     * 这一段定稿后按实际字宽把标点收回上一行 */
    desc_kinsoku_fit(dm->cookdescriptin);
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
