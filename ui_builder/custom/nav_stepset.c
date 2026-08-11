#include "nav.h"

// stepset 页面：左侧 roller_main 选择大类（烹饪功能/特殊功能），
// 右侧 roller_mode 跟随切换对应子功能列表
static void stepset_on_main_change(lv_event_t *e)
{
    stepset_t *scr = lv_event_get_user_data(e);
    if (!scr) return;
    uint32_t sel = lv_roller_get_selected(scr->roller_main);
    if (sel == 0)
        lv_roller_set_options(scr->roller_mode,
                              "热风\n节能热风\n上下烧烤\n底部烧烤\n热风烧烤\n集中烧烤\n热风对流\n顶部烧烤",
                              LV_ROLLER_MODE_NORMAL);
    else
        lv_roller_set_options(scr->roller_mode,
                              "空气炸\n发酵\n慢煮\n解冻",
                              LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(scr->roller_mode, 0, LV_ANIM_OFF);
}

void stepset_bind_events(void)
{
    stepset_t *scr = stepset_get(&ui_manager);
    if (!scr || !scr->roller_main) return;
    lv_obj_add_event_cb(scr->roller_main, stepset_on_main_change,
                        LV_EVENT_VALUE_CHANGED, scr);
}
