#!/usr/bin/perl
# SDK 素材清理(2026-09-16): 零引用图片移出 assets, 到 ui_builder 之外的 trash 目录
# 引用判定(全 ui_builder 树 .c/.h):
#   1) LVGL_IMAGE_PATH(arg)  — 生成层, arg 相对 image/
#   2) func("arg.png/jpg")   — custom 层 lang_img_src 等带引号字面量
#   3) 原始文本边界扫描       — 兜底裸字符串(icon.png 不得误撞 hotwindicon.png)
# 任何一条命中即保留不删; 移动 = rename 到 trash(保 used/ 子目录结构)
use strict; use warnings; use utf8;
binmode STDOUT, ':utf8';
my $SDKUI = 'D:/My_STM32_Project/artin/luban_lite_D21_SKU/luban_lite_D21_SKU/packages/artinchip/lvgl-ui/aic_demo/ui_builder';
my $TRASH = 'D:/My_STM32_Project/artin/luban_lite_D21_SKU/luban_lite_D21_SKU/packages/artinchip/lvgl-ui/aic_demo/ui_builder_assets_trash_20260916';
my $APPLY = (@ARGV && $ARGV[0] eq 'apply') ? 1 : 0;

my @NAMES = (   # 审计得出的零引用清单(相对 assets/image/)
  'Btn_bg_N.png','Btn_bg_Y.png','Pointer_icon.png','ball.png','bg_q95.jpg',
  'bottom.png','central.png','centralbk.png','cooking.png',
  'count_down_reset_icon_1.png','count_down_reset_icon_2.png','div1button.png',
  'highlevel.png','hot_change.png','hot_wind.png','hot_windbk.png',
  'hotchangebk.png','hotwind.png','hotwindbk.png','icon.png','icon2.png',
  'lockbg_1.png','longtext.png','loudness_bar_icon.png','loudness_bar_var1.png',
  'loudness_bar_var2.png','loudness_bar_var3.png','loudness_icon.png',
  'luminance_bar_icon.png','mainbk.png','mainbk_q95.jpg','pageupdown_1.png',
  'pengrenmenu.png','prehot.png','prehotbk.png','probecookingfrt.png',
  'save.png','savebk.png','set2bg.png','settingline3_75x4.png',
  'settingline3_87x3.png','start.png','stepsetbg.png','stepsetbg1.png',
  'stop.png','sure.png','switch_bg_focused.png','switch_bg_yuankuang.png',
  'teshutzbk.png','teshutzbk30.png','top.png','topbk.png','up_down.png',
  'upbbq.png','updbbq_right.png','updownbbq_2.png','upsettingbk.png',
  'waitbg.png','watercleanmenufrt.png','weightbg.png','yanshibk.png',
  "\x{8D44}\x{6E90} 34.png","\x{8D44}\x{6E90} 4.png",
  'used/HowWellDone_underline_101x4.png','used/HowWellDone_underline_121x4.png',
  'used/HowWellDone_underline_88x4.png','used/HowWellDone_underline_90x4.png',
);

# 1) 引用集
my @files = (glob("$SDKUI/*.c"), glob("$SDKUI/*.h"),
             glob("$SDKUI/custom/*.c"), glob("$SDKUI/custom/*.h"));
die "no source files?" unless @files > 100;
my %ref; my $allref = '';
for my $f (@files) {
    open my $F, '<:utf8', $f or die "open $f: $!"; local $/; my $s = <$F>; close $F;
    $allref .= $s;
    while ($s =~ /LVGL_IMAGE_PATH\(([^)]*)\)/g) { my $a=$1; $a =~ s/^\s+|\s+$//g; $ref{$a}=1 }
    while ($s =~ /[A-Za-z_]+\(\s*"([^"]+\.(?:png|jpg))"\s*\)/g) { $ref{$1}=1 }
}
printf "source files=%d, referenced names=%d\n", scalar(@files), scalar(keys %ref);

# 2) 验证 + 移动
my ($moved,$bytes,$blocked)=(0,0,0);
for my $n (@NAMES) {
    my $src = "$SDKUI/assets/image/$n";
    die "not a file: $n" unless -f $src;
    my $base = $n; $base =~ s{.*/}{};
    my $why;
    if (exists $ref{$n} || exists $ref{$base}) { $why = 'in ref set' }
    else {
        my $q = quotemeta($base);
        if ($allref =~ /(?<![A-Za-z0-9_\/.])$q(?![A-Za-z0-9_.])/) { $why = 'raw text hit' }
    }
    if (defined $why) { $blocked++; print "  KEEP($why): $n\n"; next }
    if ($APPLY) {
        my $dst = "$TRASH/$n"; my $dir = $dst; $dir =~ s{/[^/]+$}{};
        mkdir $dir unless -d $dir;
        rename $src, $dst or die "move $n: $!";
    }
    $moved++; $bytes += -s $src;
}
printf "candidates=%d, moved=%d, freed=%.2f MB, blocked=%d, APPLY=%d\n",
       scalar(@NAMES), $moved, $bytes/1048576, $blocked, $APPLY;
