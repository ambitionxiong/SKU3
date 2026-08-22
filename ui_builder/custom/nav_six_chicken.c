/*
 * nav_six_chicken.c - 第六感-家禽/鸡
 *
 * 链路：sixmenu → (chick) → chick6menu → (chicken) → chickenmenu → 烤鸡菜谱
 * 本文件当前只做菜单链路（sixmenu→chick6menu→chickenmenu）；
 * 菜谱选择（烤全鸡等）与 chickencooking 烹饪页在后续步骤接入。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"

lv_group_t *g_chick6menu = NULL;
lv_group_t *g_chickenmenu = NULL;
lv_group_t *g_duckmenu = NULL;
lv_group_t *g_probeneedtip = NULL;
int g_six_probe_temp = 80;
static uint8_t s_meatdish_mode = 0;   /* 1=肉菜模式（复用 duckmenu） */
static uint8_t s_fish_mode = 0;       /* 0=默认(鸡) 1=鱼/海鲜首页 2=烤鱼子页 */
static uint8_t s_seafood_mode = 0;    /* 1=烤海鲜模式（复用 chickenmenu） */
static uint8_t s_fish_home_focus = 0; /* 鱼首页返回焦点: 0=烤鱼(chicken) 1=烤海鲜(duck) */

void six_chick_reset_fish_mode(void) { s_fish_mode = 0; s_seafood_mode = 0; s_fish_home_focus = 0; }
int six_chick_get_fish_mode(void) { return s_fish_mode; }
/* 烤鱼子页 BACK:整页重建回鱼/海鲜首页(不弹页,页面共用 PAGE_CHICK6MENU)
   注意:必须走 rebuild 重新绑定 mode1 事件,否则子页的直接处理器残留会导致
   文字是首页、点击却触发子页菜(如"烤海鲜"点出"烤全鱼分量页") */
void six_chick_fish_go_back(void)
{
    if (s_fish_mode != 2) return;
    s_fish_mode = 1;
    s_fish_home_focus = 0;   /* 从烤鱼子页回首页:入口是"烤鱼" */
    chick6menu_rebuild(PAGE_CHICK6MENU);
}
static void on_meatdish_sausage_click(lv_event_t *e);
static void on_fish_grillfish_click(lv_event_t *e);
static void on_fish_seafood_click(lv_event_t *e);
static void on_fish_cod_click(lv_event_t *e);
static void on_fish_wholefish_click(lv_event_t *e);
static void on_seafood_dish_click(lv_event_t *e);

/* ==================== 烤鸡翅类（份量驱动）菜谱配置表 ==================== */
typedef struct {
    const char *name;
    const int *weights;     /* 份量选项(g) */
    const int *times;       /* 对应烹饪分钟(与 weights 对齐) */
    int count;              /* 选项数 */
    int default_idx;        /* 默认份量下标 */
    uint8_t mode;           /* 热风对流/空气炸 */
    int temp;
    const char *desc;
} chick_dish_t;

static const int w_wing_w[] = { 400, 600, 800, 1000 };
static const int w_wing_t[] = { 21, 24, 27, 30 };
static const int w_fwing_w[] = { 400, 600, 800 };
static const int w_fwing_t[] = { 22, 24, 26 };
static const int w_fleg_w[]  = { 600, 800, 1000 };
static const int w_fleg_t[]  = { 27, 30, 33 };
static const int w_breast_w[] = { 400, 600, 800, 1000 };
static const int w_breast_t[] = { 20, 25, 30, 35 };
/* 鱼:烤鳕鱼/烤全鱼 份量表 */
static const int w_cod_w[] = { 400, 600, 800, 1000 };
static const int w_cod_t[] = { 18, 21, 24, 27 };
static const int w_wf_w[]  = { 400, 600, 800, 1000 };
static const int w_wf_t[]  = { 20, 25, 30, 35 };

/* 烤海鲜:固定参数菜配置表(无份量/烤色/发酵; 类型在 nav.h) */
static const seafood_dish_t s_seafood_dishes[] = {
    /* 烤扇贝(热风对流200) */
    { "烤扇贝", MODE_WINDCHANGE_BBQ, 200, 18,
      "根据个人喜好，用油、蒜、胡椒和香芹进行调味\n现在将食物放在第3层\n使用烤盘" },
    /* 烤青口贝(上下烧烤230) */
    { "烤青口贝", MODE_UPDOWN_BBQ, 230, 18,
      "根据个人喜好，用油、白葡萄酒、蒜、胡椒和欧芹进行调味\n现在将食物放在第3层\n使用烤盘" },
    /* 烤生蚝(上下烧烤230) */
    { "烤生蚝", MODE_UPDOWN_BBQ, 230, 15,
      "根据你的喜好，用油、大蒜、胡椒和欧芹调味\n现在将食物放在第3层\n使用烤盘" },
    /* 烤大虾(热风对流230) */
    { "烤大虾", MODE_WINDCHANGE_BBQ, 230, 20,
      "刷油，抹上盐和胡椒。根据个人喜好，用蒜和香草调味\n现在将食物放在第3层\n使用烤盘" },
    /* 烤鱿鱼(热风对流230) */
    { "烤鱿鱼", MODE_WINDCHANGE_BBQ, 230, 18,
      "根据个人喜好调味，刷上油，撒上烧烤调料\n现在将食物放在第3层\n使用烤盘" },
};

const seafood_dish_t *seafood_dish_cfg(void)
{
    if (g_six_bread_type < SIX_SEAFOOD_SCALLOP || g_six_bread_type > SIX_SEAFOOD_SQUID)
        return NULL;
    return &s_seafood_dishes[g_six_bread_type - SIX_SEAFOOD_SCALLOP];
}

/* 当前类型是否为烤海鲜固定参数菜 */
int six_chick_is_seafood(void)
{
    return (g_six_bread_type >= SIX_SEAFOOD_SCALLOP && g_six_bread_type <= SIX_SEAFOOD_SQUID);
}

static const chick_dish_t s_chick_dishes[] = {
    /* 烤鸡翅 */
    { "烤鸡翅", w_wing_w, w_wing_t, 4, 2, MODE_WINDCHANGE_BBQ, 250,
      "烹饪说明：\n刷油，并根据个人喜好进行调味。在烤盘内均匀铺开，表皮朝上\n现在将食物放在第3层\n使用深盘" },
    /* 炸鸡中翅 */
    { "炸鸡中翅", w_fwing_w, w_fwing_t, 3, 1, MODE_AIR, 250,
      "烹饪说明：\n根据你的喜好加盐和胡椒调味。把鸡翅裹上蛋液，然后裹上面包屑。均匀分布在气炸盘中，轻轻喷上食用油\n现在将食物放在第3层\n使用气炸盘和深盘" },
    /* 炸鸡腿 */
    { "炸鸡腿", w_fleg_w, w_fleg_t, 3, 1, MODE_AIR, 250,
      "烹饪说明：\n根据你的喜好加盐和胡椒调味。把鸡腿裹上蛋液，然后裹上面包屑。均匀分布在气炸盘中，轻轻喷上食用油\n现在将食物放在第3层\n使用气炸盘和深盘" },
    /* 烤鸡胸肉 */
    { "烤鸡胸肉", w_breast_w, w_breast_t, 4, 2, MODE_WINDCHANGE_BBQ, 250,
      "烹饪说明：\n刷油，并根据个人喜好进行调味。在烤盘内均匀铺开，表皮朝上\n现在将食物放在第3层\n使用深盘" },
};

/* 当前类型是否为烤鸡翅类(份量驱动) + 烤羊肉串(程度→时间) */
int six_chick_is_kind(void)
{
    return (g_six_bread_type >= SIX_CHICK_KIND_MIN && g_six_bread_type <= SIX_CHICK_KIND_MAX)
        || g_six_bread_type == SIX_MEAT_FRIED_STEAK
        || g_six_bread_type == SIX_MEAT_GRILL_SKEWER
        || g_six_bread_type == SIX_MEAT_FRIED_RIB
        || g_six_bread_type == SIX_MEAT_SAUSAGE
        || g_six_bread_type == SIX_FISH_COD
        || g_six_bread_type == SIX_FISH_WHOLEFISH;
}

/* 当前是否为程度→时间驱动（烤羊肉串） */
int six_chick_is_degree_time(void)
{
    return g_six_bread_type == SIX_MEAT_GRILL_SKEWER;
}

/* 程度→烹饪分钟（1浅2中3深: 18/20/24） */
int six_chick_degree_min(int degree)
{
    static const int m[] = { 18, 20, 24 };
    if (degree < 1 || degree > 3) degree = 2;
    return m[degree - 1];
}

static const chick_dish_t *chick_dish_cfg(void)
{
    if (g_six_bread_type == SIX_MEAT_FRIED_STEAK ||
        g_six_bread_type == SIX_MEAT_GRILL_SKEWER ||
        g_six_bread_type == SIX_MEAT_FRIED_RIB ||
        g_six_bread_type == SIX_MEAT_SAUSAGE ||
        g_six_bread_type == SIX_FISH_COD ||
        g_six_bread_type == SIX_FISH_WHOLEFISH)
        return NULL;  /* 独立表 */
    return six_chick_is_kind() ? &s_chick_dishes[g_six_bread_type - SIX_CHICK_KIND_MIN] : NULL;
}

/* ==================== 探针菜（烤全鸡/烤全鸭）配置表 ==================== */
typedef struct {
    const char *name;
    uint8_t mode;
    int temp;
    int probe[3];       /* 浅/中/深 探针目标温度 */
    int max_min;        /* 最长防护分钟(探针异常兜底,非倒计时) */
    const char *desc;
} chick_probe_t;

static const chick_probe_t s_chick_probes[] = {
    /* 烤全鸡 */
    { "烤全鸡", MODE_WINDCHANGE_BBQ, 230, {75, 80, 85}, 80,
      "烹饪说明：\n刷油，并根据个人喜好进行调味。抹上盐和胡椒。放入烤箱，胸脯面朝上\n现在将食物放在第2层\n使用深盘" },
    /* 烤全鸭 */
    { "烤全鸭", MODE_WINDCHANGE_BBQ, 230, {86, 88, 90}, 90,
      "烹饪说明：\n刷油，抹上盐和胡椒。根据个人喜好，用蒜和香草调味\n现在将食物放在第2层\n使用深盘" },
};

/* 当前类型是否为探针菜 */
int six_chick_is_probe(void)
{
    return (g_six_bread_type == SIX_CHICK_WHOLE || g_six_bread_type == SIX_CHICK_DUCK_WHOLE);
}

static const chick_probe_t *chick_probe_cfg(void)
{
    if (!six_chick_is_probe()) return NULL;
    return &s_chick_probes[(g_six_bread_type == SIX_CHICK_DUCK_WHOLE) ? 1 : 0];
}

/* 探针菜档位(1浅2中3深)→探针目标温度 */
int six_chick_probe_temp(int level)
{
    const chick_probe_t *p = chick_probe_cfg();
    if (!p) return 80;
    if (level < 1 || level > 3) level = 2;
    return p->probe[level - 1];
}

/* 烤鸡菜名：份量驱动类按份量表；探针菜按探针配置 */
const char *six_chick_name(void)
{
    if (g_six_bread_type == SIX_MEAT_FRIED_STEAK) return "炸牛排";
    if (g_six_bread_type == SIX_MEAT_GRILL_SKEWER) return "烤羊肉串";
    if (g_six_bread_type == SIX_MEAT_FRIED_RIB) return "炸排骨";
    if (g_six_bread_type == SIX_MEAT_SAUSAGE) return "烤香肠";
    if (g_six_bread_type == SIX_FISH_COD) return "烤鳕鱼";
    if (g_six_bread_type == SIX_FISH_WHOLEFISH) return "烤全鱼";
    if (six_chick_is_seafood()) {
        const seafood_dish_t *sd = seafood_dish_cfg();
        return sd ? sd->name : "烤海鲜";
    }
    const chick_dish_t *c = chick_dish_cfg();
    if (c) return c->name;
    const chick_probe_t *p = chick_probe_cfg();
    return p ? p->name : "烤全鸡";
}

uint8_t six_chick_mode(void) {
    if (g_six_bread_type == SIX_MEAT_FRIED_STEAK) return MODE_AIR;
    if (g_six_bread_type == SIX_MEAT_GRILL_SKEWER) return MODE_HOTWIND_BBQ;
    if (g_six_bread_type == SIX_MEAT_FRIED_RIB) return MODE_AIR;
    if (g_six_bread_type == SIX_MEAT_SAUSAGE) return MODE_HOTWIND_BBQ;
    if (g_six_bread_type == SIX_FISH_COD) return MODE_WINDCHANGE_BBQ;   /* 热风对流 */
    if (g_six_bread_type == SIX_FISH_WHOLEFISH) return MODE_HOTWIND_BBQ;/* 热风 */
    if (six_chick_is_seafood()) {
        const seafood_dish_t *sd = seafood_dish_cfg();
        return sd ? sd->mode : MODE_WINDCHANGE_BBQ;
    }
    const chick_dish_t *c = chick_dish_cfg(); if (c) return c->mode;
    const chick_probe_t *p = chick_probe_cfg(); if (p) return p->mode;
    return MODE_WINDCHANGE_BBQ;
}
int six_chick_temp(void) {
    if (g_six_bread_type == SIX_MEAT_FRIED_STEAK) return 250;
    if (g_six_bread_type == SIX_MEAT_GRILL_SKEWER) return 230;
    if (g_six_bread_type == SIX_MEAT_FRIED_RIB) return 250;
    if (g_six_bread_type == SIX_MEAT_SAUSAGE) return 180;
    if (g_six_bread_type == SIX_FISH_COD) return 250;
    if (g_six_bread_type == SIX_FISH_WHOLEFISH) return 250;
    if (six_chick_is_seafood()) {
        const seafood_dish_t *sd = seafood_dish_cfg();
        return sd ? sd->temp : 230;
    }
    const chick_dish_t *c = chick_dish_cfg(); if (c) return c->temp;
    const chick_probe_t *p = chick_probe_cfg(); if (p) return p->temp;
    return 230;
}

/* 份量→烹饪分钟（查表，找不到取中间档兜底） */
int six_chick_cook_min(int weight_g)
{
    /* 炸牛排：独立份量表 */
    if (g_six_bread_type == SIX_MEAT_FRIED_STEAK) {
        static const int fw[] = { 250, 500, 750 };
        static const int ft[] = { 18, 21, 24 };
        for (int i = 0; i < 3; i++) {
            if (fw[i] == weight_g) return ft[i];
        }
        return 21;
    }
    /* 炸排骨：独立份量表 */
    if (g_six_bread_type == SIX_MEAT_FRIED_RIB) {
        static const int rw[] = { 600, 800, 1000 };
        static const int rt[] = { 24, 27, 30 };
        for (int i = 0; i < 3; i++) {
            if (rw[i] == weight_g) return rt[i];
        }
        return 27;
    }
    /* 烤香肠：独立份量表 */
    if (g_six_bread_type == SIX_MEAT_SAUSAGE) {
        static const int sw[] = { 200, 400, 600, 800 };
        static const int st[] = { 12, 14, 16, 18 };
        for (int i = 0; i < 4; i++) {
            if (sw[i] == weight_g) return st[i];
        }
        return 16;  /* 默认600g */
    }
    /* 烤鳕鱼/烤全鱼：独立份量表 */
    if (g_six_bread_type == SIX_FISH_COD) {
        for (int i = 0; i < 4; i++) {
            if (w_cod_w[i] == weight_g) return w_cod_t[i];
        }
        return 24;  /* 默认800g */
    }
    if (g_six_bread_type == SIX_FISH_WHOLEFISH) {
        for (int i = 0; i < 4; i++) {
            if (w_wf_w[i] == weight_g) return w_wf_t[i];
        }
        return 30;  /* 默认800g */
    }
    const chick_dish_t *c = chick_dish_cfg();
    if (!c) return 0;
    for (int i = 0; i < c->count; i++) {
        if (c->weights[i] == weight_g) return c->times[i];
    }
    return c->count > 0 ? c->times[c->count / 2] : 0;
}

/* 烹饪说明（按菜谱表；descriptionmenu/延迟页显示） */
const char *six_chick_desc(void)
{
    if (g_six_bread_type == SIX_MEAT_FRIED_STEAK)
        return "根据个人喜好进行调味。抹上盐和黑胡椒碎，在炸盘内均匀铺开，在表面喷一层薄油\n现在将食物放在第3层\n使用气炸盘和深盘";
    if (g_six_bread_type == SIX_MEAT_GRILL_SKEWER)
        return "刷油，根据个人喜好，撒上盐、烧烤料、孜然。在烤盘内均匀铺开\n现在将食物放在第3层\n使用深盘";
    if (g_six_bread_type == SIX_MEAT_FRIED_RIB)
        return "根据个人喜好进行调味。在炸盘内均匀铺开，在表面喷一层薄油\n现在将食物放在第3层\n使用气炸盘和深盘";
    if (g_six_bread_type == SIX_MEAT_SAUSAGE)
        return "均匀分布在深盘中\n现在将食物放在第3层\n使用深盘";
    if (g_six_bread_type == SIX_FISH_COD || g_six_bread_type == SIX_FISH_WHOLEFISH)
        return "刷油，抹上盐和胡椒。根据个人喜好，用蒜和香草调味\n现在将食物放在第3层\n使用烤盘";
    if (six_chick_is_seafood()) {
        const seafood_dish_t *sd = seafood_dish_cfg();
        if (sd) return sd->desc;
    }
    const chick_dish_t *c = chick_dish_cfg();
    if (c) return c->desc;
    const chick_probe_t *p = chick_probe_cfg();
    if (p) return p->desc;
    return "烹饪说明：\n刷油，并根据个人喜好进行调味。抹上盐和胡椒。放入烤箱，胸脯面朝上\n现在将食物放在第2层\n使用深盘";
}

/* 当前六感菜名/说明：烤鸡/鸭走独立名，其余走面包/蛋糕共用表 */
const char *six_current_name(void)
{
    return (six_chick_is_probe() || six_chick_is_kind() || six_chick_is_seafood())
           ? six_chick_name() : six_bread_name();
}
const char *six_current_desc(void)
{
    return (six_chick_is_probe() || six_chick_is_kind() || six_chick_is_seafood())
           ? six_chick_desc() : six_bread_desc();
}

static void on_chick6menu_chicken_click(lv_event_t *e);
static void on_chick6menu_duck_click(lv_event_t *e);
static void on_probe_dish_click(lv_event_t *e);
static void on_chickenmenu_weight_dish_click(lv_event_t *e);
static void on_duckmenu_wholeduck_click(lv_event_t *e);
static void on_probeneedtip_sure_click(lv_event_t *e);

/* ================= chick6menu（家禽：鸡/鸭） ================= */

void jump_to_chick6menu(void)
{
    page_push(PAGE_CHICK6MENU);
    lv_obj_clean(lv_scr_act());
    chick6menu_create(&ui_manager);

    chick6menu_t *cm = chick6menu_get(&ui_manager);
    if (cm) {
        /* fish 模式：设置标签 */
        if (s_fish_mode == 1) {
            if (cm->label_1) lv_label_set_text(cm->label_1, "鱼/海鲜");
            if (cm->chicken) { lv_obj_t *c = lv_obj_get_child(cm->chicken, 0); if (c) lv_label_set_text(c, "烤鱼"); }
            if (cm->duck)    { lv_obj_t *c = lv_obj_get_child(cm->duck, 0);    if (c) lv_label_set_text(c, "烤海鲜"); }
        } else if (s_fish_mode == 2) {
            if (cm->label_1) lv_label_set_text(cm->label_1, "烤鱼");
            if (cm->chicken) { lv_obj_t *c = lv_obj_get_child(cm->chicken, 0); if (c) lv_label_set_text(c, "烤鳕鱼"); }
            if (cm->duck)    { lv_obj_t *c = lv_obj_get_child(cm->duck, 0);    if (c) lv_label_set_text(c, "烤全鱼"); }
        }

        lv_obj_t *btns[] = { cm->chicken, cm->duck };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_chick6menu) lv_group_del(g_chick6menu);
        g_chick6menu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        /* fish 模式事件 */
        if (s_fish_mode == 1) {
            if (cm->chicken) {
                lv_obj_add_event_cb(cm->chicken, on_fish_grillfish_click, LV_EVENT_CLICKED, NULL);
                lv_group_focus_obj(cm->chicken);
            }
            if (cm->duck) lv_obj_add_event_cb(cm->duck, on_fish_seafood_click, LV_EVENT_CLICKED, NULL);
        } else if (s_fish_mode == 2) {
            if (cm->chicken) {
                lv_obj_add_event_cb(cm->chicken, on_fish_cod_click, LV_EVENT_CLICKED, NULL);
                lv_group_focus_obj(cm->chicken);
            }
            if (cm->duck) lv_obj_add_event_cb(cm->duck, on_fish_wholefish_click, LV_EVENT_CLICKED, NULL);
        } else {
            if (cm->chicken) {
                lv_obj_add_event_cb(cm->chicken, on_chick6menu_chicken_click, LV_EVENT_CLICKED, NULL);
                lv_group_focus_obj(cm->chicken);
            }
            if (cm->duck) lv_obj_add_event_cb(cm->duck, on_chick6menu_duck_click, LV_EVENT_CLICKED, NULL);
        }
    }
    current_group = g_chick6menu;

    lv_scr_load_anim(chick6menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] jump: chick6menu (fish=%d)\n", s_fish_mode);
}

void chick6menu_rebuild(page_id_t child)
{
    if (g_chick6menu) { lv_group_del(g_chick6menu); g_chick6menu = NULL; }
    lv_obj_clean(lv_scr_act());
    chick6menu_create(&ui_manager);

    chick6menu_t *cm = chick6menu_get(&ui_manager);
    if (cm) {
        /* fish 模式：恢复标签 */
        if (s_fish_mode == 1) {
            if (cm->label_1) lv_label_set_text(cm->label_1, "鱼/海鲜");
            if (cm->chicken) { lv_obj_t *c = lv_obj_get_child(cm->chicken, 0); if (c) lv_label_set_text(c, "烤鱼"); }
            if (cm->duck)    { lv_obj_t *c = lv_obj_get_child(cm->duck, 0);    if (c) lv_label_set_text(c, "烤海鲜"); }
        } else if (s_fish_mode == 2) {
            if (cm->label_1) lv_label_set_text(cm->label_1, "烤鱼");
            if (cm->chicken) { lv_obj_t *c = lv_obj_get_child(cm->chicken, 0); if (c) lv_label_set_text(c, "烤鳕鱼"); }
            if (cm->duck)    { lv_obj_t *c = lv_obj_get_child(cm->duck, 0);    if (c) lv_label_set_text(c, "烤全鱼"); }
        }

        lv_obj_t *btns[] = { cm->chicken, cm->duck };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_chick6menu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        /* fish 模式事件 */
        if (s_fish_mode == 1) {
            if (cm->chicken) lv_obj_add_event_cb(cm->chicken, on_fish_grillfish_click, LV_EVENT_CLICKED, NULL);
            if (cm->duck)    lv_obj_add_event_cb(cm->duck, on_fish_seafood_click, LV_EVENT_CLICKED, NULL);
        } else if (s_fish_mode == 2) {
            if (cm->chicken) lv_obj_add_event_cb(cm->chicken, on_fish_cod_click, LV_EVENT_CLICKED, NULL);
            if (cm->duck)    lv_obj_add_event_cb(cm->duck, on_fish_wholefish_click, LV_EVENT_CLICKED, NULL);
        } else {
            if (cm->chicken) lv_obj_add_event_cb(cm->chicken, on_chick6menu_chicken_click, LV_EVENT_CLICKED, NULL);
            if (cm->duck)    lv_obj_add_event_cb(cm->duck, on_chick6menu_duck_click, LV_EVENT_CLICKED, NULL);
        }
        /* 焦点恢复（按 fish 模式/进入的菜） */
        if (s_fish_mode == 1 && s_fish_home_focus && cm->duck)
            lv_group_focus_obj(cm->duck);                 /* 鱼首页:从"烤海鲜"返回 */
        else if (s_fish_mode == 2 && g_six_bread_type == SIX_FISH_WHOLEFISH && cm->duck)
            lv_group_focus_obj(cm->duck);                 /* 烤鱼子页:从"烤全鱼"返回 */
        else if (child == PAGE_DUCK6MENU && cm->duck)
            lv_group_focus_obj(cm->duck);                 /* 鸭菜单返回 */
        else if (cm->chicken)
            lv_group_focus_obj(cm->chicken);
    }
    current_group = g_chick6menu;

    lv_scr_load_anim(chick6menu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] rebuild: chick6menu (child=%d, fish=%d)\n", (int)child, s_fish_mode);
}

static void on_chick6menu_chicken_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_chickenmenu();
}

static void on_chick6menu_duck_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    jump_to_duckmenu();
}

/* ================= chickenmenu（鸡：烤全鸡/烤鸡翅/炸鸡翅/炸鸡腿/烤鸡胸） ================= */

void jump_to_chickenmenu(void)
{
    page_push(PAGE_CHICKENMENU);
    lv_obj_clean(lv_scr_act());
    chickenmenu_create(&ui_manager);

    chickenmenu_t *cm = chickenmenu_get(&ui_manager);
    if (cm) {
        /* seafood 模式：设置标签 */
        if (s_seafood_mode) {
            if (cm->label_2) lv_label_set_text(cm->label_2, "烤海鲜");
            if (cm->image_2) lv_obj_add_flag(cm->image_2, LV_OBJ_FLAG_HIDDEN);
            /* 隐藏鸡菜独立标签(烤全鸡/烤鸡翅/炸鸡中翅/炸鸡腿/烤鸡胸肉) */
            if (cm->label_4) lv_obj_add_flag(cm->label_4, LV_OBJ_FLAG_HIDDEN);
            if (cm->label_5) lv_obj_add_flag(cm->label_5, LV_OBJ_FLAG_HIDDEN);
            if (cm->label_6) lv_obj_add_flag(cm->label_6, LV_OBJ_FLAG_HIDDEN);
            if (cm->label_7) lv_obj_add_flag(cm->label_7, LV_OBJ_FLAG_HIDDEN);
            if (cm->label_8) lv_obj_add_flag(cm->label_8, LV_OBJ_FLAG_HIDDEN);
            static const char *sn[] = { "烤扇贝", "烤青口贝", "烤生蚝", "烤大虾", "烤鱿鱼" };
            lv_obj_t *dish_btns[] = { cm->wholechicken, cm->grillchickenwing, cm->friedchickenwing, cm->friedchickenleg, cm->grillchickenbreast };
            for (int i = 0; i < 5; i++) {
                if (dish_btns[i]) { lv_obj_t *c = lv_obj_get_child(dish_btns[i], 0); if (c) lv_label_set_text(c, sn[i]); }
            }
        }

        lv_obj_t *btns[] = {
            cm->wholechicken, cm->grillchickenwing, cm->friedchickenwing,
            cm->friedchickenleg, cm->grillchickenbreast,
        };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_chickenmenu) lv_group_del(g_chickenmenu);
        g_chickenmenu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (s_seafood_mode) {
            if (cm->wholechicken)      lv_obj_add_event_cb(cm->wholechicken,      on_seafood_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_SEAFOOD_SCALLOP);
            if (cm->grillchickenwing)  lv_obj_add_event_cb(cm->grillchickenwing,  on_seafood_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_SEAFOOD_MUSSEL);
            if (cm->friedchickenwing)  lv_obj_add_event_cb(cm->friedchickenwing,  on_seafood_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_SEAFOOD_OYSTER);
            if (cm->friedchickenleg)   lv_obj_add_event_cb(cm->friedchickenleg,   on_seafood_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_SEAFOOD_PRAWN);
            if (cm->grillchickenbreast) lv_obj_add_event_cb(cm->grillchickenbreast, on_seafood_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_SEAFOOD_SQUID);
            if (cm->wholechicken) lv_group_focus_obj(cm->wholechicken);
        } else {
            if (cm->wholechicken) {
                lv_obj_add_event_cb(cm->wholechicken, on_probe_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CHICK_WHOLE);
                lv_group_focus_obj(cm->wholechicken);
            }
            if (cm->grillchickenwing)  lv_obj_add_event_cb(cm->grillchickenwing,  on_chickenmenu_weight_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CHICK_WING);
            if (cm->friedchickenwing)  lv_obj_add_event_cb(cm->friedchickenwing,  on_chickenmenu_weight_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CHICK_FRIED_WING);
            if (cm->friedchickenleg)   lv_obj_add_event_cb(cm->friedchickenleg,   on_chickenmenu_weight_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CHICK_FRIED_LEG);
            if (cm->grillchickenbreast) lv_obj_add_event_cb(cm->grillchickenbreast, on_chickenmenu_weight_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CHICK_GRILL_BREAST);
        }
    }
    current_group = g_chickenmenu;

    lv_scr_load_anim(chickenmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] jump: chickenmenu\n");
}

void chickenmenu_rebuild(page_id_t child)
{
    if (g_chickenmenu) { lv_group_del(g_chickenmenu); g_chickenmenu = NULL; }
    lv_obj_clean(lv_scr_act());
    chickenmenu_create(&ui_manager);

    chickenmenu_t *cm = chickenmenu_get(&ui_manager);
    if (cm) {
        /* seafood 模式：恢复标签 */
        if (s_seafood_mode) {
            if (cm->label_2) lv_label_set_text(cm->label_2, "烤海鲜");
            if (cm->image_2) lv_obj_add_flag(cm->image_2, LV_OBJ_FLAG_HIDDEN);
            /* 隐藏鸡菜独立标签(烤全鸡/烤鸡翅/炸鸡中翅/炸鸡腿/烤鸡胸肉) */
            if (cm->label_4) lv_obj_add_flag(cm->label_4, LV_OBJ_FLAG_HIDDEN);
            if (cm->label_5) lv_obj_add_flag(cm->label_5, LV_OBJ_FLAG_HIDDEN);
            if (cm->label_6) lv_obj_add_flag(cm->label_6, LV_OBJ_FLAG_HIDDEN);
            if (cm->label_7) lv_obj_add_flag(cm->label_7, LV_OBJ_FLAG_HIDDEN);
            if (cm->label_8) lv_obj_add_flag(cm->label_8, LV_OBJ_FLAG_HIDDEN);
            static const char *sn[] = { "烤扇贝", "烤青口贝", "烤生蚝", "烤大虾", "烤鱿鱼" };
            lv_obj_t *dish_btns[] = { cm->wholechicken, cm->grillchickenwing, cm->friedchickenwing, cm->friedchickenleg, cm->grillchickenbreast };
            for (int i = 0; i < 5; i++) {
                if (dish_btns[i]) { lv_obj_t *c = lv_obj_get_child(dish_btns[i], 0); if (c) lv_label_set_text(c, sn[i]); }
            }
        }

        lv_obj_t *btns[] = {
            cm->wholechicken, cm->grillchickenwing, cm->friedchickenwing,
            cm->friedchickenleg, cm->grillchickenbreast,
        };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_chickenmenu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (s_seafood_mode) {
            if (cm->wholechicken)      lv_obj_add_event_cb(cm->wholechicken,      on_seafood_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_SEAFOOD_SCALLOP);
            if (cm->grillchickenwing)  lv_obj_add_event_cb(cm->grillchickenwing,  on_seafood_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_SEAFOOD_MUSSEL);
            if (cm->friedchickenwing)  lv_obj_add_event_cb(cm->friedchickenwing,  on_seafood_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_SEAFOOD_OYSTER);
            if (cm->friedchickenleg)   lv_obj_add_event_cb(cm->friedchickenleg,   on_seafood_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_SEAFOOD_PRAWN);
            if (cm->grillchickenbreast) lv_obj_add_event_cb(cm->grillchickenbreast, on_seafood_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_SEAFOOD_SQUID);
            /* 返回时恢复到进入前的菜(fish 子页同款机制) */
            if (g_six_bread_type == SIX_SEAFOOD_MUSSEL && cm->grillchickenwing)
                lv_group_focus_obj(cm->grillchickenwing);       /* 烤青口贝 */
            else if (g_six_bread_type == SIX_SEAFOOD_OYSTER && cm->friedchickenwing)
                lv_group_focus_obj(cm->friedchickenwing);       /* 烤生蚝 */
            else if (g_six_bread_type == SIX_SEAFOOD_PRAWN && cm->friedchickenleg)
                lv_group_focus_obj(cm->friedchickenleg);        /* 烤大虾 */
            else if (g_six_bread_type == SIX_SEAFOOD_SQUID && cm->grillchickenbreast)
                lv_group_focus_obj(cm->grillchickenbreast);     /* 烤鱿鱼 */
            else if (cm->wholechicken)
                lv_group_focus_obj(cm->wholechicken);           /* 烤扇贝/首次进入默认 */
        } else {
            if (cm->wholechicken)
                lv_obj_add_event_cb(cm->wholechicken, on_probe_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CHICK_WHOLE);
            if (cm->grillchickenwing)  lv_obj_add_event_cb(cm->grillchickenwing,  on_chickenmenu_weight_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CHICK_WING);
            if (cm->friedchickenwing)  lv_obj_add_event_cb(cm->friedchickenwing,  on_chickenmenu_weight_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CHICK_FRIED_WING);
            if (cm->friedchickenleg)   lv_obj_add_event_cb(cm->friedchickenleg,   on_chickenmenu_weight_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CHICK_FRIED_LEG);
            if (cm->grillchickenbreast) lv_obj_add_event_cb(cm->grillchickenbreast, on_chickenmenu_weight_dish_click, LV_EVENT_CLICKED, (void *)(intptr_t)SIX_CHICK_GRILL_BREAST);

            if (g_six_bread_type == SIX_CHICK_WING && cm->grillchickenwing)
                lv_group_focus_obj(cm->grillchickenwing);
            else if (g_six_bread_type == SIX_CHICK_FRIED_WING && cm->friedchickenwing)
                lv_group_focus_obj(cm->friedchickenwing);
            else if (g_six_bread_type == SIX_CHICK_FRIED_LEG && cm->friedchickenleg)
                lv_group_focus_obj(cm->friedchickenleg);
            else if (g_six_bread_type == SIX_CHICK_GRILL_BREAST && cm->grillchickenbreast)
                lv_group_focus_obj(cm->grillchickenbreast);
            else if (cm->wholechicken)
                lv_group_focus_obj(cm->wholechicken);
        }
    }
    current_group = g_chickenmenu;

    lv_scr_load_anim(chickenmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] rebuild: chickenmenu (child=%d, seafood=%d)\n", (int)child, s_seafood_mode);
}

/* 探针菜（烤全鸡/烤全鸭）：非探针模式入口仅跳探针提示页
   （完整探针流程在探针模式 sixmenutz→chickmenutz 里走） */
static void on_probe_dish_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = (uint8_t)(intptr_t)lv_event_get_user_data(e);
    g_toast_mode = TOAST_MODE_DEGREE;
    jump_to_probeneedtip();
}

/* 烤鸡翅类（份量驱动）：进 toastcolor 份量/种类组（选项/默认按当前菜配置表） */
static void on_chickenmenu_weight_dish_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    int t = (int)(intptr_t)lv_event_get_user_data(e);
    g_six_bread_type = (uint8_t)t;
    const chick_dish_t *c = chick_dish_cfg();
    if (c)
        toastcolor_set_weight_options(c->weights, c->count, c->default_idx);
    g_toast_mode = TOAST_MODE_WEIGHT;   /* 份量/种类组 */
    jump_to_toastcolor();
}

/* ================= duckmenu（鸭：烤全鸭） ================= */

void jump_to_duckmenu(void)
{
    page_push(PAGE_DUCK6MENU);
    lv_obj_clean(lv_scr_act());
    duckmenu_create(&ui_manager);

    duckmenu_t *dm = duckmenu_get(&ui_manager);
    if (dm) {
        lv_obj_t *btns[] = { dm->wholeduck };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_duckmenu) lv_group_del(g_duckmenu);
        g_duckmenu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (dm->wholeduck) {
            lv_obj_add_event_cb(dm->wholeduck, on_duckmenu_wholeduck_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(dm->wholeduck);
        }
    }
    current_group = g_duckmenu;

    lv_scr_load_anim(duckmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] jump: duckmenu\n");
}

void duckmenu_rebuild(page_id_t child)
{
    if (g_duckmenu) { lv_group_del(g_duckmenu); g_duckmenu = NULL; }
    lv_obj_clean(lv_scr_act());
    duckmenu_create(&ui_manager);

    duckmenu_t *dm = duckmenu_get(&ui_manager);
    if (dm) {
        if (s_meatdish_mode) {
            /* 肉菜模式：恢复自定义标签 */
            if (dm->label_1) lv_label_set_text(dm->label_1, "肉菜");
            if (dm->wholeduck) {
                lv_obj_t *child = lv_obj_get_child(dm->wholeduck, 0);
                if (child) lv_label_set_text(child, "烤香肠");
            }
            if (dm->label_2) lv_obj_add_flag(dm->label_2, LV_OBJ_FLAG_HIDDEN);
            if (dm->image_2) lv_obj_add_flag(dm->image_2, LV_OBJ_FLAG_HIDDEN);
        }

        lv_obj_t *btns[] = { dm->wholeduck };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_duckmenu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (dm->wholeduck) {
            lv_obj_add_event_cb(dm->wholeduck,
                s_meatdish_mode ? on_meatdish_sausage_click : on_duckmenu_wholeduck_click,
                LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(dm->wholeduck);
        }
    }
    current_group = g_duckmenu;

    lv_scr_load_anim(duckmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] rebuild: duckmenu (child=%d, meatdish=%d)\n", (int)child, s_meatdish_mode);
}

/* 烤全鸭：与烤全鸡同款探针流程（温度/描述按菜谱配置表） */
/* 烤全鸭：非探针模式入口仅跳探针提示页（完整流程在探针模式 chickmenutz 走） */
static void on_duckmenu_wholeduck_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = SIX_CHICK_DUCK_WHOLE;
    g_toast_mode = TOAST_MODE_DEGREE;
    jump_to_probeneedtip();
}

/* ================= meatdishmenu（肉菜：复用 duckmenu UI） ================= */

/* 烤香肠份量表 */
static const int w_sausage_w[] = { 200, 400, 600, 800 };

static void on_meatdish_sausage_click(lv_event_t *e)
{
    (void)e;
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = SIX_MEAT_SAUSAGE;
    toastcolor_set_weight_options(w_sausage_w, 4, 2);  /* 默认600g */
    g_toast_mode = TOAST_MODE_WEIGHT;
    jump_to_toastcolor();
}

void jump_to_meatdish_menu(void)
{
    s_meatdish_mode = 1;
    page_push(PAGE_DUCK6MENU);
    lv_obj_clean(lv_scr_act());
    duckmenu_create(&ui_manager);

    duckmenu_t *dm = duckmenu_get(&ui_manager);
    if (dm) {
        /* label1 = 肉菜, 按钮label = 烤香肠, 隐藏 img2/label2 */
        if (dm->label_1) lv_label_set_text(dm->label_1, "肉菜");
        if (dm->wholeduck) {
            lv_obj_t *child = lv_obj_get_child(dm->wholeduck, 0);
            if (child) lv_label_set_text(child, "烤香肠");
        }
        if (dm->label_2) lv_obj_add_flag(dm->label_2, LV_OBJ_FLAG_HIDDEN);
        if (dm->image_2) lv_obj_add_flag(dm->image_2, LV_OBJ_FLAG_HIDDEN);

        lv_obj_t *btns[] = { dm->wholeduck };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_duckmenu) lv_group_del(g_duckmenu);
        g_duckmenu = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (dm->wholeduck) {
            lv_obj_add_event_cb(dm->wholeduck, on_meatdish_sausage_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(dm->wholeduck);
        }
    }
    current_group = g_duckmenu;

    lv_scr_load_anim(duckmenu_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] jump: meatdishmenu\n");
}

/* ================= fishmenu（鱼/海鲜：复用 chick6menu / chickenmenu UI） ================= */

static void on_fish_grillfish_click(lv_event_t *e)
{
    (void)e;
    if (screen_is_loading(lv_scr_act())) return;
    if (s_fish_mode == 1) {
        /* 鱼/海鲜首页 → 烤鱼子页 */
        s_fish_mode = 2;
        chick6menu_t *cm = chick6menu_get(&ui_manager);
        if (cm) {
            if (cm->label_1) lv_label_set_text(cm->label_1, "烤鱼");
            if (cm->chicken) { lv_obj_t *c = lv_obj_get_child(cm->chicken, 0); if (c) lv_label_set_text(c, "烤鳕鱼"); }
            if (cm->duck)    { lv_obj_t *c = lv_obj_get_child(cm->duck, 0);    if (c) lv_label_set_text(c, "烤全鱼"); }
        }
    } else if (s_fish_mode == 2) {
        /* 烤鱼子页 → 烤鳕鱼 */
        on_fish_cod_click(e);
    }
}
static void on_fish_seafood_click(lv_event_t *e)
{
    (void)e;
    if (screen_is_loading(lv_scr_act())) return;
    if (s_fish_mode == 1) {
        /* 鱼/海鲜首页 → 烤海鲜 */
        s_fish_home_focus = 1;   /* 记录从"烤海鲜"进入(返回首页时焦点恢复) */
        s_seafood_mode = 1;
        jump_to_chickenmenu();
    } else if (s_fish_mode == 2) {
        /* 烤鱼子页 → 烤全鱼 */
        on_fish_wholefish_click(e);
    }
}
static void on_fish_cod_click(lv_event_t *e)
{
    (void)e;
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = SIX_FISH_COD;
    toastcolor_set_weight_options(w_cod_w, 4, 2);   /* 默认800g */
    g_toast_mode = TOAST_MODE_WEIGHT;
    jump_to_toastcolor();
}

static void on_fish_wholefish_click(lv_event_t *e)
{
    (void)e;
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = SIX_FISH_WHOLEFISH;
    toastcolor_set_weight_options(w_wf_w, 4, 2);    /* 默认800g */
    g_toast_mode = TOAST_MODE_WEIGHT;
    jump_to_toastcolor();
}
static void on_seafood_dish_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    g_six_bread_type = (uint8_t)(intptr_t)lv_event_get_user_data(e);
    jump_to_descriptionmenu();   /* 固定参数菜:无份量/烤色,直接描述页 */
}

void jump_to_fish_menu(void)
{
    s_fish_mode = 1;   /* 鱼/海鲜首页:复用 chick6menu,由 jump_to_chick6menu 完成设置 */
    jump_to_chick6menu();
}

/* ================= probeneedtip（烤鸡探针提示页） =================
 * 未插探针时的提示页：仅 sure 进焦点组；点 sure 或 BACK 都回到 chickenmenu。 */

/* 探针提示页左上角菜名（按进入的菜） */
static const char *probeneedtip_dish_name(void)
{
    switch (g_six_bread_type) {
    case SIX_CHICK_WHOLE:        return "烤全鸡";
    case SIX_CHICK_DUCK_WHOLE:   return "烤全鸭";
    case SIX_MEAT_GRILL_STEAK:   return "烤牛排";
    case SIX_MEAT_GRILL_BEEF:    return "烤牛肉";
    case SIX_MEAT_GRILL_LEG:     return "烤羊腿";
    case SIX_MEAT_GRILL_LAMBS:   return "烤羊排";
    default:                     return "第六感";
    }
}

void jump_to_probeneedtip(void)
{
    page_push(PAGE_PROBENEEDTIP);
    lv_obj_clean(lv_scr_act());
    probeneedtip_create(&ui_manager);

    probeneedtip_t *pt = probeneedtip_get(&ui_manager);
    if (pt) {
        /* 左上角显示进入时的菜名 */
        if (pt->name)
            lv_label_set_text(pt->name, probeneedtip_dish_name());

        lv_obj_t *btns[] = { pt->sure };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_probeneedtip) lv_group_del(g_probeneedtip);
        g_probeneedtip = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (pt->sure) {
            lv_obj_add_event_cb(pt->sure, on_probeneedtip_sure_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(pt->sure);
        }
    }
    current_group = g_probeneedtip;

    lv_scr_load_anim(probeneedtip_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] jump: probeneedtip\n");
}

/* sure：弹出提示页，回到 chickenmenu（与 BACK 行为一致） */
static void on_probeneedtip_sure_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    page_pop();
    printf("[six_chicken] probeneedtip sure -> back to chickenmenu\n");
}

/* ================= 烤全鸡烹饪页（chickencooking UI） =================
 * 探针温度标记完成（无倒计时）：
 *   进度条 = 实时探针温度 → 设置探针温度的进度，bar 从 13 起、bartemp 跟随，
 *   与 preheatcooking 的 preheat_progress/preheat_update_bar 逻辑一致（13+87×推进）。
 * BACK → 遮罩确认（text1/2 + 按钮变"确 定"）；完成 → text1/2 高温防烫提示 + 隐藏按钮。 */

lv_group_t *g_chick_cooking = NULL;
static uint8_t s_chick_running = 0;   /* 运行中 */
static uint8_t s_chick_paused = 0;    /* 暂停中 */
static uint8_t s_chick_overlay = 0;   /* 遮罩确认态 */
static uint8_t s_chick_done = 0;      /* 完成态（高温防烫提示，按钮隐藏） */
static int s_chick_start_probe = 0;   /* 进入时起始探针温度（进度条 13 基准） */

static void six_chick_timer_cb(lv_timer_t *timer);
static void six_chick_apply_display(void);
static void six_chick_exit_to_menu(void);

static void on_chick_cooking_stop_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    if (s_chick_done && !s_chick_overlay) return; /* 完成页(无遮罩)按钮已隐藏，不会触发 */
    if (s_chick_overlay) {                    /* 遮罩/完成确认 → 退出回鸡菜菜单 */
        six_chick_exit_to_menu();
        return;
    }
    if (!s_chick_paused) {
        /* 暂停 */
        if (is_door_open()) { g_send.buzzer_req = BUZZER_KEY_INVALID; return; }
        cook_elapsed_saved = lv_tick_get() - cook_start_time;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        s_chick_paused = 1;
        g_send.iface_status = IFACE_PAUSE;
    } else {
        /* 恢复 */
        if (is_door_open()) { g_send.buzzer_req = BUZZER_KEY_INVALID; return; }
        cook_start_time = lv_tick_get() - cook_elapsed_saved;
        s_chick_paused = 0;
        s_chick_running = 1;
        g_send.iface_status = IFACE_COOKING;
        if (cook_timer) lv_timer_del(cook_timer);
        cook_timer = lv_timer_create(six_chick_timer_cb, 1000, NULL);
    }
    six_chick_apply_display();
}

/* 探针菜烹饪页（烤全鸡/烤全鸭，探针温度标记完成，无倒计时） */
void jump_to_chick_cooking(void)
{
    if (is_door_open()) {   /* 门开不直接进烹饪（与面包 jump_to_six_cooking 一致） */
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    page_push(PAGE_CHICKENCOOKING);
    lv_obj_clean(lv_scr_act());
    chickencooking_create(&ui_manager);

    chickencooking_t *ck = chickencooking_get(&ui_manager);
    if (ck) {
        lv_obj_t *btns[] = { ck->stop };
        const int n = (int)(sizeof(btns) / sizeof(btns[0]));
        for (int k = 0; k < n; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_chick_cooking) lv_group_del(g_chick_cooking);
        g_chick_cooking = group_create_for_page(btns, n);
        clear_focus_states(btns, n);

        if (ck->stop) {
            lv_obj_add_event_cb(ck->stop, on_chick_cooking_stop_click, LV_EVENT_CLICKED, NULL);
            lv_group_focus_obj(ck->stop);
        }
        /* icon：六感图标 */
        if (ck->icon) lv_img_set_src(ck->icon, LVGL_IMAGE_PATH(sixicon.png));
    }
    current_group = g_chick_cooking;

    s_chick_running = 1;
    s_chick_paused = 0;
    s_chick_overlay = 0;
    s_chick_done = 0;
    s_chick_start_probe = get_probe_temp();   /* 进度条起点：起始探针温度 */
    cook_elapsed_saved = 0;
    cook_start_time = lv_tick_get();
    const chick_probe_t *pcfg = chick_probe_cfg();
    cook_total_ms = (pcfg ? pcfg->max_min : 80) * 60 * 1000;   /* 探针异常时兜底，非倒计时 */

    g_send.iface_status = IFACE_COOKING;
    g_send.cook_mode = six_chick_mode();   /* 烤全鸡/烤全鸭:热风对流 */
    g_send.set_temp = six_chick_temp();
    g_send.set_temp_lower = 0;
    g_send.cook_flag = 2;                     /* 探针模式 */
    g_send.probe_temp = (uint8_t)g_six_probe_temp;
    g_send.remaining_ms = 0;   /* 探针驱动：无倒计时 */

    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(six_chick_timer_cb, 1000, NULL);
    six_chick_apply_display();

    lv_scr_load_anim(chickencooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_chicken] jump: chick cooking (probe target=%d)\n", g_six_probe_temp);
}

/* 进度条：与 preheatcooking 一致——起始为 13，按探针温度推进到 100 */
static int six_chick_bar_value(int cur)
{
    int range = g_six_probe_temp - s_chick_start_probe;
    if (range <= 0) return 100;
    int p = 13 + (int)((int64_t)87 * (cur - s_chick_start_probe) / range);
    if (p < 13) p = 13;
    if (p > 100) p = 100;
    return p;
}

/* 烤色程度文字：按已选探针目标温度映射（阈值按菜谱配置） */
const char *six_chick_degree_text(void)
{
    const chick_probe_t *p = chick_probe_cfg();
    if (!p) return "中等色";
    if (g_six_probe_temp <= p->probe[0]) return "浅色";
    if (g_six_probe_temp >= p->probe[2]) return "深色";
    return "中等色";
}
const char *six_chick_degree_short(void)
{
    const chick_probe_t *p = chick_probe_cfg();
    if (!p) return "中等";
    if (g_six_probe_temp <= p->probe[0]) return "浅";
    if (g_six_probe_temp >= p->probe[2]) return "深";
    return "中等";
}

/* 烹饪页显示刷新 */
static void six_chick_apply_display(void)
{
    chickencooking_t *ck = chickencooking_get(&ui_manager);
    if (!ck) return;

    /* 探针实时温度（无效/未插按 0，封顶到目标温度） */
    int cur = get_probe_temp();
    if (cur < 0) cur = 0;
    if (cur > g_six_probe_temp) cur = g_six_probe_temp;

    /* status:菜名+烤色程度（浅色/中等色/深色） */
    if (ck->status)
        lv_label_set_text_fmt(ck->status, "| %s | %s |", six_chick_name(), six_chick_degree_text());

    /* cookstatus:烹饪中/暂停中/已完成（遮罩确认态不改 cookstatus） */
    if (ck->cookstatus) {
        if (s_chick_done)           lv_label_set_text(ck->cookstatus, "已完成");
        else if (s_chick_paused)    lv_label_set_text(ck->cookstatus, "暂停中...");
        else                        lv_label_set_text(ck->cookstatus, "烹饪中...");
    }

    /* 按钮:遮罩→"确 定";暂停→"开 始";运行→"暂 停";完成页(无遮罩)→隐藏 */
    if (ck->stop) {
        lv_obj_t *bl = lv_obj_get_child(ck->stop, 0);
        if (s_chick_done && !s_chick_overlay) {
            lv_obj_add_flag(ck->stop, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(ck->stop, LV_OBJ_FLAG_HIDDEN);
            if (bl) lv_label_set_text(bl, s_chick_overlay ? "确 定" :
                                             s_chick_paused  ? "开 始" : "暂 停");
        }
    }

    /* 遮罩容器 + 右侧 text1/text2：
     遮罩容器仅返回确定确认态显示（完成页不遮罩）；
     text1/text2 在确认界面（遮罩/完成页）显示，运行与暂停时隐藏 */
    int show_tip = (s_chick_overlay || s_chick_done);
    if (ck->container_1) {
        if (s_chick_overlay) lv_obj_clear_flag(ck->container_1, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(ck->container_1, LV_OBJ_FLAG_HIDDEN);
    }
    if (ck->text1) {
        if (show_tip) {
            lv_obj_clear_flag(ck->text1, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ck->text1, (s_chick_done && !s_chick_overlay) ?
                              "高温防烫" : "是否结束当前任务");
        } else {
            lv_obj_add_flag(ck->text1, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (ck->text2) {
        if (show_tip) {
            lv_obj_clear_flag(ck->text2, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(ck->text2, (s_chick_done && !s_chick_overlay) ?
                              "请缓慢打开门体！" : "回到主页");
        } else {
            lv_obj_add_flag(ck->text2, LV_OBJ_FLAG_HIDDEN);
        }
    }

    /* 进度条 + bartemp：探针温度 起始→目标，bartemp 跟随进度条移动（preheat 同款算法） */
    if (ck->bar_2) {
        int p = six_chick_bar_value(cur);
        lv_bar_set_range(ck->bar_2, 0, 100);
        lv_bar_set_value(ck->bar_2, p, LV_ANIM_OFF);
        if (ck->bartemp) {
            lv_label_set_text_fmt(ck->bartemp, "%d℃", cur);
            int bx = 122 + (637 * p) / 100 - 80;
            lv_obj_set_pos(ck->bartemp, bx, 323);
        }
    }
}

/* 1s 定时器：探针温度标记完成（无倒计时；最长时长仅兜底） */
static void six_chick_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if (!s_chick_running || s_chick_paused) return;

    /* 门开自动暂停 */
    if (is_door_open()) {
        cook_elapsed_saved = lv_tick_get() - cook_start_time;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        s_chick_paused = 1;
        g_send.iface_status = IFACE_PAUSE;
        six_chick_apply_display();
        return;
    }

    uint32_t elapsed = lv_tick_get() - cook_start_time;
    /* 完成：探针达到目标温度；或超最长安全时间（探针异常兜底） */
    if (get_probe_temp() >= g_six_probe_temp || elapsed >= (uint32_t)cook_total_ms) {
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        s_chick_running = 0;
        s_chick_done = 1;
        s_chick_overlay = 0;   /* 完成页:高温防烫提示、按钮隐藏、无遮罩；BACK 再进返回确定页 */
        g_send.iface_status = IFACE_COMPLETE;
        g_send.cook_flag = 0;               /* 退出探针模式标记 */
        g_send.remaining_ms = 0;
        g_send.buzzer_req = BUZZER_COOK_DONE;
        six_chick_apply_display();
        return;
    }
    g_send.remaining_ms = 0;   /* 探针驱动：无剩余时间 */
    six_chick_apply_display();
}

/* 烤全鸡烹饪页 BACK */
void six_chick_handle_back(void)
{
    if (s_chick_done) {
        /* 完成页:BACK → 返回确定确认页（再按 BACK 回完成页；确定→退出） */
        s_chick_overlay = !s_chick_overlay;
        six_chick_apply_display();
        printf("[six_chicken] done back -> overlay=%d\n", s_chick_overlay);
        return;
    }
    if (s_chick_overlay) {        /* 已在遮罩：取消遮罩继续烹饪 */
        s_chick_overlay = 0;
        six_chick_apply_display();
        return;
    }
    s_chick_overlay = 1;          /* 显示遮罩确认（text1/2 + 按钮变"确 定"） */
    six_chick_apply_display();
    printf("[six_chicken] overlay\n");
}

/* 退出：回到第六感主菜单（弹掉 cooking + toastcolor，不残留栈） */
static void six_chick_exit_to_menu(void)
{
    s_chick_running = 0;
    s_chick_paused = 0;
    s_chick_overlay = 0;
    s_chick_done = 0;
    s_chick_start_probe = 0;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    cook_elapsed_saved = 0; cook_bar_saved = 0; cook_total_ms = 0;
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.cook_flag = 0;
    g_send.set_temp = 0; g_send.set_temp_lower = 0; g_send.remaining_ms = -1;
    g_send.probe_temp = 0;
    depth = 0;
    page_push(PAGE_WAITMENU_24);
    if (is_probe_inserted())
        jump_to_sixmenutz();   /* 探针模式：返回主页 = 探针版第六感菜单 */
    else
        jump_to_sixmenu();     /* 非探针（异常路径兜底） */
    printf("[six_chicken] exit -> %s\n", is_probe_inserted() ? "sixmenutz" : "sixmenu");
}