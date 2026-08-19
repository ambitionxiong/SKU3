#ifndef I18N_H
#define I18N_H

#include <stdint.h>

/* ==============================
 * 国际化 (i18n)
 * g_lang_en 独立英文标志位: 0=中文 1=英文(由模拟器按键/外部控制)
 * 英文模式: 文本查表替换为英文, 字体切换为 AktivGroteskMedium
 * ============================== */

extern uint8_t g_lang_en;
int  is_english(void);                 /* g_lang_en == 1 */
const char *tr(const char *zh);        /* 中文→当前语言文本 */

/* 各页面语言应用(设置文本+字体) */
void cookmenu_apply_lang(void);
void major_menu_apply_lang(void);

#endif
