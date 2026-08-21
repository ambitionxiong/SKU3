/*
 * nav_sixop3page.c - 六选项3页（复用页面）
 *
 * 通用三栏菜单：name 标题 + 三个 tile(bt1/bt2/bt3) + 三个标签(op1/op2/op3)。
 * 通过参数设定文字，点击 bt 暂留空（后续接菜谱）。
 * 注：probe1/2/3 是图片(lv_img)，只做显隐，不写文字（避免类型错配）。
 * 复用场景：牛肉(烤牛排/炸牛排/烤牛肉)、羊肉、猪肉等。
 */
#include "protocol.h"
#include "nav.h"

lv_group_t *g_sixop3page = NULL;

/* 重建时需要恢复的标签 */
static const char *s_op3_name  = NULL;
static const char *s_op3_ops[3] = { NULL, NULL, NULL };
static int s_op3_probe_mask = 0;   /* probe 图标显隐掩码: bit0=p1 bit1=p2 bit2=p3 */

/* bt 按钮点击 */
static void on_sixop3page_bt1_click(lv_event_t *e) { (void)e; g_six_bread_type = SIX_MEAT_GRILL_STEAK; jump_to_probeneedtip(); }  /* 烤牛排 */
static void on_sixop3page_bt2_click(lv_event_t *e) { (void)e; }  /* 炸牛排：TODO */
static void on_sixop3page_bt3_click(lv_event_t *e) { (void)e; g_six_bread_type = SIX_MEAT_GRILL_BEEF; jump_to_probeneedtip(); }  /* 烤牛肉 */

static void sixop3page_apply_labels(sixop3page_t *sp)
{
    if (!sp) return;
    if (sp->name && s_op3_name)  lv_label_set_text(sp->name, s_op3_name);
    if (sp->op1 && s_op3_ops[0]) lv_label_set_text(sp->op1, s_op3_ops[0]);
    if (sp->op2 && s_op3_ops[1]) lv_label_set_text(sp->op2, s_op3_ops[1]);
    if (sp->op3 && s_op3_ops[2]) lv_label_set_text(sp->op3, s_op3_ops[2]);
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
    /* 焦点恢复：按 g_six_bread_type 恢复到进入时的按钮 */
    if (g_six_bread_type == SIX_MEAT_GRILL_BEEF && sp->bt3)
        lv_group_focus_obj(sp->bt3);
    else if (g_six_bread_type == SIX_MEAT_GRILL_STEAK && sp->bt1)
        lv_group_focus_obj(sp->bt1);
    else if (sp->bt1)
        lv_group_focus_obj(sp->bt1);
}

void jump_to_sixop3page(const char *name, const char *op1, const char *op2, const char *op3, int probe_mask)
{
    s_op3_name  = name;
    s_op3_ops[0] = op1;
    s_op3_ops[1] = op2;
    s_op3_ops[2] = op3;
    s_op3_probe_mask = probe_mask;

    page_push(PAGE_SIXOP3PAGE);
    lv_obj_clean(lv_scr_act());
    sixop3page_create(&ui_manager);

    sixop3page_t *sp = sixop3page_get(&ui_manager);
    if (sp) {
        sixop3page_apply_labels(sp);
        sixop3page_apply_probe(sp);
        sixop3page_setup_groups(sp);
    }
    current_group = g_sixop3page;

    lv_scr_load_anim(sixop3page_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixop3page] jump: %s\n", name ? name : "?");
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
    }
    current_group = g_sixop3page;

    lv_scr_load_anim(sixop3page_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);
    printf("[sixop3page] rebuild\n");
}
