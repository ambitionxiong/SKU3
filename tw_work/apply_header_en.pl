#!/usr/bin/perl
# i18n.c 英文描述头部对齐菜谱: "Instructions:\n -> "Cooking Instruction:\n
# 菜谱(SKU3惠而浦烤箱6th菜单菜谱-中英文-2026.xlsx)所有 EN 块头部均为 Cooking Instruction:,
# 原表中面包/蛋糕/禽肉类 19 条用 Instructions:。预期替换 19 处。
use strict; use warnings;
my $f = 'D:/My_STM32_Project/UIBUILDWER/first_ui_project_code/ui_builder/custom/i18n.c';
open my $F, '<:utf8', $f or die "open $f: $!";
local $/; my $src = <$F>; close $F;
my $n = ($src =~ s/"Instructions:\\n/"Cooking Instruction:\\n/g);
die "expected 19 replacements, got $n - abort\n" if $n != 19;
open my $O, '>:utf8', $f or die "write $f: $!";
print $O $src; close $O;
print "header replaced=$n\n";
