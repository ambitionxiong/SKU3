#include "nav.h"
#include "protocol.h"

lv_group_t *g_somecook_btns;   /* somecook 主页面 3 大按钮焦点组 */
lv_group_t *g_somecook_edit;   /* somecook 步骤容器 编辑/删除 焦点组 */
lv_group_t *g_stepset;         /* stepset 步骤设置页焦点组 */

static void on_somecook_edit_click(lv_event_t *e);
static void on_somecook_delete_click(lv_event_t *e);
static void on_stepset_next_click(lv_event_t *e);

/* 多段烹饪步骤数据 */
somecook_step_t g_steps[3];
int g_cur_step;             /* 当前编辑步骤 0-2 */

// 按步骤状态刷新 somecook 显示（容器/plus/sure 显隐 + 容器内容文本）
static void somecook_refresh(void)
{
    somecook_t *sc = somecook_get(&ui_manager);
    if (!sc) return;
    lv_obj_t *steps[3] = { sc->step1, sc->step2, sc->step3 };
    lv_obj_t *pluses[3] = { sc->plus1, sc->plus2, sc->plus3 };
    lv_obj_t *mode_lb[3] = { sc->modestatus, sc->modestatus2, sc->modestatus3 };
    lv_obj_t *temp_lb[3] = { sc->tempstatus, sc->tempstatus2, sc->tempstatus3 };
    lv_obj_t *time_lb[3] = { sc->timestatus, sc->timestatus2, sc->timestatus3 };
    int nset = 0;
    for (int i = 0; i < 3; i++) {
        if (g_steps[i].set) {
            lv_obj_clear_flag(steps[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(pluses[i], LV_OBJ_FLAG_HIDDEN);
            /* 容器显示时清除编辑/删除按钮聚焦状态 */
            lv_obj_t *e_btns[2];
            if (i == 0) { e_btns[0] = sc->edit;   e_btns[1] = sc->delete; }
            else if (i == 1) { e_btns[0] = sc->edit2;  e_btns[1] = sc->delete2; }
            else { e_btns[0] = sc->edit3; e_btns[1] = sc->delete3; }
            clear_focus_states(e_btns, 2);
            nset++;
            /* 容器内容文本 */
            const char *name = "未知";
            switch (g_steps[i].mode) {
            case MODE_HOTWIND_BBQ:    name = "热风"; break;
            case MODE_SAVE_BBQ:       name = "节能热风"; break;
            case MODE_UPDOWN_BBQ:     name = "上下烧烤"; break;
            case MODE_BOTTOM_BBQ:     name = "底部烧烤"; break;
            case MODE_HOT_BBQ:        name = "热风烧烤"; break;
            case MODE_CENTRAL_BBQ:    name = "集中烧烤"; break;
            case MODE_WINDCHANGE_BBQ: name = "热风对流"; break;
            case MODE_TOP_BBQ:        name = "顶部烧烤"; break;
            case MODE_AIR:            name = "空气炸"; break;
            case MODE_RISING:         name = "发酵"; break;
            case MODE_SLOWCOOK:       name = "慢煮"; break;
            case MODE_UNFROZEN:       name = "解冻"; break;
            default: break;
            }
            if (mode_lb[i]) lv_label_set_text_fmt(mode_lb[i], "模式: %s", name);
            if (temp_lb[i]) {
                if (g_steps[i].mode == MODE_UPDOWN_BBQ)
                    lv_label_set_text_fmt(temp_lb[i], "温度: ↑%d℃/↓%d℃", g_steps[i].temp, g_steps[i].temp);
                else
                    lv_label_set_text_fmt(temp_lb[i], "温度: %d℃", g_steps[i].temp);
            }
            if (time_lb[i]) {
                if (g_steps[i].hour == 0)
                    lv_label_set_text_fmt(time_lb[i], "时间: %d分钟", g_steps[i].min);
                else
                    lv_label_set_text_fmt(time_lb[i], "时间: %d小时%d分钟", g_steps[i].hour, g_steps[i].min);
            }
        } else {
            lv_obj_add_flag(steps[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(pluses[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    /* sure/surelabel：至少设置 2 个步骤才显示 */
    if (nset >= 2) {
        lv_obj_clear_flag(sc->sure, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(sc->surelabel, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(sc->sure, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(sc->surelabel, LV_OBJ_FLAG_HIDDEN);
    }
}

// 重建按钮组（3 按钮 + sure 可见时加入；focus_btn=-1 → step1）
void somecook_back_to_btns(int focus_btn)
{
    somecook_t *sc = somecook_get(&ui_manager);
    if (!sc) return;
    /* 删除编辑组并清除按钮聚焦 */
    if (g_somecook_edit) { lv_group_del(g_somecook_edit); g_somecook_edit = NULL; }
    lv_obj_clear_state(sc->edit, LV_STATE_FOCUSED);
    lv_obj_clear_state(sc->delete, LV_STATE_FOCUSED);
    lv_obj_clear_state(sc->edit2, LV_STATE_FOCUSED);
    lv_obj_clear_state(sc->delete2, LV_STATE_FOCUSED);
    lv_obj_clear_state(sc->edit3, LV_STATE_FOCUSED);
    lv_obj_clear_state(sc->delete3, LV_STATE_FOCUSED);
    lv_obj_t *btns[4];
    int n = 0;
    btns[n++] = sc->step1button;
    btns[n++] = sc->step2button;
    btns[n++] = sc->step3button;
    if (!lv_obj_has_flag(sc->sure, LV_OBJ_FLAG_HIDDEN))
        btns[n++] = sc->sure;
    if (g_somecook_btns) lv_group_del(g_somecook_btns);
    g_somecook_btns = group_create_for_page(btns, n);
    clear_focus_states(btns, n);
    current_group = g_somecook_btns;
    lv_group_focus_obj(focus_btn == 1 ? sc->step2button :
                       focus_btn == 2 ? sc->step3button : sc->step1button);
}

// 进入步骤容器编辑组（edit/delete，默认焦点编辑）
static void somecook_enter_edit_group(int i)
{
    somecook_t *sc = somecook_get(&ui_manager);
    if (!sc) return;
    /* 大按钮退出聚焦（编辑组接管焦点） */
    lv_obj_clear_state(sc->step1button, LV_STATE_FOCUSED);
    lv_obj_clear_state(sc->step2button, LV_STATE_FOCUSED);
    lv_obj_clear_state(sc->step3button, LV_STATE_FOCUSED);
    lv_obj_t *btns[2];
    if (i == 0) { btns[0] = sc->edit;   btns[1] = sc->delete; }
    else if (i == 1) { btns[0] = sc->edit2;  btns[1] = sc->delete2; }
    else { btns[0] = sc->edit3; btns[1] = sc->delete3; }
    if (g_somecook_edit) { lv_group_del(g_somecook_edit); g_somecook_edit = NULL; }
    g_somecook_edit = group_create_for_page(btns, 2);
    clear_focus_states(btns, 2);
    lv_group_focus_obj(btns[0]);
    current_group = g_somecook_edit;
    /* 去重:编辑/删除按钮跨"进编辑组→BACK"复用对象,避免回调累积导致多重跳转 */
    lv_obj_remove_event_cb(btns[0], on_somecook_edit_click);
    lv_obj_remove_event_cb(btns[1], on_somecook_delete_click);
    lv_obj_add_event_cb(btns[0], on_somecook_edit_click, LV_EVENT_CLICKED, (void *)(intptr_t)i);
    lv_obj_add_event_cb(btns[1], on_somecook_delete_click, LV_EVENT_CLICKED, (void *)(intptr_t)i);
}

// 大按钮确认：未设置 → 进 stepset；已设置 → 进容器编辑组
static void on_somecook_btn_click(lv_event_t *e)
{
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    if (i < 0 || i > 2) return;
    if (!g_steps[i].set)
        jump_to_stepset(i);
    else
        somecook_enter_edit_group(i);
}

// 编辑 → 重新进 stepset 设置此步骤
static void on_somecook_edit_click(lv_event_t *e)
{
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    if (i < 0 || i > 2) return;
    jump_to_stepset(i);
}

// 删除 → 清除步骤，恢复 plus，焦点回该按钮
static void on_somecook_delete_click(lv_event_t *e)
{
    int i = (int)(intptr_t)lv_event_get_user_data(e);
    if (i < 0 || i > 2) return;
    g_steps[i].set = 0;
    somecook_refresh();
    somecook_back_to_btns(i);
}

// somecook 主页面重建（不压栈；child=PAGE_STEPSET 时焦点回发起步骤的按钮）
void somecook_rebuild(page_id_t child)
{
    /* 页面重建:清理旧编辑组（引用已销毁对象/焦点状态残留的防御） */
    if (g_somecook_edit) { lv_group_del(g_somecook_edit); g_somecook_edit = NULL; }
    lv_obj_clean(lv_scr_act());
    somecook_create(&ui_manager);

    somecook_t *sc = somecook_get(&ui_manager);
    if (sc) {
        somecook_refresh();

        /* 防御：先移出默认组（lv_btn 创建时自动加入默认组，
           未进编辑组的编辑/删除/sure 可见时会遭默认组 refocus 聚焦） */
        {
            lv_obj_t *e_all[7] = { sc->edit, sc->delete, sc->edit2, sc->delete2,
                                   sc->edit3, sc->delete3, sc->sure };
            clear_focus_states(e_all, 7);
            for (int k = 0; k < 7; k++)
                lv_group_remove_obj(e_all[k]);
        }

        /* 焦点组：step1/2/3 按钮 + sure（可见时加入） */
        lv_obj_t *btns[4];
        int n_btns = 0;
        btns[n_btns++] = sc->step1button;
        btns[n_btns++] = sc->step2button;
        btns[n_btns++] = sc->step3button;
        if (!lv_obj_has_flag(sc->sure, LV_OBJ_FLAG_HIDDEN))
            btns[n_btns++] = sc->sure;
        if (g_somecook_btns) lv_group_del(g_somecook_btns);
        g_somecook_btns = group_create_for_page(btns, n_btns);
        clear_focus_states(btns, n_btns);

        if (child == PAGE_STEPSET) {
            /* 设置完成返回:聚焦从步骤一开始最前面的未设置步骤;全部设置完 → "确 定" */
            int f = -1;
            for (int i = 0; i < 3; i++)
                if (!g_steps[i].set) { f = i; break; }
            if (f < 0) {
                if (!lv_obj_has_flag(sc->sure, LV_OBJ_FLAG_HIDDEN))
                    lv_group_focus_obj(sc->sure);
                else
                    lv_group_focus_obj(sc->step1button);   /* 防御兜底 */
            } else {
                lv_group_focus_obj(f == 1 ? sc->step2button :
                                   f == 2 ? sc->step3button : sc->step1button);
            }
        } else {
            /* 首次进入:聚焦第一个未设置的步骤按钮 */
            int f = 0;
            for (int i = 0; i < 3; i++)
                if (!g_steps[i].set) { f = i; break; }
            lv_group_focus_obj(f == 1 ? sc->step2button :
                               f == 2 ? sc->step3button : sc->step1button);
        }
        lv_obj_add_event_cb(sc->step1button, on_somecook_btn_click,
                            LV_EVENT_CLICKED, (void *)(intptr_t)0);
        lv_obj_add_event_cb(sc->step2button, on_somecook_btn_click,
                            LV_EVENT_CLICKED, (void *)(intptr_t)1);
        lv_obj_add_event_cb(sc->step3button, on_somecook_btn_click,
                            LV_EVENT_CLICKED, (void *)(intptr_t)2);
        lv_obj_add_event_cb(sc->sure, on_somecook_sure_click,
                            LV_EVENT_CLICKED, NULL);
    }
    current_group = g_somecook_btns;

    lv_scr_load_anim(somecook_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[somecook] rebuild: child=%d\n", (int)child);
}

// 入口：special_menu "多段烹饪" → somecook 主页面
void jump_to_somecook(void)
{
    edit_clear();
    page_push(PAGE_SOMECOOK);
    somecook_rebuild(PAGE_SOMECOOK);
    printf("[somecook] jump: special_menu -> somecook\n");
}

// stepset "下一步" 确认：保存当前步骤 → 回 somecook（焦点移下一按钮）
static void on_stepset_next_click(lv_event_t *e)
{
    lv_obj_t *act_scr = lv_scr_act();
    if (screen_is_loading(act_scr)) return;
    if (g_cur_step < 0 || g_cur_step > 2) return;
    g_steps[g_cur_step].mode = g_send.cook_mode;
    g_steps[g_cur_step].temp = set_temp;
    g_steps[g_cur_step].hour = set_hour;
    g_steps[g_cur_step].min = set_min;
    g_steps[g_cur_step].set = 1;
    if (g_cur_step < 2)
        g_cur_step++;   /* 返回后焦点移下一按钮 */
    page_pop();
}

// 入口：somecook 大按钮/编辑 → stepset 步骤设置页
void jump_to_stepset(int i)
{
    edit_clear();
    if (i < 0 || i > 2) i = 0;
    g_cur_step = i;
    page_push(PAGE_STEPSET);
    lv_obj_clean(lv_scr_act());
    stepset_create(&ui_manager);

    stepset_t *ss = stepset_get(&ui_manager);
    if (ss) {
        static const char *names[] = { "步骤一", "步骤二", "步骤三" };
        lv_label_set_text(ss->stepname, names[i]);

        /* 焦点组：大类 → 子类 → 温度 → 小时 → 分钟 → 下一步 */
        lv_obj_t *btns[] = { ss->roller_main, ss->roller_mode, ss->temp, ss->hour, ss->min, ss->next };
        if (g_stepset) lv_group_del(g_stepset);
        g_stepset = group_create_for_page(btns, 6);
        clear_focus_states(btns, 6);
        lv_group_focus_obj(ss->next);

        /* 编辑注册（范围/默认按当前选中模式，由 stepset_apply_sel_mode 设置） */
        edit_register(ss->temp, ss->templine2, ss->templine3,
                      &set_temp, 30, 300, 5, "%d");
        edit_register(ss->hour, ss->hourline, NULL,
                      &set_hour, 0, 4, 1, "%02d");
        edit_register(ss->min, ss->minline, NULL,
                      &set_min, 0, 59, 1, "%02d");

        /* 事件绑定 */
        lv_obj_add_event_cb(ss->roller_main, stepset_on_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(ss->roller_mode, stepset_on_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(ss->temp, stepset_on_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(ss->hour, stepset_on_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(ss->min, stepset_on_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(ss->next, stepset_on_focus, LV_EVENT_FOCUSED, NULL);
        lv_obj_add_event_cb(ss->next, on_stepset_next_click,
                            LV_EVENT_CLICKED, NULL);

        stepset_bind_events();
        /* 已设置步骤：定位 roller 并回填；新设置：按当前选中模式给默认值 */
        if (g_steps[i].set) {
            stepset_restore_mode(g_steps[i].mode);
        } else {
            /* 首进新设置: roller_main 默认"烹饪功能",先设置 roller_mode 为 cook 菜单新顺序 */
            lv_roller_set_options(ss->roller_mode,
                                  "上下烧烤\n顶部烧烤\n热风烧烤\n热风\n节能热风\n底部烧烤\n集中烧烤\n热风对流",
                                  LV_ROLLER_MODE_NORMAL);
            stepset_apply_sel_mode(true);
        }
    }
    current_group = g_stepset;

    lv_scr_load_anim(stepset_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[somecook] jump: somecook -> stepset (%d)\n", i);
}

