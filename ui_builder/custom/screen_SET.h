#ifndef __SCREEN_SET_H__
#define __SCREEN_SET_H__

#include "ui_objects.h"

typedef struct {
    lv_obj_t *obj;
    lv_obj_t *Title_Lb;
    lv_obj_t *Time_Lb;
    lv_obj_t *SET_bg_Img;
    lv_obj_t *DSQ_Btn;
    lv_obj_t *TS_Btn;
    lv_obj_t *ZDBW_Btn;
    lv_obj_t *TFLQYX_Btn;
    lv_obj_t *Six_Btn;
    lv_obj_t *LDG_Btn;
    lv_obj_t *WDDW_Btn;
    lv_obj_t *SYSZ_Btn;
    lv_obj_t *LDXS_Btn;
    lv_obj_t *Time_Btn;
    lv_obj_t *DJ_Time_Btn;
    lv_obj_t *YY_Btn;
    lv_obj_t *Power_Btn;
    lv_obj_t *Demo_Btn;
    lv_obj_t *RESET_Btn;
    lv_obj_t *GYJQ_Btn;
    lv_obj_t *TXT_Img;
    lv_obj_t *TS_Lb;
    lv_obj_t *ZDBW_Lb;
    lv_obj_t *Six_Lb;
    lv_obj_t *WDDW_Lb;
    lv_obj_t *YY_Lb;
    lv_obj_t *Power_Lb;
    lv_obj_t *Demo_Lb;
    lv_obj_t *CountDown_icon_Img;
} screen_SET_t;

/* 独立全局页面对象:不挂 ui_manager,上位机同步 ui_objects.h 不受影响 */
extern screen_SET_t screen_SET;

static inline screen_SET_t *screen_SET_get(ui_manager_t *ui)
{
    (void)ui;
    return &screen_SET;
}

void screen_SET_create(ui_manager_t *ui);

#endif /* __SCREEN_SET_H__ */
