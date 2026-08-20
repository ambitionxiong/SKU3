/*
 * nav.c - 核心控制器壳
 *
 * 代码已拆分到:
 *   nav_internal.h   - 跨文件共享符号
 *   nav_core.c       - 全局变量 + 编辑字段 + 公共 helper + page_push + groups_create
 *   nav_pop.c        - page_pop (页面栈弹出/重建)
 *   nav_jump.c       - 页面跳转(主菜单/烹调/烧烤/额外上色)
 *   nav_key.c        - 按键处理(process_key + 白名单)
 *   nav_keyio.c      - 按键输入状态机 + 长按
 *   nav_events.c     - 事件绑定 + 菜单/延迟/预约回调
 *   nav_cooktimer.c  - cooking_timer_cb + 门开自动暂停
 *   nav_stop.c       - 暂停/恢复 + 额外上色暂停/恢复
 *   nav_settings.c   - Cooking 设置页
 *   nav_system.c     - 系统定时器 + 初始化 + topflag 时钟
 */
#include "nav.h"
#include "nav_internal.h"
