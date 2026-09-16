#!/usr/bin/perl
# 字形审计: six_desc_tw_overrides.tsv 新繁体串的字符集 vs c_taiwanpearl_regular_*.c 已嵌入字形
# 用法: perl tw_work/check_glyphs.pl [报告输出文件]
use strict; use warnings;
my $tsv  = 'D:/My_STM32_Project/UIBUILDWER/first_ui_project_code/tw_work/six_desc_tw_overrides.tsv';
my $out  = shift || 'D:/My_STM32_Project/UIBUILDWER/first_ui_project_code/tw_work/glyph_audit.txt';
my @fonts = glob('D:/My_STM32_Project/UIBUILDWER/first_ui_project_code/ui_builder/font/c_taiwanpearl_regular_*.c');
open my $M, '<:utf8', $tsv or die "open $tsv: $!";
my %need;
while (my $l = <$M>) {
    chomp $l; next if $l =~ /^\s*(#|$)/;
    $l =~ /\t/ or next;
    my (undef, $tw) = split /\t/, $l, 2;
    $tw =~ s/\x{23CE}//g;                      # 去掉 ⏎ 占位
    for my $ch (split //, $tw) { $need{ord $ch} = $ch unless ord $ch == 10; }
}
close $M;
open my $O, '>:utf8', $out or die "open $out: $!";
printf $O "needed unique chars: %d\n", scalar keys %need;
for my $f (sort @fonts) {
    my ($sz) = $f =~ /regular_(\d+)\.c$/;
    open my $F, '<:utf8', $f or die "open $f: $!";
    local $/; my $s = <$F>; close $F;
    my %lists;
    while ($s =~ /unicode_list_(\d+)\[\]\s*=\s*\{([^}]*)\}/gs) {
        my ($idx, $body) = ($1, $2);
        my @v = map { /^0x/i ? hex $_ : $_ + 0 } ($body =~ /(0x[0-9a-fA-F]+|\d+)/g);
        $lists{$idx} = \@v;
    }
    my %cov;
    while ($s =~ /\{\s*\.range_start\s*=\s*(\d+)\s*,\s*\.range_length\s*=\s*(\d+)\s*,.*?\.unicode_list\s*=\s*(NULL|unicode_list_\d+)\s*,.*?\}/gs) {
        my ($rs, $rl, $ul) = ($1, $2, $3);
        if ($ul eq 'NULL') { $cov{$_} = 1 for $rs .. $rs + $rl - 1; }
        else {
            my ($idx) = $ul =~ /(\d+)$/;
            $cov{ $rs + $_ } = 1 for @{ $lists{$idx} || [] };
        }
    }
    my @miss = sort { $a <=> $b } grep { !$cov{$_} } keys %need;
    printf $O "font %s: missing %d", $sz, scalar @miss;
    if (@miss) {
        print $O ": ";
        print $O map { sprintf "U+%04X(%s) ", $_, chr($_) } @miss;
    }
    print $O "\n";
}
close $O;
print "report: $out\n";
