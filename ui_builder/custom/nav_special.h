#ifndef NAV_SPECIAL_H
#define NAV_SPECIAL_H

#include "nav.h"
#include "ui_objects.h"

// 通用字段索引（从 obj 开始计，所有 special_menu 组一致）
enum {
    S_OBJ    = 4,   // lv_obj_t *obj
    S_NEXT   = 5,   // lv_obj_t *next
    S_TEMP   = 9,   // lv_obj_t *temp
    S_HOUR   = 12,  // lv_obj_t *hour
    S_MIN    = 15,  // lv_obj_t *min
    S_TMPL3  = 18,  // lv_obj_t *templine3
    S_TMPL2  = 19,  // lv_obj_t *templine2
    S_HRLN   = 20,  // lv_obj_t *hourline
    S_MINLN  = 21,  // lv_obj_t *minline
    S_STOP   = 2,   // lv_obj_t *stop  (cooking)
    S_LITTLE = 3,   // lv_obj_t *little
    S_STATUS = 1,   // lv_obj_t *status (cooking/stop/stop_back/complete)
    S_TIMELBL= 2,   // lv_obj_t *timelabel (cooking)
    S_START  = 1,   // lv_obj_t *start  (stop)
    S_SURE   = 3,   // lv_obj_t *sure   (stop_back/setting/complete)
    S_OPREHT = 1,   // offpreheat (set)
    S_ONPREHT= 6,   // onpreheat  (set)
    S_ODELAY = 2,   // offdelay   (set)
    S_ONDELAY= 7,   // ondelay    (set)
    S_OCONTA = 3,   // offcontain (set)
    S_ONCONTA= 8,   // oncontain  (set)
    S_ICON2  = 10,  // icon2 (set)
    S_ICON3  = 9,   // icon3 (set)
};

// 组描述符
typedef struct {
    const char  *tag;
    page_id_t    page_ids[7];     // MENU~COMPLETE
    lv_group_t **groups[7];
    int          cook_mode;
    const char  *status_text;
    int          bar_cook;        // cooking 页 bar 编号
    int          bar_stop;        // stop 页 bar 编号
    int          bar_sback;       // stop_back 页 bar 编号
} special_group_t;

// 字段访问宏
#define FLD(p, idx) (((lv_obj_t **)(p))[idx])

// 泛型函数
void special_jump_to(const special_group_t *g, int page_idx);
void special_rebuild(const special_group_t *g, int page_idx, page_id_t child);
int  special_dispatch_pop(page_id_t prev, page_id_t child);

// 所有 special 组描述符表（nav_special.c 中定义）
extern const special_group_t SPECIAL_GROUPS[];
extern const int SPECIAL_GROUPS_COUNT;

#endif
