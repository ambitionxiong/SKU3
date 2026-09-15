#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gen_tw_table.py — 为 i18n_tw.c 生成 s_tw_table 繁体查表（查表模式的数据源）

用法:  cd tw_work && python3 gen_tw_table.py [中英繁对照表.xlsx]

流程:
  1. 解析 ui_builder/custom/i18n.c 的 s_table 全部 zh key（注释/转义感知扫描）
  2. 语料 = s_table key ∪ 多行 key 拆行 ∪ custom 层与生成层 CJK 字符串字面量（查表全集）
  3. 生成 tw_keys_gen.c；宿主 dump_tw.c #include 真实 i18n_tw.c，用转换器本体逐条转换
  4. 读中英繁对照表 xlsx：仅"zh 精确命中且繁体非空"的行覆盖转换输出（人工值优先）
  5. 字形检查：列出覆盖引入的、转换器输出之外的新字符（需上位机补字形的候选）
  6. 在 i18n_tw.c 的 BEGIN/END 标记间幂等拼接 s_tw_table，输出 report.md
"""
import glob
import os
import re
import shutil
import subprocess
import sys

HERE   = os.path.dirname(os.path.abspath(__file__))
ROOT   = os.path.dirname(HERE)
UIB    = os.path.join(ROOT, 'ui_builder')
CUSTOM = os.path.join(UIB, 'custom')
TW_C   = os.path.join(CUSTOM, 'i18n_tw.c')
TW_TABLE_C = os.path.join(CUSTOM, 'i18n_tw_table.c')
I18N_C = os.path.join(CUSTOM, 'i18n.c')
XLSX_A = sys.argv[1] if len(sys.argv) > 1 else \
    '/mnt/d/My_STM32_Project/sku3_最新资料/SKU123456中英繁对照表2026-5-9.xlsx'

BEGIN_MK = '/* ==== [gen] s_tw_table 自动生成 BEGIN (tw_work/gen_tw_table.py, 勿手改) ==== */'
END_MK   = '/* ==== [gen] s_tw_table 自动生成 END ==== */'

# ---------------- C 源码扫描（注释/字符串/字符字面量感知） ----------------

def scan_events(text):
    """产出 ('str', start, end, raw) / ('lb',) / ('rb',) 事件，跳过注释与字符字面量。"""
    ev, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if c == '"':
            j = i + 1
            while j < n:
                if text[j] == '\\':
                    j += 2
                    continue
                if text[j] == '"':
                    j += 1
                    break
                j += 1
            ev.append(('str', i, j, text[i:j]))
            i = j
            continue
        if c == "'":
            j = i + 1
            while j < n:
                if text[j] == '\\':
                    j += 2
                    continue
                if text[j] == "'":
                    j += 1
                    break
                j += 1
            i = j
            continue
        if c == '/' and i + 1 < n and text[i + 1] == '*':
            k = text.find('*/', i + 2)
            i = n if k < 0 else k + 2
            continue
        if c == '/' and i + 1 < n and text[i + 1] == '/':
            k = text.find('\n', i)
            i = n if k < 0 else k
            continue
        if c == '{':
            ev.append(('lb', i, i + 1, ''))
        elif c == '}':
            ev.append(('rb', i, i + 1, ''))
        i += 1
    return ev

def merge_adjacent(ev, text):
    """相邻且中间仅空白的字符串字面量按 C 规则拼接。"""
    out = []
    for e in ev:
        if (e[0] == 'str' and out and out[-1][0] == 'str'
                and not text[out[-1][2]:e[1]].strip()):
            p = out[-1]
            out[-1] = ('str', p[1], e[2], p[3][:-1] + e[3][1:])
        else:
            out.append(e)
    return out

ESC = {'n': '\n', 't': '\t', 'r': '\r', '"': '"', "'": "'",
       '\\': '\\', '0': '\0', 'a': '\a', 'b': '\b', 'f': '\f', 'v': '\v'}

def unescape(raw):
    s, out, i = raw[1:-1], [], 0
    while i < len(s):
        c = s[i]
        if c == '\\' and i + 1 < len(s):
            d = s[i + 1]
            if d in ESC:
                out.append(ESC[d])
                i += 2
                continue
            if d == 'x':
                j = i + 2
                while j < len(s) and j < i + 4 and s[j] in '0123456789abcdefABCDEF':
                    j += 1
                if j > i + 2:
                    out.append(chr(int(s[i + 2:j], 16)))
                    i = j
                    continue
            out.append(d)
            i += 2
            continue
        out.append(c)
        i += 1
    return ''.join(out)

def c_escape(s):
    out = []
    for c in s:
        if c == '\\':
            out.append('\\\\')
        elif c == '"':
            out.append('\\"')
        elif c == '\n':
            out.append('\\n')
        elif c == '\r':
            out.append('\\r')
        elif c == '\t':
            out.append('\\t')
        else:
            out.append(c)
    return ''.join(out)

def parse_s_table(path):
    """返回 s_table 条目列表 [[zh, en, ...], ...]。"""
    text = open(path, encoding='utf-8').read()
    m = re.search(r'static\s+const\s+i18n_t\s+s_table\s*\[\s*\]\s*=\s*\{', text)
    if not m:
        raise SystemExit('i18n.c 里找不到 s_table 定义')
    seg = text[m.end() - 1:]
    ev = merge_adjacent(scan_events(seg), seg)
    entries, cur, depth = [], None, 0
    for e in ev:
        if e[0] == 'lb':
            depth += 1
            if depth == 2:
                cur = []
                entries.append(cur)
        elif e[0] == 'rb':
            depth -= 1
            if depth == 0:
                break
        elif e[0] == 'str' and depth == 2:
            cur.append(unescape(e[3]))
    return entries

def file_strings(path):
    text = open(path, encoding='utf-8').read()
    ev = merge_adjacent(scan_events(text), text)
    return [unescape(e[3]) for e in ev if e[0] == 'str']

def has_cjk(s):
    return any(0x3400 <= ord(c) <= 0x9FFF for c in s)

# ---------------- 1/2. 语料 ----------------

corpus, tag = {}, {}

def add(s, src):
    if s and s not in corpus and has_cjk(s):
        corpus[s] = None
        tag[s] = src
        if '\n' in s:
            for line in s.split('\n'):
                add(line, src + '|line')

entries = parse_s_table(I18N_C)
for g in entries:
    add(g[0], 's_table')
n_st = len(corpus)

for f in sorted(glob.glob(os.path.join(CUSTOM, '*.c')) + glob.glob(os.path.join(CUSTOM, '*.h'))):
    # i18n.c=英文表(en列非文案) i18n_tw.c=旧转换数据(十六进制转义) i18n_tw_table.c=本脚本产物(防自反馈污染)
    if os.path.basename(f) in ('i18n.c', 'i18n_tw.c', 'i18n_tw_table.c'):
        continue
    for s in file_strings(f):
        add(s, 'custom:' + os.path.basename(f))
n_custom = len(corpus) - n_st

for f in sorted(glob.glob(os.path.join(UIB, '*.c'))):
    # textpage.c = AiUIBuilder 字库预览孤儿页(textpage_create 无人调用),
    # 其 label 文本是各档字库的生成字符集, 不是 UI 文案, 不入表
    if os.path.basename(f) == 'textpage.c':
        continue
    for s in file_strings(f):
        add(s, 'gen:' + os.path.basename(f))
n_gen = len(corpus) - n_st - n_custom

keys = list(corpus)
print('[corpus] s_table key=%d, custom 新增=%d, 生成层新增=%d, 总语料=%d'
      % (n_st, n_custom, n_gen, len(keys)))

# ---------------- 3. 转换器本体生成 ----------------

with open(os.path.join(HERE, 'tw_keys_gen.c'), 'w', encoding='utf-8', newline='\n') as f:
    f.write('/* 自动生成: tw_work/gen_tw_table.py 语料清单, 勿手改 */\n')
    f.write('const char *tw_keys[] = {\n')
    for s in keys:
        f.write('"%s",\n' % c_escape(s))
    f.write('};\nconst int tw_keys_n = (int)(sizeof(tw_keys) / sizeof(tw_keys[0]));\n')

with open(os.path.join(HERE, 'dump_tw.c'), 'w', encoding='utf-8', newline='\n') as f:
    f.write(r'''/* 宿主工具: #include 真实 i18n_tw.c, 用转换器本体逐条转换语料 (勿手改) */
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
''')

subprocess.run(['gcc', '-O1', '-w', '-o', 'dump_tw', 'dump_tw.c',
                '-I', os.path.join('..', 'ui_builder', 'custom')],
               cwd=HERE, check=True)
out = subprocess.run(['./dump_tw'], cwd=HERE, capture_output=True,
                     text=True, check=True).stdout

conv_map, truncated = {}, []
for line in out.splitlines():
    idx, _, hx = line.partition('\t')
    b = bytes.fromhex(hx)
    if len(b) >= 4000:
        truncated.append(keys[int(idx)])
    conv_map[keys[int(idx)]] = b.decode('utf-8')
if truncated:
    print('[warn] 转换截断(>4KB):', len(truncated), '条')
assert len(conv_map) == len(keys), 'dump 输出行数与语料不一致'
pre_chars = set(''.join(conv_map.values()))
print('[convert] 转换完成, 覆盖前输出字符集=%d 字' % len(pre_chars))

# ---------------- 4. xlsx 人工值覆盖 ----------------
# 该 xlsx 单元格是"简体\nEnglish\n繁體"三行合一且有多词条连排/缺行/错位,
# 因此只采纳恰好 3 行的规整单元格, 且繁体侧必须含 CJK; 之后还要过转换器幂等回检。

# 人工复核否决的 xlsx 行: zh → 原因 (保留转换器输出)
DENY_OVERLAY = {
    '预约中…': 'xlsx 疑似数据错位: 预约状态被写成预热, 两个状态会同屏混淆',
}

overlay, drift, n_a = [], [], 0
if os.path.exists(XLSX_A):
    import openpyxl
    wb = openpyxl.load_workbook(XLSX_A, data_only=True)
    for ws in wb.worksheets:
        for row in ws.iter_rows():
            for cell in row:
                v = cell.value
                if not isinstance(v, str) or len(v.split('\n')) != 3:
                    continue
                zh, tw = v.split('\n')[0].strip(), v.split('\n')[2].strip()
                if not tw or tw == zh or not has_cjk(tw):
                    continue
                n_a += 1
                if zh in DENY_OVERLAY:
                    print('[xlsx] 拒绝 %s → %s (%s)' % (zh, tw, DENY_OVERLAY[zh]))
                    continue
                if zh in conv_map:
                    if tw != conv_map[zh]:
                        overlay.append((zh, conv_map[zh], tw))
                        conv_map[zh] = tw
                else:
                    drift.append((zh, tw))
    print('[xlsx] 规整三元组=%d, 覆盖=%d, A 有而代码无=%d' % (n_a, len(overlay), len(drift)))
else:
    print('[xlsx] 找不到 %s , 跳过人工值覆盖' % XLSX_A)

# 幂等回检: 真繁体再转换应原样; 含简体字的假繁体会被转换器改写 → 退回转换值
if overlay:
    inp = '\n'.join(tw.encode('utf-8').hex() for _, _, tw in overlay) + '\n'
    out2 = subprocess.run(['./dump_tw', '--check'], cwd=HERE, input=inp,
                          capture_output=True, text=True, check=True).stdout.splitlines()
    rejected, keep = [], []
    for (zh, old, tw), hexline in zip(overlay, out2):
        if bytes.fromhex(hexline).decode('utf-8') == tw:
            keep.append((zh, old, tw))
        else:
            rejected.append((zh, tw))
            conv_map[zh] = old
    if rejected:
        print('[xlsx] 幂等回检退回可疑覆盖 %d 条:' % len(rejected),
              ' / '.join('%s→%s' % r for r in rejected))
    overlay = keep

new_chars = {}
for zh, tw in conv_map.items():
    for c in tw:
        if ord(c) > 0x7F and c not in pre_chars:
            new_chars.setdefault(c, []).append(zh)

# ---------------- 6. 拼接 i18n_tw.c ----------------

rows = ['    { "%s", "%s" },' % (c_escape(s), c_escape(conv_map[s])) for s in keys]
block = '\n'.join([
    BEGIN_MK,
    '/* 查表模式: {简体原文, 繁体} 全量映射 — tr_tw() 精确匹配命中即返回永久指针, */',
    '/* 未命中回退 zh 原文(与英文查表未命中同策略)。人工改译文直接编辑本表,      */',
    '/* 或改对照表 xlsx 后重跑 tw_work/gen_tw_table.py 再生成。                  */',
    '/* 独立成 .c: i18n_tw.c 的死字符串(词组表数据, 固件不再调用)可被整节 GC 回收 */',
    'typedef struct { const char *zh; const char *tw; } tw_ent_t;',
    'const tw_ent_t s_tw_table[] = {',
] + rows + [
    '};',
    'const int s_tw_table_n = (int)(sizeof(s_tw_table) / sizeof(s_tw_table[0]));',
    END_MK,
])

# 表写入独立翻译单元 i18n_tw_table.c(标记块幂等替换/首建)
if os.path.exists(TW_TABLE_C):
    traw = open(TW_TABLE_C, 'rb').read().decode('utf-8')
    traw = traw[:traw.index(BEGIN_MK)] + block + traw[traw.index(END_MK) + len(END_MK):]
    open(TW_TABLE_C, 'wb').write(traw.encode('utf-8'))
    print('[splice] i18n_tw_table.c 已替换标记块')
else:
    content = ('/* i18n_tw_table.c — 繁体查表数据, 全部由 tw_work/gen_tw_table.py 生成, 勿手改 */\n'
               '/* i18n_tw.c 里的 tr_tw() 通过 extern 引用 s_tw_table/s_tw_table_n            */\n\n'
               + block + '\n')
    open(TW_TABLE_C, 'wb').write(content.encode('utf-8'))
    print('[splice] i18n_tw_table.c 已创建')

# 迁移: i18n_tw.c 内旧版内嵌表整体移除(查表数据已拆到 i18n_tw_table.c)
raw = open(TW_C, 'rb').read().decode('utf-8')
if BEGIN_MK in raw:
    raw = raw[:raw.index(BEGIN_MK)] + raw[raw.index(END_MK) + len(END_MK):].lstrip('\r\n')
    open(TW_C, 'wb').write(raw.encode('utf-8'))
    print('[splice] 已从 i18n_tw.c 移除旧内嵌表')

# ---------------- report ----------------

rep = ['# gen_tw_table 生成报告', '',
       '- 语料: s_table key %d + custom 新增 %d + 生成层新增 %d = **%d 条**'
       % (n_st, n_custom, n_gen, len(keys)),
       '- 转换器输出字符集 %d 字; xlsx 覆盖 %d 条; 新增字形候选 %d 个'
       % (len(pre_chars), len(overlay), len(new_chars)), '']

if overlay:
    rep += ['## xlsx 覆盖明细 (转换值 → 人工值)', '',
            '| 简体 | 转换器输出 | xlsx 人工值 |', '|---|---|---|']
    for zh, old, new in overlay:
        rep.append('| %s | %s | %s |' % (zh.replace('|', '\\|').replace('\n', '⏎'),
                                         old.replace('|', '\\|'), new.replace('|', '\\|')))
    rep.append('')

rep += ['## xlsx 覆盖引入的新字符(需核对字体)', '']
if new_chars:
    for c, zhs in new_chars.items():
        rep.append('- `%s` U+%04X ← %s' % (c, ord(c), ' / '.join(zhs[:3])))
else:
    rep += ['- 无']

if drift:
    rep += ['## xlsx 有而代码语料无 (措辞漂移/未使用, 未采纳)', '',
            '| xlsx 简体 | xlsx 繁体 |', '|---|---|']
    for zh, tw in drift:
        rep.append('| %s | %s |' % (zh.replace('|', '\\|'), tw.replace('|', '\\|')))
    rep.append('')

open(os.path.join(HERE, 'report.md'), 'w', encoding='utf-8', newline='\n').write('\n'.join(rep))
print('[done] 表 %d 行; 报告 tw_work/report.md' % len(keys))
