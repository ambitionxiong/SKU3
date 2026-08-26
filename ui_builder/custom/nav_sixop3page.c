/*
 * nav_sixop3page.c - 六选项3页（复用页面）
 *
 * 通用三栏菜单：name 标题 + 三个 tile(bt1/bt2/bt3) + 三个标签(op1/op2/op3)。
 * 通过参数设定文字，点击 bt 跳对应流程。
 * 注：probe1/2/3 是图片(lv_img)，只做显隐，不写文字（避免类型错配）。
 * 复用场景：牛肉(烤牛排/炸牛排/烤牛肉)、羊肉、猪肉等。
 */
#include "protocol.h"
#include "nav.h"

/* ---- 炸牛排（份量驱动）菜谱配置 ---- */
static const int w_fsteak_w[] = { 250, 500, 750 };

/* ---- 炸排骨（份量驱动）菜谱配置 ---- */
static const int w_frib_w[] = { 600, 800, 1000 };
static const int w_fsteak_t[] = { 18, 21, 24 };

lv_group_t *g_sixop3page = NULL;

/* 重建时需要恢复的标签 */
static const char *s_op3_name  = NULL;
static const char *s_op3_ops[3] = { NULL, NULL, NULL };
static const char *s_op3_bts[3] = { NULL, NULL, NULL };   /* 按钮内文字(探针版用;NULL=保持生成) */
static int s_op3_probe_mask = 0;   /* probe 图标显隐掩码: bit0=p1 bit1=p2 bit2=p3 */
static int s_op3_kind = SIX_OP3_KIND_BEEF;   /* 当前菜父类别（牛肉/羊肉/…） */
static int s_op3_enter_bt = 0;   /* 探针版:本次进入的按钮(1/2/3),返回焦点依据(不用全局 bread_type 防残留) */
int six_op3_get_kind(void)       { return s_op3_kind; }   /* tune 排版分支用(探针版/非探针版布局不同) */

/* bt 按钮点击（按菜类别分发） */
static void on_sixop3page_bt1_click(lv_event_t *e)
{
    (void)e;
    if (screen_is_loading(lv_scr_act())) return;
    if (s_op3_kind == SIX_OP3_KIND_MEAT_TZ) { s_op3_enter_bt = 1; jump_to_beefmenutz(); return; }   /* 探针版:牛肉→牛肉二级菜单 */
    if (s_op3_kind == SIX_OP3_KIND_MUTTON)  { g_six_bread_type = SIX_MEAT_GRILL_LEG; jump_to_probeneedtip(); return; }
    if (s_op3_kind == SIX_OP3_KIND_PORK)    { g_six_bread_type = SIX_MEAT_GRILL_TENDERLOIN; jump_to_probeneedtip(); return; }
    g_six_bread_type = SIX_MEAT_GRILL_STEAK;   /* 烤牛排 */
    jump_to_probeneedtip();
}
static void on_sixop3page_bt2_click(lv_event_t *e)
{
    (void)e;
    if (screen_is_loading(lv_scr_act())) return;
    if (s_op3_kind == SIX_OP3_KIND_MEAT_TZ) { s_op3_enter_bt = 2; jump_to_muttonmenutz(); return; }   /* 探针版:羊肉→羊肉二级菜单 */
    if (s_op3_kind == SIX_OP3_KIND_MUTTON)  { g_six_bread_type = SIX_MEAT_GRILL_LAMBS; jump_to_probeneedtip(); return; }
    if (s_op3_kind == SIX_OP3_KIND_PORK)    { g_six_bread_type = SIX_MEAT_GRILL_BELLY; jump_to_probeneedtip(); return; }
    g_six_bread_type = SIX_MEAT_FRIED_STEAK;   /* 炸牛排:份量驱动 */
    toastcolor_set_weight_options(w_fsteak_w, 3, 1);  /* 默认500g */
    g_toast_mode = TOAST_MODE_WEIGHT;
    jump_to_toastcolor();
}
static void on_sixop3page_bt3_click(lv_event_t *e)
{
    (void)e;
    if (screen_is_loading(lv_scr_act())) return;
    if (s_op3_kind == SIX_OP3_KIND_MEAT_TZ) { s_op3_enter_bt = 3; jump_to_porkmenutz(); return; }   /* 探针版:猪肉→猪肉二级菜单 */
    if (s_op3_kind == SIX_OP3_KIND_MUTTON) {
        g_six_bread_type = SIX_MEAT_GRILL_SKEWER;
        g_toast_mode = TOAST_MODE_DEGREE;
        jump_to_toastcolor();
        return;
    }
    if (s_op3_kind == SIX_OP3_KIND_PORK) {
        g_six_bread_type = SIX_MEAT_FRIED_RIB;   /* 炸排骨:份量驱动 */
        toastcolor_set_weight_options(w_frib_w, 3, 1);  /* 默认800g */
        g_toast_mode = TOAST_MODE_WEIGHT;
        jump_to_toastcolor();
        return;
    }
    g_six_bread_type = SIX_MEAT_GRILL_BEEF;   /* 烤牛肉 */
    jump_to_probeneedtip();
}

static void sixop3page_apply_labels(sixop3page_t *sp)
{
    if (!sp) return;
    if (sp->name && s_op3_name)  lv_label_set_text(sp->name, s_op3_name);

    /* op 标签:传入文字则设置;探针版(全 NULL)隐藏 */
    lv_obj_t *ops[3] = { sp->op1, sp->op2, sp->op3 };
    for (int i = 0; i < 3; i++) {
        if (!ops[i]) continue;
        if (s_op3_ops[i])
            lv_label_set_text(ops[i], s_op3_ops[i]);
        else
            lv_obj_add_flag(ops[i], LV_OBJ_FLAG_HIDDEN);
    }

    /* bt 按钮文字(探针版用):子 label 设置 */
    lv_obj_t *bts[3] = { sp->bt1, sp->bt2, sp->bt3 };
    for (int i = 0; i < 3; i++) {
        if (!bts[i] || !s_op3_bts[i]) continue;
        lv_obj_t *bl = lv_obj_get_child(bts[i], 0);
        if (bl) lv_label_set_text(bl, s_op3_bts[i]);
    }
}

/* probe1/2/3 是图片(lv_img):按掩码显隐,不写文字 */
static void sixop3page_apply_probe(sixop3page_t *sp)
{
    if (!sp) return;
    lv_obj_t *ps[3] = { sp->probe1, sp->probe2, sp->probe3 };
    for (int i = 0; i < 3; i++) {
        if (!ps[i]) continue;
        if (s_op3_probe_mask & (1 << i))
            lv_obj_clear_flag(ps[i], LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(ps[i], LV_OBJ_FLAG_HIDDEN);
    }
}

/* 构建三按钮焦点组 + 事件 */
static void sixop3page_setup_groups(sixop3page_t *sp)
{
    if (!sp) return;
    lv_obj_t *btns[] = { sp->bt1, sp->bt2, sp->bt3 };
    const int n = (int)(sizeof(btns) / sizeof(btns[0]));
    for (int k = 0; k < n; k++) {
        if (btns[k]) lv_group_remove_obj(btns[k]);
    }
    if (g_sixop3page) lv_group_del(g_sixop3page);
    g_sixop3page = group_create_for_page(btns, n);
    clear_focus_states(btns, n);

    if (sp->bt1) lv_obj_add_event_cb(sp->bt1, on_sixop3page_bt1_click, LV_EVENT_CLICKED, NULL);
    if (sp->bt2) lv_obj_add_event_cb(sp->bt2, on_sixop3page_bt2_click, LV_EVENT_CLICKED, NULL);
    if (sp->bt3) lv_obj_add_event_cb(sp->bt3, on_sixop3page_bt3_click, LV_EVENT_CLICKED, NULL);
}

/* 仅返回重建时恢复焦点：按 g_six_bread_type 回到进入时的按钮 */
static void sixop3page_restore_focus(sixop3page_t *sp)
{
    if (!sp) return;
    if (s_op3_kind == SIX_OP3_KIND_MEAT_TZ && sp->bt1) {
        /* 探针版:按本次进入的按钮恢复(不依赖全局 bread_type,防上次残留误聚焦) */
        if (s_op3_enter_bt == 2)
            lv_group_focus_obj(sp->bt2 ? sp->bt2 : sp->bt1);
        else if (s_op3_enter_bt == 3)
            lv_group_focus_obj(sp->bt3 ? sp->bt3 : sp->bt1);
        else
            lv_group_focus_obj(sp->bt1);
        return;
    }
    if (g_six_bread_type == SIX_MEAT_GRILL_LEG && sp->bt1)
        lv_group_focus_obj(sp->bt1);
    else if (g_six_bread_type == SIX_MEAT_GRILL_LAMBS && sp->bt2)
        lv_group_focus_obj(sp->bt2);
    else if (g_six_bread_type == SIX_MEAT_GRILL_SKEWER && sp->bt3)
        lv_group_focus_obj(sp->bt3);
    else if (g_six_bread_type == SIX_MEAT_GRILL_TENDERLOIN && sp->bt1)
        lv_group_focus_obj(sp->bt1);
    else if (g_six_bread_type == SIX_MEAT_GRILL_BELLY && sp->bt2)
        lv_group_focus_obj(sp->bt2);
    else if (g_six_bread_type == SIX_MEAT_FRIED_RIB && sp->bt3)
        lv_group_focus_obj(sp->bt3);
    else if (g_six_bread_type == SIX_MEAT_GRILL_BEEF && sp->bt3)
        lv_group_focus_obj(sp->bt3);
    else if (g_six_bread_type == SIX_MEAT_FRIED_STEAK && sp->bt2)
        lv_group_focus_obj(sp->bt2);
    else if (g_six_bread_type == SIX_MEAT_GRILL_STEAK && sp->bt1)
        lv_group_focus_obj(sp->bt1);
    else if (sp->bt1)
        lv_group_focus_obj(sp->bt1);
}

void jump_to_sixop3page(const char *name, const char *op1, const char *op2, const char *op3, int probe_mask, int kind)
{
    s_op3_name  = name;
    s_op3_ops[0] = op1;
    s_op3_ops[1] = op2;
    s_op3_ops[2] = op3;
    s_op3_bts[0] = NULL;   /* 非探针版:按钮文字用生成默认 */
    s_op3_bts[1] = NULL;
    s_op3_bts[2] = NULL;
    s_op3_probe_mask = probe_mask;
    s_op3_kind = kind;

    page_push(PAGE_SIXOP3PAGE);
    lv_obj_clean(lv_scr_act());
    sixop3page_create(&ui_manager);

    sixop3page_t *sp = sixop3page_get(&ui_manager);
    if (sp) {
        sixop3page_apply_labels(sp);
        sixop3page_apply_probe(sp);
        sixop3page_setup_groups(sp);
        /* 新进入:默认焦点第一个按钮(不继承上次使用页面的残留焦点) */
        if (sp->bt1) lv_group_focus_obj(sp->bt1);
    }
    current_group = g_sixop3page;

    lang_scr_load_anim(sixop3page_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixop3page] jump: %s\n", name ? name : "?");
}

/* 探针版肉菜单:name=肉,op/probe 全隐藏,bt1-3=牛肉/羊肉/猪肉 */
void jump_to_sixop3page_tz(const char *name, const char *bt1, const char *bt2, const char *bt3)
{
    s_op3_name  = name;
    s_op3_ops[0] = NULL;   /* op 标签全隐藏 */
    s_op3_ops[1] = NULL;
    s_op3_ops[2] = NULL;
    s_op3_bts[0] = bt1;
    s_op3_bts[1] = bt2;
    s_op3_bts[2] = bt3;
    s_op3_probe_mask = 0;                    /* probe 图标全隐藏 */
    s_op3_kind = SIX_OP3_KIND_MEAT_TZ;

    page_push(PAGE_SIXOP3PAGE);
    lv_obj_clean(lv_scr_act());
    sixop3page_create(&ui_manager);

    sixop3page_t *sp = sixop3page_get(&ui_manager);
    if (sp) {
        sixop3page_apply_labels(sp);
        sixop3page_apply_probe(sp);
        sixop3page_setup_groups(sp);
        if (sp->bt1) lv_group_focus_obj(sp->bt1);
    }
    current_group = g_sixop3page;

    lang_scr_load_anim(sixop3page_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixop3page] jump tz: %s\n", name ? name : "?");
}

void sixop3page_rebuild(page_id_t child)
{
    if (g_sixop3page) { lv_group_del(g_sixop3page); g_sixop3page = NULL; }
    lv_obj_clean(lv_scr_act());
    sixop3page_create(&ui_manager);

    sixop3page_t *sp = sixop3page_get(&ui_manager);
    if (sp) {
        sixop3page_apply_labels(sp);
        sixop3page_apply_probe(sp);
        sixop3page_setup_groups(sp);
        sixop3page_restore_focus(sp);   /* 返回重建:按进入的菜恢复焦点 */
    }
    current_group = g_sixop3page;

    lang_scr_load_anim(sixop3page_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixop3page] rebuild\n");
}
