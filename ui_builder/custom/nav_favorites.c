/*
 * 收藏功能导航壳（nav_favorites.c）
 * 职责：
 *   1. 进入 cooking 时的参数快照（fav_snapshot_save，收藏用初始值）
 *   2. 收藏保存（favorites_save_current：普通/探针/六感/多段）
 *   3. 收藏启动（favorites_start_selected：直接进入运行）
 *   4. 收藏页导航（jump_to_favorites / favorites_rebuild）
 * 页面 UI 与业务逻辑见 screen_favorites.c（独立页面，不依赖 ui_manager）
 */
#include "nav_favorites.h"
#include "nav_internal.h"
#include <string.h>

/* ===================== 进入 cooking 时参数快照 ===================== */
int fav_init_temp = 0;
int fav_init_hour = 0;
int fav_init_min = 0;
int fav_init_probe_temp = 0;
int fav_init_temp_up = 0;
int fav_init_temp_down = 0;

/* 收藏页实例（独立页面，控件全部由 screen_favorites_create 手动创建） */
screen_favorites_t g_fav_screen;

/* 多段收藏时 g_steps 的转换载体（Add_favorites_of_Multi / Favorites_Cover_Func 使用） */
Fun_Multi_SUM_Value Func_SUM_Value_step;

/* Fun_Multi_SUM_Value 的步骤字段非数组，按序号取指针 */
static Fun_Multi_step *fav_step_ptr(Fun_Multi_SUM_Value *v, int i)
{
    if (i == 0) return &v->Func_Value_step_1;
    if (i == 1) return &v->Func_Value_step_2;
    return &v->Func_Value_step_3;
}

static int8_t *fav_num_ptr(Fun_Multi_SUM_Value *v, int i)
{
    if (i == 0) return &v->Func_num_1;
    if (i == 1) return &v->Func_num_2;
    return &v->Func_num_3;
}

/* 进入 cooking 时保存初始参数（运行中 setting 页改动不影响收藏） */
void fav_snapshot_save(void)
{
    fav_init_temp       = set_temp;
    fav_init_hour       = set_hour;
    fav_init_min        = set_min;
    fav_init_probe_temp = probe_target_temp;
    fav_init_temp_up    = set_temp_up;
    fav_init_temp_down  = set_temp_down;
}

/* 按收藏值返回模式名（含 FROZEN_BAKE/COOK4 子类，Six/多段） */
const char *fav_mode_name(const Fun_favorites_Value *fav)
{
    int m = fav->PengTiaoMode_name;
    if (m == FAV_MODE_SIX) {
        uint8_t old = g_six_bread_type;
        g_six_bread_type = (uint8_t)fav->Six_Cook_Fun;
        const char *name = six_bread_name();
        g_six_bread_type = old;
        return name;
    }
    if (m == FAV_MODE_MULTI)
        return tr("多段烹饪");
    if (m == MODE_FROZEN_BAKE) {
        switch ((page_id_t)fav->source_page) {
        case PAGE_LASAGNA_SET: return tr("千层面");
        case PAGE_STRUDEL_SET: return tr("果馅卷");
        case PAGE_BREAD_SET:   return tr("面包");
        case PAGE_PIZZA3_SET:  return tr("披萨");
        case PAGE_CHIP_SET:    return tr("薯条");
        case PAGE_CUSTOM_SET:  return tr("自定义");
        default:               return tr("冷食速烹");
        }
    }
    if (m == MODE_COOK4) {
        switch ((page_id_t)fav->source_page) {
        case PAGE_COOKIE_SET:    return tr("曲奇");
        case PAGE_WEST_SET:      return tr("西式");
        case PAGE_PIZZA_SET:     return tr("披萨");
        case PAGE_MENU_COOK_SET: return tr("菜单");
        default:                 return tr("多层烹调");
        }
    }
    switch (m) {
    case MODE_UPDOWN_BBQ:     return tr("上下烧烤");
    case MODE_TOP_BBQ:        return tr("顶部烧烤");
    case MODE_BOTTOM_BBQ:     return tr("底部烧烤");
    case MODE_HOT_BBQ:        return tr("热风烧烤");
    case MODE_HOTWIND_BBQ:    return tr("热风");
    case MODE_SAVE_BBQ:       return tr("节能热风");
    case MODE_CENTRAL_BBQ:    return tr("集中烧烤");
    case MODE_WINDCHANGE_BBQ: return tr("热风对流");
    case MODE_AIR:            return tr("空气炸");
    case MODE_PIZZA_2:        return tr("披萨");
    case MODE_SLOWCOOK:       return tr("慢煮");
    case MODE_UNFROZEN:       return tr("解冻");
    case MODE_RISING:         return tr("发酵");
    case MODE_CORN:           return tr("干果");
    case MODE_HEATCONTAIN:    return tr("保温");
    default:                  return tr("上下烧烤");
    }
}

/* ===================== 收藏保存（KEY_FAV complete 态调用） ===================== */
void favorites_save_current(void)
{
    Fav_Select_By_Probe();
    if (Fav_Cur->has_favorites_byte == 0xFF) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;   /* 收藏已满 */
        return;
    }

    /* 收集当前参数（读进入 cooking 时快照，运行中 setting 改动不参与收藏） */
    input_Temp                  = (uint16_t)fav_init_temp;
    input_Hour                  = (int8_t)fav_init_hour;
    input_Minute                = (int8_t)fav_init_min;
    input_Is_Steam              = g_send.steam_level;
    input_Temp_Conventional_Dowm = (uint16_t)fav_init_temp_down;
    input_Temp_probe[0]         = (uint8_t)fav_init_probe_temp;
    input_Six_num               = (int8_t)g_six_bread_type;

    if (g_somecook_running) {
        /* 多段：g_steps → Fun_Multi_SUM_Value */
        memset(&Func_SUM_Value_step, 0, sizeof(Func_SUM_Value_step));
        for (int i = 0; i < 3; i++) {
            Fun_Multi_step *m = fav_step_ptr(&Func_SUM_Value_step, i);
            m->PengTiaoMode_num = g_steps[i].mode;
            m->Func_Temp        = (int16_t)g_steps[i].temp;
            m->Func_Hour        = (int8_t)g_steps[i].hour;
            m->Func_Minute      = (int8_t)g_steps[i].min;
            *fav_num_ptr(&Func_SUM_Value_step, i) = g_steps[i].set ? (int8_t)g_steps[i].mode : -1;
        }
        input_Mode_name   = FAV_MODE_MULTI;
        input_Cooking_Mode = (uint8_t)g_steps[0].mode;
    } else if (g_six_running) {
        /* 六感：菜谱编号 + 档位（时间由菜谱数据决定，不收藏时间） */
        input_Mode_name    = FAV_MODE_SIX;
        input_Cooking_Mode = g_send.cook_mode;
    } else {
        /* 普通/探针模式 */
        input_Mode_name    = (int8_t)g_send.cook_mode;
        input_Cooking_Mode = g_send.cook_mode;
    }

    if (Favorites_Check_Exists())
        Favorites_Cover_Func();
    else if (input_Mode_name == FAV_MODE_MULTI)
        Add_favorites_of_Multi(Func_SUM_Value_step);
    else
        Add_favorites();

    fav_succeed_no_repetitive = 1;
    g_send.buzzer_req = BUZZER_KEY_VALID;
    printf("[fav] save: mode=%d temp=%d time=%dh%dm count=%d\n",
           input_Mode_name, input_Temp, input_Hour, input_Minute, favorites_how_many);
}

/* ===================== 收藏启动（直接进入运行） ===================== */
void favorites_start_selected(void)
{
    Fav_Select_By_Probe();
    int idx = favorites_choose_which - 1;
    if (idx < 0 || idx >= 8) {
        g_send.buzzer_req = BUZZER_KEY_INVALID;
        return;
    }
    Fun_favorites_Value *fav = &Fav_Cur->favorites_val[idx];

    favo_safety_group_delete();
    edit_clear();

    if (fav->PengTiaoMode_name == FAV_MODE_MULTI) {
        /* 多段：恢复 g_steps 后直接启动运行 */
        for (int i = 0; i < 3; i++) {
            Fun_Multi_step *m = fav_step_ptr(&Favorites_Value, i);
            g_steps[i].mode = m->PengTiaoMode_num;
            g_steps[i].temp = m->Func_Temp;
            g_steps[i].hour = m->Func_Hour;
            g_steps[i].min  = m->Func_Minute;
            g_steps[i].set  = (*fav_num_ptr(&Favorites_Value, i) >= 0);
        }
        g_cur_step = 0;
        g_somecook_run_idx = 0;
        g_send.cook_mode = g_steps[0].mode;
        somecook_cooking_start();
        return;
    }

    if (fav->PengTiaoMode_name == FAV_MODE_SIX) {
        /* 六感：恢复菜谱编号 + 发酵选择后进入六感烹饪 */
        g_six_bread_type = (uint8_t)fav->Six_Cook_Fun;
        g_rising_choice  = fav->Six_FaJiao ? 1 : -1;
        g_send.cook_mode = six_cook_mode();
        jump_to_six_cooking();
        return;
    }

    /* 普通/探针模式：恢复参数后按 cook_mode 分发 */
    set_temp       = fav->temperature;
    set_hour       = fav->Func_Hour;
    set_min        = fav->Func_Minute;
    set_temp_up    = fav->temperature;
    set_temp_down  = (fav->temp_down > 0) ? fav->temp_down : fav->temperature;
    probe_target_temp = fav->Probe_temp;
    g_send.cook_mode  = fav->PengTiao_Mode;
    g_delay_source_page = (page_id_t)fav->source_page;
    delay_start_cook();
}

/* ===================== 收藏页导航 ===================== */
/* 销毁旧页面实例（收藏页 obj 是独立屏幕对象，不会随 lv_obj_clean 自动删除） */
static void fav_screen_reset(void)
{
    favo_safety_group_delete();
    if (g_fav_screen.obj) {
        lv_obj_del(g_fav_screen.obj);
        g_fav_screen.obj = NULL;
    }
}

void jump_to_favorites(void)
{
    edit_clear();
    fav_screen_reset();
    page_push(PAGE_FAVORITES);
    lv_obj_clean(lv_scr_act());
    screen_favorites_create();
    g_favorites = g_fav_screen.group;
    current_group = g_favorites;
    lang_scr_load_anim(g_fav_screen.obj, LV_SCR_LOAD_ANIM_NONE, 0, 0, 0);
    printf("[fav] jump: -> favorites\n");
}

void favorites_rebuild(page_id_t child)
{
    (void)child;
    fav_screen_reset();
    lv_obj_clean(lv_scr_act());
    screen_favorites_create();
    g_favorites = g_fav_screen.group;
    current_group = g_favorites;
    lang_scr_load_anim(g_fav_screen.obj, LV_SCR_LOAD_ANIM_NONE, 0, 0, 0);
    printf("[fav] back to favorites\n");
}