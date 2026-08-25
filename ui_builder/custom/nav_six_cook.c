/*
 * nav_six_cook.c - 第六感运行核心
 * 发酵→烹饪→询问(烤色)→烤色→额外上色循环，
 * 状态机+定时驱动，负责六感全流程的通讯状态与页面跳转。
 */
#include "nav.h"
#include "protocol.h"
#include "custom_defs.h"

/* ==============================
 * 第六感-面包卷 运行流程（复用 somecook_cooking 页面 UI）
 * 发酵(45min) → 烹饪(20min, 160℃) → 完成询问烤色 → 上色准备 → 上色(2/4/6min) → 再询问(循环)
 * 独立状态机/组/timer，不干扰多段烹饪的 cooking_timer_cb 逻辑
 * ============================== */

/* ==============================
 * 第六感-面包类 运行流程（复用 somecook_cooking 页面 UI）
 * 发酵(45min) → 烹饪 → 完成询问烤色(可选) → 上色准备 → 上色 → 再询问(循环)
 * 支持四个面包品类：面包卷 / 全麦面包 / 土司 / 可颂
 * ============================== */

lv_group_t *g_six_cooking = NULL;
uint8_t g_six_running = 0;
int g_six_color_min = 4;   /* 当前烤色分钟 */

/* 六感菜品类（面包 + 蛋糕共用同一套 cooking/description 流程） */
#define SIX_BREAD_ROLL      0   /* 面包卷 */
#define SIX_BREAD_WHEAT     1   /* 全麦面包 */
#define SIX_BREAD_TOAST     2   /* 土司 */
#define SIX_BREAD_CROISSANT 3   /* 可颂 */
#define SIX_CAKE_SWISSROLL      4   /* 瑞士卷 */
#define SIX_CAKE_STRUDEL        5   /* 果馅卷 */
#define SIX_CAKE_MADELEINE      6   /* 玛德琳 */
#define SIX_CAKE_SPONGECAKE     7   /* 海绵蛋糕 */
#define SIX_CAKE_CUPCAKE        8   /* 纸杯蛋糕 */
#define SIX_CAKE_CHOCOLATECAKE  9   /* 巧克力蛋糕 */
#define SIX_CAKE_CREAMPUFF      10  /* 泡芙 */
#define SIX_CAKE_EGGTART        11  /* 蛋挞 */
#define SIX_CAKE_MILLEFEUILLE   12  /* 千层酥 */
#define SIX_CAKE_COOKIES        13  /* 曲奇饼干 */
#define SIX_CAKE_MUFFIN         14  /* 松饼 */
uint8_t g_six_bread_type = SIX_BREAD_ROLL;

/* 每菜配置 */
typedef struct {
    const char *name;          /* 显示名称 */
    uint8_t mode;              /* 烹饪模式 */
    int cook_temp;             /* 烹饪温度(上腔) */
    int cook_sec;              /* 烹饪秒数 */
    int color_min[3];          /* 烤色分钟(浅/中/深) */
    int has_color;             /* 是否支持烤色加强 */
    int has_rising;            /* 是否有发酵阶段 */
    int cook_min;              /* 烹饪分钟(descriptionmenu 显示) */
    const char *cook_desc;     /* 烹饪说明(descriptionmenu 显示) */
} six_bread_cfg_t;

static const six_bread_cfg_t s_bread_cfg[SIX_CAKE_MUFFIN + 1] = {
    /* 面包卷 */
    { "面包卷", MODE_UPDOWN_BBQ, 160, 1 * 60, {2, 4, 6}, 1, 1, 1,  /* 调试:1分钟 */
      "烹饪说明：\n根据你最喜欢的食谱准备面团，放在烤盘上\n现在将食物放在第3层\n使用网架和盘" },
    /* 全麦面包 */
    { "全麦面包", MODE_UPDOWN_BBQ, 160, 1 * 60, {2, 4, 6}, 1, 1, 1,  /* 调试:1分钟 */
      "烹饪说明：\n根据你最喜欢的食谱准备面团，放在烤盘上\n现在将食物放在第3层\n使用烤盘" },
    /* 土司 */
    { "土司", MODE_UPDOWN_BBQ, 170, 1 * 60, {3, 6, 10}, 1, 1, 1,  /* 调试:1分钟 */
      "烹饪说明：\n按照你喜欢的白面包配方准备面团，面团卷成长条状，并把它放在烤盘中\n现在将食物放在第2层\n使用网架和面包容器" },
    /* 可颂(热风对流 windchange, 无烤色) */
    { "可颂", MODE_WINDCHANGE_BBQ, 180, 1 * 60, {0, 0, 0}, 0, 1, 1,  /* 调试:1分钟 */
      "烹饪说明：\n根据你最喜欢的食谱准备面团，放在烤盘上\n现在将食物放在第3层\n使用烤盘" },
    /* 瑞士卷(蛋糕:上下烧烤, 无发酵无烤色) */
    { "瑞士卷", MODE_UPDOWN_BBQ, 160, 1 * 60, {0, 0, 0}, 0, 0, 1,   /* 调试:1分钟 */
      "烹饪说明：\n准备500~900g无脂肪海绵蛋糕面糊。倒入铺了烹调纸并刷油的烤盘中\n现在将食物放在第3层\n使用网架和盘" },
    /* 果馅卷(热风 hotwind, 无发酵无烤色) */
    { "果馅卷", MODE_HOTWIND_BBQ, 200, 1 * 60, {2, 4, 6}, 1, 0, 1,   /* 调试:1分钟; 烤色 轻+2/中+4/深+6 */
      "烹饪说明：\n根据你喜欢的食谱准备面团，放在烤盘上。烹饪结束后，让它冷却下来，然后撒上糖粉\n现在将食物放在第3层\n使用烤盘" },
    /* 玛德琳(上下烧烤, 无发酵无烤色) */
    { "玛德琳", MODE_UPDOWN_BBQ, 190, 1 * 60, {0, 0, 0}, 0, 0, 1,   /* 调试:1分钟 */
      "烹饪说明：\n根据你最喜欢的食谱准备和冷却面糊。将冷面糊直接倒入涂油的玛德琳烤盘\n现在将食物放在第3层\n使用网架和蛋糕模具盘" },
    /* 海绵蛋糕(上下烧烤, 无发酵无烤色) */
    { "海绵蛋糕", MODE_UPDOWN_BBQ, 180, 1 * 60, {3, 5, 10}, 1, 0, 1,   /* 调试:1分钟; 烤色 轻+3/中+5/深+10 */
      "烹饪说明：\n准备无脂海绵蛋糕面糊500-900克。倒入涂了油的烤盘\n现在将食物放在第2层\n使用网架和模具盘" },
    /* 纸杯蛋糕(上下烧烤, 无发酵无烤色) */
    { "纸杯蛋糕", MODE_UPDOWN_BBQ, 160, 1 * 60, {0, 0, 0}, 0, 0, 1,   /* 调试:1分钟 */
      "烹饪说明：\n根据你喜欢的食谱准备面糊，每个纸杯填满2/3左右。烹饪结束后移到网架冷却\n现在将食物放在第3层\n使用烤盘" },
    /* 巧克力蛋糕(上下烧烤, 无发酵无烤色) */
    { "巧克力蛋糕", MODE_UPDOWN_BBQ, 160, 1 * 60, {0, 0, 0}, 0, 0, 1,   /* 调试:1分钟 */
      "烹饪说明：\n准备500~900g巧克力蛋糕面糊。倒入铺了烹调纸并刷油的烤盘中\n现在将食物放在第2层\n使用网架和模具盘" },
    /* 泡芙(上下烧烤, 无发酵无烤色) */
    { "泡芙", MODE_UPDOWN_BBQ, 170, 1 * 60, {0, 0, 0}, 0, 0, 1,   /* 调试:1分钟 */
      "烹饪说明：\n根据你最喜欢的食谱准备面团，放到烤盘上。烹饪结束时移到网架\n现在将食物放在第3层\n使用烤盘" },
    /* 蛋挞(上下烧烤, 无发酵无烤色) */
    { "蛋挞", MODE_UPDOWN_BBQ, 230, 1 * 60, {0, 0, 0}, 0, 0, 1,   /* 调试:1分钟 */
      "烹饪说明：\n根据你最喜欢的食谱准备面团和蛋奶冻。将面团压入模具中，倒入蛋奶液至八成\n现在将食物放在第3层\n使用烤盘" },
    /* 千层酥(热风 hotwind, 无发酵无烤色) */
    { "千层酥", MODE_HOTWIND_BBQ, 200, 1 * 60, {0, 0, 0}, 0, 0, 1,   /* 调试:1分钟 */
      "烹饪说明：\n根据你喜欢的食谱准备酥皮和冰奶油。将酥皮放在烤盘上，撒上糖粉\n现在将食物放在第3层\n使用烤盘" },
    /* 曲奇饼干(上下烧烤, 无发酵无烤色) */
    { "曲奇饼干", MODE_UPDOWN_BBQ, 160, 1 * 60, {0, 0, 0}, 0, 0, 1,   /* 调试:1分钟 */
      "烹饪说明：\n根据你最喜欢的食谱准备面团。团出1.5汤匙大小的面团，放在烤盘上\n现在将食物放在第3层\n使用烤盘" },
    /* 松饼(上下烧烤, 无发酵无烤色) */
    { "松饼", MODE_UPDOWN_BBQ, 180, 1 * 60, {0, 0, 0}, 0, 0, 1,   /* 调试:1分钟 */
      "烹饪说明：\n根据你最喜欢的食谱准备面糊，放在烤盘上\n现在将食物放在第3层\n使用烤盘" },
};

/* 越界钳制: g_six_bread_type 异常时回退到面包卷,防数组越界死机 */
static uint8_t six_bread_type_safe(void)
{
    return (g_six_bread_type <= SIX_CAKE_MUFFIN) ? g_six_bread_type : SIX_BREAD_ROLL;
}
const six_bread_cfg_t *six_bread_cfg(void) { return &s_bread_cfg[six_bread_type_safe()]; }
const char *six_bread_name(void)     { return tr(six_bread_cfg()->name); }
const char *six_bread_desc(void)     { return tr(six_bread_cfg()->cook_desc); }
int six_bread_cook_min(void)         { return six_bread_cfg()->cook_min; }
int six_bread_has_color(void)  { return (six_chick_is_kind() || six_chick_is_seafood() || six_chick_is_veg() || six_chick_is_jacket()) ? 0 : six_bread_cfg()->has_color; }   /* 烤鸡翅类/海鲜/蔬菜:无烤色 */
int six_bread_has_rising(void) { return (six_chick_is_kind() || six_chick_is_seafood() || six_chick_is_veg() || six_chick_is_jacket()) ? 0 : six_bread_cfg()->has_rising; }  /* 烤鸡翅类/海鲜/蔬菜:无发酵 */
int six_bread_color_min(int level)   /* 1浅 2中 3深 */
{
    const six_bread_cfg_t *cfg = six_bread_cfg();
    if (level < 1 || level > 3) return 0;
    return cfg->color_min[level - 1];
}

enum {
    SIX_PHASE_RISING,      /* 发酵 45min */
    SIX_PHASE_COOKING,     /* 烹饪 */
    SIX_PHASE_ASK,         /* 完成:询问是否需要烤色(有烤色) */
    SIX_PHASE_COLOR_SETUP, /* 上色准备:等待"开 始" */
    SIX_PHASE_COLOR_COOKING, /* 上色烹饪 */
    SIX_PHASE_ASK_COLOR,   /* 上色完成:询问是否还需要烤色 */
};
static uint8_t g_six_phase = SIX_PHASE_COOKING;
static uint8_t g_six_paused = 0;
static uint8_t g_six_overlay = 0;   /* 遮罩确认态 */
static uint8_t g_six_from = SIX_PHASE_COOKING;  /* 遮罩进入源 */
static uint8_t g_six_has_rising = 0;   /* 是否有发酵段 */

#define SIX_RISING_SEC     (1 * 60)   /* 调试:临时 1 分钟,正式 45 分钟 */
int six_rising_min(void)  { return SIX_RISING_SEC / 60; }   /* 发酵分钟(预计烹饪时间显示) */
/* 烹饪秒数按菜: SIX_COOKING_SEC() 宏 */
#define SIX_COOKING_SEC(cfg)    ((cfg)->cook_sec)

static void six_cook_apply_display(void);
static void six_cook_exit(void);
static void six_cook_set_phase(int phase);

// 烹饪阶段秒数:烤鸡翅类按所选份量对应时间,面包/蛋糕按配置
static int32_t six_cook_sec(void)
{
    if (six_chick_is_degree_time()) {
        int d = toastcolor_degree_value();   /* 烤羊肉串:程度→时间 */
        if (d < 1 || d > 3) d = 2;
        return six_chick_degree_min(d) * 60;
    }
    if (six_chick_is_kind()) {
        int w = toastcolor_weight_value();
        if (w < 0) w = 800;   /* 兜底 */
        return six_chick_cook_min(w) * 60;
    }
    if (six_chick_is_seafood()) {           /* 烤海鲜:固定时长 */
        const seafood_dish_t *sd = seafood_dish_cfg();
        return (sd ? sd->cook_min : 18) * 60;
    }
    if (six_chick_is_veg()) {               /* 蔬菜:固定时长 */
        const seafood_dish_t *sd = veg_fixed_cfg();
        return (sd ? sd->cook_min : 22) * 60;
    }
    if (six_chick_is_jacket()) {            /* 烤带皮土豆:份量×程度 */
        return jacket_cook_min() * 60;
    }
    return six_bread_cfg()->cook_sec;
}

// 烹饪页 status 文案:程度→时间=菜名+程度+时间;份量=菜名+克重+时间;面包/蛋糕=菜名+时间
static void six_label_status(somecook_cooking_t *sc)
{
    if (six_chick_is_degree_time()) {
        int d = toastcolor_degree_value();
        if (d < 1 || d > 3) d = 2;
        const char *dt = (d == 1) ? "浅色" : (d == 3) ? "深色" : "中等色";
        lv_label_set_text_fmt(sc->label_12, tr("| %s | %s | %d分钟 |"),
                              six_chick_name(), dt, six_chick_degree_min(d));
    } else if (six_chick_is_kind()) {
        int w = toastcolor_weight_value();
        if (w < 0) w = 800;   /* 兜底 */
        lv_label_set_text_fmt(sc->label_12, tr("| %s | %d | %s色 |"),
                              six_chick_name(), w, toastcolor_weight_unit());
    } else if (six_chick_is_seafood()) {
        const seafood_dish_t *sd = seafood_dish_cfg();
        lv_label_set_text_fmt(sc->label_12, tr("| %s | %d分钟 |"),
                              six_chick_name(), sd ? sd->cook_min : 18);

    } else if (six_chick_is_veg()) {
        const seafood_dish_t *sd = veg_fixed_cfg();
        lv_label_set_text_fmt(sc->label_12, tr("| %s | %d分钟 |"),
                              six_chick_name(), sd ? sd->cook_min : 22);
    } else if (six_chick_is_jacket()) {
        lv_label_set_text_fmt(sc->label_12, tr("| %s | %dg | %s色 |"),
                              six_chick_name(), jacket_weight(), jacket_deg_text());
    } else {
        lv_label_set_text_fmt(sc->label_12, tr("| %s | %d分钟"), six_bread_name(), six_bread_cfg()->cook_min);
    }
}

// 运行模式/温度:烤鸡翅类按菜谱表(热风对流/空气炸250℃),面包/蛋糕按配置
static uint8_t six_cook_mode(void)
{
    if (six_chick_is_kind() || six_chick_is_seafood() || six_chick_is_veg() || six_chick_is_jacket()) return six_chick_mode();
    return six_bread_cfg()->mode;
}
static int six_cook_temp(void)
{
    if (six_chick_is_kind() || six_chick_is_seafood() || six_chick_is_veg() || six_chick_is_jacket()) return six_chick_temp();
    return six_bread_cfg()->cook_temp;
}

// 当前阶段秒数
static int32_t six_phase_sec(int phase)
{
    switch (phase) {
    case SIX_PHASE_RISING:        return SIX_RISING_SEC;
    case SIX_PHASE_COOKING:       return six_cook_sec();
    case SIX_PHASE_COLOR_COOKING: return g_six_color_min * 60;
    default:                      return 0;
    }
}

// 已走秒数（暂停时用保存值）
static int32_t six_elapsed_sec(void)
{
    if (g_six_paused)
        return (int32_t)(cook_elapsed_saved / 1000);
    return (int32_t)((lv_tick_get() - cook_start_time) / 1000);
}

// 倒计时显示秒（发酵+烹饪走总时长；上色独立）
static int32_t six_remaining_sec(void)
{
    int32_t e = six_elapsed_sec();
    int32_t ph = six_phase_sec(g_six_phase);
    if (e > ph) e = ph;
    switch (g_six_phase) {
    case SIX_PHASE_RISING:        return (SIX_RISING_SEC + six_cook_sec()) - e;
    case SIX_PHASE_COOKING:
        return g_six_has_rising ? (SIX_RISING_SEC + six_cook_sec()) - (SIX_RISING_SEC + e)
                                : six_cook_sec() - e;
    case SIX_PHASE_COLOR_COOKING: return ph - e;
    default:                      return 0;
    }
}

// 显示刷新：按当前 phase/遮罩/暂停刷新页面元素
static void six_cook_apply_display(void)
{
    somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
    if (!sc) return;

    /* 防御:所需子对象任一缺失则跳过刷新,避免空指针崩溃 */
    if (!sc->icon || !sc->cookstatus || !sc->image_10 || !sc->bar_1 ||
        !sc->stop || !sc->activestatus || !sc->container_1 ||
        !sc->text1 || !sc->text2 || !sc->timelabel || !sc->label_12)
        return;

    /* 基础显隐 */
    lv_obj_clear_flag(sc->icon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->cookstatus, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->image_10, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->bar_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(sc->stop, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sc->activestatus, LV_OBJ_FLAG_HIDDEN);   /* 恒隐藏 */
    lv_obj_add_flag(sc->container_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sc->text1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(sc->text2, LV_OBJ_FLAG_HIDDEN);
    /* 提示激活期间不恢复 timelabel 显示(无效提示隐藏右侧元素,防被每秒刷新覆盖) */
    if (!nav_hint_active())
        lv_obj_clear_flag(sc->timelabel, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *bl = lv_obj_get_child(sc->stop, 0);

    switch (g_six_phase) {
    case SIX_PHASE_RISING:
        lv_label_set_text(sc->cookstatus, g_six_paused ? tr("暂停中...") : tr("发酵中..."));
        lv_label_set_text_fmt(sc->label_12, tr("| %s | 45℃ | 45分钟"), six_bread_name());
        if (bl) lv_label_set_text(bl, g_six_paused ? tr("开 始") : tr("暂 停"));
        break;
    case SIX_PHASE_COOKING:
        lv_label_set_text(sc->cookstatus, g_six_paused ? tr("暂停中...") : tr("烹饪中..."));
        six_label_status(sc);   /* 烤鸡翅=菜名+克重+时间 */
        if (bl) lv_label_set_text(bl, g_six_paused ? tr("开 始") : tr("暂 停"));
        break;
    case SIX_PHASE_ASK:
    case SIX_PHASE_ASK_COLOR:
        lv_obj_add_flag(sc->timelabel, LV_OBJ_FLAG_HIDDEN);
        six_label_status(sc);   /* 完成询问态保持最后烹饪信息 */
        lv_label_set_text(sc->cookstatus, tr("已完成"));
        if (six_bread_has_color()) {
            /* 有烤色:显示烤色询问 */
            if (g_six_phase == SIX_PHASE_ASK) {
                lv_label_set_text(sc->text1, tr("请问需要增加"));
                lv_label_set_text(sc->text2, tr("烤色吗!"));
            } else {
                lv_label_set_text(sc->text1, tr("请问还需要增加"));
                lv_label_set_text(sc->text2, tr("烤色吗!"));
            }
            lv_obj_clear_flag(sc->text1, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(sc->text2, LV_OBJ_FLAG_HIDDEN);
            if (bl) lv_label_set_text(bl, tr("需 要"));
        } else {
            /* 无烤色:完成提示,彻底隐藏按钮(移出焦点组,按确定不再触发) */
            lv_obj_add_flag(sc->stop, LV_OBJ_FLAG_HIDDEN);
            if (g_six_cooking) lv_group_remove_obj(sc->stop);
            lv_obj_clear_flag(sc->text1, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(sc->text2, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(sc->text1, tr("高温防烫"));
            lv_label_set_text(sc->text2, tr("请缓慢打开门体！"));
        }
        lv_bar_set_range(sc->bar_1, 0, 100);
        lv_bar_set_value(sc->bar_1, 100, LV_ANIM_OFF);
        break;
    case SIX_PHASE_COLOR_SETUP:
        lv_obj_clear_flag(sc->timelabel, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text_fmt(sc->timelabel, "00:%02d:00", g_six_color_min);   /* 所选程度时长 */
        lv_label_set_text(sc->cookstatus, tr("额外上色"));
        lv_label_set_text_fmt(sc->label_12, tr("| %s | %d分钟"), six_bread_name(), g_six_color_min);
        if (bl) lv_label_set_text(bl, tr("开 始"));
        lv_bar_set_range(sc->bar_1, 0, 100);
        lv_bar_set_value(sc->bar_1, 3, LV_ANIM_OFF);
        break;
    case SIX_PHASE_COLOR_COOKING:
        lv_label_set_text(sc->cookstatus, g_six_paused ? tr("暂停中...") : tr("烹饪中..."));
        lv_label_set_text_fmt(sc->label_12, tr("| %s | 额外上色 | %d分钟"), six_bread_name(), g_six_color_min);
        if (bl) lv_label_set_text(bl, g_six_paused ? tr("开 始") : tr("暂 停"));
        break;
    default:
        break;
    }

    /* 倒计时与进度条 */
    if (g_six_phase == SIX_PHASE_RISING || g_six_phase == SIX_PHASE_COOKING ||
        g_six_phase == SIX_PHASE_COLOR_COOKING) {
        int32_t rem = six_remaining_sec();
        lv_label_set_text_fmt(sc->timelabel, "%02d:%02d:%02d",
                              (int)(rem / 3600), (int)((rem % 3600) / 60), (int)(rem % 60));
        /* 进度:发酵+烹饪按总时长,上色按阶段时长 */
        int32_t done, total;
        if (g_six_phase == SIX_PHASE_COLOR_COOKING) {
            done = six_elapsed_sec(); total = six_phase_sec(SIX_PHASE_COLOR_COOKING);
        } else if (g_six_has_rising) {
            done = six_elapsed_sec() + (g_six_phase == SIX_PHASE_COOKING ? SIX_RISING_SEC : 0);
            total = SIX_RISING_SEC + six_cook_sec();
        } else {
            done = six_elapsed_sec(); total = six_phase_sec(SIX_PHASE_COOKING);
        }
        if (done > total) done = total;
        int p = 3 + (int)((int64_t)done * 97 / (total ? total : 1));
        if (p > 100) p = 100;
        lv_bar_set_range(sc->bar_1, 0, 100);
        lv_bar_set_value(sc->bar_1, p, LV_ANIM_OFF);
        g_send.remaining_ms = (int32_t)(rem * 1000);
    } else {
        g_send.remaining_ms = -1;
    }

    /* 遮罩态:最后覆盖(进度条/状态在遮罩期间照常刷新) */
    if (g_six_overlay) {
        lv_obj_add_flag(sc->timelabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->container_1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->text1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->text2, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(sc->text1, tr("是否结束当前任务"));
        lv_label_set_text(sc->text2, tr("回到主页"));
        /* 确认层:按钮恢复显示并加回焦点组(可操作) */
        lv_obj_clear_flag(sc->stop, LV_OBJ_FLAG_HIDDEN);
        if (g_six_cooking) {
            lv_group_remove_obj(sc->stop);   /* 防重复添加 */
            lv_group_add_obj(g_six_cooking, sc->stop);
        }
        if (bl) lv_label_set_text(bl, tr("确 定"));
    }
}

// 阶段切换
static void six_cook_set_phase(int phase)
{
    g_six_phase = phase;
    g_six_paused = 0;

    switch (phase) {
    case SIX_PHASE_RISING:
        g_send.cook_mode = MODE_RISING;
        g_send.set_temp = 45;
        g_send.set_temp_lower = 0;
        g_send.iface_status = IFACE_COOKING;
        break;
    case SIX_PHASE_COOKING:
        g_send.cook_mode = six_cook_mode();   /* 烤鸡翅=热风对流25(250℃)，面包/蛋糕按配置 */
        g_send.set_temp = six_cook_temp();
        g_send.set_temp_lower = (g_send.cook_mode == MODE_UPDOWN_BBQ) ? six_cook_temp() : 0;   /* 仅上下烧烤发下温 */
        g_send.iface_status = IFACE_COOKING;
        break;
    case SIX_PHASE_COLOR_SETUP:
        g_send.cook_mode = six_cook_mode();   /* 烤色阶段保持前面模式,不发 color */
        g_send.set_temp = six_cook_temp();
        g_send.set_temp_lower = (g_send.cook_mode == MODE_UPDOWN_BBQ) ? six_cook_temp() : 0;
        /* iface_status 不设:保持完成状态(尚未开始烹饪) */
        break;
    case SIX_PHASE_COLOR_COOKING:
        g_send.cook_mode = six_cook_mode();
        g_send.set_temp = six_cook_temp();
        g_send.set_temp_lower = (g_send.cook_mode == MODE_UPDOWN_BBQ) ? six_cook_temp() : 0;
        g_send.iface_status = IFACE_COOKING;   /* 点"开 始"后才发烹饪状态 */
        break;
    case SIX_PHASE_ASK:
    case SIX_PHASE_ASK_COLOR:
        g_send.iface_status = IFACE_COMPLETE;   /* 完成状态 */
        g_send.remaining_ms = 0;
        g_send.buzzer_req = BUZZER_COOK_DONE;
        break;
    default:
        g_send.cook_mode = six_cook_mode();
        if (six_chick_is_kind()) {   /* 烤鸡翅类:按菜谱温度/模式 */
            g_send.set_temp = six_cook_temp();
            g_send.set_temp_lower = 0;
        }
        break;
    }
    six_cook_apply_display();
    printf("[six_cook] phase -> %d\n", phase);
}

// 1s 定时器:计时 + 阶段切换
static void six_cook_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if (!g_six_running) return;
    if (g_six_paused) return;

    /* 门开自动暂停(运行阶段) */
    if (is_door_open()) {
        cook_elapsed_saved = lv_tick_get() - cook_start_time;
        if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
        g_six_paused = 1;
        g_send.iface_status = IFACE_PAUSE;   /* 门开自动暂停发暂停状态 */
        six_cook_apply_display();
        printf("[six_cook] door open -> auto pause\n");
        return;
    }

    if (g_six_phase == SIX_PHASE_RISING || g_six_phase == SIX_PHASE_COOKING ||
        g_six_phase == SIX_PHASE_COLOR_COOKING) {
        int32_t ph = six_phase_sec(g_six_phase);
        int32_t e = six_elapsed_sec();
        six_cook_apply_display();
        if (e >= ph) {
            /* 阶段到点 */
            if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
            g_six_overlay = 0;   /* 遮罩中到点:退出遮罩,显示完成询问态 */
            if (g_six_phase == SIX_PHASE_RISING) {
                cook_start_time = lv_tick_get();
                cook_elapsed_saved = 0;
                six_cook_set_phase(SIX_PHASE_COOKING);
                cook_timer = lv_timer_create(six_cook_timer_cb, 1000, NULL);
            } else if (g_six_phase == SIX_PHASE_COOKING) {
                six_cook_set_phase(SIX_PHASE_ASK);
            } else {
                six_cook_set_phase(SIX_PHASE_ASK_COLOR);
            }
        }
    }
}

// 运行页 stop 按钮:暂停/开始/需要/确定
static void on_six_stop_click(lv_event_t *e)
{
    if (screen_is_loading(lv_scr_act())) return;
    if (g_six_overlay) {
        six_cook_exit();
        return;
    }
    switch (g_six_phase) {
    case SIX_PHASE_RISING:
    case SIX_PHASE_COOKING:
    case SIX_PHASE_COLOR_COOKING:
        if (!g_six_paused) {
            cook_elapsed_saved = lv_tick_get() - cook_start_time;
            if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
            g_six_paused = 1;
            g_send.iface_status = IFACE_PAUSE;   /* 暂停发暂停状态(与其他模式一致) */
            six_cook_apply_display();
        } else {
            /* 门开:无效音,不恢复(与其他模式一致) */
            if (is_door_open()) {
                g_send.buzzer_req = BUZZER_KEY_INVALID;
                return;
            }
            cook_start_time = lv_tick_get() - cook_elapsed_saved;
            cook_elapsed_saved = 0;
            g_six_paused = 0;
            g_send.iface_status = IFACE_COOKING;   /* 恢复发烹饪状态 */
            if (cook_timer) lv_timer_del(cook_timer);
            cook_timer = lv_timer_create(six_cook_timer_cb, 1000, NULL);
            six_cook_apply_display();
        }
        break;
    case SIX_PHASE_ASK:
    case SIX_PHASE_ASK_COLOR:
        if (six_bread_has_color()) {
            jump_to_toastcolor();
        } else {
            /* 无烤色:进入确认(遮罩),确认后再退出,避免直接返回 */
            six_cook_handle_back();
        }
        break;
    case SIX_PHASE_COLOR_SETUP:
        /* 门开:无效音,不开始上色(与其他模式一致) */
        if (is_door_open()) {
            g_send.buzzer_req = BUZZER_KEY_INVALID;
            return;
        }
        g_six_paused = 0;
        cook_elapsed_saved = 0;
        cook_start_time = lv_tick_get();
        six_cook_set_phase(SIX_PHASE_COLOR_COOKING);
        if (cook_timer) lv_timer_del(cook_timer);
        cook_timer = lv_timer_create(six_cook_timer_cb, 1000, NULL);
        break;
    default:
        break;
    }
}

// 返回:运行/完成态 → 遮罩;遮罩 → 回源
void six_cook_handle_back(void)
{
    if (g_six_overlay) {
        g_six_overlay = 0;
        six_cook_apply_display();
        return;
    }
    g_six_overlay = 1;
    g_six_from = g_six_phase;
    six_cook_apply_display();
    printf("[six_cook] overlay\n");
}

// 退出:回主菜单
static void six_cook_exit(void)
{
    g_six_running = 0;
    g_six_phase = SIX_PHASE_COOKING;
    g_six_paused = 0;
    g_six_overlay = 0;
    g_six_has_rising = 0;
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    g_on_stop_back = 0;
    g_stop_back_complete = NULL;
    cook_elapsed_saved = 0; cook_bar_saved = 0; cook_total_ms = 0;
    set_temp = 180; set_temp_up = 180; set_temp_down = 180; set_hour = 0; set_min = 30;
    g_send.iface_status = IFACE_SETTING;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0; g_send.set_temp_lower = 0; g_send.remaining_ms = -1;
    g_delay_source_page = PAGE_WAITMENU_24;   /* 防残留误走六感分支 */
    /* 退出 → 回第六感主菜单(与 KEY_SIXMENU 入口栈形态一致) */
    depth = 0;
    page_push(PAGE_WAITMENU_24);
    jump_to_sixmenu();
    printf("[six_cook] exit -> sixmenu\n");
}

// 进入运行页(choice=1 有发酵)
void jump_to_six_cooking(void)
{
    if (is_door_open()) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    page_push(PAGE_SIX_COOKING);
    lv_obj_clean(lv_scr_act());
    somecook_cooking_create(&ui_manager);

    somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
    if (sc) {
        lv_obj_t *btns[] = { sc->stop };
        for (int k = 0; k < 1; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        if (g_six_cooking) lv_group_del(g_six_cooking);
        g_six_cooking = group_create_for_page(btns, 1);
        clear_focus_states(btns, 1);
        if (sc->stop) {
            lv_group_focus_obj(sc->stop);
            lv_obj_add_event_cb(sc->stop, on_six_stop_click, LV_EVENT_CLICKED, NULL);
        }

        /* icon:六感图标,右移 23(115 -> 138) */
        if (sc->icon) {
            lv_img_set_src(sc->icon, LVGL_IMAGE_PATH(sixicon.png));
            lv_obj_set_pos(sc->icon, 163, 161);
        }
        if (sc->activestatus)
            lv_obj_add_flag(sc->activestatus, LV_OBJ_FLAG_HIDDEN);
    }
    current_group = g_six_cooking;

    g_six_running = 1;
    g_six_overlay = 0;
    g_six_paused = 0;
    g_six_has_rising = six_bread_has_rising() && (g_rising_choice == 1);
    g_six_color_min = six_bread_color_min(2);   /* 默认"中"档 */
    cook_elapsed_saved = 0;
    cook_start_time = lv_tick_get();
    six_cook_set_phase((six_bread_has_rising() && g_rising_choice == 1) ? SIX_PHASE_RISING : SIX_PHASE_COOKING);
    if (cook_timer) lv_timer_del(cook_timer);
    cook_timer = lv_timer_create(six_cook_timer_cb, 1000, NULL);
    g_send.iface_status = IFACE_COOKING;
    six_cook_apply_display();

    lang_scr_load_anim(somecook_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_cook] jump: cooking (choice=%d)\n", g_rising_choice);
}

// 提示恢复后重放当前显示(供 nav_hint.c 调用)
void six_cook_refresh_display(void)
{
    /* 地址复用场景下 current_group 可能误判,先校验页面对象有效再刷新 */
    somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
    if (sc && sc->obj && lv_obj_is_valid(sc->obj))
        six_cook_apply_display();
}

// 六感是否处于遮罩确认态(供 nav_hint.c 判断不弹提示)
int six_cook_is_overlay(void)
{
    return g_six_overlay;
}

// toastcolor next 确认:切到上色准备态(重建后显示)
void six_cook_goto_setup(void)
{
    /* 仅切状态与通讯;烤色页在栈顶时运行页对象已销毁(指针悬空),
       显示刷新由 next 的 page_pop → six_cooking_rebuild → apply_display 完成 */
    g_six_overlay = 0;
    g_six_paused = 0;
    g_six_phase = SIX_PHASE_COLOR_SETUP;
    g_send.cook_mode = six_bread_cfg()->mode;   /* 保持前面模式,不发 color */
    g_send.set_temp = six_bread_cfg()->cook_temp;
    g_send.set_temp_lower = (g_send.cook_mode == MODE_UPDOWN_BBQ) ? six_bread_cfg()->cook_temp : 0;   /* 仅上下烧烤发下温 */
    /* iface_status 不设:保持完成状态,点"开 始"后才发烹饪状态 */
}

// 外部路径(长按关机/探针)清理运行状态
void six_cook_reset(void)
{
    g_six_running = 0;
    g_six_phase = SIX_PHASE_COOKING;
    g_six_paused = 0;
    g_six_overlay = 0;
    g_six_has_rising = 0;
}

// 页面重建:完成询问态(从 toastcolor 返回)或上色准备态
void six_cooking_rebuild(page_id_t child)
{
    if (g_six_cooking) { lv_group_del(g_six_cooking); g_six_cooking = NULL; }
    lv_obj_clean(lv_scr_act());
    somecook_cooking_create(&ui_manager);

    somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
    if (sc) {
        lv_obj_t *btns[] = { sc->stop };
        for (int k = 0; k < 1; k++) {
            if (btns[k]) lv_group_remove_obj(btns[k]);
        }
        g_six_cooking = group_create_for_page(btns, 1);
        clear_focus_states(btns, 1);
        if (sc->stop) {
            lv_group_focus_obj(sc->stop);
            lv_obj_add_event_cb(sc->stop, on_six_stop_click, LV_EVENT_CLICKED, NULL);
        }

        if (sc->icon) {
            lv_img_set_src(sc->icon, LVGL_IMAGE_PATH(sixicon.png));
            lv_obj_set_pos(sc->icon, 163, 161);
        }
        if (sc->activestatus)
            lv_obj_add_flag(sc->activestatus, LV_OBJ_FLAG_HIDDEN);
    }
    current_group = g_six_cooking;

    g_six_overlay = 0;
    g_six_paused = 0;
    six_cook_apply_display();

    lang_scr_load_anim(somecook_cooking_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[six_cook] rebuild: child=%d\n", (int)child);
}
