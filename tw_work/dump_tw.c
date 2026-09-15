/* 宿主工具: #include 真实 i18n_tw.c, 用转换器本体逐条转换语料 (勿手改) */
#include <stdio.h>
#include <string.h>
#include "i18n_tw.c"
#include "tw_keys_gen.c"
/* 链接桩: 本工具只用 tw_convert_into, 不调 tr_tw; 表数据在固件的 i18n_tw_table.c */
/* tw_ent_t typedef 已随 i18n_tw.c 进来 */
const int s_tw_table_n = 0;
const tw_ent_t s_tw_table[1] = { { 0, 0 } };
static char buf[4096];
int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--check") == 0) {
        /* 幂等回检: stdin 每行一个 hex 串, 转换后输出 hex。繁体应不变, 含简体字的会变 */
        static char line[8192], in[4096];
        while (fgets(line, (int)sizeof line, stdin)) {
            int len = (int)strlen(line), bl = 0, k;
            while (len && (line[len - 1] == '\n' || line[len - 1] == '\r'))
                line[--len] = 0;
            for (k = 0; k + 1 < len; k += 2) {
                unsigned v = 0;
                if (sscanf(line + k, "%2x", &v) == 1)
                    in[bl++] = (char)v;
            }
            in[bl] = 0;
            tw_convert_into(buf, (int)sizeof buf, in);
            for (const unsigned char *p = (const unsigned char *)buf; *p; p++)
                printf("%02x", *p);
            printf("\n");
        }
        return 0;
    }
    for (int i = 0; i < tw_keys_n; i++) {
        tw_convert_into(buf, (int)sizeof buf, tw_keys[i]);
        printf("%d\t", i);
        for (const unsigned char *p = (const unsigned char *)buf; *p; p++)
            printf("%02x", *p);
        printf("\n");
    }
    return 0;
}
