#!/usr/bin/perl
# 第六感描述行尾句号补齐(用户定版规则 2026-09-16): 描述每一行尾都要有句号(中文。/英文.);
# 头行"烹饪说明：/Cooking Instruction:/烹調指引："冒号结尾不加; 小结/烹饪时间类串不在 43 条内不碰。
# 联动面:
#   1) nav_six_chicken.c / nav_six_cook.c  中文描述字面量(行尾加。)
#   2) i18n.c        42 条键换(土司已带句号跳过) + 英文值行尾缺 . 的补 .
#   3) i18n_tw_table.c 43 条+行片段换键 + 繁体值行尾缺。的补(预期仅烤香肠)
#   4) six_desc_tw_overrides.tsv 同步(zh/tw 新形态; 裸"现在将食物放在第2层"与已有带。条目重撞→删除)
# 用法: perl apply_desc_periods.pl          -> DRY 只打印计划
#       perl apply_desc_periods.pl apply    -> 备份到 bak_desc_periods/ 后落盘
use strict; use warnings;
use utf8;                    # 源码含中文字面量: 必须
binmode STDOUT, ':utf8';
my $root = 'D:/My_STM32_Project/UIBUILDWER/first_ui_project_code';
my $bak  = "$root/tw_work/bak_desc_periods";
my $TSV  = "$root/tw_work/six_desc_tw_overrides.tsv";
my $APPLY = (@ARGV && $ARGV[0] eq 'apply') ? 1 : 0;
my $NL = "\x{23CE}";        # TSV 里的 ⏎ = C 源 \n(两字符)

sub slurp { my $f=shift; open my $F,'<:utf8',$f or die "open $f: $!"; local $/; my $s=<$F>; close $F; return $s }
sub spew  { my ($f,$s)=@_; open my $O,'>:utf8',$f or die "write $f: $!"; print $O $s; close $O }
sub backup { my $f=shift; (my $n=$f)=~s{.*/}{}; system('cp','-f',$f,"$bak/$n")==0 or die "backup $f" }
sub toC    { my $s=shift; $s =~ s/\Q$NL\E/\\n/g; return $s }
sub addp_cn { my $s=shift; return ($s =~ /[。：]$/) ? $s : $s.'。' }
sub addp_en { my $s=shift; $s =~ s/\s+$//; return ($s =~ /[.:]$/) ? $s : $s.'.' }
sub splitnl { my $s=shift; return split /\\n/, $s }

# ---------- 1) 解析 TSV ----------
open my $M,'<:utf8',$TSV or die "open $TSV: $!";
my @tl=<$M>; close $M; chomp @tl;
my ($sec,%sec,@full,@frag1,@frag4);
for my $i (0..$#tl) {
    my $l=$tl[$i];
    if    ($l =~ /^# ==== 一、/) { $sec='f1' }
    elsif ($l =~ /^# ==== 二、/) { $sec='name' }
    elsif ($l =~ /^# ==== 三、/) { $sec='full' }
    elsif ($l =~ /^# ==== 四、/) { $sec='f4' }
    $sec{$i}=$sec // '';
    next if $l =~ /^\s*#/ or $l !~ /\t/;
    my ($zh,$tw)=split /\t/,$l,2;
    if    ($sec eq 'full') { push @full,  [$i,$zh,$tw] }
    elsif ($sec eq 'f1')   { push @frag1, [$i,$zh,$tw] }
    elsif ($sec eq 'f4')   { push @frag4, [$i,$zh,$tw] }
}
printf "TSV: full=%d frag1=%d frag4=%d\n", scalar(@full), scalar(@frag1), scalar(@frag4);
die "expect 43 full" unless @full==43;

# ---------- 2) 组装中文/繁体新串 ----------
my (%new_zh,%new_tw,%line_map,@tw_fix_full);
my ($n_unchanged,$unchanged_which)=(0,'');
for my $e (@full) {
    my ($i,$zh,$tw)=@$e;
    my @ol=split /\Q$NL\E/,$zh;
    die "zh hdr?[$ol[0]]" unless $ol[0] eq '烹饪说明：';
    my @zl=($ol[0], map { addp_cn($_) } @ol[1..$#ol]);
    my @ot=split /\Q$NL\E/,$tw;
    die "tw hdr?[$ot[0]]" unless $ot[0] eq '烹調指引：';
    my @nt=(shift @ot, map { addp_cn($_) } @ot);
    my $nzh=join($NL,@zl); my $ntw=join($NL,@nt);
    if ($nzh ne $zh) { $new_zh{$zh}=$nzh }
    else             { $n_unchanged++; $unchanged_which=$zh }
    if ($ntw ne $tw) { $new_tw{$tw}=$ntw; push @tw_fix_full,[$zh,$tw,$ntw] }
    for my $k (1..$#ol) { $line_map{$ol[$k]}=$zl[$k] if $ol[$k] ne $zl[$k] }
}
die "expect exactly 1 unchanged full (土司), got $n_unchanged\n" unless $n_unchanged==1;
printf "full: zh changed=%d tw changed=%d unchanged=[%s...]\n",
       scalar(keys %new_zh), scalar(@tw_fix_full), substr($unchanged_which,6,24);
die "expect tw changed==1 (烤香肠)" unless @tw_fix_full==1;
printf "  tw +。 full: [%s]\n", substr($tw_fix_full[0][2],-30);

# ---------- 3) 英文现值 -> 新值(补 .) ----------
my $en_src = slurp("$root/ui_builder/custom/i18n.c");
my %new_en;
for my $zh (sort keys %new_zh) {
    my $okC=toC($zh);
    my ($val);
    if    ($en_src =~ /\Q"$okC", "\E((?:[^"\\]|\\.)*)"/s)              { $val=$1 }
    elsif ($en_src =~ /\Q"$okC",\E(?:\r?\n)(?:\s*)"((?:[^"\\]|\\.)*)"/s) { $val=$1 }
    else { die "i18n.c: EN value not found for [".substr($zh,6,30)."]" }
    my $nv=join('\\n', map { addp_en($_) } splitnl($val));
    $new_en{$zh}=$nv if $nv ne $val;
}
printf "en values needing '.': %d\n", scalar(keys %new_en);
for my $k (sort keys %new_en) { print "  +'.': [".substr($k,6,22)."] tail=[".substr($new_en{$k},-42)."]\n" }

# ---------- 4) nav 字面量位点核对 ----------
my @nav = ("$root/ui_builder/custom/nav_six_chicken.c", "$root/ui_builder/custom/nav_six_cook.c");
my ($tot_sites,$toast_sites,$all_sites)=(0,0,0);
my %nav_all;
for my $f (@nav) {
    my $s=slurp($f); my $c=0;
    for my $zh (sort keys %new_zh) {
        my $oldC='"'.toC($zh).'"';
        $c += ()=($s =~ /\Q$oldC\E/g);
    }
    my $toastC='"'.toC($unchanged_which).'"';
    $toast_sites += ()=($s =~ /\Q$toastC\E/g);
    my $all=()=($s =~ /"烹饪说明：\\n(?:[^"\\]|\\.)*"/g);
    $nav_all{$f}=$all; $all_sites+=$all; $tot_sites+=$c;
    printf "nav %s: all=%d changed=%d\n", ($f =~ /([a-z_]+)\.c$/)[0], $all, $c;
}
printf "nav total: all=%d changed=%d toast=%d\n", $all_sites, $tot_sites, $toast_sites;
die "nav mismatch: changed($tot_sites)+toast($toast_sites) != all($all_sites)\n"
    unless $tot_sites+$toast_sites==$all_sites;

# ---------- 5) 行片段(frag1/frag4)处理 ----------
my (@f1_ops,@f4_lineout,@tw_ops);
my ($drop1,$f4_tw_fix)=(0,0);
{
    my %seen;
    for my $e (@frag1) { my ($i,$zh,$tw)=@$e; $seen{$zh}=1 unless exists $line_map{$zh} }
    for my $e (@frag1) {
        my ($i,$zh,$tw)=@$e;
        my $nt=join($NL, map { addp_cn($_) } split /\Q$NL\E/, $tw);
        if (exists $line_map{$zh}) {
            my $nz=$line_map{$zh};
            if ($seen{$nz}) { $drop1++; print "  frag1 DROP(撞已有带。条目): [$zh]\n"; next }
            $seen{$nz}=1; push @f1_ops,[$i,$zh,$tw,$nz,$nt];
            print "  frag1 rekey: [$zh] -> [$nz]\n" if $nz ne $zh;
        } else { push @f1_ops,[$i,$zh,$tw,$zh,$nt] }
    }
    for my $e (@frag4) {
        my ($i,$zh,$tw)=@$e;
        my $nz=$line_map{$zh} // $zh;
        my $nt=join($NL, map { addp_cn($_) } split /\Q$NL\E/, $tw);
        push @f4_lineout,[$i,$zh,$nz,$nt];
        $f4_tw_fix++ if $nt ne $tw;
        print "  frag4 tw +。: [$tw]\n" if $nt ne $tw;
    }
}
printf "frag1: ops=%d drop=%d; frag4: lines=%d tw_fix=%d\n",
       scalar(@f1_ops), $drop1, scalar(@f4_lineout), $f4_tw_fix;

# tw 表操作对: [old_zh, old_tw, new_zh, new_tw], 仅收录有变化的
for my $e (@full) { my ($i,$zh,$tw)=@$e;
    my $nz=$new_zh{$zh} // $zh; my $nt=$new_tw{$tw} // $tw;
    push @tw_ops,[$zh,$tw,$nz,$nt] if $nz ne $zh or $nt ne $tw;
}
for my $o (@f1_ops) { my ($i,$oz,$ot,$nz,$nt)=@$o;
    push @tw_ops,[$oz,$ot,$nz,$nt] if $nz ne $oz or $nt ne $ot }
for my $o (@f4_lineout) { my ($i,$oz,$nz,$nt)=@$o; my ($t)=(split /\t/,$tl[$i],2)[1] // '';
    push @tw_ops,[$oz,$t,$nz,$nt] if $nz ne $oz or $nt ne $t }
printf "tw-table ops: %d\n", scalar(@tw_ops);

# ---------- 6) 预检: i18n.c 键形态 + tw 表命中 ----------
my ($en_single,$en_wrapped)=(0,0);
for my $zh (sort keys %new_zh) {
    my $okC='"'.toC($zh).'", "';
    my $n=()=($en_src =~ /\Q$okC\E/g);
    if ($n==1) { $en_single++; next }
    my $w='"'.toC($zh).'",';
    my $m=()=($en_src =~ /\Q$w\E(?:\r?\n)(?:\s*)"/g);
    die "i18n.c key: [".substr($zh,6,24)."] single=$n wrapped=$m" unless $m==1;
    $en_wrapped++;
}
printf "i18n.c key forms: single=%d wrapped=%d\n",$en_single,$en_wrapped;
die "expect wrapped==2 (千层面/炸薯条)" unless $en_wrapped==2;

my $tw_src = slurp("$root/ui_builder/custom/i18n_tw_table.c");
my $tw_miss=0;
for my $op (@tw_ops) {
    my ($oz,$ot,$nz,$nt)=@$op;
    my $pat='"'.toC($oz).'", "'.toC($ot).'"';
    my $n=()=($tw_src =~ /\Q$pat\E/g);
    if ($n!=1) { $tw_miss++; print "  tw MISS($n): [$oz]\n" }
}
die "tw-table precheck misses=$tw_miss\n" if $tw_miss;

exit 0 unless $APPLY;
print "== APPLY ==\n";
mkdir $bak unless -d $bak;

# 7a) nav 字面量
for my $f (@nav) {
    my $s=slurp($f); backup($f);
    for my $zh (sort keys %new_zh) {
        my $oldC='"'.toC($zh).'"'; my $newC='"'.toC($new_zh{$zh}).'"';
        $s =~ s/\Q$oldC\E/$newC/g;
    }
    spew($f,$s); print "written: nav ", ($f =~ /([a-z_]+\.c$)/)[0], "\n";
}

# 7b) i18n.c: 键换 + EN 值补 .
{
    my $s=$en_src; backup("$root/ui_builder/custom/i18n.c");
    for my $zh (sort keys %new_zh) {
        my $okC=toC($zh); my $nkC=toC($new_zh{$zh});
        my $ne = $new_en{$zh};
        my $p1 = quotemeta('"'.$okC.'", "');
        my $n1=()=($s =~ /$p1/g);
        if ($n1==1) {
            if (defined $ne) {
                $s =~ s/($p1)((?:[^"\\]|\\.)*)(")/'"'.$nkC.'", "'.$ne.'"'/e;
            } else {
                $s =~ s/$p1/"$nkC", "/;
            }
            next;
        }
        my $p2 = quotemeta('"'.$okC.'",');
        my $n2=()=($s =~ /$p2(?:\r?\n)(?:\s*)"/g);
        die "i18n.c apply: [".substr($zh,6,24)."] n1=$n1 n2=$n2" unless $n2==1;
        die "wrapped entry without EN value: [".substr($zh,6,24)."]" unless defined $ne;
        $s =~ s/$p2(\r?\n)(\s*)"((?:[^"\\]|\\.)*)"/'"'.$nkC.'",'.$1.$2.'"'.$ne.'"'/e;
    }
    spew("$root/ui_builder/custom/i18n.c",$s); print "written: i18n.c\n";
}

# 7c) tw 表: 键值对替换
{
    my $s=$tw_src; backup("$root/ui_builder/custom/i18n_tw_table.c");
    for my $op (@tw_ops) {
        my ($oz,$ot,$nz,$nt)=@$op;
        my $pat='"'.toC($oz).'", "'.toC($ot).'"';
        my $rep='"'.toC($nz).'", "'.toC($nt).'"';
        my $n=()=($s =~ /\Q$pat\E/g);
        die "tw apply miss($n): [$oz]" unless $n==1;
        $s =~ s/\Q$pat\E/$rep/;
    }
    spew("$root/ui_builder/custom/i18n_tw_table.c",$s); print "written: i18n_tw_table.c\n";
}

# 7d) TSV 重写
{
    backup($TSV);
    my %out;
    for my $e (@full)      { my ($i,$zh,$tw)=@$e; $out{$i}=($new_zh{$zh}//$zh)."\t".($new_tw{$tw}//$tw) }
    for my $o (@f1_ops)    { my ($i,$oz,$ot,$nz,$nt)=@$o; $out{$i}="$nz\t$nt" }
    for my $o (@f4_lineout){ my ($i,$oz,$nz,$nt)=@$o; $out{$i}="$nz\t$nt" }
    open my $O,'>:utf8',$TSV or die;
    for my $i (0..$#tl) { print $O (exists $out{$i} ? $out{$i} : $tl[$i]),"\n" }
    close $O; print "written: TSV\n";
}

# ---------- 8) 落盘后不变量扫描 ----------
{
    my $bad=0;
    my $s=slurp("$root/ui_builder/custom/i18n.c");
    while ($s =~ /"(烹饪说明：(?:[^"\\]|\\.)*)"/g) {
        for my $seg (splitnl($1)) { my $t=$seg; $t =~ s/\s+$//;
            if ($t !~ /[。：]$/) { $bad++; print "  BAD zh key line: [$seg]\n" } }
    }
    while ($s =~ /"(Cooking Instruction:(?:[^"\\]|\\.)*)"/g) {
        for my $seg (splitnl($1)) { my $t=$seg; $t =~ s/\s+$//;
            if ($t !~ /[.:]$/) { $bad++; print "  BAD en value line: [$seg]\n" } }
    }
    print "i18n.c invariant: ",($bad?"FAIL($bad)":"OK"),"\n";
    for my $f (@nav) {
        my $s2=slurp($f);
        while ($s2 =~ /"(烹饪说明：(?:[^"\\]|\\.)*)"/g) {
            for my $seg (splitnl($1)) { my $t=$seg; $t =~ s/\s+$//;
                if ($t !~ /[。：]$/) { $bad++; print "  BAD nav literal [$f]: [$seg]\n" } }
        }
    }
    print "nav invariant: ",($bad?"FAIL($bad)":"OK"),"\n";
    die "invariant failed" if $bad;
}
print "DONE\n";
