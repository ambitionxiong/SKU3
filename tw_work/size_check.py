#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""粗算 i18n_tw_table.c 的字符串字节量(只用于 size 报告)"""
import os

p = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                 '..', 'ui_builder', 'custom', 'i18n_tw_table.c')
strs = set()
n = 0
for line in open(p, encoding='utf-8'):
    line = line.strip()
    if line.startswith('{ "') and line.endswith('" },'):
        zh, tw = line[3:-4].split('", "')
        for s in (zh, tw):
            b = s.replace('\\n', '\n').replace('\\"', '"').replace('\\\\', '\\')
            strs.add(b.encode('utf-8'))
        n += 1
total = sum(len(b) + 1 for b in strs)
print('表行数:', n)
print('唯一字符串:', len(strs), ' 字节合计(含NUL):', total)
print('指针数组: 10832 B, i18n_tw_table.o 合计约', total + 10832, 'B')
print('改动前 i18n_tw.o: str1.8 100623 + 词表数据 103668 + text 688 + bss 2048 =',
      100623 + 103668 + 688 + 2048, 'B')
