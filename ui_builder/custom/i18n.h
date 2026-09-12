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
int  is_trad(void);                    /* 繁體模式(SET_Data.Set_Language == 1) */
const char *tr(const char *zh);        /* 中文→当前语言文本 */

/* 繁体转换(i18n_tw.c 自动生成): 简→繁 词组最长匹配+字映射, 环形缓冲返回 */
const char *tr_tw(const char *zh);
void tw_convert_into(char *dst, int dst_len, const char *src);   /* 拷贝并转换到指定缓冲 */

/* 各页面语言应用(设置文本+字体) */
void cookmenu_apply_lang(void);
void major_menu_apply_lang(void);

#endif
