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

/* 页面重建后调用（page_push/page_pop 末尾 + F8 语言切换时）
 * 中文模式直接返回；英文模式：树遍历翻译+切字体 → 调当前页排版函数 */
void lang_on_page_built(void);

/* 遍历当前屏所有 label：文本命中翻译表 → 替换为英文；taiwanpearl 字体 → aktivgrotesk
 * 由 lang_on_page_built 调用；也可单独调用（如动态子界面重建后） */
void lang_refresh_screen(void);

#endif
