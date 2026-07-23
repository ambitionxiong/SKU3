#include "nav_special.h"
#include "protocol.h"

/***** 泛型 setup 函数（所有 special_menu 组共用） *****/

static void menu_setup(void *vp, lv_group_t **g_out, page_id_t child)
{
    if (!vp) return;
    lv_obj_t *temp = FLD(vp, S_TEMP);
    lv_obj_t *hour = FLD(vp, S_HOUR);
    lv_obj_t *min  = FLD(vp, S_MIN);
    lv_obj_t *next = FLD(vp, S_NEXT);
    lv_obj_t *tmpl2 = FLD(vp, S_TMPL2);
    lv_obj_t *tmpl3 = FLD(vp, S_TMPL3);
    lv_obj_t *hrln  = FLD(vp, S_HRLN);
    lv_obj_t *minln = FLD(vp, S_MINLN);

    lv_obj_t *btns[] = { temp, hour, min, next };
    if (*g_out) lv_group_del(*g_out);
    *g_out = group_create_for_page(btns, 4);
}

static void set_setup(void *vp, lv_group_t **g_out, page_id_t child)
{
    if (!vp) return;
    lv_obj_t *sure    = FLD(vp, S_SURE);
    lv_obj_t *opreh   = FLD(vp, S_OPREHT);
    lv_obj_t *onpreh  = FLD(vp, S_ONPREHT);
    lv_obj_t *odelay  = FLD(vp, S_ODELAY);
    lv_obj_t *ondelay = FLD(vp, S_ONDELAY);
    lv_obj_t *oconta  = FLD(vp, S_OCONTA);
    lv_obj_t *onconta = FLD(vp, S_ONCONTA);

    lv_obj_t *btns[] = { sure, opreh, onpreh, odelay, ondelay, oconta, onconta };
    if (*g_out) lv_group_del(*g_out);
    *g_out = group_create_for_page(btns, 7);
}
