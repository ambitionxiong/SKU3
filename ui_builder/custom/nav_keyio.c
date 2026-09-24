/*
 * nav_keyio.c - 按键输入状态机 + 长按
 *
 * 职责：
 *   1. nav_handle_key：按键状态机（KEY_IDLE/KEY_PRESSED），
 *      首按触发 process_key，编码器长按按 ENC_REPEAT_MS 重复触发，
 *      松开回空闲。由底层按键回调(硬件/模拟器)逐次调用。
 *   2. KEY1 电源键:按住 0.25s 触发电源动作——SLEEP 开机/运行中弹关机确认/其余
 *      直接关机(nav_key1_power_action);松开沿无动作,快触(<0.25s)不响应。
 *      3s 复位重启由电源板自带逻辑直接断电,显示侧不复位。
 *   3. nav_key1_hold_check：供外部周期查询 KEY1 是否已按住满 0.25s(模拟器轮询)。
 *
 * 状态变量(key_state/active_key/active_key_time)定义在 nav_key.c。
 */

#include "nav.h"
#include "nav_idle.h"
#include "nav_internal.h"

#define KEY1_HOLD_MS 250   /* 电源键动作阈值:按住满 0.25s 触发(电源板 3s 复位自行处理) */

static uint8_t s_key1_fired = 0;   /* 本次按压电源动作已触发:按住不重复触发,松开沿无动作 */

/* 关机：清全部运行状态 → SLEEP 待机暗屏。
   KEY1 电源动作与待机页 20 分钟无操作超时(nav_idle)共用，行为完全一致 */
void nav_power_off(void)
{
    nav_hint_tone_cancel();           /* 提示音重复引擎随关机停止 */
    probetip_cancel_auto_dismiss();   /* 取消陈旧的探针提示自动关闭定时器,防止跨会话误触发 */
    screen_set_reset();               /* 覆盖层若打开:清理对象/组/焦点指针,防悬空 */
    count_down_poweroff_reset();      /* 计时器后台/超时状态一并清:防关机后到期自动退出拽屏 */
    if (nav_favask_active()) nav_favask_cancel();   /* 收藏确认弹层随关机收起(标志残留会吞键) */
    nav_poweroff_ask_cancel();        /* 关机确认弹层同款卫生 */
    if (cook_timer) { lv_timer_del(cook_timer); cook_timer = NULL; }
    g_on_stop_back = 0;
    g_complete_to_stop_back = 0;
    g_cooling_to_stop_back = 0;
    g_extra_color_to_stop_back = 0;
    g_keepwarm_active = 0;
    g_keepwarm_sec = 0;
    cook_is_color = 0;
    g_stop_back_complete = NULL;
    g_delay_cancel_btn = 0;
    cook_elapsed_saved = 0; cook_bar_saved = 0;
    delay_on = 0; preheat_on = 0; contain_on = 0;
    delay_hour = 0; delay_min = 0;
    g_delay_target = -1;
    g_somecook_running = 0;
    g_somecook_run_idx = 0;
    six_cook_reset();   /* 六感运行:清理状态(定时器已由上面 cook_timer 删除覆盖) */
    set_temp = 180; set_temp_up = 180; set_temp_down = 180;
    g_send.cook_mode = MODE_NONE;
    g_send.set_temp = 0;
    g_send.set_temp_lower = 0;
    g_send.remaining_ms = -1;

    if (nav_childlock_active()) nav_childlock_set(0);   /* 关机不显示锁层(Set_Lock 保持 1,开机恢复) */
    g_send.buzzer_req = BUZZER_POWER_OFF;
    g_send.iface_status = IFACE_SLEEP;
    depth = 0;
    page_push(PAGE_WAITMENU_24);
    lv_obj_clean(lv_scr_act());
    waitmenu_24_create(&ui_manager);
    waitmenu_clock_cache_reset();   /* 强制刷新为真实时间 */
    current_group = NULL;
    lang_scr_load_anim(waitmenu_24_get(&ui_manager)->obj,
                     LV_SCR_LOAD_ANIM_NONE, 0, 0,
                     ui_manager.auto_del);   /* 统一出口:英文模式关机待机页翻译+排版 */
    waitmenu_apply_clock();   /* 立即刷新为真实时间 */
#ifndef LV_USE_AIC_SIMULATOR
    backlight_set_level(nav_standby_backlight_level());   /* 待机显示:开=正常/关=黑屏/夜间=最低 */
#endif
    printf("[KEY] power off -> SLEEP (dim waitmenu)\n");
}

/* 待机显示亮度等级:Set_StandbyTime 0开/1关/2夜间(18:00-6:00 最低,其余正常)。
   关=0 黑屏;开/夜间白天=设置亮度档(Set_Brightness 0~7,档位表与 nav_loudness.c
   亮度数值条同一映射);夜间 18-6=最低档 lvl[0] */
static int standby_night_now(void)
{
    rtc_time_t t;
    if (rtc_get_time(&t) != 0) return 0;
    return (t.hour >= 18 || t.hour < 6);
}

int nav_standby_backlight_level(void)
{
    static const int lvl[8] = {12, 25, 37, 50, 62, 75, 87, 100};   /* 与 nav_loudness.c 亮度条同表 */
    int v = SET_Data.Set_Brightness;
    if (v < 0 || v > 7) v = 7;
    if (SET_Data.Set_StandbyTime == 1) return 0;              /* 关:黑屏 */
    if (SET_Data.Set_StandbyTime == 2)                        /* 夜间模式 */
        return standby_night_now() ? lvl[0] : lvl[v];
    return lvl[v];                                            /* 开:设置亮度档 */
}

#ifndef LV_USE_AIC_SIMULATOR
/* 延 50ms(≥3 个 vsync)抬满背光:refr_now 完成渲染+翻转入队,但面板出新帧要等
   vsync——立刻抬亮会先照亮面板上的待机页旧帧(闪一下待机页才出菜单) */
static void wake_backlight_timer_cb(lv_timer_t *t)
{
    lv_timer_del(t);
    backlight_set_level(100);
}

void nav_backlight_100_defer(void)
{
    lv_timer_create(wake_backlight_timer_cb, 50, NULL);
}
#endif

/* 开机(SLEEP 唤醒)：回日期时间页/主菜单/探针主菜单/首设语言页(原 KEY1 长按唤醒分支原样迁移)。
   不重复清理——SLEEP 进入前已清理，SLEEP 期间按键全部被吞，状态保持干净 */
static void nav_power_on(void)
{
    depth = 0;
    page_push(PAGE_WAITMENU_24);
    if (!g_langpick_done) {
        /* 首设未完成:开机一律回语言设置页,不得进主菜单(设置链封闭,
         * 与上电 nav_init !g_langpick_done 分支同语义) */
        langpick_enter_from_standby();
    } else if (g_systime_boot_mode) {
        /* 断电重启后时间还没设完就关过机:唤醒一律回日期时间页补设,
         * 设完(OK)才放行进系统 */
        systime_enter_boot_mode();
    } else if (is_probe_inserted()) {
        jump_to_major_menu_tz();
    } else {
        page_push(PAGE_MAJOR_MENU);
        lv_obj_clean(lv_scr_act());
        major_menu_create(&ui_manager);
        groups_create();
        bind_events();
        current_group = g_major_menu;
        lang_scr_load_anim(major_menu_get(&ui_manager)->obj,
                         LV_SCR_LOAD_ANIM_NONE, 0, 0, 0);
    }
    if (SET_Data.Set_Lock && !nav_childlock_active()) nav_childlock_set(1);   /* 关机前童锁开着:开机恢复锁层 */
    g_send.buzzer_req = BUZZER_POWER_ON;
    g_send.iface_status = IFACE_SETTING;
#ifndef LV_USE_AIC_SIMULATOR
    /* 先把新页整帧刷到面板、等翻转落屏后再抬背光(顺序:刷帧→延→抬),
     * 避免抬亮照亮的是待机页旧帧 */
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
    nav_backlight_100_defer();
#endif
    printf("[KEY] power on -> %s\n", !g_langpick_done ? "langpick" :
           (g_systime_boot_mode ? "systime_boot" : "major_menu"));
}

/* KEY1 按住 0.25s 触发的电源动作：
   SLEEP=开机;运行中(烹饪/暂停/预约/保温中)=弹"结束当前任务并关机"确认;
   完成页(纯完成/完成倒计时)与其余(待机/菜单/设置/首设两页/警报页等)=直接关机 */
static void nav_key1_power_action(void)
{
    if (g_send.iface_status == IFACE_SLEEP) {
        nav_power_on();
    } else if (nav_poweroff_ask_needed()) {
        g_send.buzzer_req = BUZZER_KEY_VALID;
        nav_poweroff_ask_show();
    } else {
        nav_power_off();
    }
    uart_print();
}

/* KEY1 按住满 0.25s：触发电源动作(上面 nav_key1_power_action),每次按压只触发
 * 一次。复位重启不由显示侧处理——电源板自带 3s 复位逻辑直接断电重启,重启后
 * 属冷启,按首设标志走开机日期时间页/完整首设 */
void nav_key1_hold_trigger(void)
{
    s_key1_fired = 1;   /* 本次按压已消费:按住不重复触发,松开沿无动作 */
    printf("[KEY] KEY1 hold %dms -> power action\n", KEY1_HOLD_MS);
    nav_key1_power_action();
}
/* 供外部周期调用(模拟器 sim_scan_cb 100ms)：KEY1 按住已满 0.25s 则触发电源动作并返回 1 */
uint8_t nav_key1_hold_check(void)
{
    if (active_key == KEY1 && key_state == KEY_PRESSED && !s_key1_fired) {
        uint32_t interval = lv_tick_get() - active_key_time;
        if (interval >= KEY1_HOLD_MS) {
            active_key_time = lv_tick_get();
            nav_key1_hold_trigger();
            return 1;
        }
    }
    return 0;
}
/* 供外部周期调用(模拟器 sim_scan_cb 100ms)：童锁激活时旋钮按住满 3s 解锁。
   真机电平制喂键走 nav_handle_key KEY_PRESSED 分支同款判断,此轮询兜模拟器边缘喂数 */
void nav_childlock_hold_poll(void)
{
    if (active_key == KEY_ENCODER_PRESS && key_state == KEY_PRESSED &&
        nav_childlock_active() && nav_childlock_hold_armed()) {
        if (lv_tick_get() - active_key_time >= 3000) {
            active_key_time = lv_tick_get();   /* 防轮询周期内重复触发 */
            nav_childlock_try_unlock();
        }
    }
}
/* 按键状态机：KEY_IDLE 首按→记键值+调 process_key；
   KEY_PRESSED 按住→编码器按 50ms 重复、KEY1 按住 0.25s 触发电源动作；
   松开回 KEY_IDLE(KEY1 松开沿无动作)。 */
void nav_handle_key(uint8_t key)
{
    uint32_t now = lv_tick_get();

    if (key != 0) {
        nav_idle_touch();          /* 任何实际按键都算用户活动(含被模态吞掉的) */
        nav_hint_tone_cancel();    /* 协议 3.1:用户操作取消后续提示音(按键/旋钮) */
    }

    switch (key_state) {
    case KEY_IDLE:
        if (key != 0) {
#ifdef LV_USE_AIC_SIMULATOR
            static const char *kn[] = {
                [1]="KEY1", [3]="MENU", [5]="COLOR", [21]="BACK",
                [31]="CW", [41]="CCW", [51]="PRESS"
            };
            printf("[KEY] %s (%d)\n", key<=51&&kn[key]?kn[key]:"?", key);
#endif
            active_key = key;
            active_key_time = now;
            key_state = KEY_PRESSED;
            if (key == KEY1) {
                /* 电源键不进 process_key 分发(按下沿不响、不受守卫拦截):
                 * 电源动作挂按住 0.25s(同键分支/hold_check 触发),松开沿无动作 */
                s_key1_fired = 0;   /* 新按压:清上次已触发标志 */
                uart_data_receive[Receive_data_Touch_Key] = 0;   /* 消费键值(同 process_key 尾惯例) */
            } else {
                process_key(key);
            }
        }
        break;

    case KEY_PRESSED:
        if (key == 0) {
            // 松开 → 回到空闲(KEY1 松开沿无动作:电源动作已在按住 0.25s 时触发)
#ifdef LV_USE_AIC_SIMULATOR
            printf("[KEY] release\n");
#endif
            key_state = KEY_IDLE;
            active_key = 0;
        } else if (key == active_key) {
            // 同键按住
            uint32_t interval = now - active_key_time;
            int is_encoder = (key == KEY_ENCODER_CW || key == KEY_ENCODER_CCW);

            if (is_encoder && interval >= ENC_REPEAT_MS) {
                active_key_time = now;
                process_key(key);
            }
            if (active_key == KEY1 && !s_key1_fired && interval >= KEY1_HOLD_MS) {
                active_key_time = now;
                nav_key1_hold_trigger();
            }
    /* 童锁:旋钮按住 3s 解锁(真机路径;模拟器由 nav_childlock_hold_poll 轮询)。
       armed 门:上锁那一下的按住不算,须先松开一次(与圆环计段一致) */
    if (active_key == KEY_ENCODER_PRESS && nav_childlock_active() &&
        nav_childlock_hold_armed() && interval >= 3000) {
                active_key_time = now;
                nav_childlock_try_unlock();
            }
            // 触控键按住不重复（只有 KEY_IDLE 后的第一次触发）
        } else {
            // 键值变化（如编码器方向切换）
            active_key = key;
            active_key_time = now;
            if (key == KEY1) {
                s_key1_fired = 0;
                uart_data_receive[Receive_data_Touch_Key] = 0;
            } else {
                process_key(key);
            }
        }
        break;
    }
}

