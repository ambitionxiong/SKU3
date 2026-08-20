/*
 * nav_cooktimer.c - 烹饪定时器
 *
 * 职责：
 *   1. cooking_timer_cb：核心烹饪驱动定时器(常驻创建/按需删除)。
 *      每 100ms 更新倒计时/进度条/状态文本，处理到点完成、
 *      门开自动暂停(auto_pause_on_door)、保温/预约/六感/多段等分发。
 *   2. auto_pause_on_door：检测到门开 → 各运行页进入暂停(stop)页。
 *
 * 注意：此定时器在开始烹饪时创建，停止/完成时删除；
 *   页面切换期间通过 current_group/页面栈判断是否继续驱动。
 */

#include "nav.h"
#include "nav_internal.h"

/* 门开自动暂停：当前为烹饪页时跳入对应暂停页(各模式 stop) */
static void auto_pause_on_door(void)
{
    if (!is_door_open()) return;
    if (depth < 1) return;
    page_id_t cur = page_stack[depth - 1];
    if (cur == PAGE_UPDOWN_BBQ_COOKING) jump_to_updown_bbq_stop();
    else if (cur == PAGE_UPDOWN_BBQ_COOKING_PROBE) jump_to_updown_bbq_stop_probe();
    else if (cur == PAGE_HOT_BBQ_COOKING_PROBE) jump_to_hot_bbq_stop_probe();
    else if (cur == PAGE_BOTTOM_BBQ_COOKING_PROBE) jump_to_bottom_bbq_stop_probe();
    else if (cur == PAGE_SLOWCOOK_COOKING_PROBE) jump_to_slowcook_stop_probe();
    else if (cur == PAGE_TOP_BBQ_COOKING) jump_to_top_bbq_stop();
    else if (cur == PAGE_BOTTOM_BBQ_COOKING) jump_to_bottom_bbq_stop();
    else if (cur == PAGE_HOT_BBQ_COOKING) jump_to_hot_bbq_stop();
    else if (cur == PAGE_HOTWIND_BBQ_COOKING) jump_to_hotwind_bbq_stop();
    else if (cur == PAGE_SAVE_BBQ_COOKING) jump_to_save_bbq_stop();
    else if (cur == PAGE_CENTRAL_BBQ_COOKING) jump_to_central_bbq_stop();
    else if (cur == PAGE_WINDCHANGE_BBQ_COOKING) jump_to_windchange_bbq_stop();
    else if (cur == PAGE_UPDOWN_BBQ_SETTING) jump_to_updown_bbq_stop();
    else if (cur == PAGE_TOP_BBQ_SETTING) jump_to_top_bbq_stop();
    else if (cur == PAGE_BOTTOM_BBQ_SETTING) jump_to_bottom_bbq_stop();
    else if (cur == PAGE_HOT_BBQ_SETTING) jump_to_hot_bbq_stop();
    else if (cur == PAGE_HOTWIND_BBQ_SETTING) jump_to_hotwind_bbq_stop();
    else if (cur == PAGE_SAVE_BBQ_SETTING) jump_to_save_bbq_stop();
    else if (cur == PAGE_CENTRAL_BBQ_SETTING) jump_to_central_bbq_stop();
    else if (cur == PAGE_WINDCHANGE_BBQ_SETTING) jump_to_windchange_bbq_stop();
    else if (cur == PAGE_COOKIE_SETTING) jump_to_cookie_stop();
    else if (cur == PAGE_WEST_SETTING) jump_to_west_stop();
    else if (cur == PAGE_PIZZA_SETTING) jump_to_pizza_stop();
    else if (cur == PAGE_MENU_COOK_SETTING) jump_to_menu_stop();
    else if (cur == PAGE_AIR_SETTING) jump_to_air_stop();
    else if (cur == PAGE_PIZZA_2_SETTING) jump_to_pizza_2_stop();
    else if (cur == PAGE_SLOWCOOK_SETTING) jump_to_slowcook_stop();
    else if (cur == PAGE_UNFROZEN_SETTING) jump_to_unfrozen_stop();
    else if (cur == PAGE_RISING_SETTING) jump_to_rising_stop();
    else if (cur == PAGE_CORN_SETTING) jump_to_corn_stop();
    else if (cur == PAGE_HEATCONTAIN_SETTING) jump_to_heatcontain_stop();
    else if (cur == PAGE_LASAGNA_SETTING) jump_to_lasagna_stop();
    else if (cur == PAGE_STRUDEL_SETTING) jump_to_strudel_stop();
    else if (cur == PAGE_BREAD_SETTING) jump_to_bread_stop();
    else if (cur == PAGE_PIZZA3_SETTING) jump_to_pizza3_stop();
    else if (cur == PAGE_CHIP_SETTING) jump_to_chip_stop();
    else if (cur == PAGE_CUSTOM_SETTING) jump_to_custom_stop();
    else if (cur == PAGE_PREHEAT_COOKING) jump_to_preheat_stop();
    else if (cur == PAGE_COOKIE_COOKING) jump_to_cookie_stop();
    else if (cur == PAGE_WEST_COOKING) jump_to_west_stop();
    else if (cur == PAGE_PIZZA_COOKING) jump_to_pizza_stop();
    else if (cur == PAGE_MENU_COOK_COOKING) jump_to_menu_stop();
    else if (cur == PAGE_AIR_COOKING) jump_to_air_stop();
    else if (cur == PAGE_PIZZA_2_COOKING) jump_to_pizza_2_stop();
    else if (cur == PAGE_SLOWCOOK_COOKING) jump_to_slowcook_stop();
    else if (cur == PAGE_UNFROZEN_COOKING) jump_to_unfrozen_stop();
    else if (cur == PAGE_RISING_COOKING) jump_to_rising_stop();
    else if (cur == PAGE_CORN_COOKING) jump_to_corn_stop();
    else if (cur == PAGE_HEATCONTAIN_COOKING) jump_to_heatcontain_stop();
    else if (cur == PAGE_LASAGNA_COOKING) jump_to_lasagna_stop();
    else if (cur == PAGE_STRUDEL_COOKING) jump_to_strudel_stop();
    else if (cur == PAGE_BREAD_COOKING) jump_to_bread_stop();
    else if (cur == PAGE_PIZZA3_COOKING) jump_to_pizza3_stop();
    else if (cur == PAGE_CHIP_COOKING) jump_to_chip_stop();
    else if (cur == PAGE_CUSTOM_COOKING) jump_to_custom_stop();
    else if (cur == PAGE_WATER_CLEAN_COOKING) jump_to_wc_stop();
    else if (cur == PAGE_HOTCLEANSAVE_COOKING) jump_to_hcs_stop();
    else if (cur == PAGE_HOTCLEANMIDDLE_COOKING) jump_to_hcm_stop();
    else if (cur == PAGE_HOTCLEANHIGH_COOKING) jump_to_hch_stop();
    else if (cur == PAGE_COLOR_COOKING) jump_to_color_stop();
}
void cooking_timer_cb(lv_timer_t *timer)
{
    /* complete 结束态：timer 仅服务保温计数，其他页面不驱动任何完成检测 */
    if (g_send.iface_status == IFACE_COMPLETE) {
        /* 按当前 complete 组取"已完成/保温中"标签 */
        lv_obj_t *kw_label = NULL;
        if (current_group == g_updown_bbq_complete) {
            updown_bbq_complete_t *c = updown_bbq_complete_get(&ui_manager);
            kw_label = c ? c->complete_label : NULL;
        } else if (current_group == g_top_bbq_complete) {
            top_bbq_complete_t *c = top_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_108 : NULL;
        } else if (current_group == g_bottom_bbq_complete) {
            bottom_bbq_complete_t *c = bottom_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_156 : NULL;
        } else if (current_group == g_hot_bbq_complete) {
            hot_bbq_complete_t *c = hot_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_204 : NULL;
        } else if (current_group == g_hotwind_bbq_complete) {
            hotwind_bbq_complete_t *c = hotwind_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_242 : NULL;
        } else if (current_group == g_save_bbq_complete) {
            save_bbq_complete_t *c = save_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_280 : NULL;
        } else if (current_group == g_central_bbq_complete) {
            central_bbq_complete_t *c = central_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_318 : NULL;
        } else if (current_group == g_windchange_bbq_complete) {
            windchange_bbq_complete_t *c = windchange_bbq_complete_get(&ui_manager);
            kw_label = c ? c->label_356 : NULL;
        } else if (current_group == g_air_complete) {
            air_complete_t *c = air_complete_get(&ui_manager);
            kw_label = c ? c->label_337 : NULL;
        } else if (current_group == g_pizza_2_complete) {
            pizza_2_complete_t *c = pizza_2_complete_get(&ui_manager);
            kw_label = c ? c->label_386 : NULL;
        } else if (current_group == g_slowcook_complete) {
            slowcook_complete_t *c = slowcook_complete_get(&ui_manager);
            kw_label = c ? c->label_435 : NULL;
        } else if (current_group == g_unfrozen_complete) {
            unfrozen_complete_t *c = unfrozen_complete_get(&ui_manager);
            kw_label = c ? c->label_484 : NULL;
        } else if (current_group == g_rising_complete) {
            rising_complete_t *c = rising_complete_get(&ui_manager);
            kw_label = c ? c->label_533 : NULL;
        } else if (current_group == g_corn_complete) {
            corn_complete_t *c = corn_complete_get(&ui_manager);
            kw_label = c ? c->label_582 : NULL;
        } else if (current_group == g_heatcontain_complete) {
            heatcontain_complete_t *c = heatcontain_complete_get(&ui_manager);
            kw_label = c ? c->label_672 : NULL;
        } else if (current_group == g_lasagna_complete) {
            lasagna_complete_t *c = lasagna_complete_get(&ui_manager);
            kw_label = c ? c->label_681 : NULL;
        } else if (current_group == g_strudel_complete) {
            strudel_complete_t *c = strudel_complete_get(&ui_manager);
            kw_label = c ? c->label_722 : NULL;
        } else if (current_group == g_bread_complete) {
            bread_complete_t *c = bread_complete_get(&ui_manager);
            kw_label = c ? c->label_763 : NULL;
        } else if (current_group == g_pizza3_complete) {
            pizza3_complete_t *c = pizza3_complete_get(&ui_manager);
            kw_label = c ? c->label_804 : NULL;
        } else if (current_group == g_chip_complete) {
            chip_complete_t *c = chip_complete_get(&ui_manager);
            kw_label = c ? c->label_845 : NULL;
        } else if (current_group == g_custom_complete) {
            custom_complete_t *c = custom_complete_get(&ui_manager);
            kw_label = c ? c->label_886 : NULL;
        } else if (current_group == g_cookie_complete) {
            cookie_complete_t *c = cookie_complete_get(&ui_manager);
            kw_label = c ? c->label_143 : NULL;
        } else if (current_group == g_west_complete) {
            west_complete_t *c = west_complete_get(&ui_manager);
            kw_label = c ? c->label_192 : NULL;
        } else if (current_group == g_pizza_complete) {
            pizza_complete_t *c = pizza_complete_get(&ui_manager);
            kw_label = c ? c->label_240 : NULL;
        } else if (current_group == g_menu_cook_complete) {
            menu_complete_t *c = menu_complete_get(&ui_manager);
            kw_label = c ? c->label_288 : NULL;
        }
        if (kw_label && contain_on) {
            /* 自动保温：停留 1 分钟无操作 → 保温中（15 分钟）→ 结束回已完成 */
            g_keepwarm_sec++;
            if (!g_keepwarm_active) {
                if (g_keepwarm_sec >= 60) {
                    g_keepwarm_active = 1;
                    g_keepwarm_sec = 0;
                    lv_label_set_text(kw_label, "保温中...");
                    printf("[keepwarm] START: sec=%d active=%d buf17=%d timer=%p\n",
                           g_keepwarm_sec, g_keepwarm_active,
                           g_keepwarm_active ? 0x01 : 0x00, (void *)cook_timer);
                }
            } else {
                if (g_keepwarm_sec >= 60) {
                    g_keepwarm_active = 0;
                    lv_label_set_text(kw_label, "已完成");
                    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
                    printf("[keepwarm] END: sec=%d active=%d buf17=%d timer=%p\n",
                           g_keepwarm_sec, g_keepwarm_active,
                           g_keepwarm_active ? 0x01 : 0x00, (void *)cook_timer);
                }
            }
        }
        return;
    }
    /* 设置页覆盖层打开:跳过所有完成/跳转检测,避免下层跳转销毁覆盖层(悬空+栈错乱);
       返回时 screen_set_back 会补刷一次,行为不丢 */
    if (depth > 0 && page_stack[depth - 1] == PAGE_SCREEN_SET)
        return;
    if (g_on_stop_back) {
        if (g_send.iface_status == IFACE_DELAY_RESERVE &&
            (current_group == g_updown_bbq_stop_back ||
         current_group == g_top_bbq_stop_back ||
         current_group == g_bottom_bbq_stop_back ||
         current_group == g_hot_bbq_stop_back ||
         current_group == g_hotwind_bbq_stop_back ||
         current_group == g_save_bbq_stop_back ||
         current_group == g_central_bbq_stop_back ||
         current_group == g_windchange_bbq_stop_back ||
         current_group == g_air_stop_back ||
         current_group == g_pizza_2_stop_back ||
         current_group == g_slowcook_stop_back ||
         current_group == g_unfrozen_stop_back ||
         current_group == g_rising_stop_back ||
         current_group == g_corn_stop_back ||
         current_group == g_heatcontain_stop_back ||
         current_group == g_updown_bbq_stop_back_probe ||
         current_group == g_hot_bbq_stop_back_probe ||
         current_group == g_bottom_bbq_stop_back_probe ||
         current_group == g_slowcook_stop_back_probe ||
         current_group == g_cookie_stop_back ||
         current_group == g_west_stop_back ||
         current_group == g_pizza_stop_back ||
         current_group == g_menu_cook_stop_back ||
         current_group == g_lasagna_stop_back ||
         current_group == g_strudel_stop_back ||
         current_group == g_bread_stop_back ||
         current_group == g_pizza3_stop_back ||
         current_group == g_chip_stop_back ||
         current_group == g_custom_stop_back)) {
            /* 预约取消确认页（预约中态）：到点自动开始烹饪，不走时长完成检测 */
            if (g_delay_target > 0 && rtc_now_ms() >= g_delay_target) {
                if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
                delay_on = 0;
                delay_start_cook();
                if (current_group == g_updown_bbq_stop_back ||
         current_group == g_top_bbq_stop_back ||
         current_group == g_bottom_bbq_stop_back ||
         current_group == g_hot_bbq_stop_back ||
         current_group == g_hotwind_bbq_stop_back ||
         current_group == g_save_bbq_stop_back ||
         current_group == g_central_bbq_stop_back ||
         current_group == g_windchange_bbq_stop_back ||
         current_group == g_air_stop_back ||
         current_group == g_pizza_2_stop_back ||
         current_group == g_slowcook_stop_back ||
         current_group == g_unfrozen_stop_back ||
         current_group == g_rising_stop_back ||
         current_group == g_corn_stop_back ||
         current_group == g_heatcontain_stop_back ||
         current_group == g_updown_bbq_stop_back_probe ||
         current_group == g_hot_bbq_stop_back_probe ||
         current_group == g_bottom_bbq_stop_back_probe ||
         current_group == g_slowcook_stop_back_probe ||
         current_group == g_cookie_stop_back ||
         current_group == g_west_stop_back ||
         current_group == g_pizza_stop_back ||
         current_group == g_menu_cook_stop_back ||
         current_group == g_lasagna_stop_back ||
         current_group == g_strudel_stop_back ||
         current_group == g_bread_stop_back ||
         current_group == g_pizza3_stop_back ||
         current_group == g_chip_stop_back ||
         current_group == g_custom_stop_back) {
                    /* 门开被拦截：继续检查 */
                    cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
                }
            }
            return;
        }
        if (current_group == g_preheat_stop_back) {
            /* 预热按腔温驱动：刷新进度条并到温自动完成 */
            preheat_stop_back_t *back = preheat_stop_back_get(&ui_manager);
            if (back) {
                uint16_t cavity = get_cavity_temp();
                int range = set_temp - preheat_start_cavity;
                int p = range <= 0 ? 100 : 13 + (int)((int64_t)87 * (cavity - preheat_start_cavity) / range);
                if (p < 13) p = 13;
                if (p > 100) p = 100;
                lv_bar_set_value(back->bar_1, p, LV_ANIM_OFF);
                if (back->bartemp) {
                    int disp = cavity > set_temp ? set_temp : cavity;
                    lv_label_set_text_fmt(back->bartemp, "%d℃", disp);
                    int bx = 122 + (637 * p) / 100 - 80;
                    lv_obj_set_pos(back->bartemp, bx, 323);
                }
            }
            if (get_cavity_temp() >= set_temp && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                g_on_stop_back = 0;
                if (g_stop_back_complete) {
                    void (*fn)(void) = g_stop_back_complete;
                    g_stop_back_complete = NULL;
                    fn();
                }
            }
            return;
        }
        if (current_group == g_hcs_stop_back ||
            current_group == g_hcm_stop_back ||
            current_group == g_hch_stop_back) {
            /* hotclean stop_back（cooking 进入）：计时完成后进入冷却阶段 */
            uint32_t e = cook_timer ? (lv_tick_get() - cook_start_time) : (uint32_t)cook_elapsed_saved;
            if (e >= (uint32_t)cook_total_ms && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_on_stop_back = 0;
                g_stop_back_complete = NULL;
                if (current_group == g_hcs_stop_back) jump_to_hcs_cooling();
                else if (current_group == g_hcm_stop_back) jump_to_hcm_cooling();
                else jump_to_hch_cooling();
                return;
            }
            /* 未完成：fall through 到下方 elapsed 进度更新（hcs/hcm/hch bar）*/
        }
        uint32_t elapsed = lv_tick_get() - cook_start_time;
        if (current_group == g_updown_bbq_stop_back) {
            updown_bbq_stop_back_t *back = updown_bbq_stop_back_get(&ui_manager);
            if (back) {
                int p = stop_back_progress(elapsed, cook_total_ms);
                if (p > 100) p = 100;
                lv_bar_set_value(back->bar_2, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_updown_bbq_stop_back_probe) {
            updown_bbq_stop_back_probe_t *back = updown_bbq_stop_back_probe_get(&ui_manager);
            if (back) {
                int probe = get_probe_temp();
                int range = probe_target_temp - cook_start_probe;
                int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
                if (p > 100) p = 100;
                if (p < 3) p = 3;
                lv_bar_set_value(back->bar_3, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_hot_bbq_stop_back_probe) {
            hot_bbq_stop_back_probe_t *back = hot_bbq_stop_back_probe_get(&ui_manager);
            if (back) {
                int probe = get_probe_temp();
                int range = probe_target_temp - cook_start_probe;
                int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
                if (p > 100) p = 100; if (p < 3) p = 3;
                lv_bar_set_value(back->bar_3, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_bottom_bbq_stop_back_probe) {
            bottom_bbq_stop_back_probe_t *back = bottom_bbq_stop_back_probe_get(&ui_manager);
            if (back) {
                int probe = get_probe_temp();
                int range = probe_target_temp - cook_start_probe;
                int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
                if (p > 100) p = 100; if (p < 3) p = 3;
                lv_bar_set_value(back->bar_7, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_slowcook_stop_back_probe) {
            slowcook_stop_back_probe_t *back = slowcook_stop_back_probe_get(&ui_manager);
            if (back) {
                int probe = get_probe_temp();
                int range = probe_target_temp - cook_start_probe;
                int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
                if (p > 100) p = 100; if (p < 3) p = 3;
                lv_bar_set_value(back->bar_11, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_top_bbq_stop_back) {
            top_bbq_stop_back_t *back = top_bbq_stop_back_get(&ui_manager);
            if (back) {
                int p = stop_back_progress(elapsed, cook_total_ms);
                if (p > 100) p = 100;
                lv_bar_set_value(back->bar_8, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_hot_bbq_stop_back) {
            hot_bbq_stop_back_t *back = hot_bbq_stop_back_get(&ui_manager);
            if (back) {
                int p = stop_back_progress(elapsed, cook_total_ms);
                if (p > 100) p = 100;
                lv_bar_set_value(back->bar_16, p, LV_ANIM_OFF);
                lv_obj_invalidate(lv_scr_act());
            }
        }
        if (current_group == g_bottom_bbq_stop_back) {
            bottom_bbq_stop_back_t *back = bottom_bbq_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_12, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_hotwind_bbq_stop_back) {
            hotwind_bbq_stop_back_t *back = hotwind_bbq_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_20, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_save_bbq_stop_back) {
            save_bbq_stop_back_t *back = save_bbq_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_24, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_central_bbq_stop_back) {
            central_bbq_stop_back_t *back = central_bbq_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_28, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_windchange_bbq_stop_back) {
            windchange_bbq_stop_back_t *back = windchange_bbq_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_32, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_cookie_stop_back) {
            cookie_stop_back_t *back = cookie_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_6, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_west_stop_back) {
            west_stop_back_t *back = west_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_10, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_pizza_stop_back) {
            pizza_stop_back_t *back = pizza_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_14, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_menu_cook_stop_back) {
            menu_stop_back_t *back = menu_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_18, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_air_stop_back) {
            air_stop_back_t *back = air_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_22, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_pizza_2_stop_back) {
            pizza_2_stop_back_t *back = pizza_2_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_26, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_slowcook_stop_back) {
            slowcook_stop_back_t *back = slowcook_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_30, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_unfrozen_stop_back) {
            unfrozen_stop_back_t *back = unfrozen_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_34, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_rising_stop_back) {
            rising_stop_back_t *back = rising_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_38, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_corn_stop_back) {
            corn_stop_back_t *back = corn_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_42, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_heatcontain_stop_back) {
            heatcontain_stop_back_t *back = heatcontain_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_46, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_lasagna_stop_back) {
            lasagna_stop_back_t *back = lasagna_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_50, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_strudel_stop_back) {
            strudel_stop_back_t *back = strudel_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_54, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_bread_stop_back) {
            bread_stop_back_t *back = bread_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_58, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_pizza3_stop_back) {
            pizza3_stop_back_t *back = pizza3_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_62, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_chip_stop_back) {
            chip_stop_back_t *back = chip_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_66, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_custom_stop_back) {
            custom_stop_back_t *back = custom_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_70, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_wc_stop_back) {
            waterclean_stop_back_t *back = waterclean_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_1, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_hcs_stop_back) {
            hotcleansave_stop_back_t *back = hotcleansave_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_4, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_hcm_stop_back) {
            hotcleanmiddle_stop_back_t *back = hotcleanmiddle_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_2, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_hch_stop_back) {
            hotcleanhigh_stop_back_t *back = hotcleanhigh_stop_back_get(&ui_manager);
            if (back) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(back->bar_3, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_color_stop_back) {
            color_stop_back_t *csb = color_stop_back_get(&ui_manager);
            if (csb) { int p = stop_back_progress(elapsed, cook_total_ms); if (p > 100) p = 100; lv_bar_set_value(csb->bar_4, p, LV_ANIM_OFF); lv_obj_invalidate(lv_scr_act()); }
        }
        if (current_group == g_updown_bbq_stop_back_probe) {
            int probe = get_probe_temp();
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                g_send.cook_flag = 0;
                g_on_stop_back = 0;
                if (g_stop_back_complete) {
                    void (*fn)(void) = g_stop_back_complete;
                    g_stop_back_complete = NULL;
                    fn();
                }
            }
        }
        if (current_group == g_hot_bbq_stop_back_probe) {
            int probe = get_probe_temp();
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
                lv_timer_del(cook_timer); cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE; g_send.cook_flag = 0;
                g_on_stop_back = 0;
                if (g_stop_back_complete) { void (*fn)(void) = g_stop_back_complete; g_stop_back_complete = NULL; fn(); }
            }
        }
        if (current_group == g_bottom_bbq_stop_back_probe) {
            int probe = get_probe_temp();
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
                lv_timer_del(cook_timer); cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE; g_send.cook_flag = 0;
                g_on_stop_back = 0;
                if (g_stop_back_complete) { void (*fn)(void) = g_stop_back_complete; g_stop_back_complete = NULL; fn(); }
            }
        }
        if (current_group == g_slowcook_stop_back_probe) {
            int probe = get_probe_temp();
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
                lv_timer_del(cook_timer); cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE; g_send.cook_flag = 0;
                g_on_stop_back = 0;
                if (g_stop_back_complete) { void (*fn)(void) = g_stop_back_complete; g_stop_back_complete = NULL; fn(); }
            }
        }
        if (elapsed >= (uint32_t)cook_total_ms && cook_timer
            && g_send.iface_status != IFACE_DELAY_RESERVE
            && current_group != g_updown_bbq_stop_back_probe
            && current_group != g_hot_bbq_stop_back_probe
            && current_group != g_bottom_bbq_stop_back_probe
            && current_group != g_slowcook_stop_back_probe
            && current_group != g_preheat_stop_back
            && current_group != g_hcs_stop_back
            && current_group != g_hcm_stop_back
            && current_group != g_hch_stop_back) {
            lv_timer_del(cook_timer);
            cook_timer = NULL;
            g_send.buzzer_req = BUZZER_COOK_DONE;
            g_on_stop_back = 0;
            cook_is_color = 0;
            if (g_somecook_running) {
                somecook_cooking_next_step();
            } else if (g_stop_back_complete) {
                void (*fn)(void) = g_stop_back_complete;
                g_stop_back_complete = NULL;
                fn();
            }
        }
        return;
    }
    if (current_group == g_somecook_cooking) {
        somecook_cooking_t *sc = somecook_cooking_get(&ui_manager);
        if (sc && cook_timer) {   /* cooking/stopback 态计时中 */
            somecook_cooking_update_timer(sc);
            /* 当前段到点 → 切下一段 / 全部完成 */
            uint32_t elapsed = lv_tick_get() - cook_start_time;
            if (elapsed >= (uint32_t)cook_total_ms) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                somecook_cooking_next_step();
            }
        }
    }
    if (is_door_open()) {
        if (g_somecook_running) {
            somecook_cooking_auto_pause();
            return;
        }
        auto_pause_on_door();
        return;
    }
    if (depth > 0 && (page_stack[depth-1] == PAGE_HOTCLEANSAVE_COOLING ||
                      (page_stack[depth-1] == PAGE_HOTCLEANSAVE_STOP_BACK && !g_on_stop_back))) {
        hotcleansave_cooling_t *cool = hotcleansave_cooling_get(&ui_manager);
        if (cool) {
            uint16_t cavity = get_cavity_temp();
            if (cavity <= 270 && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                jump_to_hcs_complete();
            }
        }
        return;
    }
    if (depth > 0 && (page_stack[depth-1] == PAGE_HOTCLEANMIDDLE_COOLING ||
                      (page_stack[depth-1] == PAGE_HOTCLEANMIDDLE_STOP_BACK && !g_on_stop_back))) {
        hotcleanmiddle_cooling_t *cool = hotcleanmiddle_cooling_get(&ui_manager);
        if (cool) {
            uint16_t cavity = get_cavity_temp();
            if (cavity <= 270 && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                jump_to_hcm_complete();
            }
        }
        return;
    }
    if (depth > 0 && (page_stack[depth-1] == PAGE_HOTCLEANHIGH_COOLING ||
                      (page_stack[depth-1] == PAGE_HOTCLEANHIGH_STOP_BACK && !g_on_stop_back))) {
        hotcleanhigh_cooling_t *cool = hotcleanhigh_cooling_get(&ui_manager);
        if (cool) {
            uint16_t cavity = get_cavity_temp();
            if (cavity <= 270 && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                jump_to_hch_complete();
            }
        }
        return;
    }
    if (current_group == g_updown_bbq_stop_back) {
        uint32_t elapsed = lv_tick_get() - cook_start_time;
        if (elapsed >= (uint32_t)cook_total_ms && cook_timer) {
            lv_timer_del(cook_timer);
            cook_timer = NULL;
            g_send.buzzer_req = BUZZER_COOK_DONE;
            jump_to_updown_bbq_complete();
        }
        return;
    }
    if (current_group == g_updown_bbq_cooking_probe) {
        int probe = get_probe_temp();
        updown_bbq_cooking_probe_t *cook = updown_bbq_cooking_probe_get(&ui_manager);
        if (cook) {
            int range = probe_target_temp - cook_start_probe;
            int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
            if (p > 100) p = 100;
            if (p < 3) p = 3;
            lv_bar_set_value(cook->bar_1, p, LV_ANIM_OFF);
            int display = probe > probe_target_temp ? probe_target_temp : probe;
            lv_label_set_text_fmt(cook->temp, "%d℃", display);
            lv_obj_invalidate(lv_scr_act());
        }
            if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
            lv_timer_del(cook_timer);
            cook_timer = NULL;
            g_send.buzzer_req = BUZZER_COOK_DONE;
            g_send.cook_flag = 0;
            jump_to_updown_bbq_complete_probe();
        }
        return;
    }
    if (current_group == g_hot_bbq_cooking_probe) {
        int probe = get_probe_temp();
        hot_bbq_cooking_probe_t *cook = hot_bbq_cooking_probe_get(&ui_manager);
        if (cook) {
            int range = probe_target_temp - cook_start_probe;
            int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
            if (p > 100) p = 100; if (p < 3) p = 3;
            lv_bar_set_value(cook->bar_1, p, LV_ANIM_OFF);
            int display = probe > probe_target_temp ? probe_target_temp : probe;
            lv_label_set_text_fmt(cook->temp, "%d℃", display);
            lv_obj_invalidate(lv_scr_act());
        }
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
            lv_timer_del(cook_timer); cook_timer = NULL;
            g_send.buzzer_req = BUZZER_COOK_DONE; g_send.cook_flag = 0;
            jump_to_hot_bbq_complete_probe();
        }
        return;
    }
    if (current_group == g_bottom_bbq_cooking_probe) {
        int probe = get_probe_temp();
        bottom_bbq_cooking_probe_t *cook = bottom_bbq_cooking_probe_get(&ui_manager);
        if (cook) {
            int range = probe_target_temp - cook_start_probe;
            int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
            if (p > 100) p = 100; if (p < 3) p = 3;
            lv_bar_set_value(cook->bar_5, p, LV_ANIM_OFF);
            int display = probe > probe_target_temp ? probe_target_temp : probe;
            lv_label_set_text_fmt(cook->temp, "%d℃", display);
            lv_obj_invalidate(lv_scr_act());
        }
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
            lv_timer_del(cook_timer); cook_timer = NULL;
            g_send.buzzer_req = BUZZER_COOK_DONE; g_send.cook_flag = 0;
            jump_to_bottom_bbq_complete_probe();
        }
        return;
    }
    if (current_group == g_slowcook_cooking_probe) {
        int probe = get_probe_temp();
        slowcook_cooking_probe_t *cook = slowcook_cooking_probe_get(&ui_manager);
        if (cook) {
            int range = probe_target_temp - cook_start_probe;
            int p = range > 0 ? 3 + (int)((int64_t)(probe - cook_start_probe) * 97 / range) : 3;
            if (p > 100) p = 100; if (p < 3) p = 3;
            lv_bar_set_value(cook->bar_9, p, LV_ANIM_OFF);
            int display = probe > probe_target_temp ? probe_target_temp : probe;
            lv_label_set_text_fmt(cook->temp, "%d℃", display);
            lv_obj_invalidate(lv_scr_act());
        }
        if (probe >= probe_target_temp && probe_target_temp >= 30 && probe >= cook_start_probe && cook_timer) {
            lv_timer_del(cook_timer); cook_timer = NULL;
            g_send.buzzer_req = BUZZER_COOK_DONE; g_send.cook_flag = 0;
            jump_to_slowcook_complete_probe();
        }
        return;
    }
    if (current_group == g_delaycooking) {
        if (g_delay_target > 0 && rtc_now_ms() >= g_delay_target) {
            delay_on = 0;
            if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
            delay_start_cook();
            if (current_group == g_delaycooking) {
                /* 门开被拦截：重建定时器继续检查 */
                cook_timer = lv_timer_create(cooking_timer_cb, 1000, NULL);
            }
        }
        return;
    }
    if (current_group == g_preheat_cooking) {
        preheatcooking_t *cook = preheatcooking_get(&ui_manager);
        if (cook) {
            uint16_t cavity = get_cavity_temp();
            int range = set_temp - preheat_start_cavity;
            int p = range <= 0 ? 100 : 13 + (int)((int64_t)87 * (cavity - preheat_start_cavity) / range);
            if (p < 13) p = 13;
            if (p > 100) p = 100;
            lv_bar_set_value(cook->bar_1, p, LV_ANIM_OFF);
            if (cook->bartemp) {
                int disp = cavity > set_temp ? set_temp : cavity;
                lv_label_set_text_fmt(cook->bartemp, "%d℃", disp);
                int bx = 122 + (637 * p) / 100 - 80;
                lv_obj_set_pos(cook->bartemp, bx, 323);
            }
            if (p >= 100 && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                jump_to_preheat_complete();
            }
        }
        return;
    }
    if (current_group == g_preheat_stop_back) {
        preheat_stop_back_t *back = preheat_stop_back_get(&ui_manager);
        if (back) {
            uint16_t cavity = get_cavity_temp();
            int range = set_temp - preheat_start_cavity;
            int p = range <= 0 ? 100 : 13 + (int)((int64_t)87 * (cavity - preheat_start_cavity) / range);
            if (p < 13) p = 13;
            if (p > 100) p = 100;
            lv_bar_set_value(back->bar_1, p, LV_ANIM_OFF);
            if (back->bartemp) {
                int disp = cavity > set_temp ? set_temp : cavity;
                lv_label_set_text_fmt(back->bartemp, "%d℃", disp);
                int bx = 122 + (637 * p) / 100 - 80;
                lv_obj_set_pos(back->bartemp, bx, 323);
            }
            if (p >= 100 && cook_timer) {
                lv_timer_del(cook_timer);
                cook_timer = NULL;
                g_send.buzzer_req = BUZZER_COOK_DONE;
                jump_to_preheat_complete();
            }
        }
        return;
    }

    lv_obj_t *time_label = NULL;

    if (current_group == g_updown_bbq_setting) {
        updown_bbq_setting_t *set = updown_bbq_setting_get(&ui_manager);
        if (set) time_label = set->time_label;
    } else if (current_group == g_updown_bbq_cooking) {
        updown_bbq_cooking_t *cook = updown_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->time_label;
    } else if (current_group == g_top_bbq_setting) {
        top_bbq_setting_t *set = top_bbq_setting_get(&ui_manager);
        if (set) time_label = set->time_label;
    } else if (current_group == g_color_cookoing) {
        color_cookoing_t *cc = color_cookoing_get(&ui_manager);
        if (cc) time_label = cc->time_label;
    } else if (current_group == g_top_bbq_cooking) {
        top_bbq_cooking_t *cook = top_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->label_80;
    } else if (current_group == g_bottom_bbq_setting) {
        bottom_bbq_setting_t *set = bottom_bbq_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_bottom_bbq_cooking) {
        bottom_bbq_cooking_t *cook = bottom_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_hot_bbq_setting) {
        hot_bbq_setting_t *set = hot_bbq_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_hot_bbq_cooking) {
        hot_bbq_cooking_t *cook = hot_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_hotwind_bbq_setting) {
        hotwind_bbq_setting_t *set = hotwind_bbq_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_hotwind_bbq_cooking) {
        hotwind_bbq_cooking_t *cook = hotwind_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_save_bbq_setting) {
        save_bbq_setting_t *set = save_bbq_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_save_bbq_cooking) {
        save_bbq_cooking_t *cook = save_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_central_bbq_setting) {
        central_bbq_setting_t *set = central_bbq_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_central_bbq_cooking) {
        central_bbq_cooking_t *cook = central_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_air_cooking) {
        air_cooking_t *cook = air_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_air_setting) {
        air_setting_t *set = air_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_pizza_2_cooking) {
        pizza_2_cooking_t *cook = pizza_2_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_pizza_2_setting) {
        pizza_2_setting_t *set = pizza_2_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_slowcook_cooking) {
        slowcook_cooking_t *cook = slowcook_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_slowcook_setting) {
        slowcook_setting_t *set = slowcook_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_unfrozen_cooking) {
        unfrozen_cooking_t *cook = unfrozen_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_unfrozen_setting) {
        unfrozen_setting_t *set = unfrozen_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_rising_cooking) {
        rising_cooking_t *cook = rising_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_rising_setting) {
        rising_setting_t *set = rising_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_corn_cooking) {
        corn_cooking_t *cook = corn_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_corn_setting) {
        corn_setting_t *set = corn_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_heatcontain_cooking) {
        heatcontain_cooking_t *cook = heatcontain_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_heatcontain_setting) {
        heatcontain_setting_t *set = heatcontain_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_lasagna_cooking) {
        lasagna_cooking_t *cook = lasagna_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_lasagna_setting) {
        lasagna_setting_t *set = lasagna_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_strudel_cooking) {
        strudel_cooking_t *cook = strudel_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_strudel_setting) {
        strudel_setting_t *set = strudel_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_bread_cooking) {
        bread_cooking_t *cook = bread_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_bread_setting) {
        bread_setting_t *set = bread_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_pizza3_cooking) {
        pizza3_cooking_t *cook = pizza3_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_pizza3_setting) {
        pizza3_setting_t *set = pizza3_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_chip_cooking) {
        chip_cooking_t *cook = chip_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_chip_setting) {
        chip_setting_t *set = chip_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_custom_cooking) {
        custom_cooking_t *cook = custom_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_custom_setting) {
        custom_setting_t *set = custom_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_wc_cooking) {
        waterclean_cooking_t *cook = waterclean_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_wc_stop) {
        waterclean_stop_t *stop = waterclean_stop_get(&ui_manager);
        if (stop) time_label = stop->timelabel;
    } else if (current_group == g_hcs_cooking) {
        hotcleansave_cooking_t *cook = hotcleansave_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_hcs_stop) {
        hotcleansave_stop_t *stop = hotcleansave_stop_get(&ui_manager);
        if (stop) time_label = stop->timelabel;
    } else if (current_group == g_hcm_cooking) {
        hotcleanmiddle_cooking_t *cook = hotcleanmiddle_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_hcm_stop) {
        hotcleanmiddle_stop_t *stop = hotcleanmiddle_stop_get(&ui_manager);
        if (stop) time_label = stop->timelabel;
    } else if (current_group == g_hch_cooking) {
        hotcleanhigh_cooking_t *cook = hotcleanhigh_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_hch_stop) {
        hotcleanhigh_stop_t *stop = hotcleanhigh_stop_get(&ui_manager);
        if (stop) time_label = stop->timelabel;
    } else if (current_group == g_windchange_bbq_setting) {
        windchange_bbq_setting_t *set = windchange_bbq_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_windchange_bbq_cooking) {
        windchange_bbq_cooking_t *cook = windchange_bbq_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_cookie_cooking) {
        cookie_cooking_t *cook = cookie_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_cookie_setting) {
        cookie_setting_t *set = cookie_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_west_cooking) {
        west_cooking_t *cook = west_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_west_setting) {
        west_setting_t *set = west_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_pizza_cooking) {
        pizza_cooking_t *cook = pizza_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_pizza_setting) {
        pizza_setting_t *set = pizza_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else if (current_group == g_menu_cook_cooking) {
        menu_cooking_t *cook = menu_cooking_get(&ui_manager);
        if (cook) time_label = cook->timelabel;
    } else if (current_group == g_menu_cook_setting) {
        menu_setting_t *set = menu_setting_get(&ui_manager);
        if (set) time_label = set->timelabel;
    } else {
        /* 不在已识别的烹饪/设置/保温链内：可能页面已切走，禁止访问悬空对象 */
        time_label = NULL;
    }

    if (!time_label) return;

    uint32_t elapsed = lv_tick_get() - cook_start_time;
    if (elapsed >= (uint32_t)cook_total_ms) {
        lv_timer_del(cook_timer);
        cook_timer = NULL;
        g_send.buzzer_req = BUZZER_COOK_DONE;
        if (current_group == g_updown_bbq_setting) {
            if (depth > 0) depth--;
            lv_obj_clean(lv_scr_act());
        } else if (current_group == g_top_bbq_setting || current_group == g_top_bbq_cooking) {
            /* top_bbq 计时结束，直接跳 complete */
            int top_depth = depth;
            while (top_depth > 1 && page_stack[top_depth - 1] != PAGE_TOP_BBQ_COOKING &&
                   page_stack[top_depth - 1] != PAGE_TOP_BBQ_SETTING) {
                top_depth--;
            }
            if (top_depth > 1) depth = top_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_top_bbq_complete();
            return;
        } else if (current_group == g_bottom_bbq_setting || current_group == g_bottom_bbq_cooking) {
            int btm_depth = depth;
            while (btm_depth > 1 && page_stack[btm_depth - 1] != PAGE_BOTTOM_BBQ_COOKING &&
                   page_stack[btm_depth - 1] != PAGE_BOTTOM_BBQ_SETTING) {
                btm_depth--;
            }
            if (btm_depth > 1) depth = btm_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_bottom_bbq_complete();
            return;
        } else if (current_group == g_hot_bbq_setting || current_group == g_hot_bbq_cooking) {
            int hot_depth = depth;
            while (hot_depth > 1 && page_stack[hot_depth - 1] != PAGE_HOT_BBQ_COOKING &&
                   page_stack[hot_depth - 1] != PAGE_HOT_BBQ_SETTING) {
                hot_depth--;
            }
            if (hot_depth > 1) depth = hot_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_hot_bbq_complete();
            return;
        } else if (current_group == g_hotwind_bbq_setting || current_group == g_hotwind_bbq_cooking) {
            int hotwind_depth = depth;
            while (hotwind_depth > 1 && page_stack[hotwind_depth - 1] != PAGE_HOTWIND_BBQ_COOKING &&
                   page_stack[hotwind_depth - 1] != PAGE_HOTWIND_BBQ_SETTING) {
                hotwind_depth--;
            }
            if (hotwind_depth > 1) depth = hotwind_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_hotwind_bbq_complete();
            return;
        } else if (current_group == g_save_bbq_setting || current_group == g_save_bbq_cooking) {
            int save_depth = depth;
            while (save_depth > 1 && page_stack[save_depth - 1] != PAGE_SAVE_BBQ_COOKING &&
                   page_stack[save_depth - 1] != PAGE_SAVE_BBQ_SETTING) {
                save_depth--;
            }
            if (save_depth > 1) depth = save_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_save_bbq_complete();
            return;
        } else if (current_group == g_central_bbq_setting || current_group == g_central_bbq_cooking) {
            int central_depth = depth;
            while (central_depth > 1 && page_stack[central_depth - 1] != PAGE_CENTRAL_BBQ_COOKING &&
                   page_stack[central_depth - 1] != PAGE_CENTRAL_BBQ_SETTING) {
                central_depth--;
            }
            if (central_depth > 1) depth = central_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_central_bbq_complete();
            return;
        } else if (current_group == g_windchange_bbq_setting || current_group == g_windchange_bbq_cooking) {
            int windchange_depth = depth;
            while (windchange_depth > 1 && page_stack[windchange_depth - 1] != PAGE_WINDCHANGE_BBQ_COOKING &&
                   page_stack[windchange_depth - 1] != PAGE_WINDCHANGE_BBQ_SETTING) {
                windchange_depth--;
            }
            if (windchange_depth > 1) depth = windchange_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_windchange_bbq_complete();
            return;
        } else if (current_group == g_cookie_cooking || current_group == g_cookie_setting) {
            int c_depth = depth;
            while (c_depth > 1 && page_stack[c_depth - 1] != PAGE_COOKIE_COOKING &&
                   page_stack[c_depth - 1] != PAGE_COOKIE_SETTING) {
                c_depth--;
            }
            if (c_depth > 1) depth = c_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_cookie_complete();
            return;
        } else if (current_group == g_west_cooking || current_group == g_west_setting) {
            int w_depth = depth;
            while (w_depth > 1 && page_stack[w_depth - 1] != PAGE_WEST_COOKING &&
                   page_stack[w_depth - 1] != PAGE_WEST_SETTING) {
                w_depth--;
            }
            if (w_depth > 1) depth = w_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_west_complete();
            return;
        } else if (current_group == g_pizza_cooking || current_group == g_pizza_setting) {
            int p_depth = depth;
            while (p_depth > 1 && page_stack[p_depth - 1] != PAGE_PIZZA_COOKING &&
                   page_stack[p_depth - 1] != PAGE_PIZZA_SETTING) {
                p_depth--;
            }
            if (p_depth > 1) depth = p_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_pizza_complete();
            return;
        } else if (current_group == g_air_cooking || current_group == g_air_setting) {
            int a_depth = depth;
            while (a_depth > 1 && page_stack[a_depth - 1] != PAGE_AIR_COOKING &&
                   page_stack[a_depth - 1] != PAGE_AIR_SETTING) {
                a_depth--;
            }
            if (a_depth > 1) depth = a_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_air_complete();
            return;
        } else if (current_group == g_pizza_2_cooking || current_group == g_pizza_2_setting) {
            int p2_depth = depth;
            while (p2_depth > 1 && page_stack[p2_depth - 1] != PAGE_PIZZA_2_COOKING &&
                   page_stack[p2_depth - 1] != PAGE_PIZZA_2_SETTING) {
                p2_depth--;
            }
            if (p2_depth > 1) depth = p2_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_pizza_2_complete();
            return;
        } else if (current_group == g_slowcook_cooking || current_group == g_slowcook_setting) {
            int sc_depth = depth;
            while (sc_depth > 1 && page_stack[sc_depth - 1] != PAGE_SLOWCOOK_COOKING &&
                   page_stack[sc_depth - 1] != PAGE_SLOWCOOK_SETTING) {
                sc_depth--;
            }
            if (sc_depth > 1) depth = sc_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_slowcook_complete();
            return;
        } else if (current_group == g_unfrozen_cooking || current_group == g_unfrozen_setting) {
            int uf_depth = depth;
            while (uf_depth > 1 && page_stack[uf_depth - 1] != PAGE_UNFROZEN_COOKING &&
                   page_stack[uf_depth - 1] != PAGE_UNFROZEN_SETTING) {
                uf_depth--;
            }
            if (uf_depth > 1) depth = uf_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_unfrozen_complete();
            return;
        } else if (current_group == g_rising_cooking || current_group == g_rising_setting) {
            int rs_depth = depth;
            while (rs_depth > 1 && page_stack[rs_depth - 1] != PAGE_RISING_COOKING &&
                   page_stack[rs_depth - 1] != PAGE_RISING_SETTING) {
                rs_depth--;
            }
            if (rs_depth > 1) depth = rs_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_rising_complete();
            return;
        } else if (current_group == g_corn_cooking || current_group == g_corn_setting) {
            int cn_depth = depth;
            while (cn_depth > 1 && page_stack[cn_depth - 1] != PAGE_CORN_COOKING &&
                   page_stack[cn_depth - 1] != PAGE_CORN_SETTING) {
                cn_depth--;
            }
            if (cn_depth > 1) depth = cn_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_corn_complete();
            return;
        } else if (current_group == g_heatcontain_cooking || current_group == g_heatcontain_setting) {
            int hc_depth = depth;
            while (hc_depth > 1 && page_stack[hc_depth - 1] != PAGE_HEATCONTAIN_COOKING &&
                   page_stack[hc_depth - 1] != PAGE_HEATCONTAIN_SETTING) {
                hc_depth--;
            }
            if (hc_depth > 1) depth = hc_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_heatcontain_complete();
            return;
        } else if (current_group == g_lasagna_cooking || current_group == g_lasagna_setting) {
            int ls_depth = depth;
            while (ls_depth > 1 && page_stack[ls_depth - 1] != PAGE_LASAGNA_COOKING &&
                   page_stack[ls_depth - 1] != PAGE_LASAGNA_SETTING) {
                ls_depth--;
            }
            if (ls_depth > 1) depth = ls_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_lasagna_complete();
            return;
        } else if (current_group == g_strudel_cooking || current_group == g_strudel_setting) {
            int st_depth = depth;
            while (st_depth > 1 && page_stack[st_depth - 1] != PAGE_STRUDEL_COOKING &&
                   page_stack[st_depth - 1] != PAGE_STRUDEL_SETTING) {
                st_depth--;
            }
            if (st_depth > 1) depth = st_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_strudel_complete();
            return;
        } else if (current_group == g_bread_cooking || current_group == g_bread_setting) {
            int br_depth = depth;
            while (br_depth > 1 && page_stack[br_depth - 1] != PAGE_BREAD_COOKING &&
                   page_stack[br_depth - 1] != PAGE_BREAD_SETTING) {
                br_depth--;
            }
            if (br_depth > 1) depth = br_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_bread_complete();
            return;
        } else if (current_group == g_pizza3_cooking || current_group == g_pizza3_setting) {
            int p3_depth = depth;
            while (p3_depth > 1 && page_stack[p3_depth - 1] != PAGE_PIZZA3_COOKING &&
                   page_stack[p3_depth - 1] != PAGE_PIZZA3_SETTING) {
                p3_depth--;
            }
            if (p3_depth > 1) depth = p3_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_pizza3_complete();
            return;
        } else if (current_group == g_chip_cooking || current_group == g_chip_setting) {
            int ch_depth = depth;
            while (ch_depth > 1 && page_stack[ch_depth - 1] != PAGE_CHIP_COOKING &&
                   page_stack[ch_depth - 1] != PAGE_CHIP_SETTING) {
                ch_depth--;
            }
            if (ch_depth > 1) depth = ch_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_chip_complete();
            return;
        } else if (current_group == g_custom_cooking || current_group == g_custom_setting) {
            int cu_depth = depth;
            while (cu_depth > 1 && page_stack[cu_depth - 1] != PAGE_CUSTOM_COOKING &&
                   page_stack[cu_depth - 1] != PAGE_CUSTOM_SETTING) {
                cu_depth--;
            }
            if (cu_depth > 1) depth = cu_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_custom_complete();
            return;
        } else if (current_group == g_wc_cooking) {
            int wc_depth = depth;
            while (wc_depth > 1 && page_stack[wc_depth - 1] != PAGE_WATER_CLEAN_COOKING) {
                wc_depth--;
            }
            if (wc_depth > 1) depth = wc_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_wc_complete();
            return;
        } else if (current_group == g_hcs_cooking) {
            int hcs_depth = depth;
            while (hcs_depth > 1 && page_stack[hcs_depth - 1] != PAGE_HOTCLEANSAVE_COOKING) {
                hcs_depth--;
            }
            if (hcs_depth > 1) depth = hcs_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_hcs_cooling();
            return;
        } else if (current_group == g_hcm_cooking) {
            int hcm_depth = depth;
            while (hcm_depth > 1 && page_stack[hcm_depth - 1] != PAGE_HOTCLEANMIDDLE_COOKING) {
                hcm_depth--;
            }
            if (hcm_depth > 1) depth = hcm_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_hcm_cooling();
            return;
        } else if (current_group == g_hch_cooking) {
            int hch_depth = depth;
            while (hch_depth > 1 && page_stack[hch_depth - 1] != PAGE_HOTCLEANHIGH_COOKING) {
                hch_depth--;
            }
            if (hch_depth > 1) depth = hch_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_hch_cooling();
            return;
        } else if (current_group == g_menu_cook_cooking || current_group == g_menu_cook_setting) {
            int m_depth = depth;
            while (m_depth > 1 && page_stack[m_depth - 1] != PAGE_MENU_COOK_COOKING &&
                   page_stack[m_depth - 1] != PAGE_MENU_COOK_SETTING) {
                m_depth--;
            }
            if (m_depth > 1) depth = m_depth;
            lv_obj_clean(lv_scr_act());
            jump_to_menu_complete();
            return;
        }
        if (current_group == g_color_cookoing) {
            jump_to_color_complete();
        } else if (current_group == g_updown_bbq_cooking ||
                   current_group == g_updown_bbq_setting) {
            jump_to_updown_bbq_complete();
        } else {
            /* 未识别页面：安全返回，不强制跳转（防止在错误页面跳转/破坏页面栈） */
            return;
        }
        return;
    }

    /* 时间显示（半秒四舍五入） */
    int elapsed_sec = (elapsed + 500) / 1000;
    int total_sec = cook_total_ms / 1000;
    int remaining_sec = total_sec - elapsed_sec;
    if (remaining_sec < 0) remaining_sec = 0;
    int h = remaining_sec / 3600;
    int m = (remaining_sec % 3600) / 60;
    int s = remaining_sec % 60;
    lv_label_set_text_fmt(time_label, "%02d:%02d:%02d", h, m, s);
    g_send.remaining_ms = remaining_sec * 1000;
}

