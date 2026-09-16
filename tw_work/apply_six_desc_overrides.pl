#!/usr/bin/perl
# 把 six_desc_tw_overrides.tsv 的手工校准词条(繁体描述/菜名, 按 6th 菜谱 Excel 繁体块逐字)
# 应用到 ui_builder/custom/i18n_tw_table.c。
# 用途: gen_tw_table.py 重生成会整体冲掉 BEGIN/END 块内手改, 用本脚本按 TSV 重放即可。
# 用法: perl apply_six_desc_overrides.pl six_desc_tw_overrides.tsv <i18n_tw_table.c> [--dry]
# TSV 格式: 每行 "zh<TAB>tw"; C 源里的 \n(反斜杠+n 两字符)在 TSV 里写作 ⏎ 占位。
use strict; use warnings;
my ($mapfile, $target, $dry) = @ARGV;
die "usage: $0 <tsv> <i18n_tw_table.c> [--dry]\n" unless $target;
$dry = (defined $dry && $dry eq '--dry');
open my $M, '<:utf8', $mapfile or die "open $mapfile: $!";
my (%map, %ln);
while (my $line = <$M>) {
    chomp $line;
    next if $line =~ /^\s*(#|$)/;
    $line =~ /\t/ or die "tsv line $. has no TAB\n";
    my ($zh, $tw) = split /\t/, $line, 2;
    die "tsv line $. tw empty\n" if $tw eq '';
    for ($zh, $tw) { s/\x{23CE}/\\n/g; }   # ⏎ 占位 → C 源反斜杠+n
    $map{$zh} = $tw; $ln{$zh} = $.;
}
close $M;
open my $F, '<:utf8', $target or die "open $target: $!";
local $/; my $src = <$F>; close $F;
my ($ok, $miss, $multi) = (0, 0, 0);
for my $zh (sort { $ln{$a} <=> $ln{$b} } keys %map) {
    my $tw  = $map{$zh};
    my $pat = '"' . quotemeta($zh) . '", "[^"]*"';
    my $n = () = ($src =~ /$pat/g);
    if ($n == 0) { $miss++; print "MISS  tsv line $ln{$zh}\n"; next; }
    $multi++ if $n > 1;
    print "MULTI tsv line $ln{$zh}: $n hits\n" if $n > 1;
    my $rep = '"' . $zh . '", "' . $tw . '"';
    $src =~ s/$pat/$rep/g;
    $ok++;
}
print "entries ok=$ok miss=$miss multi=$multi\n";
exit 1 if $miss > 0;
exit 0 if $dry;
open my $O, '>:utf8', $target or die "write $target: $!";
print $O $src; close $O;
print "written: $target\n";
