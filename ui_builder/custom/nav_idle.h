/*
 * nav_idle.h - 空闲策略(无操作自动回页/待机/关机)对外接口
 *
 * 时间基准：lv_tick(SDK 移植层硬件节拍驱动,单调递增)。
 * 墙钟(RTC)只管"时刻"语义(时钟显示/预约)；时长一律 lv_tick——
 * 墙钟可被对表跳变，做时长差值会跳变。
 */
#ifndef NAV_IDLE_H
#define NAV_IDLE_H

#include "nav.h"

void nav_idle_touch(void);      /* 用户活动打点:nav_handle_key 每个实际按键调用 */
void nav_idle_init(void);       /* 创建常驻 1s 空闲调度定时器(nav_init 调用) */
void nav_goto_major_menu(void); /* 栈重置为 [WAITMENU_24, MAJOR_MENU(_TZ)] 并重建主菜单
                                   ——返回链终点统一入口(主菜单按 BACK 不再回待机页) */
void nav_enter_standby(void);   /* 进待机页(waitmenu + IFACE_STANDBY),空闲 5 分钟等共用 */
void nav_power_off(void);       /* 关机(清理运行状态+IFACE_SLEEP+暗屏待机),
                                   KEY1 长按与待机 20 分钟超时共用 */

#endif /*NAV_IDLE_H*/
