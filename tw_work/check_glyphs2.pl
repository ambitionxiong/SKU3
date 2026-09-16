#!/usr/bin/perl
# 字形审计 v2: 以字体文件头 Opts 行的 -r 码点列表(生成命令原文, 权威)为准
# 比对 six_desc_tw_overrides.tsv 新繁体串字符集, 输出每个字号缺失字符
use strict; use warnings;
my $tsv = 'D:/My_STM32_Project/UIBUILDWER/first_ui_project_code/tw_work/six_desc_tw_overrides.tsv';
my $out = 'D:/My_STM32_Project/UIBUILDWER/first_ui_project_code/tw_work/glyph_audit2.txt';
my @fonts = glob('D:/My_STM32_Project/UIBUILDWER/first_ui_project_code/ui_builder/font/c_taiwanpearl_regular_*.c');
open my $M, '<:utf8', $tsv or die "open $tsv: $!";
my %need;
while (my $l = <$M>) {
    chomp $l; next if $l =~ /^\s*(#|$)/;
    $l =~ /\t/ or next;
    my (undef, $tw) = split /\t/, $l, 2;
    $tw =~ s/\x{23CE}//g;
    for my $ch (split //, $tw) { $need{ord $ch} = $ch unless ord $ch == 10; }
}
close $M;
open my $O, '>:utf8', $out or die;
printf $O "needed unique chars: %d\n", scalar keys %need;
for my $f (sort @fonts) {
    my ($sz) = $f =~ /regular_(\d+)\.c$/;
    open my $F, '<:utf8', $f or die "open $f: $!";
    local $/; my $s = <$F>; close $F;
    my ($opts) = $s =~ / \* Opts: (.*)$/m or die "no Opts in $f";
    my %cov;
    for my $r ($opts =~ /-r\s+(\S+)/g) {
        if ($r =~ /^0x([0-9a-fA-F]+)-0x([0-9a-fA-F]+)$/) { $cov{$_} = 1 for hex($1) .. hex($2); }
        elsif ($r =~ /^0x([0-9a-fA-F]+)$/) { $cov{hex $1} = 1; }
    }
    my @miss = sort { $a <=> $b } grep { !$cov{$_} } keys %need;
    printf $O "font %s: covered=%d missing=%d", $sz, scalar keys %cov, scalar @miss;
    if (@miss) {
        print $O ": ";
        print $O map { sprintf "U+%04X(%s) ", $_, chr($_) } @miss;
    }
    print $O "\n";
}
close $O;
print "report: $out\n";
