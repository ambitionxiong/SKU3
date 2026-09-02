#ifndef NAV_LANG_H
#define NAV_LANG_H

#include "nav.h"

/* =====================================================================
 * 英文排版调整层（nav_lang.c）
 * 职责：
 *   1. 页面重建后（英文模式）自动翻译静态标签文本 + 切换 aktivgrotesk 字体
 *   2. 每页可注册一个「排版微调函数」——同事在此手动调整英文版
 *      坐标/字号/文本覆盖（见 docs/I18N_GUIDE.md）
 * 中文模式：全部早退，零开销零影响。
 * ===================================================================== */

/* 页面重建后调用（所有 lv_scr_load_anim 出口 + 语言切换时）
 * 中文模式直接返回；英文模式：树遍历翻译+切字体 → 调当前页排版函数 */
void lang_on_page_built(void);

/* lv_scr_load_anim 包装：显示新页面后自动触发 lang_on_page_built
 * （custom 代码统一调用本函数替代原 LVGL 函数） */
void lang_scr_load_anim(lv_obj_t *scr, lv_scr_load_anim_t anim_type,
                        uint32_t time, uint32_t delay, bool auto_del);

/* 遍历当前屏所有 label：文本命中翻译表 → 替换为英文；taiwanpearl 字体 → aktivgrotesk
 * 由 lang_on_page_built 调用；也可单独调用（如动态子界面重建后） */
void lang_refresh_screen(void);

/* 当前页定时器重写对象（bartemp 等）的整体平移偏移（注册表 dx/dy，同事编辑）
 * 中文模式或未注册页返回 0——业务代码动态 set_pos 公式加此偏移 */
int lang_dyn_dx(void);
int lang_dyn_dy(void);

/* topflag 顶层提示层摆位:tip1 高自适应(EN 三行/CN 单行),常驻 layer_top
 * 不进 tune 注册表,由 nav_hint 显示提示时调用 */
void topflagpage_lang_tune(void);

#endif
