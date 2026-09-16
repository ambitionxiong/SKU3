#!/usr/bin/perl
# 第六感中文描述补"烹饪说明："头(与菜谱每谱都有的说明头一致), 并联动:
#   1) nav_six_chicken.c  19 个无头中文字面量(21 处, 鱼/薯条串各 2 处共用)加头
#   2) i18n.c             19 条整串词条换键(键加头)+英文值加 Cooking Instruction:\n
#   3) i18n_tw_table.c    19 条整串词条换键(键加头)+繁体值加 烹調指引：\n
#   4) six_desc_tw_overrides.tsv 同步改这 19 条的 zh/tw, 保证重放流程一致
# 修改前各文件先备份到 tw_work/bak_desc_header/
use strict; use warnings;
use utf8;                    # 源码含中文字面量: 必须, 否则字节串 vs 解码串永远匹配不上
binmode STDOUT, ':utf8';
my $root = 'D:/My_STM32_Project/UIBUILDWER/first_ui_project_code';
my $bak  = "$root/tw_work/bak_desc_header";
mkdir $bak unless -d $bak;
my $HDR_ZH = "烹饪说明：\\n";
my $HDR_EN = "Cooking Instruction:\\n";
my $HDR_TW = "烹調指引：\\n";

my $S3AIR    = "\\n现在将食物放在第3层\\n使用气炸盘和深盘";
my $S3DEEP   = "\\n现在将食物放在第3层\\n使用深盘";
my $S3PAN    = "\\n现在将食物放在第3层\\n使用烤盘";
my $S2SHELF  = "\\n现在将食物放在第2层\\n使用网架和器皿";
my @items = (   # [正文, 层数/器皿后缀]
 ["根据个人喜好进行调味。抹上盐和黑胡椒碎，在炸盘内均匀铺开，在表面喷一层薄油", $S3AIR],
 ["刷油，根据个人喜好，撒上盐、烧烤料、孜然。在烤盘内均匀铺开",                 $S3DEEP],
 ["根据个人喜好进行调味。在炸盘内均匀铺开，在表面喷一层薄油",                   $S3AIR],
 ["均匀分布在深盘中",                                                           $S3DEEP],
 ["刷油，抹上盐和胡椒。根据个人喜好，用蒜和香草调味",                           $S3PAN],
 ["均匀分布在气炸盘中",                                                         $S3AIR],
 ["去掉玉米的皮和须。刷上油，撒上盐。根据你的喜好调味。均匀分布在烤盘上",       $S3PAN],
 ["用叉子在土豆上扎6到8次。在放入烤箱之前，用油刷皮，并在皮上撒上粗盐",         $S3PAN],
 ["参照您喜欢的食谱进行准备。顶部浇上白浆和奶酪碎，以便完美的上色效果",         $S2SHELF],
 ["用叉子刺红薯6到8次。在放入烤箱之前，将表皮抹上油并撒上盐",                   $S3PAN],
 ["根据个人喜好，用油、蒜、胡椒和香芹进行调味",                                 $S3PAN],
 ["根据个人喜好，用油、白葡萄酒、蒜、胡椒和欧芹进行调味",                       $S3PAN],
 ["根据你的喜好，用油、大蒜、胡椒和欧芹调味",                                   $S3PAN],
 ["根据个人喜好调味，刷上油，撒上烧烤调料",                                     $S3PAN],
 ["刷油，根据个人喜好，撒上盐、烧烤料。在烤盘内均匀铺开",                       $S3PAN],
 ["刷上油。根据喜好用蒜蓉酱、盐和油调味",                                       $S3PAN],
 ["参照您喜欢的食谱进行准备。调味拌入黑椒碎、盐、橄榄油和自己喜欢的酱料等",     $S3PAN],
 ["把土豆切成片，在烤盘里叠放成排。把奶油均匀地倒在上面，撒上奶酪。重复做两层，最后在上面涂上一层奶油和奶酪，以达到完美的褐色", $S2SHELF],
 ["从包装中取出，摆在烤盘上",                                                   $S3PAN],
);

sub slurp { my $f = shift; open my $F, '<:utf8', $f or die "open $f: $!"; local $/; my $s = <$F>; close $F; return $s }
sub spew  { my ($f, $s) = @_; open my $O, '>:utf8', $f or die "write $f: $!"; print $O $s; close $O }
sub backup { my $f = shift; (my $name = $f) =~ s{.*/}{}; system('cp', '-f', $f, "$bak/$name") == 0 or die "backup $f" }

# ---------- 1) nav_six_chicken.c ----------
my $f_ch = "$root/ui_builder/custom/nav_six_chicken.c";
backup($f_ch);
my $s = slurp($f_ch);
my $total = 0;
for my $it (@items) {
    my ($body, $suf) = @$it;
    my $old = '"' . $body . $suf . '"';
    my $new = '"' . $HDR_ZH . $body . $suf . '"';
    my $n = () = ($s =~ /\Q$old\E/g);
    die "chicken: expect>=1 hit for [$body], got $n\n" unless $n >= 1;
    $total += $n;
    $s =~ s/\Q$old\E/$new/g;
}
die "chicken: expect 21 sites total, got $total\n" unless $total == 21;
spew($f_ch, $s);
print "nav_six_chicken.c: $total literal sites got header\n";

# ---------- 2) i18n.c ----------
# 注意: 千层面/炸薯条两条词条在源码里键值跨两行(键行以 ", 结尾, 值在下一行), 需回退分支
my $f_en = "$root/ui_builder/custom/i18n.c";
backup($f_en);
$s = slurp($f_en);
for my $it (@items) {
    my ($body, $suf) = @$it;
    my $key    = $body . $suf;
    my $oldkey = '"' . $key . '", "';
    my $newkey = '"' . $HDR_ZH . $key . '", "' . $HDR_EN;
    my $n = () = ($s =~ /\Q$oldkey\E/g);
    if ($n == 1) {
        $s =~ s/\Q$oldkey\E/$newkey/;
        next;
    }
    die "i18n.c: [$body] single-line hits=$n, and not wrapped either\n"
        if $n != 0;
    # 跨行形态: { "<key>",\n<缩进>"<value>
    my $wrapped = '"' . $key . '",';
    # 注意: ()=/(/g) 数的是捕获组元素数, 分组必须用 (?:) 非捕获, 否则 1 命中会被数成 2
    my $w = () = ($s =~ /\Q$wrapped\E(?:\r?\n)(?:\s*)"/g);
    die "i18n.c: expect 1 wrapped hit for [$body], got $w\n" unless $w == 1;
    # 匹配吃掉 [键首引号..值行首引号], 重建时每段只还原一次, EN 头接在值行首引号后
    $s =~ s/\Q$wrapped\E(\r?\n)(\s*)"/ '"' . $HDR_ZH . $key . '",' . $1 . $2 . '"' . $HDR_EN /e;
}
spew($f_en, $s);
print "i18n.c: 19 entries rekeyed + EN header\n";

# ---------- 3) i18n_tw_table.c ----------
my $f_tw = "$root/ui_builder/custom/i18n_tw_table.c";
backup($f_tw);
$s = slurp($f_tw);
for my $it (@items) {
    my ($body, $suf) = @$it;
    my $oldkey = '"' . $body . $suf . '", "';
    my $newkey = '"' . $HDR_ZH . $body . $suf . '", "' . $HDR_TW;
    my $n = () = ($s =~ /\Q$oldkey\E/g);
    die "tw: expect 1 hit for [$body], got $n\n" unless $n == 1;
    $s =~ s/\Q$oldkey\E/$newkey/;
}
spew($f_tw, $s);
print "i18n_tw_table.c: 19 entries rekeyed + TW header\n";

# ---------- 4) TSV 同步(重放一致性) ----------
my $f_tsv = "$root/tw_work/six_desc_tw_overrides.tsv";
backup($f_tsv);
open my $M, '<:utf8', $f_tsv or die;
my @lines = <$M>; close $M;
chomp @lines;
my $changed = 0;
for my $l (@lines) {
    next if $l =~ /^\s*(#|$)/;
    $l =~ /\t/ or next;
    my ($zh, $tw) = split /\t/, $l, 2;
    for my $it (@items) {
        my ($body, $suf) = @$it;
        my $oldzh = $body . $suf;
        $oldzh =~ s/\\n/\x{23CE}/g;      # C 源 \n 两字符 → TSV 的 ⏎
        if ($zh eq $oldzh) {
            my $newzh = "烹饪说明：\x{23CE}" . $zh;
            my $newtw = "烹調指引：\x{23CE}" . $tw;
            $l = "$newzh\t$newtw";
            $changed++;
        }
    }
}
die "tsv: expect 19 lines changed, got $changed\n" unless $changed == 19;
spew($f_tsv, join("\n", @lines) . "\n");
print "tsv: $changed entries synced\n";
