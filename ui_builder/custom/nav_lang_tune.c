/* =====================================================================
 * 英文排版调整层：同事编辑区（nav_lang_tune.c）— 全页面布局基准
 *
 * 每个函数对应一个页面，内含该页面全部对象的布局基准数据：
 *   标签文本 "..." / 坐标 (x,y) / 尺寸 WxH / 字号 font / 对齐 align
 *   / 图片源 img / 背景图 bg（值来自生成文件中文布局；img/bg 仅注释）
 *
 * 使用：改数值即可调整英文版布局（仅英文模式执行，中文模式零影响）
 *   lv_obj_set_pos(pg->xxx, 新x, 新y)      —— 移动
 *   lv_obj_set_size(pg->xxx, 新w, 新h)     —— 改尺寸
 *   字号：英文文本建议 aktivgroteskmedium_XX（树遍历已自动映射）
 *   换图：注释里的 img:/bg: 是当前图名，需换图自行处理（本文件不改图）
 *
 * 动态定位对象（注释标注'动态定位'）：位置由业务代码按状态切换
 *   （set_hour 有无小时 / cook_mode 模式 / 进度），本文件默认不设位置。
 *   需要微调时在此函数里加判断（tune 执行时状态变量已是显示值）：
 *
 *     以 set 页分钟+单位标签为例：
 *     if (set_hour == 0) {                       // 无小时
 *         lv_obj_set_pos(pg->min, 312 + 8, 254);
 *         lv_obj_set_pos(pg->label_306, 365 + 8, 269);
 *     } else {                                   // 有小时
 *         lv_obj_set_pos(pg->min, 395 + 8, 254);
 *         lv_obj_set_pos(pg->label_306, 448 + 8, 269);
 *     }
 *     // icon 按模式： if (g_send.cook_mode == MODE_UNFROZEN) ...
 *
 * 同 PAGE 多入口页面按入口模式微调（一个函数服务多入口，用业务状态区分）：
 *   delaycooking_lang_tune   预约烹饪页（29 个模式共用）→ g_send.cook_mode
 *   descriptionmenu_lang_tune 菜谱说明页（六感蛋糕/鸡/面包等）→ g_six_bread_type
 *   duckmenu_lang_tune        鸡鸭配菜菜单（鸡/肉/配菜）→ g_six_bread_type
 *
 *     // 例：delaycooking 只给披萨/空气炸单独调，其余共用
 *     switch (g_send.cook_mode) {
 *     case MODE_PIZZA: lv_obj_set_pos(pg->icon, 180, 161); break;   // 披萨预约
 *     case MODE_AIR:   lv_obj_set_pos(pg->icon, 163, 161); break;   // 空气炸预约
 *     default:         lv_obj_set_pos(pg->icon, 115, 161); break;   // 其他模式
 *     }
 *
 * 复用页面（1 个结构服务 2 个 PAGE）已拆成 公共 common + 每页独立函数：
 *   somecook_cooking_common + six_cooking_lang_tune(第六感烹饪页) /
 *     somecook_cooking_lang_tune(多段烹饪页)
 *   preheatmenu_common + preheatmenu_lang_tune(预热菜单页) /
 *     color_menu_lang_tune(额外上色设置页)
 *   → 各页差异直接改对应函数里的数字；公共布局改 common。
 *
 *   bartemp（进度条跟随，定时器每秒重写）：判断无效，只能用
 *   注册表 dx/dy 整体偏移（见文件末尾注册表）。
 *
 * 注册表在文件末尾：页面 → 函数 一一对应；新增页面照格式加一行
 * 详见 docs/I18N_GUIDE_zh.md
 * ===================================================================== */
#include "nav_lang.h"
#include "i18n.h"
#include "protocol.h"   /* g_send 状态变量（icon/模式判断用） */
#include "screen_SET.h"   /* 独立页面结构体（不挂 ui_manager） */

typedef void (*lang_tune_fn)(void);

/* ==============================================================================
 * air_complete 英文布局基准（对应 PAGE_AIR_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void air_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_AIR_COMPLETE) return;
    air_complete_t *pg = air_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_153: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_153, 115, 320);

    /* status: 标签 | "| 空气炸 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_23: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_23, 122, 326);
    lv_obj_set_size(pg->bar_23, 637, 20);

    /* label_337: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_337, 273, 157);
    lv_obj_set_size(pg->label_337, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_155: 图片 | (115,161) | img: airicon.png */
    lv_obj_set_pos(pg->image_155, 115, 161);

    /* image_6: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_6, 902, 160);

}


/* ==============================================================================
 * air_cooking 英文布局基准（对应 PAGE_AIR_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void air_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_AIR_COOKING) return;
    air_cooking_t *pg = air_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 空气炸 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_142: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_142, 115, 320);

    /* bar_20: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_20, 122, 326);
    lv_obj_set_size(pg->bar_20, 637, 20);

    /* label_318: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_318, 273, 157);
    lv_obj_set_size(pg->label_318, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_143: 图片 | (115,161) | img: airicon.png */
    lv_obj_set_pos(pg->image_143, 115, 161);

}


/* ==============================================================================
 * air_menu 英文布局基准（对应 PAGE_AIR_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void air_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_AIR_MENU) return;
    air_menu_t *pg = air_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_135: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_135, 24, 118);

    /* image_136: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_136, 645, 118);

    /* label_289: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_289, 300, 137);
    lv_obj_set_size(pg->label_289, 64, 32);

    /* temp: 标签 | "230" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_291: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_291, 372, 281);
    lv_obj_set_size(pg->label_291, 38, 32);

    /* label_292: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_292, 891, 137);
    lv_obj_set_size(pg->label_292, 125, 32);

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 793, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_294: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_294, 872, 280);
    lv_obj_set_size(pg->label_294, 38, 32);

    /* label_295: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_295, 1075, 281);
    lv_obj_set_size(pg->label_295, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_297: 标签 | "空气炸" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_297, 24, 25);
    lv_obj_set_size(pg->label_297, 144, 32);

    /* label_298: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_298, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * air_set 英文布局基准（对应 PAGE_AIR_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void air_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_AIR_SET) return;
    air_set_t *pg = air_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_299: 标签 | "空气炸" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_299, 24, 25);
    lv_obj_set_size(pg->label_299, 144, 32);

    /* label_300: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_300, 165, 157);
    lv_obj_set_size(pg->label_300, 26, 32);

    /* label_301: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_301, 252, 157);
    lv_obj_set_size(pg->label_301, 64, 32);

    /* label_302: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_302, 141, 270);
    lv_obj_set_size(pg->label_302, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_305: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_305, 368, 270);
    lv_obj_set_size(pg->label_305, 30, 32);

    /* label_306: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_306, 64, 32);

    /* image_141: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_141, 164, 376);

    /* label_307: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_307, 232, 385);
    lv_obj_set_size(pg->label_307, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_308: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_308, 830, 154);
    lv_obj_set_size(pg->label_308, 130, 32);

    /* label_309: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_309, 840, 270);
    lv_obj_set_size(pg->label_309, 26, 32);

    /* label_310: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_310, 930, 270);
    lv_obj_set_size(pg->label_310, 26, 32);

    /* label_311: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_311, 831, 381);
    lv_obj_set_size(pg->label_311, 130, 32);

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_313: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_313, 1010, 39);
    lv_obj_set_size(pg->label_313, 85, 36);

    /* onpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat, 983, 144);
    lv_obj_set_size(pg->onpreheat, 135, 53);

    /* ondelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 260);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 372);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * air_setting 英文布局基准（对应 PAGE_AIR_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void air_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_AIR_SETTING) return;
    air_setting_t *pg = air_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_15: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_15, 0, 0);
    lv_obj_set_size(pg->container_15, 1280, 480);

    /* image_144: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_144, 115, 115);

    /* label_320: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_320, 242, 136);
    lv_obj_set_size(pg->label_320, 73, 30);

    /* label_321: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_321, 454, 136);
    lv_obj_set_size(pg->label_321, 135, 30);

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 213);
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 300, 229);
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 374, 213);
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_326: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_326, 449, 229);
    lv_obj_set_size(pg->label_326, 37, 30);

    /* label_327: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_327, 612, 229);
    lv_obj_set_size(pg->label_327, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 288, 229);
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * air_stop 英文布局基准（对应 PAGE_AIR_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void air_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_AIR_STOP) return;
    air_stop_t *pg = air_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 空气炸 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_149: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_149, 115, 320);

    /* bar_21: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_21, 122, 326);
    lv_obj_set_size(pg->bar_21, 637, 20);

    /* label_331: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_331, 273, 157);
    lv_obj_set_size(pg->label_331, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_150: 图片 | (115,161) | img: airicon.png */
    lv_obj_set_pos(pg->image_150, 115, 161);

}


/* ==============================================================================
 * air_stop_back 英文布局基准（对应 PAGE_AIR_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void air_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_AIR_STOP_BACK) return;
    air_stop_back_t *pg = air_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_151: 图片 | (115,161) | img: airicon.png */
    lv_obj_set_pos(pg->image_151, 115, 161);

    /* status: 标签 | "| 空气炸 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_152: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_152, 115, 320);

    /* bar_22: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_22, 122, 326);
    lv_obj_set_size(pg->bar_22, 637, 20);

    /* label_333: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_333, 273, 157);
    lv_obj_set_size(pg->label_333, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_16: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_16, 0, 0);
    lv_obj_set_size(pg->container_16, 1280, 480);

    /* label_334: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_334, 895, 160);
    lv_obj_set_size(pg->label_334, 252, 36);

    /* label_335: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_335, 895, 198);
    lv_obj_set_size(pg->label_335, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * bottom_bbq_complete 英文布局基准（对应 PAGE_BOTTOM_BBQ_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bottom_bbq_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BOTTOM_BBQ_COMPLETE) return;
    bottom_bbq_complete_t *pg = bottom_bbq_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_74: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_74, 115, 320);

    /* status: 标签 | "| 底部烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_13: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_13, 122, 326);
    lv_obj_set_size(pg->bar_13, 637, 20);

    /* label_156: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_156, 273, 157);
    lv_obj_set_size(pg->label_156, 330, 60);   /* 对齐 updown bbq 系 */

    /* button_43: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->button_43, 609, 170);
    lv_obj_set_size(pg->button_43, 50, 43);

    /* image_76: 图片 | (115,161) | img: dwbbqicon.png */
    lv_obj_set_pos(pg->image_76, 115, 161);

    /* image_2: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_2, 902, 160);

}


/* ==============================================================================
 * bottom_bbq_complete_probe 英文布局基准（对应 PAGE_BOTTOM_BBQ_COMPLETE_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bottom_bbq_complete_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BOTTOM_BBQ_COMPLETE_PROBE) return;
    bottom_bbq_complete_probe_t *pg = bottom_bbq_complete_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_31: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_31, 115, 320);

    /* status: 标签 | "| 底部烧烤 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_8: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_8, 122, 326);
    lv_obj_set_size(pg->bar_8, 637, 20);

    /* image_32: 图片 | (115,161) | img: dwbbqicon.png */
    lv_obj_set_pos(pg->image_32, 115, 161);

    /* label_76: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_76, 273, 157);
    lv_obj_set_size(pg->label_76, 235, 60);

    /* label_77: 标签 | "高温防烫" | (896,161) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_77, 896, 161);
    lv_obj_set_size(pg->label_77, 252, 36);

    /* label_78: 标签 | "请缓慢打开门体！" | (896,197) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_78, 896, 197);
    lv_obj_set_size(pg->label_78, 252, 36);

}


/* ==============================================================================
 * bottom_bbq_cooking 英文布局基准（对应 PAGE_BOTTOM_BBQ_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bottom_bbq_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BOTTOM_BBQ_COOKING) return;
    bottom_bbq_cooking_t *pg = bottom_bbq_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 底部烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_63: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_63, 115, 320);

    /* bar_10: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_10, 122, 326);
    lv_obj_set_size(pg->bar_10, 637, 20);

    /* label_137: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_137, 273, 157);
    lv_obj_set_size(pg->label_137, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* littlebutton: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->littlebutton, 609, 170);
    lv_obj_set_size(pg->littlebutton, 50, 43);

    /* image_64: 图片 | (115,161) | img: dwbbqicon.png */
    lv_obj_set_pos(pg->image_64, 115, 161);

}


/* ==============================================================================
 * bottom_bbq_cooking_probe 英文布局基准（对应 PAGE_BOTTOM_BBQ_COOKING_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bottom_bbq_cooking_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BOTTOM_BBQ_COOKING_PROBE) return;
    bottom_bbq_cooking_probe_t *pg = bottom_bbq_cooking_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 底部烧烤 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* temp: 标签 | "36℃" | (919,156) | 134x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->temp, 919, 156);
    lv_obj_set_size(pg->temp, 134, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_23: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_23, 115, 320);

    /* bar_5: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_5, 122, 326);
    lv_obj_set_size(pg->bar_5, 637, 20);

    /* image_24: 图片 | (115,161) | img: dwbbqicon.png */
    lv_obj_set_pos(pg->image_24, 115, 161);

    /* label_67: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_67, 273, 157);
    lv_obj_set_size(pg->label_67, 235, 60);

    /* image_25: 图片 | (1068,180) | img: probeicon.png */
    lv_obj_set_pos(pg->image_25, 1068, 180);

}


/* ==============================================================================
 * bottom_bbq_menu 英文布局基准（对应 PAGE_BOTTOM_BBQ_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bottom_bbq_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BOTTOM_BBQ_MENU) return;
    bottom_bbq_menu_t *pg = bottom_bbq_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_56: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_56, 24, 118);

    /* image_57: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_57, 645, 118);

    /* label_109: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_109, 275, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_109, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* temp: 标签 | "180" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 192, 249);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->temp, 180, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_111: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_111, 372, 281);
    lv_obj_set_size(pg->label_111, 38, 32);

    /* label_112: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_112, 855, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_112, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 788, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->hour, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_114: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_114, 872, 280);
    lv_obj_set_size(pg->label_114, 38, 32);

    /* label_115: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_115, 1075, 281);
    lv_obj_set_size(pg->label_115, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_pos(pg->min, 990, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->min, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_117: 标签 | "底部烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_117, 24, 24);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_117, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_118: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_118, 1007, 41);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_118, 117, 32);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * bottom_bbq_menu_probe 英文布局基准（对应 PAGE_BOTTOM_BBQ_MENU_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bottom_bbq_menu_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BOTTOM_BBQ_MENU_PROBE) return;
    bottom_bbq_menu_probe_t *pg = bottom_bbq_menu_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_17: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_17, 24, 118);

    /* image_18: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_18, 645, 118);

    /* label_46: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_46, 300, 137);
    lv_obj_set_size(pg->label_46, 64, 32);

    /* temp: 标签 | "180" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_48: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_48, 372, 281);
    lv_obj_set_size(pg->label_48, 38, 32);

    /* label_49: 标签 | "探针温度" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_49, 891, 137);
    lv_obj_set_size(pg->label_49, 125, 32);

    /* label_50: 标签 | "底部烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_50, 24, 25);
    lv_obj_set_size(pg->label_50, 144, 32);

    /* label_51: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_51, 996, 39);
    lv_obj_set_size(pg->label_51, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* probetemp: 标签 | "80" | (853,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->probetemp, 853, 249);
    lv_obj_set_size(pg->probetemp, 120, 72);

    /* label_53: 标签 | "℃" | (973,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_53, 973, 281);
    lv_obj_set_size(pg->label_53, 38, 32);

    /* probetempline: 图片 | (891,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->probetempline, 891, 328);

}


/* ==============================================================================
 * bottom_bbq_set 英文布局基准（对应 PAGE_BOTTOM_BBQ_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bottom_bbq_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BOTTOM_BBQ_SET) return;
    bottom_bbq_set_t *pg = bottom_bbq_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_119: 标签 | "底部烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_119, 24, 25);
    lv_obj_set_size(pg->label_119, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_120: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_120, 163, 155);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_120, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_121: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_121, 252, 157);
    lv_obj_set_size(pg->label_121, 64, 32);

    /* label_122: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_122, 150, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_122, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->min, 317, 258);
    lv_obj_set_size(pg->min, 68, 66);   /* 对齐 updown 系英文排版(A) */

    /* label_125: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_125, 368, 270);
    lv_obj_set_size(pg->label_125, 30, 32);

    /* label_126: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->label_126, 371, 271);
    lv_obj_set_size(pg->label_126, 42, 32);   /* 对齐 updown 系英文排版(A) */

    /* image_62: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_62, 164, 376);

    /* label_127: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_127, 232, 385);
    lv_obj_set_size(pg->label_127, 220, 32);   /* 对齐 updown 系英文排版(A) */

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_128: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_128, 785, 154);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_128, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_129: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_129, 885, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_129, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_130: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_130, 930, 270);
    lv_obj_set_size(pg->label_130, 26, 32);

    /* label_131: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_131, 765, 381);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_131, 200, 32);   /* 对齐 updown 系英文排版(A) */

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 269, 143);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->temp, 100, 53);   /* 对齐 top_bbq 系排版 */

    /* label_133: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_133, 1007, 41);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_133, 85, 36);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */

    /* onpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat, 983, 144);
    lv_obj_set_size(pg->onpreheat, 135, 53);

    /* ondelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 260);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 372);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 355, 157);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 327, 157);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * bottom_bbq_set_probe 英文布局基准（对应 PAGE_BOTTOM_BBQ_SET_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bottom_bbq_set_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BOTTOM_BBQ_SET_PROBE) return;
    bottom_bbq_set_probe_t *pg = bottom_bbq_set_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_54: 标签 | "底部烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_54, 24, 25);
    lv_obj_set_size(pg->label_54, 144, 32);

    /* label_55: 标签 | "温" | (165,155) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_55, 165, 155);
    lv_obj_set_size(pg->label_55, 26, 32);

    /* label_56: 标签 | "度：" | (252,155) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_56, 252, 155);
    lv_obj_set_size(pg->label_56, 64, 32);

    /* label_57: 标签 | "探针温度：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_57, 141, 270);
    lv_obj_set_size(pg->label_57, 175, 32);

    /* image_22: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_22, 164, 376);

    /* label_58: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_58, 232, 385);
    lv_obj_set_size(pg->label_58, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_59: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_59, 85, 36);

    /* probetemp: 标签 | "80" | (315,254) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->probetemp, 315, 254);
    lv_obj_set_size(pg->probetemp, 83, 53);

    /* label_61: 标签 | "℃" | (369,270) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_61, 369, 270);
    lv_obj_set_size(pg->label_61, 32, 30);

    /* temp: 标签 | "200" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* image_4: 图片 | (839,154) | img: delaytext.png */
    lv_obj_set_pos(pg->image_4, 839, 154);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

}


/* ==============================================================================
 * bottom_bbq_setting 英文布局基准（对应 PAGE_BOTTOM_BBQ_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bottom_bbq_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BOTTOM_BBQ_SETTING) return;
    bottom_bbq_setting_t *pg = bottom_bbq_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 257, 60);   /* 对齐 top_bbq 系排版 */

    /* container_6: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_6, 0, 0);
    lv_obj_set_size(pg->container_6, 1280, 480);

    /* image_66: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_66, 115, 115);

    /* label_139: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_139, 203, 134);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_139, 150, 30);   /* 对齐 top_bbq 系排版 */

    /* label_140: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_140, 444, 134);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_140, 150, 30);   /* 对齐 top_bbq 系排版 */

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 217);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 303, 229);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 377, 217);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_145: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_145, 450, 231);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_145, 37, 30);

    /* label_146: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_146, 612, 231);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_146, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 289, 229);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon2, 37, 30);

    /* surebutton: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->surebutton, 959, 295);
    lv_obj_set_size(pg->surebutton, 129, 83);

}


/* ==============================================================================
 * bottom_bbq_stop 英文布局基准（对应 PAGE_BOTTOM_BBQ_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bottom_bbq_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BOTTOM_BBQ_STOP) return;
    bottom_bbq_stop_t *pg = bottom_bbq_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 底部烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_70: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_70, 115, 320);

    /* bar_11: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_11, 122, 326);
    lv_obj_set_size(pg->bar_11, 637, 20);

    /* label_150: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_150, 273, 157);
    lv_obj_set_size(pg->label_150, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* littlebutton: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->littlebutton, 609, 170);
    lv_obj_set_size(pg->littlebutton, 50, 43);

    /* image_71: 图片 | (115,161) | img: dwbbqicon.png */
    lv_obj_set_pos(pg->image_71, 115, 161);

}


/* ==============================================================================
 * bottom_bbq_stop_back 英文布局基准（对应 PAGE_BOTTOM_BBQ_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bottom_bbq_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BOTTOM_BBQ_STOP_BACK) return;
    bottom_bbq_stop_back_t *pg = bottom_bbq_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_72: 图片 | (115,161) | img: dwbbqicon.png */
    lv_obj_set_pos(pg->image_72, 115, 161);

    /* status: 标签 | "| 底部烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_73: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_73, 115, 320);

    /* bar_12: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_12, 122, 326);
    lv_obj_set_size(pg->bar_12, 637, 20);

    /* label_152: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_152, 273, 157);
    lv_obj_set_size(pg->label_152, 330, 60);   /* 对齐 updown bbq 系 */   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_7: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_7, 0, 0);
    lv_obj_set_size(pg->container_7, 1280, 480);

    /* label_153: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_153, 895, 160);
    lv_obj_set_size(pg->label_153, 252, 36);

    /* label_154: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_154, 895, 198);
    lv_obj_set_size(pg->label_154, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * bottom_bbq_stop_back_probe 英文布局基准（对应 PAGE_BOTTOM_BBQ_STOP_BACK_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bottom_bbq_stop_back_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BOTTOM_BBQ_STOP_BACK_PROBE) return;
    bottom_bbq_stop_back_probe_t *pg = bottom_bbq_stop_back_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 底部烧烤 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_29: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_29, 115, 320);

    /* bar_7: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_7, 122, 326);
    lv_obj_set_size(pg->bar_7, 637, 20);

    /* image_30: 图片 | (115,161) | img: dwbbqicon.png */
    lv_obj_set_pos(pg->image_30, 115, 161);

    /* label_72: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_72, 273, 157);
    lv_obj_set_size(pg->label_72, 235, 60);

    /* container_2: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_2, 0, 0);
    lv_obj_set_size(pg->container_2, 1280, 480);

    /* label_73: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_73, 895, 160);
    lv_obj_set_size(pg->label_73, 252, 36);

    /* label_74: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_74, 895, 198);
    lv_obj_set_size(pg->label_74, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * bottom_bbq_stop_probe 英文布局基准（对应 PAGE_BOTTOM_BBQ_STOP_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bottom_bbq_stop_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BOTTOM_BBQ_STOP_PROBE) return;
    bottom_bbq_stop_probe_t *pg = bottom_bbq_stop_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_68: 标签 | "| 底部烧烤 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_68, 274, 232);
    lv_obj_set_size(pg->label_68, 490, 39);

    /* temp: 标签 | "36℃" | (919,156) | 134x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->temp, 919, 156);
    lv_obj_set_size(pg->temp, 134, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_26: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_26, 115, 320);

    /* bar_6: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_6, 122, 326);
    lv_obj_set_size(pg->bar_6, 637, 20);

    /* image_27: 图片 | (115,161) | img: dwbbqicon.png */
    lv_obj_set_pos(pg->image_27, 115, 161);

    /* label_70: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_70, 273, 157);
    lv_obj_set_size(pg->label_70, 235, 60);

    /* image_28: 图片 | (1068,180) | img: probeicon.png */
    lv_obj_set_pos(pg->image_28, 1068, 180);

}


/* ==============================================================================
 * bread6menu 英文布局基准（对应 PAGE_BREAD6MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bread6menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BREAD6MENU) return;
    bread6menu_t *pg = bread6menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_11: 标签 | "面包" | (24,24) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_11, 24, 24);
    lv_obj_set_size(pg->label_11, 144, 32);

    /* breadroll: 按钮 | (5,65) | 340x406 | font taiwanpearl_regular_30 | bg: div4bg30.png */
    lv_obj_set_pos(pg->breadroll, 5, 65);
    lv_obj_set_size(pg->breadroll, 340, 406);

    /* wheat_bread: 按钮 | (315,65) | 340x406 | font taiwanpearl_regular_30 | bg: div4bg30.png */
    lv_obj_set_pos(pg->wheat_bread, 315, 65);
    lv_obj_set_size(pg->wheat_bread, 340, 406);

    /* toast: 按钮 | (625,65) | 340x406 | font taiwanpearl_regular_30 | bg: div4bg30.png */
    lv_obj_set_pos(pg->toast, 625, 65);
    lv_obj_set_size(pg->toast, 340, 406);

    /* croissant: 按钮 | (936,65) | 340x406 | font taiwanpearl_regular_30 | bg: div4bg30.png */
    lv_obj_set_pos(pg->croissant, 936, 65);
    lv_obj_set_size(pg->croissant, 340, 406);

    /* ---- 场景分支: 四栏布局相同,仅标题与栏内文字不同 ----
       面包(默认): 第六感→面包(面包卷/全麦面包/土司/可颂)
       肉菜单:     第六感→肉(非探针; 牛肉/羊肉/猪肉/肉菜), label_11="肉" ---- */
    if (six_menu_is_meat_mode()) {
        /* 肉菜单: 布局同基准,label 文字不同 */
        /* TODO: 英文实测后调整(label_11 "Meat"/栏内 Roast Leg of Lamb 等宽度) */
    }

}


/* ==============================================================================
 * bread_complete 英文布局基准（对应 PAGE_BREAD_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bread_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BREAD_COMPLETE) return;
    bread_complete_t *pg = bread_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_353: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_353, 115, 320);

    /* status: 标签 | "| 面包 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_59: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_59, 122, 326);
    lv_obj_set_size(pg->bar_59, 637, 20);

    /* label_763: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_763, 273, 157);
    lv_obj_set_size(pg->label_763, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_355: 图片 | (115,161) | img: breadicon_1.png */
    lv_obj_set_pos(pg->image_355, 115, 161);

    /* image_14: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_14, 902, 160);

}


/* ==============================================================================
 * bread_cooking 英文布局基准（对应 PAGE_BREAD_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bread_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BREAD_COOKING) return;
    bread_cooking_t *pg = bread_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 面包 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_344: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_344, 115, 320);

    /* bar_56: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_56, 122, 326);
    lv_obj_set_size(pg->bar_56, 637, 20);

    /* label_748: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_748, 273, 157);
    lv_obj_set_size(pg->label_748, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_345: 图片 | (115,161) | img: breadicon_1.png */
    lv_obj_set_pos(pg->image_345, 115, 161);

}


/* ==============================================================================
 * bread_menu 英文布局基准（对应 PAGE_BREAD_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bread_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BREAD_MENU) return;
    bread_menu_t *pg = bread_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_340: 图片 | (24,117) | img: div1bg.png */
    lv_obj_set_pos(pg->image_340, 24, 117);

    /* label_723: 标签 | "烹饪时间" | (578,139) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_723, 578, 139);
    lv_obj_set_size(pg->label_723, 125, 32);

    /* hour: 标签 | "00" | (482,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 482, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_725: 标签 | "时" | (562,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_725, 562, 280);
    lv_obj_set_size(pg->label_725, 38, 32);

    /* label_726: 标签 | "分" | (762,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_726, 762, 280);
    lv_obj_set_size(pg->label_726, 38, 32);

    /* min: 标签 | "30" | (682,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_728: 标签 | "面包" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_728, 24, 25);
    lv_obj_set_size(pg->label_728, 144, 32);

    /* label_729: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_729, 996, 39);
    lv_obj_set_size(pg->label_729, 117, 32);

    /* hourline: 图片 | (480,327) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 480, 327);

    /* minline: 图片 | (680,327) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 680, 327);

}


/* ==============================================================================
 * bread_set 英文布局基准（对应 PAGE_BREAD_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bread_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BREAD_SET) return;
    bread_set_t *pg = bread_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_730: 标签 | "面包" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_730, 24, 25);
    lv_obj_set_size(pg->label_730, 144, 32);

    /* label_731: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_731, 165, 157);
    lv_obj_set_size(pg->label_731, 26, 32);

    /* label_732: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_732, 252, 157);
    lv_obj_set_size(pg->label_732, 64, 32);

    /* label_733: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_733, 141, 270);
    lv_obj_set_size(pg->label_733, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_736: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_736, 368, 270);
    lv_obj_set_size(pg->label_736, 30, 32);

    /* label_737: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_737, 64, 32);

    /* image_343: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_343, 164, 376);

    /* label_738: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_738, 232, 385);
    lv_obj_set_size(pg->label_738, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_739: 标签 | "延" | (840,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_739, 840, 154);
    lv_obj_set_size(pg->label_739, 26, 32);

    /* label_740: 标签 | "迟" | (930,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_740, 930, 154);
    lv_obj_set_size(pg->label_740, 26, 32);

    /* label_741: 标签 | "自动保温" | (831,270) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_741, 831, 270);
    lv_obj_set_size(pg->label_741, 130, 32);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 260);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "200" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_743: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_743, 1010, 39);
    lv_obj_set_size(pg->label_743, 85, 36);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 260);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * bread_setting 英文布局基准（对应 PAGE_BREAD_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bread_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BREAD_SETTING) return;
    bread_setting_t *pg = bread_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_33: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_33, 0, 0);
    lv_obj_set_size(pg->container_33, 1280, 480);

    /* image_346: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_346, 115, 115);

    /* label_750: 标签 | "烹饪时间" | (373,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_750, 373, 136);
    lv_obj_set_size(pg->label_750, 135, 30);

    /* hour: 标签 | "00" | (266,212) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 266, 212);
    lv_obj_set_size(pg->hour, 102, 46);

    /* label_752: 标签 | "时" | (340,228) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_752, 340, 228);
    lv_obj_set_size(pg->label_752, 37, 30);

    /* hourline: 图片 | (291,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 291, 269);

    /* min: 标签 | "00" | (483,212) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_754: 标签 | "分" | (558,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_754, 558, 229);
    lv_obj_set_size(pg->label_754, 37, 30);

    /* minline: 图片 | (508,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 508, 269);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * bread_stop 英文布局基准（对应 PAGE_BREAD_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bread_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BREAD_STOP) return;
    bread_stop_t *pg = bread_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 面包 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_349: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_349, 115, 320);

    /* bar_57: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_57, 122, 326);
    lv_obj_set_size(pg->bar_57, 637, 20);

    /* label_757: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_757, 273, 157);
    lv_obj_set_size(pg->label_757, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_350: 图片 | (115,161) | img: breadicon_1.png */
    lv_obj_set_pos(pg->image_350, 115, 161);

}


/* ==============================================================================
 * bread_stop_back 英文布局基准（对应 PAGE_BREAD_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void bread_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_BREAD_STOP_BACK) return;
    bread_stop_back_t *pg = bread_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_351: 图片 | (115,161) | img: breadicon_1.png */
    lv_obj_set_pos(pg->image_351, 115, 161);

    /* status: 标签 | "| 面包 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_352: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_352, 115, 320);

    /* bar_58: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_58, 122, 326);
    lv_obj_set_size(pg->bar_58, 637, 20);

    /* label_759: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_759, 273, 157);
    lv_obj_set_size(pg->label_759, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_34: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_34, 0, 0);
    lv_obj_set_size(pg->container_34, 1280, 480);

    /* label_760: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_760, 895, 160);
    lv_obj_set_size(pg->label_760, 252, 36);

    /* label_761: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_761, 895, 198);
    lv_obj_set_size(pg->label_761, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * cake6menu 英文布局基准（对应 PAGE_CAKE6MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void cake6menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CAKE6MENU) return;
    cake6menu_t *pg = cake6menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_1: 标签 | "蛋糕" | (24,24) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_1, 24, 24);
    lv_obj_set_size(pg->label_1, 144, 32);

    /* swissroll: 按钮 | (22,79) | 203x378 | font taiwanpearl_regular_30 | bg: div11_1_30.png */
    lv_obj_set_pos(pg->swissroll, 22, 79);
    lv_obj_set_size(pg->swissroll, 203, 378);

    /* strudel: 按钮 | (228,77) | 205x191 | font taiwanpearl_regular_30 | bg: div11_2_30.png */
    lv_obj_set_pos(pg->strudel, 228, 77);
    lv_obj_set_size(pg->strudel, 205, 191);

    /* muffin: 按钮 | (228,270) | 205x191 | font taiwanpearl_regular_30 | bg: div11_2_30.png */
    lv_obj_set_pos(pg->muffin, 228, 270);
    lv_obj_set_size(pg->muffin, 205, 191);

    /* madeleine: 按钮 | (435,77) | 205x191 | font taiwanpearl_regular_30 | bg: div11_2_30.png */
    lv_obj_set_pos(pg->madeleine, 435, 77);
    lv_obj_set_size(pg->madeleine, 205, 191);

    /* spongecake: 按钮 | (642,77) | 205x191 | font taiwanpearl_regular_30 | bg: div11_2_30.png */
    lv_obj_set_pos(pg->spongecake, 642, 77);
    lv_obj_set_size(pg->spongecake, 205, 191);

    /* cupcake: 按钮 | (849,77) | 205x191 | font taiwanpearl_regular_30 | bg: div11_2_30.png */
    lv_obj_set_pos(pg->cupcake, 849, 77);
    lv_obj_set_size(pg->cupcake, 205, 191);

    /* chocolatecake: 按钮 | (1056,77) | 205x191 | font taiwanpearl_regular_30 | bg: div11_2_30.png */
    lv_obj_set_pos(pg->chocolatecake, 1056, 77);
    lv_obj_set_size(pg->chocolatecake, 205, 191);

    /* creampuff: 按钮 | (435,270) | 205x191 | font taiwanpearl_regular_30 | bg: div11_2_30.png */
    lv_obj_set_pos(pg->creampuff, 435, 270);
    lv_obj_set_size(pg->creampuff, 205, 191);

    /* eggtart: 按钮 | (642,270) | 205x191 | font taiwanpearl_regular_30 | bg: div11_2_30.png */
    lv_obj_set_pos(pg->eggtart, 642, 270);
    lv_obj_set_size(pg->eggtart, 205, 191);

    /* millefeuille: 按钮 | (849,270) | 205x191 | font taiwanpearl_regular_30 | bg: div11_2_30.png */
    lv_obj_set_pos(pg->millefeuille, 849, 270);
    lv_obj_set_size(pg->millefeuille, 205, 191);

    /* cookies: 按钮 | (1056,270) | 205x191 | font taiwanpearl_regular_30 | bg: div11_2_30.png */
    lv_obj_set_pos(pg->cookies, 1056, 270);
    lv_obj_set_size(pg->cookies, 205, 191);

     /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ En modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     lv_obj_t *obj = pg->chocolatecake;
     obj = lv_obj_get_child(obj, 0);
     lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
}


/* ==============================================================================
 * central_bbq_complete 英文布局基准（对应 PAGE_CENTRAL_BBQ_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void central_bbq_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CENTRAL_BBQ_COMPLETE) return;
    central_bbq_complete_t *pg = central_bbq_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_143: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_143, 115, 320);

    /* status: 标签 | "| 集中烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_29: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_29, 122, 326);
    lv_obj_set_size(pg->bar_29, 637, 20);

    /* label_318: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_318, 273, 157);
    lv_obj_set_size(pg->label_318, 330, 60);   /* 对齐 updown bbq 系 */

    /* button_104: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->button_104, 609, 170);
    lv_obj_set_size(pg->button_104, 50, 43);

    /* image_145: 图片 | (115,161) | img: centralbbqicon.png */
    lv_obj_set_pos(pg->image_145, 115, 161);

    /* image_5: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_5, 902, 160);

}


/* ==============================================================================
 * central_bbq_cooking 英文布局基准（对应 PAGE_CENTRAL_BBQ_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void central_bbq_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CENTRAL_BBQ_COOKING) return;
    central_bbq_cooking_t *pg = central_bbq_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 集中烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_137: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_137, 115, 320);

    /* bar_26: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_26, 122, 326);
    lv_obj_set_size(pg->bar_26, 637, 20);

    /* label_309: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_309, 273, 157);
    lv_obj_set_size(pg->label_309, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_138: 图片 | (115,161) | img: centralbbqicon.png */
    lv_obj_set_pos(pg->image_138, 115, 161);

}


/* ==============================================================================
 * central_bbq_menu 英文布局基准（对应 PAGE_CENTRAL_BBQ_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void central_bbq_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CENTRAL_BBQ_MENU) return;
    central_bbq_menu_t *pg = central_bbq_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_130: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_130, 24, 118);

    /* image_131: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_131, 645, 118);

    /* label_281: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_281, 275, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_281, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* temp: 标签 | "180" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 192, 249);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->temp, 180, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_283: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_283, 372, 281);
    lv_obj_set_size(pg->label_283, 38, 32);

    /* label_284: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_284, 855, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_284, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 788, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->hour, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_286: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_286, 872, 280);
    lv_obj_set_size(pg->label_286, 38, 32);

    /* label_287: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_287, 1075, 281);
    lv_obj_set_size(pg->label_287, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_pos(pg->min, 990, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->min, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_289: 标签 | "集中烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_289, 24, 24);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_289, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_290: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_290, 1007, 41);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_290, 117, 32);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * central_bbq_set 英文布局基准（对应 PAGE_CENTRAL_BBQ_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void central_bbq_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CENTRAL_BBQ_SET) return;
    central_bbq_set_t *pg = central_bbq_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_291: 标签 | "集中烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_291, 24, 25);
    lv_obj_set_size(pg->label_291, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_292: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_292, 163, 155);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_292, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_293: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_293, 252, 157);
    lv_obj_set_size(pg->label_293, 64, 32);

    /* label_294: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_294, 150, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_294, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->min, 317, 258);
    lv_obj_set_size(pg->min, 68, 66);   /* 对齐 updown 系英文排版(A) */

    /* label_297: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_297, 368, 270);
    lv_obj_set_size(pg->label_297, 30, 32);

    /* label_298: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->label_298, 371, 271);
    lv_obj_set_size(pg->label_298, 42, 32);   /* 对齐 updown 系英文排版(A) */

    /* image_136: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_136, 164, 376);

    /* label_299: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_299, 232, 385);
    lv_obj_set_size(pg->label_299, 220, 32);   /* 对齐 updown 系英文排版(A) */

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_300: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_300, 785, 154);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_300, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_301: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_301, 885, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_301, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_302: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_302, 930, 270);
    lv_obj_set_size(pg->label_302, 26, 32);

    /* label_303: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_303, 765, 381);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_303, 200, 32);   /* 对齐 updown 系英文排版(A) */

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 269, 143);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->temp, 100, 53);   /* 对齐 top_bbq 系排版 */

    /* label_305: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_305, 1007, 41);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_305, 85, 36);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */

    /* onpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat, 983, 144);
    lv_obj_set_size(pg->onpreheat, 135, 53);

    /* ondelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 260);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 372);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 355, 157);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 327, 157);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * central_bbq_setting 英文布局基准（对应 PAGE_CENTRAL_BBQ_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void central_bbq_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CENTRAL_BBQ_SETTING) return;
    central_bbq_setting_t *pg = central_bbq_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 257, 60);   /* 对齐 top_bbq 系排版 */

    /* container_3: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_3, 0, 0);
    lv_obj_set_size(pg->container_3, 1280, 480);

    /* image_11: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_11, 115, 115);

    /* label_22: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_22, 203, 134);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_22, 150, 30);   /* 对齐 top_bbq 系排版 */

    /* label_23: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_23, 444, 134);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_23, 150, 30);   /* 对齐 top_bbq 系排版 */

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 217);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 303, 229);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 377, 217);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_28: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_28, 450, 231);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_28, 37, 30);

    /* label_29: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_29, 612, 231);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_29, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 289, 229);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * central_bbq_stop 英文布局基准（对应 PAGE_CENTRAL_BBQ_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void central_bbq_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CENTRAL_BBQ_STOP) return;
    central_bbq_stop_t *pg = central_bbq_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 集中烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_139: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_139, 115, 320);

    /* bar_27: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_27, 122, 326);
    lv_obj_set_size(pg->bar_27, 637, 20);

    /* label_312: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_312, 273, 157);
    lv_obj_set_size(pg->label_312, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_140: 图片 | (115,161) | img: centralbbqicon.png */
    lv_obj_set_pos(pg->image_140, 115, 161);

}


/* ==============================================================================
 * central_bbq_stop_back 英文布局基准（对应 PAGE_CENTRAL_BBQ_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void central_bbq_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CENTRAL_BBQ_STOP_BACK) return;
    central_bbq_stop_back_t *pg = central_bbq_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_141: 图片 | (115,161) | img: centralbbqicon.png */
    lv_obj_set_pos(pg->image_141, 115, 161);

    /* status: 标签 | "| 集中烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_142: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_142, 115, 320);

    /* bar_28: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_28, 122, 326);
    lv_obj_set_size(pg->bar_28, 637, 20);

    /* label_314: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_314, 273, 157);
    lv_obj_set_size(pg->label_314, 330, 60);   /* 对齐 updown bbq 系 */   /* 对齐 updown 系英文排版(A) */

    /* button_102: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->button_102, 609, 170);
    lv_obj_set_size(pg->button_102, 50, 43);

    /* container_12: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_12, 0, 0);
    lv_obj_set_size(pg->container_12, 1280, 480);

    /* label_315: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_315, 895, 160);
    lv_obj_set_size(pg->label_315, 252, 36);

    /* label_316: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_316, 895, 198);
    lv_obj_set_size(pg->label_316, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * chick6menu 英文布局基准（对应 PAGE_CHICK6MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void chick6menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CHICK6MENU) return;
    chick6menu_t *pg = chick6menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_1: 标签 | "家禽" | (24,24) | 118x25 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_1, 24, 24);
    lv_obj_set_size(pg->label_1, 118, 25);

    /* chicken: 按钮 | (12,75) | 638x387 | font taiwanpearl_regular_30 | bg: div230bg.png */
    lv_obj_set_pos(pg->chicken, 12, 75);
    lv_obj_set_size(pg->chicken, 638, 387);

    /* duck: 按钮 | (633,74) | 637x389 | font taiwanpearl_regular_30 | bg: div230bg.png */
    lv_obj_set_pos(pg->duck, 633, 74);
    lv_obj_set_size(pg->duck, 637, 389);

    /* ---- 场景分支: 两栏布局相同,仅 label_1 标题与两栏文字不同(运行时设置) ----
       家禽首页(默认): 第六感→家禽(烤全鸡/烤全鸭)
       鱼/海鲜首页(fish=1): 第六感→鱼/海鲜(烤鱼/烤海鲜)
       烤鱼子页(fish=2):   鱼/海鲜→烤鱼(烤鳕鱼/烤全鱼)
       蔬菜/配菜首页:       第六感→蔬菜/配菜
       砂锅菜/烤意面首页:   第六感→意面(千层面/卡内罗尼) ---- */
    if (six_chick_get_vegetable_mode() || six_chick_get_pasta_mode()) {
        /* 蔬菜配菜 / 砂锅菜烤意面: 标题文字较长("蔬菜/配菜"/"砂锅菜/烤意面") */
        /* TODO: 英文实测后调整(label_1 "Vegetables & Side Dishes" 等宽度) */
    } else if (six_chick_get_fish_mode()) {
        /* 鱼海鲜首页 / 烤鱼子页 */
        /* TODO: 英文实测后按需调整 */
    }
    /* 家禽首页用基准值 */

}


/* ==============================================================================
 * chickencooking 英文布局基准（对应 PAGE_CHICKENCOOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void chickencooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CHICKENCOOKING) return;
    chickencooking_t *pg = chickencooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* ---- 场景分支: 本页为探针烹饪页,8 种探针菜共用 ----
       烤全鸡/烤全鸭:        status "| 菜名 | 浅色/中等色/深色 |"(短)
       烤牛排:               status "| 烤牛排 | 几成熟 |"
       二维菜(牛肉/羊腿/羊排): status "| 菜名 | 颜色 | 成熟度 |"(最长)
       猪里脊/五花肉:         status "| 菜名 | 颜色 |" ---- */
    /* status: 标签 | "| 烤全鸡 | 中等色" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    if (six_chick_is_matdeg()) {
        /* 二维菜文本最长("| 烤猪里脊肉 | 中等色 | 五成熟 |") */
        lv_obj_set_size(pg->status, 490, 39);
        /* TODO: 英文实测后调整(如 "Roasted Pork Tenderloin | Mid | Medium Well" 是否超宽/需缩小字号) */
    } else {
        lv_obj_set_size(pg->status, 490, 39);
    }

    /* image_10: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_10, 115, 320);

    /* bar_2: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_2, 122, 326);
    lv_obj_set_size(pg->bar_2, 637, 20);

    /* cookstatus: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->cookstatus, 273, 157);
    lv_obj_set_size(pg->cookstatus, 235, 60);

    /* icon: 图片 | (163,161) | img: sixicon.png | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */

    /* bartemp: 标签 | "10℃" | (125,323) | 66x24 | font taiwanpearl_regular_24 | 定时器每秒重写位置(tune无效, 用注册表dx/dy) */
    lv_obj_set_size(pg->bartemp, 66, 24);

    /* container_1: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_1, 0, 0);
    lv_obj_set_size(pg->container_1, 1280, 480);

    /* text2: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->text2, 895, 198);
    lv_obj_set_size(pg->text2, 252, 36);

    /* text1: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->text1, 895, 160);
    lv_obj_set_size(pg->text1, 252, 36);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

     /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     lv_obj_t *obj = lv_screen_active();
     //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_time_En.png), 0);
}


/* ==============================================================================
 * chickenmenu 英文布局基准（对应 PAGE_CHICKENMENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void chickenmenu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CHICKENMENU) return;
    chickenmenu_t *pg = chickenmenu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_2: 标签 | "鸡" | (24,24) | 97x25 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_2, 24, 24);
    lv_obj_set_size(pg->label_2, 97, 25);

    /* wholechicken: 按钮 | (19,75) | 248x387 | font taiwanpearl_regular_30 | bg: updown80bk.png */
    lv_obj_set_pos(pg->wholechicken, 14, 75);
    lv_obj_set_size(pg->wholechicken, 425, 387);

    /* grillchickenwing: 按钮 | (267,75) | 248x387 | font taiwanpearl_regular_30 | bg: updown80bk.png */
    lv_obj_set_pos(pg->grillchickenwing, 435, 76);
    lv_obj_set_size(pg->grillchickenwing, 410, 192);

    /* friedchickenwing: 按钮 | (516,75) | 248x387 | font taiwanpearl_regular_30 | bg: updown80bk.png */
    lv_obj_set_pos(pg->friedchickenwing, 849, 76);
    lv_obj_set_size(pg->friedchickenwing, 410, 192);

    /* friedchickenleg: 按钮 | (764,75) | 248x387 | font taiwanpearl_regular_30 | bg: updown80bk.png */
    lv_obj_set_pos(pg->friedchickenleg, 435, 269);
    lv_obj_set_size(pg->friedchickenleg, 410, 192);

    /* grillchickenbreast: 按钮 | (1012,75) | 248x387 | font taiwanpearl_regular_30 | bg: updown80bk.png */
    lv_obj_set_pos(pg->grillchickenbreast, 849, 269);
    lv_obj_set_size(pg->grillchickenbreast, 410, 192);

    /* label_4: 标签 | "烤全鸡" | (66,274) | 158x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_4, 66, 274);
    lv_obj_set_size(pg->label_4, 158, 32);

    /* label_5: 标签 | "烤鸡翅" | (315,274) | 158x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_5, 315, 274);
    lv_obj_set_size(pg->label_5, 158, 32);

    /* label_6: 标签 | "炸鸡中翅" | (562,274) | 158x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_6, 562, 274);
    lv_obj_set_size(pg->label_6, 158, 32);

    /* label_7: 标签 | "炸鸡腿" | (812,274) | 158x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_7, 812, 274);
    lv_obj_set_size(pg->label_7, 158, 32);

    /* label_8: 标签 | "烤鸡胸肉" | (1060,274) | 158x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_8, 1060, 274);
    lv_obj_set_size(pg->label_8, 158, 32);

    /* image_2: 图片 | (192,213) | img: probeiconbig.png */
    lv_obj_set_pos(pg->image_2, 192, 213);

     /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ En modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     lv_obj_t *obj = pg->label_4;
     lv_obj_t *parent = pg->wholechicken;
     lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
     lv_obj_align_to(obj, parent, LV_ALIGN_CENTER, 0, 18);
 
     obj = pg->label_5;
     parent = pg->grillchickenwing;
     lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
     lv_obj_align_to(obj, parent, LV_ALIGN_CENTER, 0, 0);
 
     obj = pg->label_6;
     parent = pg->friedchickenwing;
     lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
     lv_obj_align_to(obj, parent, LV_ALIGN_CENTER, 0, 0);
 
     obj = pg->label_7;
     parent = pg->friedchickenleg;
     lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
     lv_obj_align_to(obj, parent, LV_ALIGN_CENTER, 0, 0);
 
     obj = pg->label_8;
     parent = pg->grillchickenbreast;
     lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
     lv_obj_align_to(obj, parent, LV_ALIGN_CENTER, 0, 0);

    /* ---- 场景分支: 六栏布局相同,仅栏内文字不同(运行时设置) ----
       家禽子页(默认): 第六感→家禽→(烤全鸡/烤鸡翅/炸鸡中翅/炸鸡腿/烤鸡胸肉等)
       海鲜子页:       第六感→鱼/海鲜→烤海鲜(扇贝/青口贝/生蚝/大虾/鱿鱼) ---- */
    if (six_chick_get_seafood_mode()) {
        /* 海鲜子页 */
        /* TODO: 英文实测后调整(Grilled Scallops 等文本宽度) */
    }
}


/* ==============================================================================
 * chickmenutz 英文布局基准（对应 PAGE_CHICKMENUTZ ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void chickmenutz_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CHICKMENUTZ) return;
    chickmenutz_t *pg = chickmenutz_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* wholechicken: 按钮 | (12,75) | 638x387 | font montserratmedium_16 | bg: div2bk30.png */
    lv_obj_set_pos(pg->wholechicken, 12, 75);
    lv_obj_set_size(pg->wholechicken, 638, 387);

    /* label_4: 标签 | "家禽" | (24,24) | 97x25 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_4, 24, 24);
    lv_obj_set_size(pg->label_4, 97, 25);

    /* label_5: 标签 | "烤全鸡" | (265,274) | 131x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_5, 265, 274);
    lv_obj_set_size(pg->label_5, 131, 32);

    /* image_1: 图片 | (296,213) | img: probeiconbig.png */
    lv_obj_set_pos(pg->image_1, 296, 213);

    /* wholeduck: 按钮 | (633,74) | 638x387 | font montserratmedium_16 | bg: div2bk30.png */
    lv_obj_set_pos(pg->wholeduck, 633, 74);
    lv_obj_set_size(pg->wholeduck, 638, 387);

    /* image_2: 图片 | (917,213) | img: probeiconbig.png */
    lv_obj_set_pos(pg->image_2, 917, 213);

    /* label_6: 标签 | "烤全鸭" | (886,274) | 131x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_6, 886, 274);
    lv_obj_set_size(pg->label_6, 131, 32);

     /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     lv_obj_t *obj = NULL;
     lv_obj_t *parent = NULL;
     obj = pg->label_5;
     parent = pg->wholechicken;
     lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
     lv_obj_align_to(obj, parent, LV_ALIGN_CENTER, 0, 18);
 
     obj = pg->label_6;
     parent = pg->wholeduck;
     lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
     lv_obj_align_to(obj, parent, LV_ALIGN_CENTER, 0, 18);

    /* ---- 场景分支: 两栏布局相同,仅文字不同 ----
       家禽(默认): 探针模式→第六感→家禽(烤全鸡/烤全鸭)
       肉二级:     探针模式→第六感→肉→牛肉(烤牛排/烤牛肉) 羊肉(烤羊腿/烤羊排) 猪肉(烤猪里脊肉/烤五花肉)
                   label_4=父类名 label_5/6=菜名,由运行时按 six_tz_get_meat_mode() 设置 ---- */
    if (six_tz_get_meat_mode()) {
        /* 肉二级菜单: 布局同基准,label 文字较长 */
        /* TODO: 英文实测后调整(label_5 "Roasted Pork Tenderloin" 等较长文本的尺寸/居中) */
    }
}


/* ==============================================================================
 * chip_complete 英文布局基准（对应 PAGE_CHIP_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void chip_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CHIP_COMPLETE) return;
    chip_complete_t *pg = chip_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_385: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_385, 115, 320);

    /* status: 标签 | "| 炸薯条 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_67: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_67, 122, 326);
    lv_obj_set_size(pg->bar_67, 637, 20);

    /* label_845: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_845, 273, 157);
    lv_obj_set_size(pg->label_845, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_387: 图片 | (115,161) | img: chipicon.png */
    lv_obj_set_pos(pg->image_387, 115, 161);

    /* image_16: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_16, 902, 160);

}


/* ==============================================================================
 * chip_cooking 英文布局基准（对应 PAGE_CHIP_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void chip_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CHIP_COOKING) return;
    chip_cooking_t *pg = chip_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 炸薯条 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_376: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_376, 115, 320);

    /* bar_64: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_64, 122, 326);
    lv_obj_set_size(pg->bar_64, 637, 20);

    /* label_830: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_830, 273, 157);
    lv_obj_set_size(pg->label_830, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_377: 图片 | (115,161) | img: chipicon.png */
    lv_obj_set_pos(pg->image_377, 115, 161);

}


/* ==============================================================================
 * chip_menu 英文布局基准（对应 PAGE_CHIP_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void chip_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CHIP_MENU) return;
    chip_menu_t *pg = chip_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_372: 图片 | (24,117) | img: div1bg.png */
    lv_obj_set_pos(pg->image_372, 24, 117);

    /* label_805: 标签 | "烹饪时间" | (578,139) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_805, 578, 139);
    lv_obj_set_size(pg->label_805, 125, 32);

    /* hour: 标签 | "00" | (482,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 482, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_807: 标签 | "时" | (562,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_807, 562, 280);
    lv_obj_set_size(pg->label_807, 38, 32);

    /* label_808: 标签 | "分" | (762,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_808, 762, 280);
    lv_obj_set_size(pg->label_808, 38, 32);

    /* min: 标签 | "30" | (682,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_810: 标签 | "炸薯条" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_810, 24, 25);
    lv_obj_set_size(pg->label_810, 144, 32);

    /* label_811: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_811, 996, 39);
    lv_obj_set_size(pg->label_811, 117, 32);

    /* hourline: 图片 | (480,327) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 480, 327);

    /* minline: 图片 | (680,327) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 680, 327);

}


/* ==============================================================================
 * chip_set 英文布局基准（对应 PAGE_CHIP_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void chip_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CHIP_SET) return;
    chip_set_t *pg = chip_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_812: 标签 | "炸薯条" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_812, 24, 25);
    lv_obj_set_size(pg->label_812, 144, 32);

    /* label_813: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_813, 165, 157);
    lv_obj_set_size(pg->label_813, 26, 32);

    /* label_814: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_814, 252, 157);
    lv_obj_set_size(pg->label_814, 64, 32);

    /* label_815: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_815, 141, 270);
    lv_obj_set_size(pg->label_815, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_818: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_818, 368, 270);
    lv_obj_set_size(pg->label_818, 30, 32);

    /* label_819: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_819, 64, 32);

    /* image_375: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_375, 164, 376);

    /* label_820: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_820, 232, 385);
    lv_obj_set_size(pg->label_820, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_821: 标签 | "延" | (840,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_821, 840, 154);
    lv_obj_set_size(pg->label_821, 26, 32);

    /* label_822: 标签 | "迟" | (930,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_822, 930, 154);
    lv_obj_set_size(pg->label_822, 26, 32);

    /* label_823: 标签 | "自动保温" | (831,270) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_823, 831, 270);
    lv_obj_set_size(pg->label_823, 130, 32);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 260);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "230" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_825: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_825, 1010, 39);
    lv_obj_set_size(pg->label_825, 85, 36);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 260);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * chip_setting 英文布局基准（对应 PAGE_CHIP_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void chip_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CHIP_SETTING) return;
    chip_setting_t *pg = chip_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_37: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_37, 0, 0);
    lv_obj_set_size(pg->container_37, 1280, 480);

    /* image_378: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_378, 115, 115);

    /* label_832: 标签 | "烹饪时间" | (373,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_832, 373, 136);
    lv_obj_set_size(pg->label_832, 135, 30);

    /* hour: 标签 | "00" | (266,212) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 266, 212);
    lv_obj_set_size(pg->hour, 102, 46);

    /* label_834: 标签 | "时" | (340,228) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_834, 340, 228);
    lv_obj_set_size(pg->label_834, 37, 30);

    /* hourline: 图片 | (291,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 291, 269);

    /* min: 标签 | "00" | (483,212) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_836: 标签 | "分" | (558,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_836, 558, 229);
    lv_obj_set_size(pg->label_836, 37, 30);

    /* minline: 图片 | (508,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 508, 269);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * chip_stop 英文布局基准（对应 PAGE_CHIP_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void chip_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CHIP_STOP) return;
    chip_stop_t *pg = chip_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 炸薯条 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_381: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_381, 115, 320);

    /* bar_65: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_65, 122, 326);
    lv_obj_set_size(pg->bar_65, 637, 20);

    /* label_839: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_839, 273, 157);
    lv_obj_set_size(pg->label_839, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_382: 图片 | (115,161) | img: chipicon.png */
    lv_obj_set_pos(pg->image_382, 115, 161);

}


/* ==============================================================================
 * chip_stop_back 英文布局基准（对应 PAGE_CHIP_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void chip_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CHIP_STOP_BACK) return;
    chip_stop_back_t *pg = chip_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_383: 图片 | (115,161) | img: chipicon.png */
    lv_obj_set_pos(pg->image_383, 115, 161);

    /* status: 标签 | "| 炸薯条 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_384: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_384, 115, 320);

    /* bar_66: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_66, 122, 326);
    lv_obj_set_size(pg->bar_66, 637, 20);

    /* label_841: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_841, 273, 157);
    lv_obj_set_size(pg->label_841, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_38: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_38, 0, 0);
    lv_obj_set_size(pg->container_38, 1280, 480);

    /* label_842: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_842, 895, 160);
    lv_obj_set_size(pg->label_842, 252, 36);

    /* label_843: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_843, 895, 198);
    lv_obj_set_size(pg->label_843, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * clean_menu 英文布局基准（对应 PAGE_CLEAN_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void clean_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CLEAN_MENU) return;
    clean_menu_t *pg = clean_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_1: 标签 | "清洁" | (24,24) | 97x25 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_1, 24, 24);
    lv_obj_set_size(pg->label_1, 97, 25);

    /* waterclean: 按钮 | (12,75) | 638x387 | font montserratmedium_16 | bg: div230bg.png */
    lv_obj_set_pos(pg->waterclean, 12, 75);
    lv_obj_set_size(pg->waterclean, 638, 387);

    /* button_2: 按钮 | (633,74) | 637x389 | font montserratmedium_16 | bg: div230bg.png */
    lv_obj_set_pos(pg->button_2, 633, 74);
    lv_obj_set_size(pg->button_2, 637, 389);

    /* image_1: 图片 | (254,187) | img: clearfrt.png */
    lv_obj_set_pos(pg->image_1, 254, 187);

}


/* ==============================================================================
 * color_cookoing 英文布局基准（对应 PAGE_COLOR_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void color_cookoing_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COLOR_COOKING) return;
    color_cookoing_t *pg = color_cookoing_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status_label: 标签 | "| 额外上色 | 5分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status_label, 274, 232);
    lv_obj_set_size(pg->status_label, 490, 39);

    /* time_label: 标签 | "00:05:00" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->time_label, 907, 157);
    lv_obj_set_size(pg->time_label, 235, 60);

    /* stop_button: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop_button, 959, 295);
    lv_obj_set_size(pg->stop_button, 129, 83);

    /* barbk_img: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->barbk_img, 115, 320);

    /* bar: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar, 122, 326);
    lv_obj_set_size(pg->bar, 637, 20);

    /* color_img: 图片 | (95,163) | img: color.png */
    lv_obj_set_pos(pg->color_img, 95, 163);

    /* cooking_label: 标签 | "烹饪中..." | (273,157) | 255x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->cooking_label, 273, 157);
    lv_obj_set_size(pg->cooking_label, 255, 60);

}


/* ==============================================================================
 * color_stop 英文布局基准（对应 PAGE_COLOR_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void color_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COLOR_STOP) return;
    color_stop_t *pg = color_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_13: 标签 | "| 额外上色 | 5分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_13, 274, 232);
    lv_obj_set_size(pg->label_13, 490, 39);

    /* time_label: 标签 | "00:05:00" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->time_label, 907, 157);
    lv_obj_set_size(pg->time_label, 235, 60);

    /* button_6: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->button_6, 959, 295);
    lv_obj_set_size(pg->button_6, 129, 83);

    /* image_11: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_11, 115, 320);

    /* bar_3: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_3, 122, 326);
    lv_obj_set_size(pg->bar_3, 637, 20);

    /* image_12: 图片 | (95,163) | img: color.png */
    lv_obj_set_pos(pg->image_12, 95, 163);

    /* label_15: 标签 | "暂停中..." | (273,157) | 255x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_15, 273, 157);
    lv_obj_set_size(pg->label_15, 255, 60);

}


/* ==============================================================================
 * color_stop_back 英文布局基准（对应 PAGE_COLOR_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void color_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COLOR_STOP_BACK) return;
    color_stop_back_t *pg = color_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_17: 标签 | "| 额外上色 | 5分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_17, 274, 232);
    lv_obj_set_size(pg->label_17, 490, 39);

    /* image_14: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_14, 115, 320);

    /* bar_4: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_4, 122, 326);
    lv_obj_set_size(pg->bar_4, 637, 20);

    /* image_15: 图片 | (95,163) | img: color.png */
    lv_obj_set_pos(pg->image_15, 95, 163);

    /* label_19: 标签 | "暂停中..." | (273,157) | 255x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_19, 273, 157);
    lv_obj_set_size(pg->label_19, 255, 60);

    /* container_2: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_2, 0, 0);
    lv_obj_set_size(pg->container_2, 1280, 480);

    /* label_20: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_20, 895, 160);
    lv_obj_set_size(pg->label_20, 252, 36);

    /* label_21: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_21, 895, 198);
    lv_obj_set_size(pg->label_21, 252, 36);

    /* button_7: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->button_7, 959, 295);
    lv_obj_set_size(pg->button_7, 129, 83);

}


/* ==============================================================================
 * colorcooking_complete 英文布局基准（对应 PAGE_COLOR_COOKING_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void colorcooking_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COLOR_COOKING_COMPLETE) return;
    colorcooking_complete_t *pg = colorcooking_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* barbk_img: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->barbk_img, 115, 320);

    /* status_label: 标签 | "| 额外上色 | 5分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status_label, 274, 232);
    lv_obj_set_size(pg->status_label, 490, 39);

    /* bar: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar, 122, 326);
    lv_obj_set_size(pg->bar, 637, 20);

    /* complete_label: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->complete_label, 273, 157);
    lv_obj_set_size(pg->complete_label, 235, 60);

    /* color_img: 图片 | (95,163) | img: color.png */
    lv_obj_set_pos(pg->color_img, 95, 163);

    /* image_3: 图片 | (845,160) | img: hotcare.png */
    lv_obj_set_pos(pg->image_3, 845, 160);

}


/* ==============================================================================
 * cook4menu 英文布局基准（对应 PAGE_COOK4_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void cook4menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COOK4_MENU) return;
    cook4menu_t *pg = cook4menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_83: 标签 | "COOK 4" | (24,24) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_83, 24, 24);
    lv_obj_set_size(pg->label_83, 144, 32);

    /* cookie: 按钮 | (5,65) | 340x406 | font montserratmedium_16 | bg: div4bg30.png */
    lv_obj_set_pos(pg->cookie, 5, 65);
    lv_obj_set_size(pg->cookie, 340, 406);

    /* west: 按钮 | (315,65) | 340x406 | font montserratmedium_16 | bg: div4bg30.png */
    lv_obj_set_pos(pg->west, 315, 65);
    lv_obj_set_size(pg->west, 340, 406);

    /* piza: 按钮 | (625,65) | 340x406 | font montserratmedium_16 | bg: div4bg30.png */
    lv_obj_set_pos(pg->piza, 625, 65);
    lv_obj_set_size(pg->piza, 340, 406);

    /* menu: 按钮 | (936,65) | 340x406 | font montserratmedium_16 | bg: div4bg30.png */
    lv_obj_set_pos(pg->menu, 936, 65);
    lv_obj_set_size(pg->menu, 340, 406);

    /* image_47: 图片 | (125,187) | img: cookieimg.png */
    lv_obj_set_pos(pg->image_47, 125, 187);

    /* image_48: 图片 | (433,186) | img: westimg.png */
    lv_obj_set_pos(pg->image_48, 433, 186);

    /* image_49: 图片 | (745,186) | img: pizaimg.png */
    lv_obj_set_pos(pg->image_49, 745, 186);

    /* image_50: 图片 | (1056,187) | img: menuimgg.png */
    lv_obj_set_pos(pg->image_50, 1056, 187);

    /* label_84: 标签 | "饼干" | (145,290) | 100x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_84, 130, 290);
    lv_obj_set_size(pg->label_84, 100, 32);

    /* label_85: 标签 | "西式塔" | (440,290) | 100x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_85, 448, 290);
    lv_obj_set_size(pg->label_85, 100, 32);

    /* label_86: 标签 | "披萨" | (765,290) | 100x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_86, 752, 290);
    lv_obj_set_size(pg->label_86, 100, 32);

    /* label_87: 标签 | "菜单" | (1076,290) | 100x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_87, 1071, 290);
    lv_obj_set_size(pg->label_87, 100, 32);

}


/* ==============================================================================
 * cook_menu_tz 英文布局基准（对应 PAGE_COOK_MENU_TZ ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void cook_menu_tz_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COOK_MENU_TZ) return;
    cook_menu_tz_t *pg = cook_menu_tz_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* hot_bbq_button: 按钮 | (421,60) | 439x417 | font montserratmedium_16 | bg: div3bk30.png */
    lv_obj_set_pos(pg->hot_bbq_button, 421, 60);
    lv_obj_set_size(pg->hot_bbq_button, 439, 417);

    /* updown_button: 按钮 | (7,60) | 439x417 | font montserratmedium_16 | bg: div3bk30.png */
    lv_obj_set_pos(pg->updown_button, 7, 60);
    lv_obj_set_size(pg->updown_button, 439, 417);

    /* bottom_button: 按钮 | (835,60) | 439x417 | font montserratmedium_16 | bg: div3bk30.png */
    lv_obj_set_pos(pg->bottom_button, 835, 60);
    lv_obj_set_size(pg->bottom_button, 439, 417);

    /* major_img: 图片 | (175,188) | img: pengrentzimg.png */
    lv_obj_set_pos(pg->major_img, 175, 188);

    /* cook_label: 标签 | "烹饪功能" | (24,24) | 96x22 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->cook_label, 24, 24);
    lv_obj_set_size(pg->cook_label, 96, 22);

    /* updown_label: 标签 | "上下烧烤" | (153,291) | 147x28 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->updown_label, 153, 291);
    lv_obj_set_size(pg->updown_label, 147, 28);

    /* hot_label: 标签 | "热风烧烤" | (567,291) | 147x28 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->hot_label, 567, 291);
    lv_obj_set_size(pg->hot_label, 147, 28);

    /* bottom_label: 标签 | "底部烧烤" | (981,291) | 147x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->bottom_label, 981, 291);
    lv_obj_set_size(pg->bottom_label, 147, 30);

}


/* ==============================================================================
 * cookie_complete 英文布局基准（对应 PAGE_COOKIE_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void cookie_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COOKIE_COMPLETE) return;
    cookie_complete_t *pg = cookie_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_69: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_69, 115, 320);

    /* status: 标签 | "| 饼干 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_7: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_7, 122, 326);
    lv_obj_set_size(pg->bar_7, 637, 20);

    /* label_143: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_143, 273, 157);
    lv_obj_set_size(pg->label_143, 235, 60);

    /* image_70: 图片 | (845,160) | img: hotcare.png */
    lv_obj_set_pos(pg->image_70, 845, 160);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_71: 图片 | (115,161) | img: cookieicon.png */
    lv_obj_set_pos(pg->image_71, 115, 161);

}


/* ==============================================================================
 * cookie_cooking 英文布局基准（对应 PAGE_COOKIE_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void cookie_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COOKIE_COOKING) return;
    cookie_cooking_t *pg = cookie_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 饼干 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_58: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_58, 115, 320);

    /* bar_4: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_4, 122, 326);
    lv_obj_set_size(pg->bar_4, 637, 20);

    /* label_124: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_124, 273, 157);
    lv_obj_set_size(pg->label_124, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_59: 图片 | (115,161) | img: cookieicon.png */
    lv_obj_set_pos(pg->image_59, 115, 161);

}


/* ==============================================================================
 * cookie_menu 英文布局基准（对应 PAGE_COOKIE_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void cookie_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COOKIE_MENU) return;
    cookie_menu_t *pg = cookie_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_51: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_51, 24, 118);

    /* image_52: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_52, 645, 118);

    /* label_88: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_88, 300, 137);
    lv_obj_set_size(pg->label_88, 64, 32);

    /* temp: 标签 | "135" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_90: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_90, 372, 281);
    lv_obj_set_size(pg->label_90, 38, 32);

    /* label_91: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_91, 891, 137);
    lv_obj_set_size(pg->label_91, 125, 32);

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 793, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_93: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_93, 872, 280);
    lv_obj_set_size(pg->label_93, 38, 32);

    /* label_94: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_94, 1075, 281);
    lv_obj_set_size(pg->label_94, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_96: 标签 | "饼干" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_96, 24, 25);
    lv_obj_set_size(pg->label_96, 144, 32);

    /* label_97: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_97, 996, 39);
    lv_obj_set_size(pg->label_97, 117, 32);

    /* line3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->line3, 252, 328);

    /* line2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->line2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * cookie_set 英文布局基准（对应 PAGE_COOKIE_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void cookie_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COOKIE_SET) return;
    cookie_set_t *pg = cookie_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_98: 标签 | "饼干" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_98, 24, 25);
    lv_obj_set_size(pg->label_98, 144, 32);

    /* label_99: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_99, 165, 157);
    lv_obj_set_size(pg->label_99, 26, 32);

    /* label_100: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_100, 252, 157);
    lv_obj_set_size(pg->label_100, 64, 32);

    /* label_101: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_101, 141, 270);
    lv_obj_set_size(pg->label_101, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_104: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_104, 368, 270);
    lv_obj_set_size(pg->label_104, 30, 32);

    /* label_105: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_105, 64, 32);

    /* image_57: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_57, 164, 376);

    /* label_106: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_106, 232, 385);
    lv_obj_set_size(pg->label_106, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_108: 标签 | "延" | (840,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_108, 840, 154);
    lv_obj_set_size(pg->label_108, 26, 32);

    /* label_109: 标签 | "迟" | (930,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_109, 930, 154);
    lv_obj_set_size(pg->label_109, 26, 32);

    /* label_110: 标签 | "自动保温" | (831,270) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_110, 831, 270);
    lv_obj_set_size(pg->label_110, 130, 32);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 260);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "135" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_112: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_112, 1010, 39);
    lv_obj_set_size(pg->label_112, 85, 36);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 260);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * cookie_setting 英文布局基准（对应 PAGE_COOKIE_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void cookie_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COOKIE_SETTING) return;
    cookie_setting_t *pg = cookie_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_7: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_7, 0, 0);
    lv_obj_set_size(pg->container_7, 1280, 480);

    /* image_60: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_60, 115, 115);

    /* label_126: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_126, 73, 30);

    /* label_127: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_127, 454, 136);
    lv_obj_set_size(pg->label_127, 135, 30);

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 213);
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 300, 229);
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 374, 213);
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_132: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_132, 449, 229);
    lv_obj_set_size(pg->label_132, 37, 30);

    /* label_133: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_133, 612, 229);
    lv_obj_set_size(pg->label_133, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 288, 229);
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * cookie_stop 英文布局基准（对应 PAGE_COOKIE_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void cookie_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COOKIE_STOP) return;
    cookie_stop_t *pg = cookie_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 饼干 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_65: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_65, 115, 320);

    /* bar_5: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_5, 122, 326);
    lv_obj_set_size(pg->bar_5, 637, 20);

    /* label_137: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_137, 273, 157);
    lv_obj_set_size(pg->label_137, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_66: 图片 | (115,161) | img: cookieicon.png */
    lv_obj_set_pos(pg->image_66, 115, 161);

}


/* ==============================================================================
 * cookie_stop_back 英文布局基准（对应 PAGE_COOKIE_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void cookie_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COOKIE_STOP_BACK) return;
    cookie_stop_back_t *pg = cookie_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_67: 图片 | (115,161) | img: cookieicon.png */
    lv_obj_set_pos(pg->image_67, 115, 161);

    /* status: 标签 | "| 饼干 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_68: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_68, 115, 320);

    /* bar_6: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_6, 122, 326);
    lv_obj_set_size(pg->bar_6, 637, 20);

    /* label_139: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_139, 273, 157);
    lv_obj_set_size(pg->label_139, 235, 60);

    /* button_29: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->button_29, 609, 170);
    lv_obj_set_size(pg->button_29, 50, 43);

    /* container_8: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_8, 0, 0);
    lv_obj_set_size(pg->container_8, 1280, 480);

    /* label_140: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_140, 895, 160);
    lv_obj_set_size(pg->label_140, 252, 36);

    /* label_141: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_141, 895, 198);
    lv_obj_set_size(pg->label_141, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * cookmenu 英文布局基准（对应 PAGE_COOKMENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void cookmenu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COOKMENU) return;
    cookmenu_t *pg = cookmenu_get(&ui_manager);
    if (!pg) return;
    // lv_obj_set_style_bg_img_src(pg->obj, LVGL_IMAGE_PATH(cookmenuenbg.png), LV_PART_MAIN | LV_STATE_DEFAULT);
	// lv_obj_set_style_bg_img_recolor(pg->obj, lv_color_hex(0xff00ff), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_img_recolor_opa(pg->obj, 80, LV_PART_MAIN | LV_STATE_DEFAULT);
    /* 页面背景: 背景图 bg.jpg | 底色 0x000000 | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* top_bbq_button: 按钮 | (267,75) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->top_bbq_button, 267, 75);
    lv_obj_set_size(pg->top_bbq_button, 248, 193);

    /* hot_bbq_button: 按钮 | (516,76) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->hot_bbq_button, 516, 76);
    lv_obj_set_size(pg->hot_bbq_button, 248, 193);

    /* hot_wind_button: 按钮 | (764,76) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->hot_wind_button, 764, 76);
    lv_obj_set_size(pg->hot_wind_button, 248, 193);

    /* save_button: 按钮 | (1012,76) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->save_button, 1012, 76);
    lv_obj_set_size(pg->save_button, 248, 193);

    /* bottom_button: 按钮 | (266,268) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->bottom_button, 266, 268);
    lv_obj_set_size(pg->bottom_button, 248, 193);

    /* central_button: 按钮 | (516,268) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->central_button, 516, 268);
    lv_obj_set_size(pg->central_button, 248, 193);

    /* windchange_buttonn: 按钮 | (764,268) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->windchange_buttonn, 764, 268);
    lv_obj_set_size(pg->windchange_buttonn, 248, 193);

    /* preheater_button: 按钮 | (1012,268) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->preheater_button, 1012, 268);
    lv_obj_set_size(pg->preheater_button, 248, 193);

    /* up_down_button: 按钮 | (21,75) | 244x386 | font montserratmedium_16 | bg: updown80bk.png */
    lv_obj_set_pos(pg->up_down_button, 21, 75);
    lv_obj_set_size(pg->up_down_button, 244, 386);

    /* mainimg_1: 图片 | (95,111) | img: cookmenuicon.png */
    lv_obj_set_pos(pg->mainimg_1, 95, 111);

    /* hot_wind_labal: 标签 | "热风" | (857,213) | 63x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->hot_wind_labal, 788, 213);
    lv_obj_set_size(pg->hot_wind_labal, 200, 32);
	lv_obj_set_style_text_letter_space(pg->hot_wind_labal, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* save_labal: 标签 | "节能热风" | (1074,212) | 126x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->save_labal, 1027, 213);
    lv_obj_set_size(pg->save_labal, 220, 32);
	lv_obj_set_style_text_letter_space(pg->save_labal, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* up_down_labal: 标签 | "上下烧烤" | (69,287) | 150x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->up_down_labal, 46, 303);
    lv_obj_set_size(pg->up_down_labal, 200, 32);
	lv_obj_set_style_text_letter_space(pg->up_down_labal, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* bottom_bbq_labal: 标签 | "底部烧烤" | (327,406) | 131x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->bottom_bbq_labal, 294, 406);
    lv_obj_set_size(pg->bottom_bbq_labal, 200, 32);
	lv_obj_set_style_text_letter_space(pg->bottom_bbq_labal, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* hotwind_bbq_labal: 标签 | "热风烧烤" | (577,212) | 126x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->hotwind_bbq_labal, 541, 213);
    lv_obj_set_size(pg->hotwind_bbq_labal, 200, 32);
	lv_obj_set_style_text_letter_space(pg->hotwind_bbq_labal, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* central_labal: 标签 | "集中烧烤" | (577,405) | 126x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->central_labal, 541, 406);
    lv_obj_set_size(pg->central_labal, 200, 32);
	lv_obj_set_style_text_letter_space(pg->central_labal, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* wind_change_labal: 标签 | "热风对流" | (826,404) | 126x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->wind_change_labal, 786, 391);
    lv_obj_set_size(pg->wind_change_labal, 200, 64);
	lv_obj_set_style_text_letter_space(pg->wind_change_labal, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* preheater_labal: 标签 | "预热" | (1107,405) | 61x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->preheater_labal, 1036, 406);
    lv_obj_set_size(pg->preheater_labal, 200, 32);
	lv_obj_set_style_text_letter_space(pg->preheater_labal, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* pengren_labal: 标签 | "烹饪功能" | (24,22) | 97x25 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->pengren_labal, 24, 24);
    lv_obj_set_size(pg->pengren_labal, 250, 80);

    /* hot_bbq_labal: 标签 | "顶部烧烤" | (328,212) | 126x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->hot_bbq_labal, 294, 213);
    lv_obj_set_size(pg->hot_bbq_labal, 200, 32);
	lv_obj_set_style_text_letter_space(pg->hot_bbq_labal, 1, LV_PART_MAIN | LV_STATE_DEFAULT);

}


/* ==============================================================================
 * corn_complete 英文布局基准（对应 PAGE_CORN_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void corn_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CORN_COMPLETE) return;
    corn_complete_t *pg = corn_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_258: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_258, 115, 320);

    /* status: 标签 | "| 干果 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_43: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_43, 122, 326);
    lv_obj_set_size(pg->bar_43, 637, 20);

    /* label_582: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_582, 273, 157);
    lv_obj_set_size(pg->label_582, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_260: 图片 | (143,161) | img: cornicon.png */
    lv_obj_set_pos(pg->image_260, 143, 161);

    /* image_10: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_10, 902, 160);

}


/* ==============================================================================
 * corn_cooking 英文布局基准（对应 PAGE_CORN_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void corn_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CORN_COOKING) return;
    corn_cooking_t *pg = corn_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 干果 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_247: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_247, 115, 320);

    /* bar_40: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_40, 122, 326);
    lv_obj_set_size(pg->bar_40, 637, 20);

    /* label_563: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_563, 273, 157);
    lv_obj_set_size(pg->label_563, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_248: 图片 | (143,161) | img: cornicon.png */
    lv_obj_set_pos(pg->image_248, 143, 161);

}


/* ==============================================================================
 * corn_menu 英文布局基准（对应 PAGE_CORN_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void corn_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CORN_MENU) return;
    corn_menu_t *pg = corn_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_240: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_240, 24, 118);

    /* image_241: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_241, 645, 118);

    /* label_534: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_534, 300, 137);
    lv_obj_set_size(pg->label_534, 64, 32);

    /* temp: 标签 | "60" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_536: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_536, 372, 281);
    lv_obj_set_size(pg->label_536, 38, 32);

    /* label_537: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_537, 891, 137);
    lv_obj_set_size(pg->label_537, 125, 32);

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 793, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_539: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_539, 872, 280);
    lv_obj_set_size(pg->label_539, 38, 32);

    /* label_540: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_540, 1075, 281);
    lv_obj_set_size(pg->label_540, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_542: 标签 | "干果" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_542, 24, 25);
    lv_obj_set_size(pg->label_542, 144, 32);

    /* label_543: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_543, 996, 39);
    lv_obj_set_size(pg->label_543, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * corn_set 英文布局基准（对应 PAGE_CORN_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void corn_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CORN_SET) return;
    corn_set_t *pg = corn_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_544: 标签 | "干果" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_544, 24, 25);
    lv_obj_set_size(pg->label_544, 144, 32);

    /* label_545: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_545, 165, 157);
    lv_obj_set_size(pg->label_545, 26, 32);

    /* label_546: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_546, 252, 157);
    lv_obj_set_size(pg->label_546, 64, 32);

    /* label_547: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_547, 141, 270);
    lv_obj_set_size(pg->label_547, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_550: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_550, 368, 270);
    lv_obj_set_size(pg->label_550, 30, 32);

    /* label_551: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_551, 64, 32);

    /* image_246: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_246, 164, 376);

    /* label_552: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_552, 232, 385);
    lv_obj_set_size(pg->label_552, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_553: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_553, 830, 154);
    lv_obj_set_size(pg->label_553, 130, 32);

    /* label_554: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_554, 840, 270);
    lv_obj_set_size(pg->label_554, 26, 32);

    /* label_555: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_555, 930, 270);
    lv_obj_set_size(pg->label_555, 26, 32);

    /* label_556: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_556, 831, 381);
    lv_obj_set_size(pg->label_556, 130, 32);

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_558: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_558, 1010, 39);
    lv_obj_set_size(pg->label_558, 85, 36);

    /* onpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat, 983, 144);
    lv_obj_set_size(pg->onpreheat, 135, 53);

    /* ondelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 260);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 372);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * corn_setting 英文布局基准（对应 PAGE_CORN_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void corn_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CORN_SETTING) return;
    corn_setting_t *pg = corn_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_25: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_25, 0, 0);
    lv_obj_set_size(pg->container_25, 1280, 480);

    /* image_249: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_249, 115, 115);

    /* label_565: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_565, 242, 136);
    lv_obj_set_size(pg->label_565, 73, 30);

    /* label_566: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_566, 454, 136);
    lv_obj_set_size(pg->label_566, 135, 30);

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 213);
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 300, 229);
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 374, 213);
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_571: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_571, 449, 229);
    lv_obj_set_size(pg->label_571, 37, 30);

    /* label_572: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_572, 612, 229);
    lv_obj_set_size(pg->label_572, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 288, 229);
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * corn_stop 英文布局基准（对应 PAGE_CORN_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void corn_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CORN_STOP) return;
    corn_stop_t *pg = corn_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 干果 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_254: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_254, 115, 320);

    /* bar_41: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_41, 122, 326);
    lv_obj_set_size(pg->bar_41, 637, 20);

    /* label_576: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_576, 273, 157);
    lv_obj_set_size(pg->label_576, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_255: 图片 | (143,161) | img: cornicon.png */
    lv_obj_set_pos(pg->image_255, 143, 161);

}


/* ==============================================================================
 * corn_stop_back 英文布局基准（对应 PAGE_CORN_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void corn_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CORN_STOP_BACK) return;
    corn_stop_back_t *pg = corn_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_256: 图片 | (143,161) | img: cornicon.png */
    lv_obj_set_pos(pg->image_256, 143, 161);

    /* status: 标签 | "| 干果 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_257: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_257, 115, 320);

    /* bar_42: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_42, 122, 326);
    lv_obj_set_size(pg->bar_42, 637, 20);

    /* label_578: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_578, 273, 157);
    lv_obj_set_size(pg->label_578, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_26: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_26, 0, 0);
    lv_obj_set_size(pg->container_26, 1280, 480);

    /* label_579: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_579, 895, 160);
    lv_obj_set_size(pg->label_579, 252, 36);

    /* label_580: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_580, 895, 198);
    lv_obj_set_size(pg->label_580, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * custom_complete 英文布局基准（对应 PAGE_CUSTOM_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void custom_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CUSTOM_COMPLETE) return;
    custom_complete_t *pg = custom_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_401: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_401, 115, 320);

    /* status: 标签 | "| 自定义 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_71: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_71, 122, 326);
    lv_obj_set_size(pg->bar_71, 637, 20);

    /* label_886: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_886, 273, 157);
    lv_obj_set_size(pg->label_886, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_403: 图片 | (115,161) | img: customicon.png */
    lv_obj_set_pos(pg->image_403, 115, 161);

    /* image_17: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_17, 902, 160);

}


/* ==============================================================================
 * custom_cooking 英文布局基准（对应 PAGE_CUSTOM_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void custom_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CUSTOM_COOKING) return;
    custom_cooking_t *pg = custom_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 自定义 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_392: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_392, 115, 320);

    /* bar_68: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_68, 122, 326);
    lv_obj_set_size(pg->bar_68, 637, 20);

    /* label_871: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_871, 273, 157);
    lv_obj_set_size(pg->label_871, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_393: 图片 | (115,161) | img: customicon.png */
    lv_obj_set_pos(pg->image_393, 115, 161);

}


/* ==============================================================================
 * custom_menu 英文布局基准（对应 PAGE_CUSTOM_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void custom_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CUSTOM_MENU) return;
    custom_menu_t *pg = custom_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_388: 图片 | (24,117) | img: div1bg.png */
    lv_obj_set_pos(pg->image_388, 24, 117);

    /* label_846: 标签 | "烹饪时间" | (578,139) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_846, 578, 139);
    lv_obj_set_size(pg->label_846, 125, 32);

    /* hour: 标签 | "00" | (482,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 482, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_848: 标签 | "时" | (562,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_848, 562, 280);
    lv_obj_set_size(pg->label_848, 38, 32);

    /* label_849: 标签 | "分" | (762,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_849, 762, 280);
    lv_obj_set_size(pg->label_849, 38, 32);

    /* min: 标签 | "30" | (682,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_851: 标签 | "自定义" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_851, 24, 25);
    lv_obj_set_size(pg->label_851, 144, 32);

    /* label_852: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_852, 996, 39);
    lv_obj_set_size(pg->label_852, 117, 32);

    /* hourline: 图片 | (480,327) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 480, 327);

    /* minline: 图片 | (680,327) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 680, 327);

}


/* ==============================================================================
 * custom_set 英文布局基准（对应 PAGE_CUSTOM_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void custom_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CUSTOM_SET) return;
    custom_set_t *pg = custom_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_853: 标签 | "自定义" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_853, 24, 25);
    lv_obj_set_size(pg->label_853, 144, 32);

    /* label_854: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_854, 165, 157);
    lv_obj_set_size(pg->label_854, 26, 32);

    /* label_855: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_855, 252, 157);
    lv_obj_set_size(pg->label_855, 64, 32);

    /* label_856: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_856, 141, 270);
    lv_obj_set_size(pg->label_856, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_859: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_859, 368, 270);
    lv_obj_set_size(pg->label_859, 30, 32);

    /* label_860: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_860, 64, 32);

    /* image_391: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_391, 164, 376);

    /* label_861: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_861, 232, 385);
    lv_obj_set_size(pg->label_861, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_862: 标签 | "延" | (840,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_862, 840, 154);
    lv_obj_set_size(pg->label_862, 26, 32);

    /* label_863: 标签 | "迟" | (930,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_863, 930, 154);
    lv_obj_set_size(pg->label_863, 26, 32);

    /* label_864: 标签 | "自动保温" | (831,270) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_864, 831, 270);
    lv_obj_set_size(pg->label_864, 130, 32);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 260);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "200" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_866: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_866, 1010, 39);
    lv_obj_set_size(pg->label_866, 85, 36);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 260);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * custom_setting 英文布局基准（对应 PAGE_CUSTOM_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void custom_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CUSTOM_SETTING) return;
    custom_setting_t *pg = custom_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_39: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_39, 0, 0);
    lv_obj_set_size(pg->container_39, 1280, 480);

    /* image_394: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_394, 115, 115);

    /* label_873: 标签 | "烹饪时间" | (373,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_873, 373, 136);
    lv_obj_set_size(pg->label_873, 135, 30);

    /* hour: 标签 | "00" | (266,212) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 266, 212);
    lv_obj_set_size(pg->hour, 102, 46);

    /* label_875: 标签 | "时" | (340,228) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_875, 340, 228);
    lv_obj_set_size(pg->label_875, 37, 30);

    /* hourline: 图片 | (291,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 291, 269);

    /* min: 标签 | "00" | (483,212) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_877: 标签 | "分" | (558,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_877, 558, 229);
    lv_obj_set_size(pg->label_877, 37, 30);

    /* minline: 图片 | (508,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 508, 269);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * custom_stop 英文布局基准（对应 PAGE_CUSTOM_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void custom_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CUSTOM_STOP) return;
    custom_stop_t *pg = custom_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 自定义 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_397: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_397, 115, 320);

    /* bar_69: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_69, 122, 326);
    lv_obj_set_size(pg->bar_69, 637, 20);

    /* label_880: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_880, 273, 157);
    lv_obj_set_size(pg->label_880, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_398: 图片 | (115,161) | img: customicon.png */
    lv_obj_set_pos(pg->image_398, 115, 161);

}


/* ==============================================================================
 * custom_stop_back 英文布局基准（对应 PAGE_CUSTOM_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void custom_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CUSTOM_STOP_BACK) return;
    custom_stop_back_t *pg = custom_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_399: 图片 | (115,161) | img: customicon.png */
    lv_obj_set_pos(pg->image_399, 115, 161);

    /* status: 标签 | "| 自定义 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_400: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_400, 115, 320);

    /* bar_70: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_70, 122, 326);
    lv_obj_set_size(pg->bar_70, 637, 20);

    /* label_882: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_882, 273, 157);
    lv_obj_set_size(pg->label_882, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_40: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_40, 0, 0);
    lv_obj_set_size(pg->container_40, 1280, 480);

    /* label_883: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_883, 895, 160);
    lv_obj_set_size(pg->label_883, 252, 36);

    /* label_884: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_884, 895, 198);
    lv_obj_set_size(pg->label_884, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * delaycooking 英文布局基准（对应 PAGE_DELAYCOOKING 预约烹饪页）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void delaycooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_DELAYCOOKING) return;
    delaycooking_t *pg = delaycooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 上下烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* icon: 图片 | (115,161) | img: updown_img.png | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */

    /* label_14: 标签 | "预约中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_14, 273, 157);
    lv_obj_set_size(pg->label_14, 235, 60);

    /* cancel: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->cancel, 959, 295);
    lv_obj_set_size(pg->cancel, 129, 83);

    /* tip1: 标签 | "预约程序将在" | (907,158) | 230x34 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->tip1, 907, 158);
    lv_obj_set_size(pg->tip1, 230, 34);

    /* tip2: 标签 | "今明天12:00" | (907,194) | 230x34 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->tip2, 907, 194);
    lv_obj_set_size(pg->tip2, 230, 34);

    /* tip3: 标签 | "开始" | (907,230) | 230x34 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->tip3, 907, 230);
    lv_obj_set_size(pg->tip3, 230, 34);

}


/* ==============================================================================
 * delayset 英文布局基准（对应 PAGE_DELAYSET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void delayset_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_DELAYSET) return;
    delayset_t *pg = delayset_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_6: 图片 | (24,119) | img: div1bg.png */
    lv_obj_set_pos(pg->image_6, 24, 119);

    /* day: 标签 | "今明天" | (327,265) | 126x48 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->day, 327, 265);
    lv_obj_set_size(pg->day, 126, 48);

    /* start: 标签 | "开始" | (827,265) | 126x48 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->start, 827, 265);
    lv_obj_set_size(pg->start, 126, 48);

    /* hour: 标签 | "12" | (519,248) | 111x73 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 519, 248);
    lv_obj_set_size(pg->hour, 111, 73);

    /* min: 标签 | "00" | (651,248) | 111x73 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 111, 73);

    /* label_10: 标签 | ":" | (622,248) | 37x73 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->label_10, 622, 248);
    lv_obj_set_size(pg->label_10, 37, 73);

    /* startline: 图片 | (842,325) | img: underline_95x4.png */
    lv_obj_set_pos(pg->startline, 842, 325);

    /* image_9: 图片 | (537,325) | img: underline_76x4.png */
    lv_obj_set_pos(pg->image_9, 537, 325);

    /* image_10: 图片 | (668,325) | img: underline_76x4.png */
    lv_obj_set_pos(pg->image_10, 668, 325);

    /* name: 标签 | "上下烧烤" | (24,24) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->name, 24, 24);
    lv_obj_set_size(pg->name, 144, 32);

}


/* ==============================================================================
 * descriptionmenu 英文布局基准（对应 PAGE_DESCRIPTIONMENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void descriptionmenu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_DESCRIPTIONMENU) return;
    descriptionmenu_t *pg = descriptionmenu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_19: 标签 | "面包卷" | (24,24) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_19, 24, 24);
    lv_obj_set_size(pg->label_19, 144, 32);

    /* start: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->start, 983, 22);
    lv_obj_set_size(pg->start, 135, 71);

    /* container_1: 容器 | (24,119) | 1233x339 | bg: div1bg.png */
    lv_obj_set_pos(pg->container_1, 24, 119);
    lv_obj_set_size(pg->container_1, 1233, 339);

    /* summary: 标签 | "小结：⏎有发酵阶段⏎" | (0,0) | 894x59 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->summary, 0, 0);
    lv_obj_set_size(pg->summary, 894, 59);

    /* cookdescriptin: 标签 | "烹饪说明：⏎根据你最喜欢的食谱准备面团，放在烤盘上⏎现在将食物放在第…" | (0,0) | 894x122 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->cookdescriptin, 0, 0);
    lv_obj_set_size(pg->cookdescriptin, 894, 122);

    /* cooktime: 标签 | "预计烹饪时间：25分钟" | (0,0) | 628x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->cooktime, 0, 0);
    lv_obj_set_size(pg->cooktime, 628, 30);

    /* delay: 按钮 | (983,364) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->delay, 983, 364);
    lv_obj_set_size(pg->delay, 135, 71);

     /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     lv_obj_t *obj = NULL;
     obj = pg->label_19;
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}


/* ==============================================================================
 * duckmenu 英文布局基准（对应 PAGE_DUCK6MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void duckmenu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_DUCK6MENU) return;
    duckmenu_t *pg = duckmenu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* ---- 场景分支: 本页被四处复用 ----
       复用态(零食/配菜/肉菜): 第六感→零食(炸鸡米花) / 蔬菜配菜→配菜(炸薯条) / 蔬菜配菜→肉菜(烤香肠)
         运行时隐藏 label_2/image_2, 只显示 label_1+按钮
       默认: 家禽菜单→烤全鸭(label_2/image_2 可见) ---- */
    if (six_chick_get_snack_mode() || six_chick_get_sidedish_mode() || six_chick_get_meatdish_mode()) {
        /* 复用态: 只排 label_1 + 按钮 */
        /* wholeduck: 按钮 | (18,68) | 624x400 | font taiwanpearl_regular_30 | bg: div2bk30.png */
        lv_obj_set_pos(pg->wholeduck, 18, 68);
        lv_obj_set_size(pg->wholeduck, 624, 400);

        /* label_1: 标签 | "鸭" | (24,24) | 97x25 | font taiwanpearl_regular_24 */
        lv_obj_set_pos(pg->label_1, 24, 24);
        lv_obj_set_size(pg->label_1, 97, 25);
        /* TODO: 英文实测后调整(label_1 文字 Snacks/Side Dishes/Meat 较长, 可能需加宽) */
    } else {
        /* 默认: 家禽→烤全鸭 */
        /* wholeduck: 按钮 | (18,68) | 624x400 | font taiwanpearl_regular_30 | bg: div2bk30.png */
        lv_obj_set_pos(pg->wholeduck, 18, 68);
        lv_obj_set_size(pg->wholeduck, 624, 400);

        /* label_1: 标签 | "鸭" | (24,24) | 97x25 | font taiwanpearl_regular_24 */
        lv_obj_set_pos(pg->label_1, 24, 24);
        lv_obj_set_size(pg->label_1, 97, 25);

        /* label_2: 标签 | "烤全鸭" | (265,274) | 131x32 | font taiwanpearl_regular_30 */
        lv_obj_set_pos(pg->label_2, 265, 274);
        lv_obj_set_size(pg->label_2, 131, 32);

        /* image_2: 图片 | (296,213) | img: probeiconbig.png */
        lv_obj_set_pos(pg->image_2, 296, 213);

         /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ En modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
         lv_obj_t *obj = NULL;
         lv_obj_t *parent = NULL;
         obj = pg->label_2;
         parent = pg->wholeduck;
         lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
         lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
         lv_obj_align_to(obj, parent, LV_ALIGN_CENTER, 0, 18);
    }
}


/* ==============================================================================
 * extra_color 英文布局基准（对应 PAGE_EXTRA_COLOR ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void extra_color_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_EXTRA_COLOR) return;
    extra_color_t *pg = extra_color_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status_label: 标签 | "| 5分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status_label, 274, 232);
    lv_obj_set_size(pg->status_label, 490, 39);

    /* time_label: 标签 | "00:05:00" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->time_label, 907, 157);
    lv_obj_set_size(pg->time_label, 235, 60);

    /* start_button: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start_button, 959, 295);
    lv_obj_set_size(pg->start_button, 129, 83);

    /* barbk_img: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->barbk_img, 115, 320);

    /* bar: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar, 122, 326);
    lv_obj_set_size(pg->bar, 637, 20);

    /* color_img: 图片 | (95,163) | img: color.png */
    lv_obj_set_pos(pg->color_img, 95, 163);

    /* color_label: 标签 | "额外上色" | (273,157) | 255x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->color_label, 273, 157);
    lv_obj_set_size(pg->color_label, 255, 60);

}


/* ==============================================================================
 * frozencookpage 英文布局基准（对应 PAGE_FROZEN_COOK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void frozencookpage_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_FROZEN_COOK) return;
    frozencookpage_t *pg = frozencookpage_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_631: 标签 | "冷冻烘焙" | (24,24) | 97x25 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_631, 24, 24);
    lv_obj_set_size(pg->label_631, 200, 25);

    /* Lasagna: 按钮 | (8,62) | 437x221 | font montserratmedium_16 | bg: div6bg80.png */
    lv_obj_set_pos(pg->Lasagna, 8, 62);
    lv_obj_set_size(pg->Lasagna, 437, 221);

    /* Strudel: 按钮 | (422,62) | 437x221 | font montserratmedium_16 | bg: div6bg80.png */
    lv_obj_set_pos(pg->Strudel, 422, 62);
    lv_obj_set_size(pg->Strudel, 437, 221);

    /* bread: 按钮 | (836,62) | 437x221 | font montserratmedium_16 | bg: div6bg80.png */
    lv_obj_set_pos(pg->bread, 836, 62);
    lv_obj_set_size(pg->bread, 437, 221);

    /* pizza3: 按钮 | (8,255) | 437x221 | font montserratmedium_16 | bg: div6bg80.png */
    lv_obj_set_pos(pg->pizza3, 8, 255);
    lv_obj_set_size(pg->pizza3, 437, 221);

    /* chip: 按钮 | (421,255) | 437x221 | font montserratmedium_16 | bg: div6bg80.png */
    lv_obj_set_pos(pg->chip, 421, 255);
    lv_obj_set_size(pg->chip, 437, 221);

    /* custom: 按钮 | (836,255) | 437x221 | font montserratmedium_16 | bg: div6bg80.png */
    lv_obj_set_pos(pg->custom, 836, 255);
    lv_obj_set_size(pg->custom, 437, 221);

    /* image_279: 图片 | (177,111) | img: frozencookfr.png */
    lv_obj_set_pos(pg->image_279, 177, 111);

}


/* ==============================================================================
 * heatcontain_complete 英文布局基准（对应 PAGE_HEATCONTAIN_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void heatcontain_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HEATCONTAIN_COMPLETE) return;
    heatcontain_complete_t *pg = heatcontain_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_314: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_314, 115, 320);

    /* status: 标签 | "| 保温 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_48: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_48, 122, 326);
    lv_obj_set_size(pg->bar_48, 637, 20);

    /* label_672: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_672, 273, 157);
    lv_obj_set_size(pg->label_672, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_316: 图片 | (90,161) | img: heatcontainicon.png */
    lv_obj_set_pos(pg->image_316, 90, 161);

    /* image_11: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_11, 902, 160);

}


/* ==============================================================================
 * heatcontain_cooking 英文布局基准（对应 PAGE_HEATCONTAIN_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void heatcontain_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HEATCONTAIN_COOKING) return;
    heatcontain_cooking_t *pg = heatcontain_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 保温 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_268: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_268, 115, 320);

    /* bar_44: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_44, 122, 326);
    lv_obj_set_size(pg->bar_44, 637, 20);

    /* label_612: 标签 | "保温中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_612, 273, 157);
    lv_obj_set_size(pg->label_612, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_269: 图片 | (90,161) | img: heatcontainicon.png */
    lv_obj_set_pos(pg->image_269, 90, 161);

}


/* ==============================================================================
 * heatcontain_menu 英文布局基准（对应 PAGE_HEATCONTAIN_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void heatcontain_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HEATCONTAIN_MENU) return;
    heatcontain_menu_t *pg = heatcontain_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_261: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_261, 24, 118);

    /* image_262: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_262, 645, 118);

    /* label_583: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_583, 300, 137);
    lv_obj_set_size(pg->label_583, 64, 32);

    /* temp: 标签 | "70" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_585: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_585, 372, 281);
    lv_obj_set_size(pg->label_585, 38, 32);

    /* label_586: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_586, 891, 137);
    lv_obj_set_size(pg->label_586, 125, 32);

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 793, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_588: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_588, 872, 280);
    lv_obj_set_size(pg->label_588, 38, 32);

    /* label_589: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_589, 1075, 281);
    lv_obj_set_size(pg->label_589, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_591: 标签 | "保温" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_591, 24, 25);
    lv_obj_set_size(pg->label_591, 144, 32);

    /* label_592: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_592, 996, 39);
    lv_obj_set_size(pg->label_592, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * heatcontain_set 英文布局基准（对应 PAGE_HEATCONTAIN_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void heatcontain_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HEATCONTAIN_SET) return;
    heatcontain_set_t *pg = heatcontain_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_593: 标签 | "保温" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_593, 24, 25);
    lv_obj_set_size(pg->label_593, 144, 32);

    /* label_594: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_594, 165, 157);
    lv_obj_set_size(pg->label_594, 26, 32);

    /* label_595: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_595, 252, 157);
    lv_obj_set_size(pg->label_595, 64, 32);

    /* label_596: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_596, 141, 270);
    lv_obj_set_size(pg->label_596, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_599: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_599, 368, 270);
    lv_obj_set_size(pg->label_599, 30, 32);

    /* label_600: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_600, 64, 32);

    /* image_267: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_267, 164, 376);

    /* label_601: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_601, 232, 385);
    lv_obj_set_size(pg->label_601, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_602: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_602, 830, 154);
    lv_obj_set_size(pg->label_602, 130, 32);

    /* label_603: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_603, 840, 270);
    lv_obj_set_size(pg->label_603, 26, 32);

    /* label_604: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_604, 930, 270);
    lv_obj_set_size(pg->label_604, 26, 32);

    /* label_605: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_605, 831, 381);
    lv_obj_set_size(pg->label_605, 130, 32);

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_607: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_607, 1010, 39);
    lv_obj_set_size(pg->label_607, 85, 36);

    /* onpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat, 983, 144);
    lv_obj_set_size(pg->onpreheat, 135, 53);

    /* ondelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 260);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 372);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * heatcontain_setting 英文布局基准（对应 PAGE_HEATCONTAIN_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void heatcontain_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HEATCONTAIN_SETTING) return;
    heatcontain_setting_t *pg = heatcontain_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_27: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_27, 0, 0);
    lv_obj_set_size(pg->container_27, 1280, 480);

    /* image_270: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_270, 115, 115);

    /* label_614: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_614, 242, 136);
    lv_obj_set_size(pg->label_614, 73, 30);

    /* label_615: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_615, 454, 136);
    lv_obj_set_size(pg->label_615, 135, 30);

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 213);
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 300, 229);
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 374, 213);
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_620: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_620, 449, 229);
    lv_obj_set_size(pg->label_620, 37, 30);

    /* label_621: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_621, 612, 229);
    lv_obj_set_size(pg->label_621, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 288, 229);
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * heatcontain_stop 英文布局基准（对应 PAGE_HEATCONTAIN_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void heatcontain_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HEATCONTAIN_STOP) return;
    heatcontain_stop_t *pg = heatcontain_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 保温 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_275: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_275, 115, 320);

    /* bar_45: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_45, 122, 326);
    lv_obj_set_size(pg->bar_45, 637, 20);

    /* label_625: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_625, 273, 157);
    lv_obj_set_size(pg->label_625, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_276: 图片 | (90,161) | img: heatcontainicon.png */
    lv_obj_set_pos(pg->image_276, 90, 161);

}


/* ==============================================================================
 * heatcontain_stop_back 英文布局基准（对应 PAGE_HEATCONTAIN_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void heatcontain_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HEATCONTAIN_STOP_BACK) return;
    heatcontain_stop_back_t *pg = heatcontain_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_277: 图片 | (90,161) | img: heatcontainicon.png */
    lv_obj_set_pos(pg->image_277, 90, 161);

    /* status: 标签 | "| 保温 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_278: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_278, 115, 320);

    /* bar_46: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_46, 122, 326);
    lv_obj_set_size(pg->bar_46, 637, 20);

    /* label_627: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_627, 273, 157);
    lv_obj_set_size(pg->label_627, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_28: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_28, 0, 0);
    lv_obj_set_size(pg->container_28, 1280, 480);

    /* label_628: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_628, 895, 160);
    lv_obj_set_size(pg->label_628, 252, 36);

    /* label_629: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_629, 895, 198);
    lv_obj_set_size(pg->label_629, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * hot_bbq_complete 英文布局基准（对应 PAGE_HOT_BBQ_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hot_bbq_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOT_BBQ_COMPLETE) return;
    hot_bbq_complete_t *pg = hot_bbq_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_95: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_95, 115, 320);

    /* status: 标签 | "| 热风烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_17: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_17, 122, 326);
    lv_obj_set_size(pg->bar_17, 637, 20);

    /* label_204: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_204, 273, 157);
    lv_obj_set_size(pg->label_204, 330, 60);   /* 对齐 updown bbq 系 */

    /* button_59: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->button_59, 609, 170);
    lv_obj_set_size(pg->button_59, 50, 43);

    /* image_97: 图片 | (115,161) | img: hotbbqicon.png */
    lv_obj_set_pos(pg->image_97, 115, 161);

    /* image_3: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_3, 902, 160);

}


/* ==============================================================================
 * hot_bbq_complete_probe 英文布局基准（对应 PAGE_HOT_BBQ_COMPLETE_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hot_bbq_complete_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOT_BBQ_COMPLETE_PROBE) return;
    hot_bbq_complete_probe_t *pg = hot_bbq_complete_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_15: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_15, 115, 320);

    /* status: 标签 | "| 热风烧烤 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_4: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_4, 122, 326);
    lv_obj_set_size(pg->bar_4, 637, 20);

    /* image_16: 图片 | (115,161) | img: hotbbqicon.png */
    lv_obj_set_pos(pg->image_16, 115, 161);

    /* label_43: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_43, 273, 157);
    lv_obj_set_size(pg->label_43, 235, 60);

    /* label_44: 标签 | "高温防烫" | (896,161) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_44, 896, 161);
    lv_obj_set_size(pg->label_44, 252, 36);

    /* label_45: 标签 | "请缓慢打开门体！" | (896,197) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_45, 896, 197);
    lv_obj_set_size(pg->label_45, 252, 36);

}


/* ==============================================================================
 * hot_bbq_cooking 英文布局基准（对应 PAGE_HOT_BBQ_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hot_bbq_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOT_BBQ_COOKING) return;
    hot_bbq_cooking_t *pg = hot_bbq_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热风烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_84: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_84, 115, 320);

    /* bar_14: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_14, 122, 326);
    lv_obj_set_size(pg->bar_14, 637, 20);

    /* label_185: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_185, 273, 157);
    lv_obj_set_size(pg->label_185, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_85: 图片 | (115,161) | img: hotbbqicon.png */
    lv_obj_set_pos(pg->image_85, 115, 161);

}


/* ==============================================================================
 * hot_bbq_cooking_probe 英文布局基准（对应 PAGE_HOT_BBQ_COOKING_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hot_bbq_cooking_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOT_BBQ_COOKING_PROBE) return;
    hot_bbq_cooking_probe_t *pg = hot_bbq_cooking_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热风烧烤 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* temp: 标签 | "36℃" | (919,156) | 134x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->temp, 919, 156);
    lv_obj_set_size(pg->temp, 134, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_7: 图片 | (115,320) | img: bar.png | 场景条件定位(业务分支设置, tune不设) */
    /* 位置由业务场景分支控制, 微调按文件头模板 */

    /* bar_1: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_1, 122, 326);
    lv_obj_set_size(pg->bar_1, 637, 20);

    /* image_8: 图片 | (115,161) | img: hotbbqicon.png */
    lv_obj_set_pos(pg->image_8, 115, 161);

    /* label_34: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_34, 273, 157);
    lv_obj_set_size(pg->label_34, 235, 60);

    /* image_9: 图片 | (1068,180) | img: probeicon.png */
    lv_obj_set_pos(pg->image_9, 1068, 180);

}


/* ==============================================================================
 * hot_bbq_menu 英文布局基准（对应 PAGE_HOT_BBQ_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hot_bbq_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOT_BBQ_MENU) return;
    hot_bbq_menu_t *pg = hot_bbq_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_77: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_77, 24, 118);

    /* image_78: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_78, 645, 118);

    /* label_157: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_157, 275, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_157, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* temp: 标签 | "180" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 192, 249);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->temp, 180, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_159: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_159, 372, 281);
    lv_obj_set_size(pg->label_159, 38, 32);

    /* label_160: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_160, 855, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_160, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 788, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->hour, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_162: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_162, 872, 280);
    lv_obj_set_size(pg->label_162, 38, 32);

    /* label_163: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_163, 1075, 281);
    lv_obj_set_size(pg->label_163, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_pos(pg->min, 990, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->min, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_165: 标签 | "热风烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_165, 24, 24);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_165, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_166: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_166, 1007, 41);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_166, 117, 32);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * hot_bbq_menu_probe 英文布局基准（对应 PAGE_HOT_BBQ_MENU_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hot_bbq_menu_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOT_BBQ_MENU_PROBE) return;
    hot_bbq_menu_probe_t *pg = hot_bbq_menu_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_1: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_1, 24, 118);

    /* image_2: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_2, 645, 118);

    /* label_1: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_1, 300, 137);
    lv_obj_set_size(pg->label_1, 64, 32);

    /* temp: 标签 | "180" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_3: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_3, 372, 281);
    lv_obj_set_size(pg->label_3, 38, 32);

    /* label_4: 标签 | "探针温度" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_4, 891, 137);
    lv_obj_set_size(pg->label_4, 125, 32);

    /* label_5: 标签 | "热风烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_5, 24, 25);
    lv_obj_set_size(pg->label_5, 144, 32);

    /* label_6: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_6, 996, 39);
    lv_obj_set_size(pg->label_6, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* probetemp: 标签 | "80" | (853,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->probetemp, 853, 249);
    lv_obj_set_size(pg->probetemp, 120, 72);

    /* label_8: 标签 | "℃" | (973,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_8, 973, 281);
    lv_obj_set_size(pg->label_8, 38, 32);

    /* probetempline: 图片 | (891,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->probetempline, 891, 328);

}


/* ==============================================================================
 * hot_bbq_set 英文布局基准（对应 PAGE_HOT_BBQ_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hot_bbq_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOT_BBQ_SET) return;
    hot_bbq_set_t *pg = hot_bbq_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_167: 标签 | "热风烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_167, 24, 25);
    lv_obj_set_size(pg->label_167, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_168: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_168, 163, 155);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_168, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_169: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_169, 252, 157);
    lv_obj_set_size(pg->label_169, 64, 32);

    /* label_170: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_170, 150, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_170, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->min, 317, 258);
    lv_obj_set_size(pg->min, 68, 66);   /* 对齐 updown 系英文排版(A) */

    /* label_173: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_173, 368, 270);
    lv_obj_set_size(pg->label_173, 30, 32);

    /* label_174: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->label_174, 371, 271);
    lv_obj_set_size(pg->label_174, 42, 32);   /* 对齐 updown 系英文排版(A) */

    /* image_83: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_83, 164, 376);

    /* label_175: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_175, 232, 385);
    lv_obj_set_size(pg->label_175, 220, 32);   /* 对齐 updown 系英文排版(A) */

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_176: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_176, 785, 154);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_176, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_177: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_177, 885, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_177, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_178: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_178, 930, 270);
    lv_obj_set_size(pg->label_178, 26, 32);

    /* label_179: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_179, 765, 381);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_179, 200, 32);   /* 对齐 updown 系英文排版(A) */

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 269, 143);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->temp, 100, 53);   /* 对齐 top_bbq 系排版 */

    /* label_181: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_181, 1007, 41);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_181, 85, 36);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */

    /* onpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat, 983, 144);
    lv_obj_set_size(pg->onpreheat, 135, 53);

    /* ondelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 260);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 372);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 355, 157);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 327, 157);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * hot_bbq_set_probe 英文布局基准（对应 PAGE_HOT_BBQ_SET_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hot_bbq_set_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOT_BBQ_SET_PROBE) return;
    hot_bbq_set_probe_t *pg = hot_bbq_set_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_9: 标签 | "热风烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_9, 24, 25);
    lv_obj_set_size(pg->label_9, 144, 32);

    /* label_10: 标签 | "温" | (165,155) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_10, 165, 155);
    lv_obj_set_size(pg->label_10, 26, 32);

    /* label_11: 标签 | "度：" | (252,155) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_11, 252, 155);
    lv_obj_set_size(pg->label_11, 64, 32);

    /* label_12: 标签 | "探针温度：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_12, 141, 270);
    lv_obj_set_size(pg->label_12, 175, 32);

    /* image_6: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_6, 164, 376);

    /* label_13: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_13, 232, 385);
    lv_obj_set_size(pg->label_13, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_20: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_20, 1010, 39);
    lv_obj_set_size(pg->label_20, 85, 36);

    /* probetemp: 标签 | "80" | (315,254) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->probetemp, 315, 254);
    lv_obj_set_size(pg->probetemp, 83, 53);

    /* label_28: 标签 | "℃" | (369,270) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_28, 369, 270);
    lv_obj_set_size(pg->label_28, 32, 30);

    /* temp: 标签 | "200" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* image_3: 图片 | (839,154) | img: delaytext.png */
    lv_obj_set_pos(pg->image_3, 839, 154);

}


/* ==============================================================================
 * hot_bbq_setting 英文布局基准（对应 PAGE_HOT_BBQ_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hot_bbq_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOT_BBQ_SETTING) return;
    hot_bbq_setting_t *pg = hot_bbq_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 257, 60);   /* 对齐 top_bbq 系排版 */

    /* container_8: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_8, 0, 0);
    lv_obj_set_size(pg->container_8, 1280, 480);

    /* image_86: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_86, 115, 115);

    /* label_187: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_187, 203, 134);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_187, 150, 30);   /* 对齐 top_bbq 系排版 */

    /* label_188: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_188, 444, 134);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_188, 150, 30);   /* 对齐 top_bbq 系排版 */

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 217);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 303, 229);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 377, 217);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_193: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_193, 450, 231);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_193, 37, 30);

    /* label_194: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_194, 612, 231);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_194, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 289, 229);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * hot_bbq_stop 英文布局基准（对应 PAGE_HOT_BBQ_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hot_bbq_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOT_BBQ_STOP) return;
    hot_bbq_stop_t *pg = hot_bbq_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热风烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_91: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_91, 115, 320);

    /* bar_15: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_15, 122, 326);
    lv_obj_set_size(pg->bar_15, 637, 20);

    /* label_198: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_198, 273, 157);
    lv_obj_set_size(pg->label_198, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_92: 图片 | (115,161) | img: hotbbqicon.png */
    lv_obj_set_pos(pg->image_92, 115, 161);

}


/* ==============================================================================
 * hot_bbq_stop_back 英文布局基准（对应 PAGE_HOT_BBQ_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hot_bbq_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOT_BBQ_STOP_BACK) return;
    hot_bbq_stop_back_t *pg = hot_bbq_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_93: 图片 | (115,161) | img: hotbbqicon.png */
    lv_obj_set_pos(pg->image_93, 115, 161);

    /* status: 标签 | "| 热风烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_94: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_94, 115, 320);

    /* bar_16: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_16, 122, 326);
    lv_obj_set_size(pg->bar_16, 637, 20);

    /* label_200: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_200, 273, 157);
    lv_obj_set_size(pg->label_200, 330, 60);   /* 对齐 updown bbq 系 */   /* 对齐 updown 系英文排版(A) */

    /* button_57: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->button_57, 609, 170);
    lv_obj_set_size(pg->button_57, 50, 43);

    /* container_9: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_9, 0, 0);
    lv_obj_set_size(pg->container_9, 1280, 480);

    /* label_201: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_201, 895, 160);
    lv_obj_set_size(pg->label_201, 252, 36);

    /* label_202: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_202, 895, 198);
    lv_obj_set_size(pg->label_202, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * hot_bbq_stop_back_probe 英文布局基准（对应 PAGE_HOT_BBQ_STOP_BACK_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hot_bbq_stop_back_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOT_BBQ_STOP_BACK_PROBE) return;
    hot_bbq_stop_back_probe_t *pg = hot_bbq_stop_back_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热风烧烤 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_13: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_13, 115, 320);

    /* bar_3: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_3, 122, 326);
    lv_obj_set_size(pg->bar_3, 637, 20);

    /* image_14: 图片 | (115,161) | img: hotbbqicon.png */
    lv_obj_set_pos(pg->image_14, 115, 161);

    /* label_39: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_39, 273, 157);
    lv_obj_set_size(pg->label_39, 235, 60);

    /* container_1: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_1, 0, 0);
    lv_obj_set_size(pg->container_1, 1280, 480);

    /* label_40: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_40, 895, 160);
    lv_obj_set_size(pg->label_40, 252, 36);

    /* label_41: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_41, 895, 198);
    lv_obj_set_size(pg->label_41, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * hot_bbq_stop_probe 英文布局基准（对应 PAGE_HOT_BBQ_STOP_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hot_bbq_stop_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOT_BBQ_STOP_PROBE) return;
    hot_bbq_stop_probe_t *pg = hot_bbq_stop_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热风烧烤 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* temp: 标签 | "36℃" | (919,156) | 134x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->temp, 919, 156);
    lv_obj_set_size(pg->temp, 134, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_10: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_10, 115, 320);

    /* bar_2: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_2, 122, 326);
    lv_obj_set_size(pg->bar_2, 637, 20);

    /* image_11: 图片 | (115,161) | img: hotbbqicon.png */
    lv_obj_set_pos(pg->image_11, 115, 161);

    /* label_37: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_37, 273, 157);
    lv_obj_set_size(pg->label_37, 235, 60);

    /* image_12: 图片 | (1068,180) | img: probeicon.png */
    lv_obj_set_pos(pg->image_12, 1068, 180);

}


/* ==============================================================================
 * hotclean_menu 英文布局基准（对应 PAGE_HOTCLEAN_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotclean_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEAN_MENU) return;
    hotclean_menu_t *pg = hotclean_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xffffff | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* hotclean_middle: 按钮 | (421,60) | 439x417 | font taiwanpearl_regular_30 | bg: div3bk30.png */
    lv_obj_set_pos(pg->hotclean_middle, 421, 60);
    lv_obj_set_size(pg->hotclean_middle, 439, 417);

    /* hotcleansave: 按钮 | (7,60) | 439x417 | font taiwanpearl_regular_30 | bg: div3bk30.png */
    lv_obj_set_pos(pg->hotcleansave, 7, 60);
    lv_obj_set_size(pg->hotcleansave, 439, 417);

    /* hotclean_high: 按钮 | (835,60) | 439x417 | font taiwanpearl_regular_30 | bg: div3bk30.png */
    lv_obj_set_pos(pg->hotclean_high, 835, 60);
    lv_obj_set_size(pg->hotclean_high, 439, 417);

    /* label_1: 标签 | "热解自清洁" | (24,24) | 125x22 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_1, 24, 24);
    lv_obj_set_size(pg->label_1, 125, 22);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ En modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = NULL;
    obj = pg->label_1;
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}


/* ==============================================================================
 * hotcleanhigh_complete 英文布局基准（对应 PAGE_HOTCLEANHIGH_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleanhigh_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANHIGH_COMPLETE) return;
    hotcleanhigh_complete_t *pg = hotcleanhigh_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_23: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_23, 115, 320);

    /* status: 标签 | "| 热解自清洁 | 2小时 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_9: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_9, 122, 326);
    lv_obj_set_size(pg->bar_9, 637, 20);

    /* image_24: 图片 | (115,161) | img: hotcleanhighicon.png */
    lv_obj_set_pos(pg->image_24, 115, 161);

    /* label_28: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_28, 273, 157);
    lv_obj_set_size(pg->label_28, 235, 60);

    /* image_25: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_25, 902, 160);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_28;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->status;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}


/* ==============================================================================
 * hotcleanhigh_cooking 英文布局基准（对应 PAGE_HOTCLEANHIGH_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleanhigh_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANHIGH_COOKING) return;
    hotcleanhigh_cooking_t *pg = hotcleanhigh_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热解自清洁 | 2小时 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_16: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_16, 115, 320);

    /* bar_6: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_6, 122, 326);
    lv_obj_set_size(pg->bar_6, 637, 20);

    /* image_17: 图片 | (115,161) | img: hotcleanhighicon.png */
    lv_obj_set_pos(pg->image_17, 115, 161);

    /* label_21: 标签 | "清洁中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_21, 273, 157);
    lv_obj_set_size(pg->label_21, 235, 60);

    /* button_9: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: lockicon.png */
    lv_obj_set_pos(pg->button_9, 609, 170);
    lv_obj_set_size(pg->button_9, 50, 43);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_21;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->status;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->timelabel;
    lv_obj_set_pos(obj, 902, 162);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->stop;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * hotcleanhigh_cooling 英文布局基准（对应 PAGE_HOTCLEANHIGH_COOLING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleanhigh_cooling_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANHIGH_COOLING) return;
    hotcleanhigh_cooling_t *pg = hotcleanhigh_cooling_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_20: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_20, 115, 320);

    /* status: 标签 | "| 热解自清洁 | 2小时 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_8: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_8, 122, 326);
    lv_obj_set_size(pg->bar_8, 637, 20);

    /* image_21: 图片 | (115,161) | img: hotcleanhighicon.png */
    lv_obj_set_pos(pg->image_21, 115, 161);

    /* label_26: 标签 | "冷却中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_26, 273, 157);
    lv_obj_set_size(pg->label_26, 235, 60);

    /* image_22: 图片 | (902,160) | img: hotcleantips.png */
    lv_obj_set_pos(pg->image_22, 902, 160);

    /* button_12: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: lockicon.png */
    lv_obj_set_pos(pg->button_12, 609, 170);
    lv_obj_set_size(pg->button_12, 50, 43);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_26;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->status;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}


/* ==============================================================================
 * hotcleanhigh_set 英文布局基准（对应 PAGE_HOTCLEANHIGH_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleanhigh_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANHIGH_SET) return;
    hotcleanhigh_set_t *pg = hotcleanhigh_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_15: 图片 | (117,159) | img: hotcleanhighicon.png */
    lv_obj_set_pos(pg->image_15, 117, 159);

    /* label_16: 标签 | "热解自清洁 — 高" | (276,157) | 512x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_16, 276, 157);
    lv_obj_set_size(pg->label_16, 512, 60);

    /* status: 标签 | "| 2小时 |" | (275,231) | 152x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 275, 231);
    lv_obj_set_size(pg->status, 152, 39);

    /* label_3: 标签 | "温馨提示：取出腔体内所有附件，并关闭机门" | (120,320) | 600x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_3, 120, 320);
    lv_obj_set_size(pg->label_3, 600, 39);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_clean.png), 0);
    // lv_obj_t *obj = NULL;
    obj = pg->label_3;
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_line_space(obj, -1, 0);
    lv_obj_set_size(obj, 650, LV_SIZE_CONTENT);
    lv_obj_align(obj, LV_ALIGN_CENTER, -193, 95);
    // lv_obj_set_style_text_color(obj, lv_color_hex(0xff0000), 0);
    
    obj = pg->start;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * hotcleanhigh_stop 英文布局基准（对应 PAGE_HOTCLEANHIGH_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleanhigh_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANHIGH_STOP) return;
    hotcleanhigh_stop_t *pg = hotcleanhigh_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热解自清洁 | 2小时 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_18: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_18, 115, 320);

    /* bar_7: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_7, 122, 326);
    lv_obj_set_size(pg->bar_7, 637, 20);

    /* image_19: 图片 | (115,161) | img: hotcleanhighicon.png */
    lv_obj_set_pos(pg->image_19, 115, 161);

    /* label_24: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_24, 273, 157);
    lv_obj_set_size(pg->label_24, 235, 60);

    /* button_11: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: lockicon.png */
    lv_obj_set_pos(pg->button_11, 609, 170);
    lv_obj_set_size(pg->button_11, 50, 43);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_24;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->status;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->timelabel;
    lv_obj_set_pos(obj, 902, 162);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->start;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * hotcleanhigh_stop_back 英文布局基准（对应 PAGE_HOTCLEANHIGH_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleanhigh_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANHIGH_STOP_BACK) return;
    hotcleanhigh_stop_back_t *pg = hotcleanhigh_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热解自清洁 | 2小时 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_5: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_5, 115, 320);

    /* bar_3: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_3, 122, 326);
    lv_obj_set_size(pg->bar_3, 637, 20);

    /* image_6: 图片 | (115,161) | img: hotcleanhighicon.png */
    lv_obj_set_pos(pg->image_6, 115, 161);

    /* label_16: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_16, 273, 157);
    lv_obj_set_size(pg->label_16, 235, 60);

    /* button_5: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: lockicon.png */
    lv_obj_set_pos(pg->button_5, 609, 170);
    lv_obj_set_size(pg->button_5, 50, 43);

    /* container_3: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_3, 0, 0);
    lv_obj_set_size(pg->container_3, 1280, 480);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

    /* label_17: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_17, 895, 160);
    lv_obj_set_size(pg->label_17, 252, 36);

    /* label_18: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_18, 895, 198);
    lv_obj_set_size(pg->label_18, 252, 36);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_16;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->status;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->sure;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * hotcleanmiddle_complete 英文布局基准（对应 PAGE_HOTCLEANMIDDLE_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleanmiddle_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANMIDDLE_COMPLETE) return;
    hotcleanmiddle_complete_t *pg = hotcleanmiddle_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_12: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_12, 115, 320);

    /* status: 标签 | "| 热解自清洁 | 1小时40分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_5: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_5, 122, 326);
    lv_obj_set_size(pg->bar_5, 637, 20);

    /* image_13: 图片 | (115,161) | img: hotcleanmiddleicon.png */
    lv_obj_set_pos(pg->image_13, 115, 161);

    /* label_15: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_15, 273, 157);
    lv_obj_set_size(pg->label_15, 235, 60);

    /* image_14: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_14, 902, 160);

}


/* ==============================================================================
 * hotcleanmiddle_cooking 英文布局基准（对应 PAGE_HOTCLEANMIDDLE_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleanmiddle_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANMIDDLE_COOKING) return;
    hotcleanmiddle_cooking_t *pg = hotcleanmiddle_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热解自清洁 | 1小时40分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_5: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_5, 115, 320);

    /* bar_2: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_2, 122, 326);
    lv_obj_set_size(pg->bar_2, 637, 20);

    /* image_6: 图片 | (115,161) | img: hotcleanmiddleicon.png */
    lv_obj_set_pos(pg->image_6, 115, 161);

    /* label_8: 标签 | "清洁中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_8, 273, 157);
    lv_obj_set_size(pg->label_8, 235, 60);

    /* button_3: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: lockicon.png */
    lv_obj_set_pos(pg->button_3, 609, 170);
    lv_obj_set_size(pg->button_3, 50, 43);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_8;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->status;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->timelabel;
    lv_obj_set_pos(obj, 902, 162);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->stop;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * hotcleanmiddle_cooling 英文布局基准（对应 PAGE_HOTCLEANMIDDLE_COOLING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleanmiddle_cooling_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANMIDDLE_COOLING) return;
    hotcleanmiddle_cooling_t *pg = hotcleanmiddle_cooling_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_9: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_9, 115, 320);

    /* status: 标签 | "| 热解自清洁 | 1小时40分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_4: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_4, 122, 326);
    lv_obj_set_size(pg->bar_4, 637, 20);

    /* image_10: 图片 | (115,161) | img: hotcleanmiddleicon.png */
    lv_obj_set_pos(pg->image_10, 115, 161);

    /* label_13: 标签 | "冷却中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_13, 273, 157);
    lv_obj_set_size(pg->label_13, 235, 60);

    /* image_11: 图片 | (902,160) | img: hotcleantips.png */
    lv_obj_set_pos(pg->image_11, 902, 160);

    /* button_6: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: lockicon.png */
    lv_obj_set_pos(pg->button_6, 609, 170);
    lv_obj_set_size(pg->button_6, 50, 43);

}


/* ==============================================================================
 * hotcleanmiddle_set 英文布局基准（对应 PAGE_HOTCLEANMIDDLE_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleanmiddle_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANMIDDLE_SET) return;
    hotcleanmiddle_set_t *pg = hotcleanmiddle_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_4: 图片 | (117,159) | img: hotcleanmiddleicon.png */
    lv_obj_set_pos(pg->image_4, 117, 159);

    /* label_3: 标签 | "热解自清洁 — 中" | (276,157) | 512x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_3, 276, 157);
    lv_obj_set_size(pg->label_3, 512, 60);

    /* status: 标签 | "| 1小时40分钟 |" | (275,231) | 232x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 275, 231);
    lv_obj_set_size(pg->status, 232, 39);

    /* label_2: 标签 | "温馨提示：取出腔体内所有附件，并关闭机门" | (120,320) | 600x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_2, 120, 320);
    lv_obj_set_size(pg->label_2, 600, 39);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_clean.png), 0);
    // lv_obj_t *obj = NULL;
    obj = pg->label_2;
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_line_space(obj, -1, 0);
    lv_obj_set_size(obj, 650, LV_SIZE_CONTENT);
    lv_obj_align(obj, LV_ALIGN_CENTER, -193, 95);
    // lv_obj_set_style_text_color(obj, lv_color_hex(0xff0000), 0);
    
    obj = pg->start;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * hotcleanmiddle_stop 英文布局基准（对应 PAGE_HOTCLEANMIDDLE_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleanmiddle_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANMIDDLE_STOP) return;
    hotcleanmiddle_stop_t *pg = hotcleanmiddle_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热解自清洁 | 1小时40分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_7: 图片 | (115,320) | img: bar.png | 场景条件定位(业务分支设置, tune不设) */
    /* 位置由业务场景分支控制, 微调按文件头模板 */

    /* bar_3: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_3, 122, 326);
    lv_obj_set_size(pg->bar_3, 637, 20);

    /* image_8: 图片 | (115,161) | img: hotcleanmiddleicon.png */
    lv_obj_set_pos(pg->image_8, 115, 161);

    /* label_11: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_11, 273, 157);
    lv_obj_set_size(pg->label_11, 235, 60);

    /* button_5: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: lockicon.png */
    lv_obj_set_pos(pg->button_5, 609, 170);
    lv_obj_set_size(pg->button_5, 50, 43);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_11;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->status;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->timelabel;
    lv_obj_set_pos(obj, 902, 162);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->start;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * hotcleanmiddle_stop_back 英文布局基准（对应 PAGE_HOTCLEANMIDDLE_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleanmiddle_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANMIDDLE_STOP_BACK) return;
    hotcleanmiddle_stop_back_t *pg = hotcleanmiddle_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热解自清洁 | 1小时40分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_3: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_3, 115, 320);

    /* bar_2: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_2, 122, 326);
    lv_obj_set_size(pg->bar_2, 637, 20);

    /* image_4: 图片 | (115,161) | img: hotcleanmiddleicon.png */
    lv_obj_set_pos(pg->image_4, 115, 161);

    /* label_11: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_11, 273, 157);
    lv_obj_set_size(pg->label_11, 235, 60);

    /* button_3: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: lockicon.png */
    lv_obj_set_pos(pg->button_3, 609, 170);
    lv_obj_set_size(pg->button_3, 50, 43);

    /* container_2: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_2, 0, 0);
    lv_obj_set_size(pg->container_2, 1280, 480);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

    /* label_12: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_12, 895, 160);
    lv_obj_set_size(pg->label_12, 252, 36);

    /* label_13: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_13, 895, 198);
    lv_obj_set_size(pg->label_13, 252, 36);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_11;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->status;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->sure;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * hotcleansave_complete 英文布局基准（对应 PAGE_HOTCLEANSAVE_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleansave_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANSAVE_COMPLETE) return;
    hotcleansave_complete_t *pg = hotcleansave_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_1: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_1, 115, 320);

    /* status: 标签 | "| 热解自清洁 | 1小时15分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_1: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_1, 122, 326);
    lv_obj_set_size(pg->bar_1, 637, 20);

    /* image_2: 图片 | (115,161) | img: hotcleansaveicon.png */
    lv_obj_set_pos(pg->image_2, 115, 161);

    /* label_2: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_2, 273, 157);
    lv_obj_set_size(pg->label_2, 235, 60);

    /* image_3: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_3, 902, 160);

}


/* ==============================================================================
 * hotcleansave_cooking 英文布局基准（对应 PAGE_HOTCLEANSAVE_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleansave_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANSAVE_COOKING) return;
    hotcleansave_cooking_t *pg = hotcleansave_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热解自清洁 | 1小时15分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_2: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_2, 115, 320);

    /* bar_1: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_1, 122, 326);
    lv_obj_set_size(pg->bar_1, 637, 20);

    /* image_3: 图片 | (115,161) | img: hotcleansaveicon.png */
    lv_obj_set_pos(pg->image_3, 115, 161);

    /* label_8: 标签 | "清洁中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_8, 273, 157);
    lv_obj_set_size(pg->label_8, 235, 60);

    /* button_7: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: lockicon.png */
    lv_obj_set_pos(pg->button_7, 609, 170);
    lv_obj_set_size(pg->button_7, 50, 43);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_8;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->status;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->timelabel;
    lv_obj_set_pos(obj, 902, 162);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->stop;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * hotcleansave_cooling 英文布局基准（对应 PAGE_HOTCLEANSAVE_COOLING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleansave_cooling_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANSAVE_COOLING) return;
    hotcleansave_cooling_t *pg = hotcleansave_cooling_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_6: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_6, 115, 320);

    /* status: 标签 | "| 热解自清洁 | 1小时15分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_3: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_3, 122, 326);
    lv_obj_set_size(pg->bar_3, 637, 20);

    /* image_7: 图片 | (115,161) | img: hotcleansaveicon.png | 场景条件定位(业务分支设置, tune不设) */
    /* 位置由业务场景分支控制, 微调按文件头模板 */

    /* label_13: 标签 | "冷却中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_13, 273, 157);
    lv_obj_set_size(pg->label_13, 235, 60);

    /* image_8: 图片 | (902,160) | img: hotcleantips.png */
    lv_obj_set_pos(pg->image_8, 902, 160);

    /* button_9: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: lockicon.png */
    lv_obj_set_pos(pg->button_9, 609, 170);
    lv_obj_set_size(pg->button_9, 50, 43);

}


/* ==============================================================================
 * hotcleansave_set 英文布局基准（对应 PAGE_HOTCLEANSAVE_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleansave_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANSAVE_SET) return;
    hotcleansave_set_t *pg = hotcleansave_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_9: 图片 | (117,159) | img: hotcleansaveicon.png */
    lv_obj_set_pos(pg->image_9, 117, 159);

    /* label_14: 标签 | "热解自清洁 — 节能" | (276,157) | 512x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_14, 276, 157);
    lv_obj_set_size(pg->label_14, 512, 60);

    /* status: 标签 | "| 1小时15分钟 |" | (275,231) | 233x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 275, 231);
    lv_obj_set_size(pg->status, 233, 39);

    /* label_16: 标签 | "温馨提示：取出腔体内所有附件，并关闭机门" | (120,320) | 600x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_16, 120, 320);
    lv_obj_set_size(pg->label_16, 600, 39);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_clean.png), 0);
    // lv_obj_t *obj = NULL;
    obj = pg->label_16;
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_long_mode(obj, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_line_space(obj, -1, 0);
    lv_obj_set_size(obj, 650, LV_SIZE_CONTENT);
    lv_obj_align(obj, LV_ALIGN_CENTER, -193, 95);
    // lv_obj_set_style_text_color(obj, lv_color_hex(0xff0000), 0);
    
    obj = pg->start;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * hotcleansave_stop 英文布局基准（对应 PAGE_HOTCLEANSAVE_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleansave_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANSAVE_STOP) return;
    hotcleansave_stop_t *pg = hotcleansave_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热解自清洁 | 1小时15分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_4: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_4, 115, 320);

    /* bar_2: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_2, 122, 326);
    lv_obj_set_size(pg->bar_2, 637, 20);

    /* image_5: 图片 | (115,161) | img: hotcleansaveicon.png */
    lv_obj_set_pos(pg->image_5, 115, 161);

    /* label_11: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_11, 273, 157);
    lv_obj_set_size(pg->label_11, 235, 60);

    /* button_8: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: lockicon.png */
    lv_obj_set_pos(pg->button_8, 609, 170);
    lv_obj_set_size(pg->button_8, 50, 43);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_11;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    // lv_label_set_text(obj, "Cooking...");

    obj = pg->status;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->timelabel;
    lv_obj_set_pos(obj, 902, 162);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->start;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * hotcleansave_stop_back 英文布局基准（对应 PAGE_HOTCLEANSAVE_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotcleansave_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTCLEANSAVE_STOP_BACK) return;
    hotcleansave_stop_back_t *pg = hotcleansave_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热解自清洁 | 1小时15分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_7: 图片 | (115,320) | img: bar.png | 场景条件定位(业务分支设置, tune不设) */
    /* 位置由业务场景分支控制, 微调按文件头模板 */

    /* bar_4: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_4, 122, 326);
    lv_obj_set_size(pg->bar_4, 637, 20);

    /* image_8: 图片 | (115,161) | img: hotcleansaveicon.png */
    lv_obj_set_pos(pg->image_8, 115, 161);

    /* label_21: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_21, 273, 157);
    lv_obj_set_size(pg->label_21, 235, 60);

    /* button_7: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: lockicon.png */
    lv_obj_set_pos(pg->button_7, 609, 170);
    lv_obj_set_size(pg->button_7, 50, 43);

    /* container_4: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_4, 0, 0);
    lv_obj_set_size(pg->container_4, 1280, 480);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

    /* label_22: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_22, 895, 160);
    lv_obj_set_size(pg->label_22, 252, 36);

    /* label_23: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_23, 895, 198);
    lv_obj_set_size(pg->label_23, 252, 36);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_21;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->status;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->sure;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * hotwind_bbq_complete 英文布局基准（对应 PAGE_HOTWIND_BBQ_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotwind_bbq_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTWIND_BBQ_COMPLETE) return;
    hotwind_bbq_complete_t *pg = hotwind_bbq_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_111: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_111, 115, 320);

    /* status: 标签 | "| 热风 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_21: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_21, 122, 326);
    lv_obj_set_size(pg->bar_21, 637, 20);

    /* label_242: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_242, 273, 157);
    lv_obj_set_size(pg->label_242, 330, 60);   /* 对齐 updown bbq 系 */

    /* image_112: 图片 | (845,160) | img: hotcare.png */
    lv_obj_set_pos(pg->image_112, 845, 160);

    /* button_74: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->button_74, 609, 170);
    lv_obj_set_size(pg->button_74, 50, 43);

    /* image_113: 图片 | (115,161) | img: hotwindicon.png */
    lv_obj_set_pos(pg->image_113, 115, 161);

}


/* ==============================================================================
 * hotwind_bbq_cooking 英文布局基准（对应 PAGE_HOTWIND_BBQ_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotwind_bbq_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTWIND_BBQ_COOKING) return;
    hotwind_bbq_cooking_t *pg = hotwind_bbq_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热风 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_105: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_105, 115, 320);

    /* bar_18: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_18, 122, 326);
    lv_obj_set_size(pg->bar_18, 637, 20);

    /* label_233: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_233, 273, 157);
    lv_obj_set_size(pg->label_233, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_106: 图片 | (115,161) | img: hotwindicon.png */
    lv_obj_set_pos(pg->image_106, 115, 161);

}


/* ==============================================================================
 * hotwind_bbq_menu 英文布局基准（对应 PAGE_HOTWIND_BBQ_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotwind_bbq_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTWIND_BBQ_MENU) return;
    hotwind_bbq_menu_t *pg = hotwind_bbq_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_98: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_98, 24, 118);

    /* image_99: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_99, 645, 118);

    /* label_205: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_205, 275, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_205, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* temp: 标签 | "180" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 192, 249);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->temp, 180, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_207: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_207, 372, 281);
    lv_obj_set_size(pg->label_207, 38, 32);

    /* label_208: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_208, 855, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_208, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 788, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->hour, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_210: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_210, 38, 32);

    /* label_211: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_211, 1075, 281);
    lv_obj_set_size(pg->label_211, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_pos(pg->min, 990, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->min, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_213: 标签 | "热风" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_213, 24, 24);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_213, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_214: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_214, 1007, 41);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_214, 117, 32);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * hotwind_bbq_set 英文布局基准（对应 PAGE_HOTWIND_BBQ_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotwind_bbq_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTWIND_BBQ_SET) return;
    hotwind_bbq_set_t *pg = hotwind_bbq_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_215: 标签 | "热风" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_215, 24, 25);
    lv_obj_set_size(pg->label_215, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_216: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_216, 163, 155);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_216, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_217: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_217, 252, 157);
    lv_obj_set_size(pg->label_217, 64, 32);

    /* label_218: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_218, 150, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_218, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->min, 317, 258);
    lv_obj_set_size(pg->min, 68, 66);   /* 对齐 updown 系英文排版(A) */

    /* label_221: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_221, 368, 270);
    lv_obj_set_size(pg->label_221, 30, 32);

    /* label_222: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->label_222, 371, 271);
    lv_obj_set_size(pg->label_222, 42, 32);   /* 对齐 updown 系英文排版(A) */

    /* image_104: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_104, 164, 376);

    /* label_223: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_223, 232, 385);
    lv_obj_set_size(pg->label_223, 220, 32);   /* 对齐 updown 系英文排版(A) */

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_224: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_224, 785, 154);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_224, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_225: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_225, 885, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_225, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_226: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_226, 930, 270);
    lv_obj_set_size(pg->label_226, 26, 32);

    /* label_227: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_227, 765, 381);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_227, 200, 32);   /* 对齐 updown 系英文排版(A) */

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 269, 143);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->temp, 100, 53);   /* 对齐 top_bbq 系排版 */

    /* label_229: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_229, 1007, 41);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_229, 85, 36);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */

    /* onpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat, 983, 144);
    lv_obj_set_size(pg->onpreheat, 135, 53);

    /* ondelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 260);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 372);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 355, 157);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 327, 157);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * hotwind_bbq_setting 英文布局基准（对应 PAGE_HOTWIND_BBQ_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotwind_bbq_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTWIND_BBQ_SETTING) return;
    hotwind_bbq_setting_t *pg = hotwind_bbq_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 257, 60);   /* 对齐 top_bbq 系排版 */

    /* container_1: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_1, 0, 0);
    lv_obj_set_size(pg->container_1, 1280, 480);

    /* image_1: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_1, 115, 115);

    /* label_2: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_2, 203, 134);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_2, 150, 30);   /* 对齐 top_bbq 系排版 */

    /* label_3: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_3, 444, 134);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_3, 150, 30);   /* 对齐 top_bbq 系排版 */

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 217);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 303, 229);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 377, 217);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_8: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_8, 450, 231);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_8, 37, 30);

    /* label_9: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_9, 612, 231);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_9, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 289, 229);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * hotwind_bbq_stop 英文布局基准（对应 PAGE_HOTWIND_BBQ_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotwind_bbq_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTWIND_BBQ_STOP) return;
    hotwind_bbq_stop_t *pg = hotwind_bbq_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热风 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_107: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_107, 115, 320);

    /* bar_19: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_19, 122, 326);
    lv_obj_set_size(pg->bar_19, 637, 20);

    /* label_236: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_236, 273, 157);
    lv_obj_set_size(pg->label_236, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_108: 图片 | (115,161) | img: hotwindicon.png */
    lv_obj_set_pos(pg->image_108, 115, 161);

}


/* ==============================================================================
 * hotwind_bbq_stop_back 英文布局基准（对应 PAGE_HOTWIND_BBQ_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void hotwind_bbq_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_HOTWIND_BBQ_STOP_BACK) return;
    hotwind_bbq_stop_back_t *pg = hotwind_bbq_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_109: 图片 | (115,161) | img: hotwindicon.png */
    lv_obj_set_pos(pg->image_109, 115, 161);

    /* status: 标签 | "| 热风 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_110: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_110, 115, 320);

    /* bar_20: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_20, 122, 326);
    lv_obj_set_size(pg->bar_20, 637, 20);

    /* label_238: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_238, 273, 157);
    lv_obj_set_size(pg->label_238, 330, 60);   /* 对齐 updown bbq 系 */   /* 对齐 updown 系英文排版(A) */

    /* button_72: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->button_72, 609, 170);
    lv_obj_set_size(pg->button_72, 50, 43);

    /* container_10: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_10, 0, 0);
    lv_obj_set_size(pg->container_10, 1280, 480);

    /* label_239: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_239, 895, 160);
    lv_obj_set_size(pg->label_239, 252, 36);

    /* label_240: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_240, 895, 198);
    lv_obj_set_size(pg->label_240, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * lasagna_complete 英文布局基准（对应 PAGE_LASAGNA_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void lasagna_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_LASAGNA_COMPLETE) return;
    lasagna_complete_t *pg = lasagna_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_321: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_321, 115, 320);

    /* status: 标签 | "| 千层面 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_51: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_51, 122, 326);
    lv_obj_set_size(pg->bar_51, 637, 20);

    /* label_681: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_681, 273, 157);
    lv_obj_set_size(pg->label_681, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_323: 图片 | (115,161) | img: lasagnaicon.png */
    lv_obj_set_pos(pg->image_323, 115, 161);

    /* image_12: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_12, 902, 160);

}


/* ==============================================================================
 * lasagna_cooking 英文布局基准（对应 PAGE_LASAGNA_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void lasagna_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_LASAGNA_COOKING) return;
    lasagna_cooking_t *pg = lasagna_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 千层面 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_288: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_288, 115, 320);

    /* bar_47: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_47, 122, 326);
    lv_obj_set_size(pg->bar_47, 637, 20);

    /* label_660: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_660, 273, 157);
    lv_obj_set_size(pg->label_660, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_289: 图片 | (115,161) | img: lasagnaicon.png */
    lv_obj_set_pos(pg->image_289, 115, 161);

}


/* ==============================================================================
 * lasagna_menu 英文布局基准（对应 PAGE_LASAGNA_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void lasagna_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_LASAGNA_MENU) return;
    lasagna_menu_t *pg = lasagna_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_282: 图片 | (24,117) | img: div1bg.png */
    lv_obj_set_pos(pg->image_282, 24, 117);

    /* label_635: 标签 | "烹饪时间" | (578,139) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_635, 578, 139);
    lv_obj_set_size(pg->label_635, 125, 32);

    /* hour: 标签 | "00" | (482,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 482, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_637: 标签 | "时" | (562,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_637, 562, 280);
    lv_obj_set_size(pg->label_637, 38, 32);

    /* label_638: 标签 | "分" | (762,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_638, 762, 280);
    lv_obj_set_size(pg->label_638, 38, 32);

    /* min: 标签 | "30" | (682,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_640: 标签 | "千层面" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_640, 24, 25);
    lv_obj_set_size(pg->label_640, 144, 32);

    /* label_641: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_641, 996, 39);
    lv_obj_set_size(pg->label_641, 117, 32);

    /* hourline: 图片 | (480,327) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 480, 327);

    /* minline: 图片 | (680,327) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 680, 327);

}


/* ==============================================================================
 * lasagna_set 英文布局基准（对应 PAGE_LASAGNA_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void lasagna_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_LASAGNA_SET) return;
    lasagna_set_t *pg = lasagna_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_642: 标签 | "千层面" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_642, 24, 25);
    lv_obj_set_size(pg->label_642, 144, 32);

    /* label_643: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_643, 165, 157);
    lv_obj_set_size(pg->label_643, 26, 32);

    /* label_644: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_644, 252, 157);
    lv_obj_set_size(pg->label_644, 64, 32);

    /* label_645: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_645, 141, 270);
    lv_obj_set_size(pg->label_645, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_648: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_648, 368, 270);
    lv_obj_set_size(pg->label_648, 30, 32);

    /* label_649: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_649, 64, 32);

    /* image_287: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_287, 164, 376);

    /* label_650: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_650, 232, 385);
    lv_obj_set_size(pg->label_650, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_651: 标签 | "延" | (840,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_651, 840, 154);
    lv_obj_set_size(pg->label_651, 26, 32);

    /* label_652: 标签 | "迟" | (930,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_652, 930, 154);
    lv_obj_set_size(pg->label_652, 26, 32);

    /* label_653: 标签 | "自动保温" | (831,270) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_653, 831, 270);
    lv_obj_set_size(pg->label_653, 130, 32);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 260);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "230" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_655: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_655, 1010, 39);
    lv_obj_set_size(pg->label_655, 85, 36);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 260);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * lasagna_setting 英文布局基准（对应 PAGE_LASAGNA_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void lasagna_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_LASAGNA_SETTING) return;
    lasagna_setting_t *pg = lasagna_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_29: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_29, 0, 0);
    lv_obj_set_size(pg->container_29, 1280, 480);

    /* image_313: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_313, 115, 115);

    /* label_663: 标签 | "烹饪时间" | (373,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_663, 373, 136);
    lv_obj_set_size(pg->label_663, 135, 30);

    /* hour: 标签 | "00" | (266,212) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 266, 212);
    lv_obj_set_size(pg->hour, 102, 46);

    /* label_668: 标签 | "时" | (340,228) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_668, 340, 228);
    lv_obj_set_size(pg->label_668, 37, 30);

    /* hourline: 图片 | (291,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 291, 269);

    /* min: 标签 | "00" | (483,212) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_669: 标签 | "分" | (558,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_669, 558, 229);
    lv_obj_set_size(pg->label_669, 37, 30);

    /* minline: 图片 | (508,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 508, 269);

    /* button_200: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->button_200, 959, 295);
    lv_obj_set_size(pg->button_200, 129, 83);

}


/* ==============================================================================
 * lasagna_stop 英文布局基准（对应 PAGE_LASAGNA_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void lasagna_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_LASAGNA_STOP) return;
    lasagna_stop_t *pg = lasagna_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 千层面 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_317: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_317, 115, 320);

    /* bar_49: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_49, 122, 326);
    lv_obj_set_size(pg->bar_49, 637, 20);

    /* label_675: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_675, 273, 157);
    lv_obj_set_size(pg->label_675, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_318: 图片 | (115,161) | img: lasagnaicon.png */
    lv_obj_set_pos(pg->image_318, 115, 161);

}


/* ==============================================================================
 * lasagna_stop_back 英文布局基准（对应 PAGE_LASAGNA_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void lasagna_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_LASAGNA_STOP_BACK) return;
    lasagna_stop_back_t *pg = lasagna_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_319: 图片 | (115,161) | img: lasagnaicon.png */
    lv_obj_set_pos(pg->image_319, 115, 161);

    /* status: 标签 | "| 千层面 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_320: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_320, 115, 320);

    /* bar_50: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_50, 122, 326);
    lv_obj_set_size(pg->bar_50, 637, 20);

    /* label_677: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_677, 273, 157);
    lv_obj_set_size(pg->label_677, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_30: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_30, 0, 0);
    lv_obj_set_size(pg->container_30, 1280, 480);

    /* label_678: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_678, 895, 160);
    lv_obj_set_size(pg->label_678, 252, 36);

    /* label_679: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_679, 895, 198);
    lv_obj_set_size(pg->label_679, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * major_menu 英文布局基准（对应 PAGE_MAJOR_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void major_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_MAJOR_MENU) return;
    major_menu_t *pg = major_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xffffff | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* cook4_button: 按钮 | (421,60) | 439x417 | font montserratmedium_16 | bg: div3bk30.png */
    lv_obj_set_pos(pg->cook4_button, 421, 60);
    lv_obj_set_size(pg->cook4_button, 439, 417);

    /* cook_button: 按钮 | (7,60) | 439x417 | font montserratmedium_16 | bg: div3bk30.png */
    lv_obj_set_pos(pg->cook_button, 7, 60);
    lv_obj_set_size(pg->cook_button, 439, 417);

    /* special_button: 按钮 | (835,60) | 439x417 | font montserratmedium_16 | bg: div3bk30.png */
    lv_obj_set_pos(pg->special_button, 835, 60);
    lv_obj_set_size(pg->special_button, 439, 417);

    /* major_img: 图片 | (196,187) | img: menuimg.png */
    lv_obj_set_pos(pg->major_img, 196, 187);

    /* menu_label: 标签 | "菜单" | (24,24) | 58x22 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->menu_label, 24, 24);
    lv_obj_set_size(pg->menu_label, 80, 22);

    /* cook_label: 标签 | "烹饪功能" | (153,291) | 147x28 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->cook_label, 153, 291);
    lv_obj_set_size(pg->cook_label, 147, 28);

    /* cook4_label: 标签 | "COOK 4" | (567,291) | 147x28 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->cook4_label, 567, 291);
    lv_obj_set_size(pg->cook4_label, 147, 28);

    /* special_label: 标签 | "特殊功能" | (981,291) | 147x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->special_label, 914, 291);
    lv_obj_set_size(pg->special_label, 260, 30);

}


/* ==============================================================================
 * major_menu_tz 英文布局基准（对应 PAGE_MAJOR_MENU_TZ ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void major_menu_tz_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_MAJOR_MENU_TZ) return;
    major_menu_tz_t *pg = major_menu_tz_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* cook_button: 按钮 | (9,73) | 642x390 | font montserratmedium_16 | bg: div2bk30.png */
    lv_obj_set_pos(pg->cook_button, 9, 73);
    lv_obj_set_size(pg->cook_button, 642, 390);

    /* special_button: 按钮 | (630,73) | 642x390 | font montserratmedium_16 | bg: div2bk30.png */
    lv_obj_set_pos(pg->special_button, 630, 73);
    lv_obj_set_size(pg->special_button, 642, 390);

    /* menu_label: 标签 | "菜单" | (24,24) | 48x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->menu_label, 24, 24);
    lv_obj_set_size(pg->menu_label, 80, 32);

    /* tz_img: 图片 | (96,24) | img: tanzhen.png */
    lv_obj_set_pos(pg->tz_img, 96, 24);

    /* major_img: 图片 | (300,185) | img: menutzimg.png */
    lv_obj_set_pos(pg->major_img, 300, 185);

    /* cook_label: 标签 | "烹饪功能" | (270,291) | 120x28 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->cook_label, 270, 291);
    lv_obj_set_size(pg->cook_label, 120, 28);

    /* special_label: 标签 | "特殊功能" | (891,291) | 120x28 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->special_label, 891, 291);
    lv_obj_set_size(pg->special_label, 120, 28);

}


/* ==============================================================================
 * menu_complete 英文布局基准（对应 PAGE_MENU_COOK_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void menu_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_MENU_COOK_COMPLETE) return;
    menu_complete_t *pg = menu_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_132: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_132, 115, 320);

    /* status: 标签 | "| 菜单 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_19: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_19, 122, 326);
    lv_obj_set_size(pg->bar_19, 637, 20);

    /* label_288: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_288, 273, 157);
    lv_obj_set_size(pg->label_288, 235, 60);

    /* image_133: 图片 | (845,160) | img: hotcare.png */
    lv_obj_set_pos(pg->image_133, 845, 160);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_134: 图片 | (115,161) | img: menuicon.png */
    lv_obj_set_pos(pg->image_134, 115, 161);

}


/* ==============================================================================
 * menu_cooking 英文布局基准（对应 PAGE_MENU_COOK_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void menu_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_MENU_COOK_COOKING) return;
    menu_cooking_t *pg = menu_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 菜单 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_121: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_121, 115, 320);

    /* bar_16: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_16, 122, 326);
    lv_obj_set_size(pg->bar_16, 637, 20);

    /* label_269: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_269, 273, 157);
    lv_obj_set_size(pg->label_269, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_122: 图片 | (115,161) | img: menuicon.png */
    lv_obj_set_pos(pg->image_122, 115, 161);

}


/* ==============================================================================
 * menu_menu 英文布局基准（对应 PAGE_MENU_COOK_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void menu_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_MENU_COOK_MENU) return;
    menu_menu_t *pg = menu_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_114: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_114, 24, 118);

    /* image_115: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_115, 645, 118);

    /* label_241: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_241, 300, 137);
    lv_obj_set_size(pg->label_241, 64, 32);

    /* temp: 标签 | "190" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_243: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_243, 372, 281);
    lv_obj_set_size(pg->label_243, 38, 32);

    /* label_244: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_244, 891, 137);
    lv_obj_set_size(pg->label_244, 125, 32);

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 793, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_246: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_246, 872, 280);
    lv_obj_set_size(pg->label_246, 38, 32);

    /* label_247: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_247, 1075, 281);
    lv_obj_set_size(pg->label_247, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_249: 标签 | "菜单" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_249, 24, 25);
    lv_obj_set_size(pg->label_249, 144, 32);

    /* label_250: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_250, 996, 39);
    lv_obj_set_size(pg->label_250, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * menu_set 英文布局基准（对应 PAGE_MENU_COOK_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void menu_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_MENU_COOK_SET) return;
    menu_set_t *pg = menu_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_251: 标签 | "菜单" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_251, 24, 25);
    lv_obj_set_size(pg->label_251, 144, 32);

    /* label_252: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_252, 165, 157);
    lv_obj_set_size(pg->label_252, 26, 32);

    /* label_253: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_253, 252, 157);
    lv_obj_set_size(pg->label_253, 64, 32);

    /* label_254: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_254, 141, 270);
    lv_obj_set_size(pg->label_254, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_257: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_257, 368, 270);
    lv_obj_set_size(pg->label_257, 30, 32);

    /* label_258: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_258, 64, 32);

    /* image_120: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_120, 164, 376);

    /* label_259: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_259, 232, 385);
    lv_obj_set_size(pg->label_259, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_260: 标签 | "延" | (840,154) | 26x32 | font taiwanpearl_regular_30 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_260, 26, 32);

    /* label_261: 标签 | "迟" | (930,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_261, 930, 154);
    lv_obj_set_size(pg->label_261, 26, 32);

    /* label_262: 标签 | "自动保温" | (831,270) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_262, 831, 270);
    lv_obj_set_size(pg->label_262, 130, 32);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 260);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "210" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_264: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_264, 1010, 39);
    lv_obj_set_size(pg->label_264, 85, 36);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 260);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * menu_setting 英文布局基准（对应 PAGE_MENU_COOK_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void menu_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_MENU_COOK_SETTING) return;
    menu_setting_t *pg = menu_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_13: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_13, 0, 0);
    lv_obj_set_size(pg->container_13, 1280, 480);

    /* image_123: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_123, 115, 115);

    /* label_271: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_271, 242, 136);
    lv_obj_set_size(pg->label_271, 73, 30);

    /* label_272: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_272, 454, 136);
    lv_obj_set_size(pg->label_272, 135, 30);

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 213);
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 300, 229);
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 374, 213);
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_277: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_277, 449, 229);
    lv_obj_set_size(pg->label_277, 37, 30);

    /* label_278: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_278, 612, 229);
    lv_obj_set_size(pg->label_278, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 288, 229);
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * menu_stop 英文布局基准（对应 PAGE_MENU_COOK_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void menu_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_MENU_COOK_STOP) return;
    menu_stop_t *pg = menu_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 菜单 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_128: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_128, 115, 320);

    /* bar_17: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_17, 122, 326);
    lv_obj_set_size(pg->bar_17, 637, 20);

    /* label_282: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_282, 273, 157);
    lv_obj_set_size(pg->label_282, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_129: 图片 | (115,161) | img: menuicon.png */
    lv_obj_set_pos(pg->image_129, 115, 161);

}


/* ==============================================================================
 * menu_stop_back 英文布局基准（对应 PAGE_MENU_COOK_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void menu_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_MENU_COOK_STOP_BACK) return;
    menu_stop_back_t *pg = menu_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_130: 图片 | (115,161) | img: menuicon.png */
    lv_obj_set_pos(pg->image_130, 115, 161);

    /* status: 标签 | "| 菜单 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_131: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_131, 115, 320);

    /* bar_18: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_18, 122, 326);
    lv_obj_set_size(pg->bar_18, 637, 20);

    /* label_284: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_284, 273, 157);
    lv_obj_set_size(pg->label_284, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_14: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_14, 0, 0);
    lv_obj_set_size(pg->container_14, 1280, 480);

    /* label_285: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_285, 895, 160);
    lv_obj_set_size(pg->label_285, 252, 36);

    /* label_286: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_286, 895, 198);
    lv_obj_set_size(pg->label_286, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * pizza3_complete 英文布局基准（对应 PAGE_PIZZA3_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza3_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA3_COMPLETE) return;
    pizza3_complete_t *pg = pizza3_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_369: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_369, 115, 320);

    /* status: 标签 | "| 披萨 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_63: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_63, 122, 326);
    lv_obj_set_size(pg->bar_63, 637, 20);

    /* label_804: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_804, 273, 157);
    lv_obj_set_size(pg->label_804, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_371: 图片 | (115,161) | img: pizza3icon.png */
    lv_obj_set_pos(pg->image_371, 115, 161);

    /* image_15: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_15, 902, 160);

}


/* ==============================================================================
 * pizza3_cooking 英文布局基准（对应 PAGE_PIZZA3_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza3_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA3_COOKING) return;
    pizza3_cooking_t *pg = pizza3_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 披萨 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_360: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_360, 115, 320);

    /* bar_60: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_60, 122, 326);
    lv_obj_set_size(pg->bar_60, 637, 20);

    /* label_789: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_789, 273, 157);
    lv_obj_set_size(pg->label_789, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_361: 图片 | (115,161) | img: pizza3icon.png */
    lv_obj_set_pos(pg->image_361, 115, 161);

}


/* ==============================================================================
 * pizza3_menu 英文布局基准（对应 PAGE_PIZZA3_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza3_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA3_MENU) return;
    pizza3_menu_t *pg = pizza3_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_356: 图片 | (24,117) | img: div1bg.png */
    lv_obj_set_pos(pg->image_356, 24, 117);

    /* label_764: 标签 | "烹饪时间" | (578,139) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_764, 578, 139);
    lv_obj_set_size(pg->label_764, 125, 32);

    /* hour: 标签 | "00" | (482,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 482, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_766: 标签 | "时" | (562,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_766, 562, 280);
    lv_obj_set_size(pg->label_766, 38, 32);

    /* label_767: 标签 | "分" | (762,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_767, 762, 280);
    lv_obj_set_size(pg->label_767, 38, 32);

    /* min: 标签 | "30" | (682,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_769: 标签 | "披萨" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_769, 24, 25);
    lv_obj_set_size(pg->label_769, 144, 32);

    /* label_770: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_770, 996, 39);
    lv_obj_set_size(pg->label_770, 117, 32);

    /* hourline: 图片 | (480,327) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 480, 327);

    /* minline: 图片 | (680,327) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 680, 327);

}


/* ==============================================================================
 * pizza3_set 英文布局基准（对应 PAGE_PIZZA3_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza3_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA3_SET) return;
    pizza3_set_t *pg = pizza3_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_771: 标签 | "披萨" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_771, 24, 25);
    lv_obj_set_size(pg->label_771, 144, 32);

    /* label_772: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_772, 165, 157);
    lv_obj_set_size(pg->label_772, 26, 32);

    /* label_773: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_773, 252, 157);
    lv_obj_set_size(pg->label_773, 64, 32);

    /* label_774: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_774, 141, 270);
    lv_obj_set_size(pg->label_774, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_777: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_777, 368, 270);
    lv_obj_set_size(pg->label_777, 30, 32);

    /* label_778: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_778, 64, 32);

    /* image_359: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_359, 164, 376);

    /* label_779: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_779, 232, 385);
    lv_obj_set_size(pg->label_779, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_780: 标签 | "延" | (840,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_780, 840, 154);
    lv_obj_set_size(pg->label_780, 26, 32);

    /* label_781: 标签 | "迟" | (930,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_781, 930, 154);
    lv_obj_set_size(pg->label_781, 26, 32);

    /* label_782: 标签 | "自动保温" | (831,270) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_782, 831, 270);
    lv_obj_set_size(pg->label_782, 130, 32);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 260);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "250" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_784: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_784, 1010, 39);
    lv_obj_set_size(pg->label_784, 85, 36);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 260);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * pizza3_setting 英文布局基准（对应 PAGE_PIZZA3_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza3_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA3_SETTING) return;
    pizza3_setting_t *pg = pizza3_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_35: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_35, 0, 0);
    lv_obj_set_size(pg->container_35, 1280, 480);

    /* image_362: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_362, 115, 115);

    /* label_791: 标签 | "烹饪时间" | (373,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_791, 373, 136);
    lv_obj_set_size(pg->label_791, 135, 30);

    /* hour: 标签 | "00" | (266,212) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 266, 212);
    lv_obj_set_size(pg->hour, 102, 46);

    /* label_793: 标签 | "时" | (340,228) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_793, 340, 228);
    lv_obj_set_size(pg->label_793, 37, 30);

    /* hourline: 图片 | (291,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 291, 269);

    /* min: 标签 | "00" | (483,212) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_795: 标签 | "分" | (558,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_795, 558, 229);
    lv_obj_set_size(pg->label_795, 37, 30);

    /* minline: 图片 | (508,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 508, 269);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * pizza3_stop 英文布局基准（对应 PAGE_PIZZA3_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza3_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA3_STOP) return;
    pizza3_stop_t *pg = pizza3_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 披萨 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_365: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_365, 115, 320);

    /* bar_61: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_61, 122, 326);
    lv_obj_set_size(pg->bar_61, 637, 20);

    /* label_798: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_798, 273, 157);
    lv_obj_set_size(pg->label_798, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_366: 图片 | (115,161) | img: pizza3icon.png */
    lv_obj_set_pos(pg->image_366, 115, 161);

}


/* ==============================================================================
 * pizza3_stop_back 英文布局基准（对应 PAGE_PIZZA3_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza3_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA3_STOP_BACK) return;
    pizza3_stop_back_t *pg = pizza3_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_367: 图片 | (115,161) | img: pizza3icon.png */
    lv_obj_set_pos(pg->image_367, 115, 161);

    /* status: 标签 | "| 披萨 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_368: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_368, 115, 320);

    /* bar_62: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_62, 122, 326);
    lv_obj_set_size(pg->bar_62, 637, 20);

    /* label_800: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_800, 273, 157);
    lv_obj_set_size(pg->label_800, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_36: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_36, 0, 0);
    lv_obj_set_size(pg->container_36, 1280, 480);

    /* label_801: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_801, 895, 160);
    lv_obj_set_size(pg->label_801, 252, 36);

    /* label_802: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_802, 895, 198);
    lv_obj_set_size(pg->label_802, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * pizza_2_complete 英文布局基准（对应 PAGE_PIZZA_2_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_2_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_2_COMPLETE) return;
    pizza_2_complete_t *pg = pizza_2_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_174: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_174, 115, 320);

    /* status: 标签 | "| 披萨 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_27: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_27, 122, 326);
    lv_obj_set_size(pg->bar_27, 637, 20);

    /* label_386: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_386, 273, 157);
    lv_obj_set_size(pg->label_386, 235, 60);

    /* image_175: 图片 | (845,160) | img: hotcare.png */
    lv_obj_set_pos(pg->image_175, 845, 160);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_176: 图片 | (140,161) | img: pizza2icon.png */
    lv_obj_set_pos(pg->image_176, 140, 161);

}


/* ==============================================================================
 * pizza_2_cooking 英文布局基准（对应 PAGE_PIZZA_2_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_2_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_2_COOKING) return;
    pizza_2_cooking_t *pg = pizza_2_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 披萨 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_163: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_163, 115, 320);

    /* bar_24: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_24, 122, 326);
    lv_obj_set_size(pg->bar_24, 637, 20);

    /* label_367: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_367, 273, 157);
    lv_obj_set_size(pg->label_367, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_164: 图片 | (140,161) | img: pizza2icon.png */
    lv_obj_set_pos(pg->image_164, 140, 161);

}


/* ==============================================================================
 * pizza_2_menu 英文布局基准（对应 PAGE_PIZZA_2_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_2_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_2_MENU) return;
    pizza_2_menu_t *pg = pizza_2_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_156: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_156, 24, 118);

    /* image_157: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_157, 645, 118);

    /* label_338: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_338, 300, 137);
    lv_obj_set_size(pg->label_338, 64, 32);

    /* temp: 标签 | "230" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_340: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_340, 372, 281);
    lv_obj_set_size(pg->label_340, 38, 32);

    /* label_341: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_341, 891, 137);
    lv_obj_set_size(pg->label_341, 125, 32);

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 793, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_343: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_343, 872, 280);
    lv_obj_set_size(pg->label_343, 38, 32);

    /* label_344: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_344, 1075, 281);
    lv_obj_set_size(pg->label_344, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_346: 标签 | "披萨" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_346, 24, 25);
    lv_obj_set_size(pg->label_346, 144, 32);

    /* label_347: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_347, 996, 39);
    lv_obj_set_size(pg->label_347, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * pizza_2_set 英文布局基准（对应 PAGE_PIZZA_2_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_2_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_2_SET) return;
    pizza_2_set_t *pg = pizza_2_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_348: 标签 | "披萨" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_348, 24, 25);
    lv_obj_set_size(pg->label_348, 144, 32);

    /* label_349: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_349, 165, 157);
    lv_obj_set_size(pg->label_349, 26, 32);

    /* label_350: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_350, 252, 157);
    lv_obj_set_size(pg->label_350, 64, 32);

    /* label_351: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_351, 141, 270);
    lv_obj_set_size(pg->label_351, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_354: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_354, 368, 270);
    lv_obj_set_size(pg->label_354, 30, 32);

    /* label_355: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_355, 64, 32);

    /* image_162: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_162, 164, 376);

    /* label_356: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_356, 232, 385);
    lv_obj_set_size(pg->label_356, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_357: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_357, 830, 154);
    lv_obj_set_size(pg->label_357, 130, 32);

    /* label_358: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_358, 840, 270);
    lv_obj_set_size(pg->label_358, 26, 32);

    /* label_359: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_359, 930, 270);
    lv_obj_set_size(pg->label_359, 26, 32);

    /* label_360: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_360, 831, 381);
    lv_obj_set_size(pg->label_360, 130, 32);

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_362: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_362, 1010, 39);
    lv_obj_set_size(pg->label_362, 85, 36);

    /* onpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat, 983, 144);
    lv_obj_set_size(pg->onpreheat, 135, 53);

    /* ondelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 260);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 372);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * pizza_2_setting 英文布局基准（对应 PAGE_PIZZA_2_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_2_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_2_SETTING) return;
    pizza_2_setting_t *pg = pizza_2_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_17: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_17, 0, 0);
    lv_obj_set_size(pg->container_17, 1280, 480);

    /* image_165: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_165, 115, 115);

    /* label_369: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_369, 242, 136);
    lv_obj_set_size(pg->label_369, 73, 30);

    /* label_370: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_370, 454, 136);
    lv_obj_set_size(pg->label_370, 135, 30);

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 213);
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 300, 229);
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 374, 213);
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_375: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_375, 449, 229);
    lv_obj_set_size(pg->label_375, 37, 30);

    /* label_376: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_376, 612, 229);
    lv_obj_set_size(pg->label_376, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 288, 229);
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * pizza_2_stop 英文布局基准（对应 PAGE_PIZZA_2_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_2_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_2_STOP) return;
    pizza_2_stop_t *pg = pizza_2_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 披萨 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_170: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_170, 115, 320);

    /* bar_25: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_25, 122, 326);
    lv_obj_set_size(pg->bar_25, 637, 20);

    /* label_380: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_380, 273, 157);
    lv_obj_set_size(pg->label_380, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_171: 图片 | (140,161) | img: pizza2icon.png */
    lv_obj_set_pos(pg->image_171, 140, 161);

}


/* ==============================================================================
 * pizza_2_stop_back 英文布局基准（对应 PAGE_PIZZA_2_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_2_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_2_STOP_BACK) return;
    pizza_2_stop_back_t *pg = pizza_2_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_172: 图片 | (140,161) | img: pizza2icon.png */
    lv_obj_set_pos(pg->image_172, 140, 161);

    /* status: 标签 | "| 披萨 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_173: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_173, 115, 320);

    /* bar_26: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_26, 122, 326);
    lv_obj_set_size(pg->bar_26, 637, 20);

    /* label_382: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_382, 273, 157);
    lv_obj_set_size(pg->label_382, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_18: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_18, 0, 0);
    lv_obj_set_size(pg->container_18, 1280, 480);

    /* label_383: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_383, 895, 160);
    lv_obj_set_size(pg->label_383, 252, 36);

    /* label_384: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_384, 895, 198);
    lv_obj_set_size(pg->label_384, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * pizza_complete 英文布局基准（对应 PAGE_PIZZA_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_COMPLETE) return;
    pizza_complete_t *pg = pizza_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_111: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_111, 115, 320);

    /* status: 标签 | "| 披萨 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_15: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_15, 122, 326);
    lv_obj_set_size(pg->bar_15, 637, 20);

    /* label_240: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_240, 273, 157);
    lv_obj_set_size(pg->label_240, 235, 60);

    /* image_112: 图片 | (845,160) | img: hotcare.png */
    lv_obj_set_pos(pg->image_112, 845, 160);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_113: 图片 | (115,161) | img: pizzaicon.png */
    lv_obj_set_pos(pg->image_113, 115, 161);

}


/* ==============================================================================
 * pizza_cooking 英文布局基准（对应 PAGE_PIZZA_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_COOKING) return;
    pizza_cooking_t *pg = pizza_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 披萨 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_100: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_100, 115, 320);

    /* bar_12: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_12, 122, 326);
    lv_obj_set_size(pg->bar_12, 637, 20);

    /* label_221: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_221, 273, 157);
    lv_obj_set_size(pg->label_221, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_101: 图片 | (115,161) | img: pizzaicon.png */
    lv_obj_set_pos(pg->image_101, 115, 161);

}


/* ==============================================================================
 * pizza_menu 英文布局基准（对应 PAGE_PIZZA_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_MENU) return;
    pizza_menu_t *pg = pizza_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_93: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_93, 24, 118);

    /* image_94: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_94, 645, 118);

    /* label_193: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_193, 300, 137);
    lv_obj_set_size(pg->label_193, 64, 32);

    /* temp: 标签 | "210" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_195: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_195, 372, 281);
    lv_obj_set_size(pg->label_195, 38, 32);

    /* label_196: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_196, 891, 137);
    lv_obj_set_size(pg->label_196, 125, 32);

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 793, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_198: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_198, 872, 280);
    lv_obj_set_size(pg->label_198, 38, 32);

    /* label_199: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_199, 1075, 281);
    lv_obj_set_size(pg->label_199, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_201: 标签 | "披萨" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_201, 24, 25);
    lv_obj_set_size(pg->label_201, 144, 32);

    /* label_202: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_202, 996, 39);
    lv_obj_set_size(pg->label_202, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * pizza_set 英文布局基准（对应 PAGE_PIZZA_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_SET) return;
    pizza_set_t *pg = pizza_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_203: 标签 | "披萨" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_203, 24, 25);
    lv_obj_set_size(pg->label_203, 144, 32);

    /* label_204: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_204, 165, 157);
    lv_obj_set_size(pg->label_204, 26, 32);

    /* label_205: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_205, 252, 157);
    lv_obj_set_size(pg->label_205, 64, 32);

    /* label_206: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_206, 141, 270);
    lv_obj_set_size(pg->label_206, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_209: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_209, 368, 270);
    lv_obj_set_size(pg->label_209, 30, 32);

    /* label_210: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_210, 64, 32);

    /* image_99: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_99, 164, 376);

    /* label_211: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_211, 232, 385);
    lv_obj_set_size(pg->label_211, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_212: 标签 | "延" | (840,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_212, 840, 154);
    lv_obj_set_size(pg->label_212, 26, 32);

    /* label_213: 标签 | "迟" | (930,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_213, 930, 154);
    lv_obj_set_size(pg->label_213, 26, 32);

    /* label_214: 标签 | "自动保温" | (831,270) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_214, 831, 270);
    lv_obj_set_size(pg->label_214, 130, 32);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 260);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "210" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_216: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_216, 1010, 39);
    lv_obj_set_size(pg->label_216, 85, 36);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 260);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * pizza_setting 英文布局基准（对应 PAGE_PIZZA_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_SETTING) return;
    pizza_setting_t *pg = pizza_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_11: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_11, 0, 0);
    lv_obj_set_size(pg->container_11, 1280, 480);

    /* image_102: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_102, 115, 115);

    /* label_223: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_223, 242, 136);
    lv_obj_set_size(pg->label_223, 73, 30);

    /* label_224: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_224, 454, 136);
    lv_obj_set_size(pg->label_224, 135, 30);

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 213);
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 300, 229);
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 374, 213);
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_229: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_229, 449, 229);
    lv_obj_set_size(pg->label_229, 37, 30);

    /* label_230: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_230, 612, 229);
    lv_obj_set_size(pg->label_230, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 288, 229);
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * pizza_stop 英文布局基准（对应 PAGE_PIZZA_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_STOP) return;
    pizza_stop_t *pg = pizza_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 披萨 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_107: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_107, 115, 320);

    /* bar_13: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_13, 122, 326);
    lv_obj_set_size(pg->bar_13, 637, 20);

    /* label_234: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_234, 273, 157);
    lv_obj_set_size(pg->label_234, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_108: 图片 | (115,161) | img: pizzaicon.png */
    lv_obj_set_pos(pg->image_108, 115, 161);

}


/* ==============================================================================
 * pizza_stop_back 英文布局基准（对应 PAGE_PIZZA_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void pizza_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PIZZA_STOP_BACK) return;
    pizza_stop_back_t *pg = pizza_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_109: 图片 | (115,161) | img: pizzaicon.png */
    lv_obj_set_pos(pg->image_109, 115, 161);

    /* status: 标签 | "| 披萨 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_110: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_110, 115, 320);

    /* bar_14: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_14, 122, 326);
    lv_obj_set_size(pg->bar_14, 637, 20);

    /* label_236: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_236, 273, 157);
    lv_obj_set_size(pg->label_236, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_12: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_12, 0, 0);
    lv_obj_set_size(pg->container_12, 1280, 480);

    /* label_237: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_237, 895, 160);
    lv_obj_set_size(pg->label_237, 252, 36);

    /* label_238: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_238, 895, 198);
    lv_obj_set_size(pg->label_238, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * preheat_stop_back 英文布局基准（对应 PAGE_PREHEAT_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void preheat_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PREHEAT_STOP_BACK) return;
    preheat_stop_back_t *pg = preheat_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 预热 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_3: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_3, 115, 320);

    /* bar_1: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_1, 122, 326);
    lv_obj_set_size(pg->bar_1, 637, 20);

    /* name: 标签 | "暂停中..." | (273,157) | 255x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->name, 273, 157);
    lv_obj_set_size(pg->name, 255, 60);

    /* icon: 图片 | (115,161) | img: preheaticon.png | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */

    /* bartemp: 标签 | "110℃" | (125,323) | 66x24 | font taiwanpearl_regular_24 | 定时器每秒重写位置(tune无效, 用注册表dx/dy) */
    lv_obj_set_size(pg->bartemp, 66, 24);

    /* container_1: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_1, 0, 0);
    lv_obj_set_size(pg->container_1, 1280, 480);

    /* tip2: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->tip2, 895, 198);
    lv_obj_set_size(pg->tip2, 252, 36);

    /* tip1: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->tip1, 895, 160);
    lv_obj_set_size(pg->tip1, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * preheatcomplete 英文布局基准（对应 PAGE_PREHEAT_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void preheatcomplete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PREHEAT_COMPLETE) return;
    preheatcomplete_t *pg = preheatcomplete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_43: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_43, 115, 320);

    /* status: 标签 | "| 预热 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_3: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_3, 122, 326);
    lv_obj_set_size(pg->bar_3, 637, 20);

    /* name: 标签 | "预热完成" | (273,157) | 246x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->name, 273, 157);
    lv_obj_set_size(pg->name, 246, 60);

    /* icon: 图片 | (115,161) | img: preheaticon.png | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */

    /* tip1: 标签 | "请放入食材，" | (932,161) | 180x29 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->tip1, 932, 161);
    lv_obj_set_size(pg->tip1, 180, 29);

    /* tip2: 标签 | "选择相应模式！" | (917,197) | 210x29 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->tip2, 917, 197);
    lv_obj_set_size(pg->tip2, 210, 29);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

    /* bartemp: 标签 | "110℃" | (679,323) | 66x24 | font taiwanpearl_regular_24 | 定时器每秒重写位置(tune无效, 用注册表dx/dy) */
    lv_obj_set_size(pg->bartemp, 66, 24);

}


/* ==============================================================================
 * preheatcooking 英文布局基准（对应 PAGE_PREHEAT_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void preheatcooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PREHEAT_COOKING) return;
    preheatcooking_t *pg = preheatcooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 预热 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* temp: 标签 | "180℃" | (940,157) | 159x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->temp, 940, 157);
    lv_obj_set_size(pg->temp, 159, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_39: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_39, 115, 320);

    /* bar_1: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_1, 122, 326);
    lv_obj_set_size(pg->bar_1, 637, 20);

    /* name: 标签 | "预热中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->name, 273, 157);
    lv_obj_set_size(pg->name, 235, 60);

    /* icon: 图片 | (115,161) | img: preheaticon.png | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */

    /* bartemp: 标签 | "110℃" | (125,323) | 66x24 | font taiwanpearl_regular_24 | 定时器每秒重写位置(tune无效, 用注册表dx/dy) */
    lv_obj_set_size(pg->bartemp, 66, 24);

}


/* ==============================================================================
 * preheatmenu 公共布局（复用结构，以下页面共用：PAGE_COLOR_MENU(额外上色设置页（复用预热菜单结构）), PAGE_PREHEAT_MENU(预热菜单页)）
 * 公共部分在此统一调整；各页差异见下方独立函数
 * ============================================================================== */
static void preheatmenu_common(void)
{
    preheatmenu_t *pg = preheatmenu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_33: 图片 | (24,118) | img: preheatbg.png */
    lv_obj_set_pos(pg->image_33, 24, 118);

    /* label_61: 标签 | "温度" | (608,139) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_61, 608, 139);
    lv_obj_set_size(pg->label_61, 64, 32);

    /* temp: 标签 | "180" | (562,248) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 562, 248);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_63: 标签 | "℃" | (682,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_63, 682, 280);
    lv_obj_set_size(pg->label_63, 38, 32);

    /* label_69: 标签 | "预热" | (24,24) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_69, 24, 24);
    lv_obj_set_size(pg->label_69, 144, 32);

    /* label_70: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_70, 996, 39);
    lv_obj_set_size(pg->label_70, 117, 32);

    /* line3: 图片 | (562,327) | img: underline.png */
    lv_obj_set_pos(pg->line3, 562, 327);

    /* line2: 图片 | (599,327) | img: underline_120x4.png */
    lv_obj_set_pos(pg->line2, 599, 327);

}

/* ==============================================================================
 * color_menu_lang_tune（对应 PAGE_COLOR_MENU 额外上色设置页（复用预热菜单结构））
 * 公共布局调 preheatmenu_common()，本页差异直接改下方数字
 * ============================================================================== */
void color_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COLOR_MENU) return;
    preheatmenu_t *pg = preheatmenu_get(&ui_manager);
    if (!pg) return;

    preheatmenu_common();

    /* 本页无差异对象 */
}

/* ==============================================================================
 * preheat_menu_lang_tune（对应 PAGE_PREHEAT_MENU 预热菜单页）
 * 公共布局调 preheatmenu_common()，本页差异直接改下方数字
 * ============================================================================== */
void preheat_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PREHEAT_MENU) return;
    preheatmenu_t *pg = preheatmenu_get(&ui_manager);
    if (!pg) return;

    preheatmenu_common();

    /* 本页无差异对象 */
}


/* ==============================================================================
 * preheatstop 英文布局基准（对应 PAGE_PREHEAT_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void preheatstop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PREHEAT_STOP) return;
    preheatstop_t *pg = preheatstop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 预热 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* temp: 标签 | "180℃" | (940,157) | 159x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->temp, 940, 157);
    lv_obj_set_size(pg->temp, 159, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_41: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_41, 115, 320);

    /* bar_2: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_2, 122, 326);
    lv_obj_set_size(pg->bar_2, 637, 20);

    /* name: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->name, 273, 157);
    lv_obj_set_size(pg->name, 235, 60);

    /* icon: 图片 | (115,161) | img: preheaticon.png | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */

    /* bartemp: 标签 | "110℃" | (125,323) | 66x24 | font taiwanpearl_regular_24 | 定时器每秒重写位置(tune无效, 用注册表dx/dy) */
    lv_obj_set_size(pg->bartemp, 66, 24);

}


/* ==============================================================================
 * probeneedtip 英文布局基准（对应 PAGE_PROBENEEDTIP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void probeneedtip_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PROBENEEDTIP) return;
    probeneedtip_t *pg = probeneedtip_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* button_20: 按钮 | (19,113) | 1243x347 | font taiwanpearl_regular_48 | bg: div1bg.png */
    lv_obj_set_pos(pg->button_20, 19, 113);
    lv_obj_set_size(pg->button_20, 1243, 347);

    /* name: 标签 | "蔬菜" | (24,24) | 97x25 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->name, 24, 24);
    lv_obj_set_size(pg->name, 97, 25);

    /* sure: 按钮 | (986,21) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 986, 21);
    lv_obj_set_size(pg->sure, 135, 71);

}


/* ==============================================================================
 * probetip 英文布局基准（对应 PAGE_PROBETIP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void probetip_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_PROBETIP) return;
    probetip_t *pg = probetip_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* button_1: 按钮 | (19,113) | 1243x347 | font taiwanpearl_regular_48 | bg: div1bg.png */
    lv_obj_set_pos(pg->button_1, 19, 113);
    lv_obj_set_size(pg->button_1, 1243, 347);

}


/* ==============================================================================
 * rising_complete 英文布局基准（对应 PAGE_RISING_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void rising_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_RISING_COMPLETE) return;
    rising_complete_t *pg = rising_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_237: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_237, 115, 320);

    /* status: 标签 | "| 发酵 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_39: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_39, 122, 326);
    lv_obj_set_size(pg->bar_39, 637, 20);

    /* label_533: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_533, 273, 157);
    lv_obj_set_size(pg->label_533, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_239: 图片 | (115,161) | img: risingicon.png */
    lv_obj_set_pos(pg->image_239, 115, 161);

    /* image_9: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_9, 902, 160);

}


/* ==============================================================================
 * rising_cooking 英文布局基准（对应 PAGE_RISING_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void rising_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_RISING_COOKING) return;
    rising_cooking_t *pg = rising_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 发酵 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_226: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_226, 115, 320);

    /* bar_36: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_36, 122, 326);
    lv_obj_set_size(pg->bar_36, 637, 20);

    /* label_514: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_514, 273, 157);
    lv_obj_set_size(pg->label_514, 235, 60);

    /* liitle: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->liitle, 609, 170);
    lv_obj_set_size(pg->liitle, 50, 43);

    /* image_227: 图片 | (115,161) | img: risingicon.png */
    lv_obj_set_pos(pg->image_227, 115, 161);

}


/* ==============================================================================
 * rising_menu 英文布局基准（对应 PAGE_RISING_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void rising_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_RISING_MENU) return;
    rising_menu_t *pg = rising_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_219: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_219, 24, 118);

    /* image_220: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_220, 645, 118);

    /* label_485: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_485, 300, 137);
    lv_obj_set_size(pg->label_485, 64, 32);

    /* temp: 标签 | "35" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_487: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_487, 372, 281);
    lv_obj_set_size(pg->label_487, 38, 32);

    /* label_488: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_488, 891, 137);
    lv_obj_set_size(pg->label_488, 125, 32);

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 793, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_490: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_490, 872, 280);
    lv_obj_set_size(pg->label_490, 38, 32);

    /* label_491: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_491, 1075, 281);
    lv_obj_set_size(pg->label_491, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_493: 标签 | "发酵" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_493, 24, 25);
    lv_obj_set_size(pg->label_493, 144, 32);

    /* label_494: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_494, 996, 39);
    lv_obj_set_size(pg->label_494, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * rising_set 英文布局基准（对应 PAGE_RISING_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void rising_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_RISING_SET) return;
    rising_set_t *pg = rising_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_495: 标签 | "发酵" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_495, 24, 25);
    lv_obj_set_size(pg->label_495, 144, 32);

    /* label_496: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_496, 165, 157);
    lv_obj_set_size(pg->label_496, 26, 32);

    /* label_497: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_497, 252, 157);
    lv_obj_set_size(pg->label_497, 64, 32);

    /* label_498: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_498, 141, 270);
    lv_obj_set_size(pg->label_498, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_501: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_501, 368, 270);
    lv_obj_set_size(pg->label_501, 30, 32);

    /* label_502: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_502, 64, 32);

    /* image_225: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_225, 164, 376);

    /* label_503: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_503, 232, 385);
    lv_obj_set_size(pg->label_503, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_504: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_504, 830, 154);
    lv_obj_set_size(pg->label_504, 130, 32);

    /* label_505: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_505, 840, 270);
    lv_obj_set_size(pg->label_505, 26, 32);

    /* label_506: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_506, 930, 270);
    lv_obj_set_size(pg->label_506, 26, 32);

    /* label_507: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_507, 831, 381);
    lv_obj_set_size(pg->label_507, 130, 32);

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_509: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_509, 1010, 39);
    lv_obj_set_size(pg->label_509, 85, 36);

    /* onpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat, 983, 144);
    lv_obj_set_size(pg->onpreheat, 135, 53);

    /* ondelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 260);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 372);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * rising_setting 英文布局基准（对应 PAGE_RISING_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void rising_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_RISING_SETTING) return;
    rising_setting_t *pg = rising_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_23: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_23, 0, 0);
    lv_obj_set_size(pg->container_23, 1280, 480);

    /* image_228: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_228, 115, 115);

    /* label_516: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_516, 242, 136);
    lv_obj_set_size(pg->label_516, 73, 30);

    /* label_517: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_517, 454, 136);
    lv_obj_set_size(pg->label_517, 135, 30);

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 213);
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 300, 229);
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 374, 213);
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_522: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_522, 449, 229);
    lv_obj_set_size(pg->label_522, 37, 30);

    /* label_523: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_523, 612, 229);
    lv_obj_set_size(pg->label_523, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 288, 229);
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * rising_stop 英文布局基准（对应 PAGE_RISING_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void rising_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_RISING_STOP) return;
    rising_stop_t *pg = rising_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 发酵 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_233: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_233, 115, 320);

    /* bar_37: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_37, 122, 326);
    lv_obj_set_size(pg->bar_37, 637, 20);

    /* label_527: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_527, 273, 157);
    lv_obj_set_size(pg->label_527, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_234: 图片 | (115,161) | img: risingicon.png */
    lv_obj_set_pos(pg->image_234, 115, 161);

}


/* ==============================================================================
 * rising_stop_back 英文布局基准（对应 PAGE_RISING_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void rising_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_RISING_STOP_BACK) return;
    rising_stop_back_t *pg = rising_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_235: 图片 | (115,161) | img: risingicon.png */
    lv_obj_set_pos(pg->image_235, 115, 161);

    /* status: 标签 | "| 发酵 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_236: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_236, 115, 320);

    /* bar_38: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_38, 122, 326);
    lv_obj_set_size(pg->bar_38, 637, 20);

    /* label_529: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_529, 273, 157);
    lv_obj_set_size(pg->label_529, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_24: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_24, 0, 0);
    lv_obj_set_size(pg->container_24, 1280, 480);

    /* label_530: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_530, 895, 160);
    lv_obj_set_size(pg->label_530, 252, 36);

    /* label_531: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_531, 895, 198);
    lv_obj_set_size(pg->label_531, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * risingpage 英文布局基准（对应 PAGE_RISINGPAGE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void risingpage_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_RISINGPAGE) return;
    risingpage_t *pg = risingpage_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_6: 图片 | (24,119) | img: div1bg.png */
    lv_obj_set_pos(pg->image_6, 24, 119);

    /* label_16: 标签 | "是否有发酵阶段" | (406,259) | 469x51 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->label_16, 406, 259);
    lv_obj_set_size(pg->label_16, 469, 51);

    /* yes: 按钮 | (885,247) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->yes, 885, 247);
    lv_obj_set_size(pg->yes, 129, 83);

    /* no: 按钮 | (268,247) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->no, 268, 247);
    lv_obj_set_size(pg->no, 129, 83);

    /* label_17: 标签 | "面包卷" | (24,24) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_17, 24, 24);
    lv_obj_set_size(pg->label_17, 144, 32);

     /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ En modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     lv_obj_t *obj = lv_screen_active();
     //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_check_rise_En.png), 0);
 
     obj = pg->label_16;
     lv_obj_set_style_text_font(obj, &c_aktivgroteskmedium_36, 0);
     // lv_obj_set_style_text_letter_space(obj, 0 ,0);
     lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);
     lv_obj_set_style_text_line_space(obj, 8 ,0);
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
     lv_obj_align(obj, LV_ALIGN_CENTER, -6, 43);
 
     obj = pg->no;
     lv_obj_set_style_bg_image_recolor(obj, lv_color_hex(0x000000), 0);
     lv_obj_set_style_bg_image_recolor_opa(obj, LV_OPA_COVER, 0);
     lv_obj_set_pos(obj, 165, 248);
 
     obj = pg->yes;
     lv_obj_set_style_bg_image_recolor(obj, lv_color_hex(0x000000), 0);
     lv_obj_set_style_bg_image_recolor_opa(obj, LV_OPA_COVER, 0);
     lv_obj_set_pos(obj, 987, 246);
 
     obj = pg->label_17;
     lv_obj_set_pos(obj, 24, 24);
}


/* ==============================================================================
 * save_bbq_complete 英文布局基准（对应 PAGE_SAVE_BBQ_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void save_bbq_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SAVE_BBQ_COMPLETE) return;
    save_bbq_complete_t *pg = save_bbq_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_127: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_127, 115, 320);

    /* status: 标签 | "| 节能热风 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_25: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_25, 122, 326);
    lv_obj_set_size(pg->bar_25, 637, 20);

    /* label_280: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_280, 273, 157);
    lv_obj_set_size(pg->label_280, 330, 60);   /* 对齐 updown bbq 系 */

    /* button_89: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->button_89, 609, 170);
    lv_obj_set_size(pg->button_89, 50, 43);

    /* image_129: 图片 | (115,161) | img: savewindicon.png */
    lv_obj_set_pos(pg->image_129, 115, 161);

    /* image_4: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_4, 902, 160);

}


/* ==============================================================================
 * save_bbq_cooking 英文布局基准（对应 PAGE_SAVE_BBQ_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void save_bbq_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SAVE_BBQ_COOKING) return;
    save_bbq_cooking_t *pg = save_bbq_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 节能热风 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_121: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_121, 115, 320);

    /* bar_22: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_22, 122, 326);
    lv_obj_set_size(pg->bar_22, 637, 20);

    /* label_271: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_271, 273, 157);
    lv_obj_set_size(pg->label_271, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_122: 图片 | (115,161) | img: savewindicon.png */
    lv_obj_set_pos(pg->image_122, 115, 161);

}


/* ==============================================================================
 * save_bbq_menu 英文布局基准（对应 PAGE_SAVE_BBQ_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void save_bbq_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SAVE_BBQ_MENU) return;
    save_bbq_menu_t *pg = save_bbq_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_114: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_114, 24, 118);

    /* image_115: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_115, 645, 118);

    /* label_243: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_243, 275, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_243, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* temp: 标签 | "180" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 192, 249);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->temp, 180, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_245: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_245, 372, 281);
    lv_obj_set_size(pg->label_245, 38, 32);

    /* label_246: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_246, 855, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_246, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 788, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->hour, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_248: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_248, 872, 280);
    lv_obj_set_size(pg->label_248, 38, 32);

    /* label_249: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_249, 1075, 281);
    lv_obj_set_size(pg->label_249, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_pos(pg->min, 990, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->min, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_251: 标签 | "节能热风" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_251, 24, 24);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_251, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_252: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_252, 1007, 41);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_252, 117, 32);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */

    /* line3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->line3, 252, 328);

    /* line2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->line2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * save_bbq_set 英文布局基准（对应 PAGE_SAVE_BBQ_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void save_bbq_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SAVE_BBQ_SET) return;
    save_bbq_set_t *pg = save_bbq_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_253: 标签 | "节能热风" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_253, 24, 25);
    lv_obj_set_size(pg->label_253, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_254: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_254, 163, 155);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_254, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_255: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_255, 252, 157);
    lv_obj_set_size(pg->label_255, 64, 32);

    /* label_256: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_256, 150, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_256, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->min, 317, 258);
    lv_obj_set_size(pg->min, 68, 66);   /* 对齐 updown 系英文排版(A) */

    /* label_259: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_259, 368, 270);
    lv_obj_set_size(pg->label_259, 30, 32);

    /* label_260: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->label_260, 371, 271);
    lv_obj_set_size(pg->label_260, 42, 32);   /* 对齐 updown 系英文排版(A) */

    /* image_120: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_120, 164, 376);

    /* label_261: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_261, 232, 385);
    lv_obj_set_size(pg->label_261, 220, 32);   /* 对齐 updown 系英文排版(A) */

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_262: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_262, 785, 154);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_262, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_263: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_263, 885, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_263, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_264: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_264, 930, 270);
    lv_obj_set_size(pg->label_264, 26, 32);

    /* label_265: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_265, 765, 381);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_265, 200, 32);   /* 对齐 updown 系英文排版(A) */

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 269, 143);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->temp, 100, 53);   /* 对齐 top_bbq 系排版 */

    /* label_267: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_267, 1007, 41);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_267, 85, 36);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */

    /* onpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat, 983, 144);
    lv_obj_set_size(pg->onpreheat, 135, 53);

    /* ondelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 260);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 372);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 355, 157);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 327, 157);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * save_bbq_setting 英文布局基准（对应 PAGE_SAVE_BBQ_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void save_bbq_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SAVE_BBQ_SETTING) return;
    save_bbq_setting_t *pg = save_bbq_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 257, 60);   /* 对齐 top_bbq 系排版 */

    /* container_2: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_2, 0, 0);
    lv_obj_set_size(pg->container_2, 1280, 480);

    /* image_6: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_6, 115, 115);

    /* label_12: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_12, 203, 134);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_12, 150, 30);   /* 对齐 top_bbq 系排版 */

    /* label_13: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_13, 444, 134);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_13, 150, 30);   /* 对齐 top_bbq 系排版 */

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 217);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 303, 229);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 377, 217);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_18: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_18, 450, 231);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_18, 37, 30);

    /* label_19: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_19, 612, 231);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_19, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 289, 229);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * save_bbq_stop 英文布局基准（对应 PAGE_SAVE_BBQ_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void save_bbq_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SAVE_BBQ_STOP) return;
    save_bbq_stop_t *pg = save_bbq_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 节能热风 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_123: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_123, 115, 320);

    /* bar_23: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_23, 122, 326);
    lv_obj_set_size(pg->bar_23, 637, 20);

    /* label_274: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_274, 273, 157);
    lv_obj_set_size(pg->label_274, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_124: 图片 | (115,161) | img: savewindicon.png */
    lv_obj_set_pos(pg->image_124, 115, 161);

}


/* ==============================================================================
 * save_bbq_stop_back 英文布局基准（对应 PAGE_SAVE_BBQ_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void save_bbq_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SAVE_BBQ_STOP_BACK) return;
    save_bbq_stop_back_t *pg = save_bbq_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_125: 图片 | (115,161) | img: savewindicon.png */
    lv_obj_set_pos(pg->image_125, 115, 161);

    /* status: 标签 | "| 节能热风 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_126: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_126, 115, 320);

    /* bar_24: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_24, 122, 326);
    lv_obj_set_size(pg->bar_24, 637, 20);

    /* label_276: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_276, 273, 157);
    lv_obj_set_size(pg->label_276, 330, 60);   /* 对齐 updown bbq 系 */   /* 对齐 updown 系英文排版(A) */

    /* button_87: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->button_87, 609, 170);
    lv_obj_set_size(pg->button_87, 50, 43);

    /* container_11: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_11, 0, 0);
    lv_obj_set_size(pg->container_11, 1280, 480);

    /* label_277: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_277, 895, 160);
    lv_obj_set_size(pg->label_277, 252, 36);

    /* label_278: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_278, 895, 198);
    lv_obj_set_size(pg->label_278, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * screen_SET 英文布局基准（对应 PAGE_SCREEN_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void screen_SET_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SCREEN_SET) return;
    screen_SET_t *pg = screen_SET_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* Title_Lb: 标签 | (24,24) | 110x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->Title_Lb, 24, 24);
    lv_obj_set_size(pg->Title_Lb, 110, 32);

    /* Time_Lb: 标签 | "12:00" | (1197,24) | 70x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->Time_Lb, 1197, 24);
    lv_obj_set_size(pg->Time_Lb, 70, 32);

    /* SET_bg_Img: 图片 | (24,80) | img: set_bg_IMG.png */
    lv_obj_set_pos(pg->SET_bg_Img, 24, 80);

    /* DSQ_Btn: 按钮 | (34,95) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->DSQ_Btn, 34, 95);
    lv_obj_set_size(pg->DSQ_Btn, 282, 67);

    /* TS_Btn: 按钮 | (34,189) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->TS_Btn, 34, 189);
    lv_obj_set_size(pg->TS_Btn, 282, 67);

    /* ZDBW_Btn: 按钮 | (34,283) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->ZDBW_Btn, 34, 283);
    lv_obj_set_size(pg->ZDBW_Btn, 282, 67);

    /* TFLQYX_Btn: 按钮 | (34,377) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->TFLQYX_Btn, 34, 377);
    lv_obj_set_size(pg->TFLQYX_Btn, 282, 67);

    /* Six_Btn: 按钮 | (345,95) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->Six_Btn, 345, 95);
    lv_obj_set_size(pg->Six_Btn, 282, 67);

    /* LDG_Btn: 按钮 | (345,189) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->LDG_Btn, 345, 189);
    lv_obj_set_size(pg->LDG_Btn, 282, 67);

    /* WDDW_Btn: 按钮 | (345,283) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->WDDW_Btn, 345, 283);
    lv_obj_set_size(pg->WDDW_Btn, 282, 67);

    /* SYSZ_Btn: 按钮 | (345,377) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->SYSZ_Btn, 345, 377);
    lv_obj_set_size(pg->SYSZ_Btn, 282, 67);

    /* LDXS_Btn: 按钮 | (656,95) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->LDXS_Btn, 656, 95);
    lv_obj_set_size(pg->LDXS_Btn, 282, 67);

    /* Time_Btn: 按钮 | (656,189) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->Time_Btn, 656, 189);
    lv_obj_set_size(pg->Time_Btn, 282, 67);

    /* DJ_Time_Btn: 按钮 | (656,283) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->DJ_Time_Btn, 656, 283);
    lv_obj_set_size(pg->DJ_Time_Btn, 282, 67);

    /* YY_Btn: 按钮 | (656,377) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->YY_Btn, 656, 377);
    lv_obj_set_size(pg->YY_Btn, 282, 67);

    /* Power_Btn: 按钮 | (966,95) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->Power_Btn, 966, 95);
    lv_obj_set_size(pg->Power_Btn, 282, 67);

    /* Demo_Btn: 按钮 | (966,189) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->Demo_Btn, 966, 189);
    lv_obj_set_size(pg->Demo_Btn, 282, 67);

    /* RESET_Btn: 按钮 | (966,283) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->RESET_Btn, 966, 283);
    lv_obj_set_size(pg->RESET_Btn, 282, 67);

    /* GYJQ_Btn: 按钮 | (966,377) | 282x67 | font montserratmedium_16 | bg: set_select_icon.png */
    lv_obj_set_pos(pg->GYJQ_Btn, 966, 377);
    lv_obj_set_size(pg->GYJQ_Btn, 282, 67);

    /* TXT_Img: 图片 | (48,112) | img: set_bg_txt.png */
    lv_obj_set_pos(pg->TXT_Img, 48, 112);

    /* TS_Lb: 标签 | (215,210) | 65x30 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->TS_Lb, 215, 210);
    lv_obj_set_size(pg->TS_Lb, 65, 30);

    /* ZDBW_Lb: 标签 | (215,304) | 65x30 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->ZDBW_Lb, 215, 304);
    lv_obj_set_size(pg->ZDBW_Lb, 65, 30);

    /* Six_Lb: 标签 | (508,116) | 80x30 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->Six_Lb, 508, 116);
    lv_obj_set_size(pg->Six_Lb, 80, 30);

    /* WDDW_Lb: 标签 | "℃" | (523,304) | 65x30 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->WDDW_Lb, 523, 304);
    lv_obj_set_size(pg->WDDW_Lb, 65, 30);

    /* YY_Lb: 标签 | (771,398) | 125x30 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->YY_Lb, 771, 398);
    lv_obj_set_size(pg->YY_Lb, 125, 30);

    /* Power_Lb: 标签 | "16A" | (1154,116) | 55x30 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->Power_Lb, 1154, 116);
    lv_obj_set_size(pg->Power_Lb, 55, 30);

    /* Demo_Lb: 标签 | (1154,210) | 55x30 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->Demo_Lb, 1154, 210);
    lv_obj_set_size(pg->Demo_Lb, 55, 30);

    /* CountDown_icon_Img: 图片 | (626,25) | img: CountDown_icon.png */
    lv_obj_set_pos(pg->CountDown_icon_Img, 626, 25);

}


/* ==============================================================================
 * sixmenu 英文布局基准（对应 PAGE_SIXMENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void sixmenu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SIXMENU) return;
    sixmenu_t *pg = sixmenu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0x000000 | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* bread: 按钮 | (21,75) | 244x386 | font montserratmedium_16 | bg: updown80bk.png */
    lv_obj_set_pos(pg->bread, 21, 75);
    lv_obj_set_size(pg->bread, 244, 386);

    /* cake: 按钮 | (267,76) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->cake, 267, 76);
    lv_obj_set_size(pg->cake, 248, 193);

    /* chick: 按钮 | (516,76) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->chick, 516, 76);
    lv_obj_set_size(pg->chick, 248, 193);

    /* meat: 按钮 | (764,76) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->meat, 764, 76);
    lv_obj_set_size(pg->meat, 248, 193);

    /* fish: 按钮 | (1012,76) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->fish, 1012, 76);
    lv_obj_set_size(pg->fish, 248, 193);

    /* vegetable: 按钮 | (266,268) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->vegetable, 266, 268);
    lv_obj_set_size(pg->vegetable, 248, 193);

    /* pizza6: 按钮 | (516,268) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->pizza6, 516, 268);
    lv_obj_set_size(pg->pizza6, 248, 193);

    /* pasta: 按钮 | (764,268) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->pasta, 764, 268);
    lv_obj_set_size(pg->pasta, 248, 193);

    /* snack: 按钮 | (1012,268) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->snack, 1012, 268);
    lv_obj_set_size(pg->snack, 248, 193);

    /* image_1: 图片 | (81,115) | img: six.png */
    lv_obj_set_pos(pg->image_1, 81, 115);

    /* label_9: 标签 | "第六感" | (24,24) | 97x25 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_9, 24, 24);
    lv_obj_set_size(pg->label_9, 97, 25);

     /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     lv_obj_t *obj = NULL;
     obj = pg->label_9;
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}


/* ==============================================================================
 * sixmenutz 英文布局基准（对应 PAGE_SIXMENUTZ ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void sixmenutz_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SIXMENUTZ) return;
    sixmenutz_t *pg = sixmenutz_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_7: 标签 | "第六感" | (24,24) | 97x25 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_7, 24, 24);
    lv_obj_set_size(pg->label_7, 97, 25);

    /* chick: 按钮 | (12,75) | 638x387 | font montserratmedium_16 | bg: div230bg.png */
    lv_obj_set_pos(pg->chick, 12, 75);
    lv_obj_set_size(pg->chick, 638, 387);

    /* meat: 按钮 | (633,74) | 637x389 | font montserratmedium_16 | bg: div230bg.png */
    lv_obj_set_pos(pg->meat, 633, 74);
    lv_obj_set_size(pg->meat, 637, 389);

    /* image_3: 图片 | (267,192) | img: sixtz.png */
    lv_obj_set_pos(pg->image_3, 267, 192);

    /* label_8: 标签 | "家禽" | (265,288) | 131x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_8, 265, 288);
    lv_obj_set_size(pg->label_8, 131, 32);

    /* label_9: 标签 | "肉" | (886,288) | 131x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_9, 886, 288);
    lv_obj_set_size(pg->label_9, 131, 32);

}


/* ==============================================================================
 * sixop3page 英文布局基准（对应 PAGE_SIXOP3PAGE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void sixop3page_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SIXOP3PAGE) return;
    sixop3page_t *pg = sixop3page_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xffffff | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* bt2: 按钮 | (421,60) | 439x417 | font montserratmedium_16 | bg: div3bk30.png */
    lv_obj_set_pos(pg->bt2, 421, 60);
    lv_obj_set_size(pg->bt2, 439, 417);

    /* bt1: 按钮 | (7,60) | 439x417 | font montserratmedium_16 | bg: div3bk30.png */
    lv_obj_set_pos(pg->bt1, 7, 60);
    lv_obj_set_size(pg->bt1, 439, 417);

    /* bt3: 按钮 | (835,60) | 439x417 | font montserratmedium_16 | bg: div3bk30.png */
    lv_obj_set_pos(pg->bt3, 835, 60);
    lv_obj_set_size(pg->bt3, 439, 417);

    /* name: 标签 | "菜单" | (24,24) | 58x22 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->name, 24, 24);
    lv_obj_set_size(pg->name, 80, 22);

    /* ---- 场景分支: 本页被两处复用 ----
       非探针: 第六感(非探针)→肉→牛肉/羊肉/猪肉(op 文字+probe 图标掩码显隐)
       探针版(MEAT_TZ): 探针模式→第六感→肉(牛肉/羊肉/猪肉; op/probe 全隐藏,bt 内文字) ---- */
    if (six_op3_get_kind() == SIX_OP3_KIND_MEAT_TZ) {
        /* 探针版: op1-3/probe1-3 运行时全隐藏,不排版; bt1-3 内文字(牛肉/羊肉/猪肉) */
        /* TODO: 英文实测后调整(bt 文字字号/位置) */
    } else {
        /* 非探针: 第六感→肉菜单 */
        /* op1: 标签 | "烹饪功能" | (153,274) | 147x28 | font taiwanpearl_regular_30 */
        lv_obj_set_pos(pg->op1, 153, 274);
        lv_obj_set_size(pg->op1, 147, 28);

        /* op2: 标签 | "COOK 4" | (567,274) | 147x28 | font taiwanpearl_regular_30 */
        lv_obj_set_pos(pg->op2, 567, 274);
        lv_obj_set_size(pg->op2, 147, 28);

        /* op3: 标签 | "特殊功能" | (981,274) | 147x30 | font taiwanpearl_regular_30 */
        lv_obj_set_pos(pg->op3, 981, 274);
        lv_obj_set_size(pg->op3, 147, 30);

        /* probe1: 图片 | (192,213) | img: probeiconbig.png */
        lv_obj_set_pos(pg->probe1, 192, 213);

        /* probe2: 图片 | (606,213) | img: probeiconbig.png */
        lv_obj_set_pos(pg->probe2, 606, 213);

        /* probe3: 图片 | (1020,213) | img: probeiconbig.png */
        lv_obj_set_pos(pg->probe3, 1020, 213);
    }

}


/* ==============================================================================
 * slowcook_complete 英文布局基准（对应 PAGE_SLOWCOOK_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void slowcook_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SLOWCOOK_COMPLETE) return;
    slowcook_complete_t *pg = slowcook_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_195: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_195, 115, 320);

    /* status: 标签 | "| 慢煮 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_31: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_31, 122, 326);
    lv_obj_set_size(pg->bar_31, 637, 20);

    /* label_435: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_435, 273, 157);
    lv_obj_set_size(pg->label_435, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_197: 图片 | (115,161) | img: slowcookicon.png */
    lv_obj_set_pos(pg->image_197, 115, 161);

    /* image_7: 图片 | (902,160) | img: tips.png | 场景条件定位(业务分支设置, tune不设) */
    /* 位置由业务场景分支控制, 微调按文件头模板 */

}


/* ==============================================================================
 * slowcook_complete_probe 英文布局基准（对应 PAGE_SLOWCOOK_COMPLETE_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void slowcook_complete_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SLOWCOOK_COMPLETE_PROBE) return;
    slowcook_complete_probe_t *pg = slowcook_complete_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_47: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_47, 115, 320);

    /* status: 标签 | "| 慢煮 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_12: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_12, 122, 326);
    lv_obj_set_size(pg->bar_12, 637, 20);

    /* image_48: 图片 | (115,161) | img: slowcookicon.png */
    lv_obj_set_pos(pg->image_48, 115, 161);

    /* label_109: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_109, 273, 157);
    lv_obj_set_size(pg->label_109, 235, 60);

    /* label_110: 标签 | "高温防烫" | (896,161) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_110, 896, 161);
    lv_obj_set_size(pg->label_110, 252, 36);

    /* label_111: 标签 | "请缓慢打开门体！" | (896,197) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_111, 896, 197);
    lv_obj_set_size(pg->label_111, 252, 36);

}


/* ==============================================================================
 * slowcook_cooking 英文布局基准（对应 PAGE_SLOWCOOK_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void slowcook_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SLOWCOOK_COOKING) return;
    slowcook_cooking_t *pg = slowcook_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 慢煮 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_184: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_184, 115, 320);

    /* bar_28: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_28, 122, 326);
    lv_obj_set_size(pg->bar_28, 637, 20);

    /* label_416: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_416, 273, 157);
    lv_obj_set_size(pg->label_416, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_185: 图片 | (115,161) | img: slowcookicon.png */
    lv_obj_set_pos(pg->image_185, 115, 161);

}


/* ==============================================================================
 * slowcook_cooking_probe 英文布局基准（对应 PAGE_SLOWCOOK_COOKING_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void slowcook_cooking_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SLOWCOOK_COOKING_PROBE) return;
    slowcook_cooking_probe_t *pg = slowcook_cooking_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 慢煮 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* temp: 标签 | "36℃" | (919,156) | 134x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->temp, 919, 156);
    lv_obj_set_size(pg->temp, 134, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_39: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_39, 115, 320);

    /* bar_9: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_9, 122, 326);
    lv_obj_set_size(pg->bar_9, 637, 20);

    /* image_40: 图片 | (115,161) | img: slowcookicon.png */
    lv_obj_set_pos(pg->image_40, 115, 161);

    /* label_100: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_100, 273, 157);
    lv_obj_set_size(pg->label_100, 235, 60);

    /* image_41: 图片 | (1068,180) | img: probeicon.png */
    lv_obj_set_pos(pg->image_41, 1068, 180);

}


/* ==============================================================================
 * slowcook_menu 英文布局基准（对应 PAGE_SLOWCOOK_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void slowcook_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SLOWCOOK_MENU) return;
    slowcook_menu_t *pg = slowcook_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_177: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_177, 24, 118);

    /* image_178: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_178, 645, 118);

    /* label_387: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_387, 300, 137);
    lv_obj_set_size(pg->label_387, 64, 32);

    /* temp: 标签 | "80" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_389: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_389, 372, 281);
    lv_obj_set_size(pg->label_389, 38, 32);

    /* label_390: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_390, 891, 137);
    lv_obj_set_size(pg->label_390, 125, 32);

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 793, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_392: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_392, 872, 280);
    lv_obj_set_size(pg->label_392, 38, 32);

    /* label_393: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_393, 1075, 281);
    lv_obj_set_size(pg->label_393, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_395: 标签 | "慢煮" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_395, 24, 25);
    lv_obj_set_size(pg->label_395, 144, 32);

    /* label_396: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_396, 996, 39);
    lv_obj_set_size(pg->label_396, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * slowcook_menu_probe 英文布局基准（对应 PAGE_SLOWCOOK_MENU_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void slowcook_menu_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SLOWCOOK_MENU_PROBE) return;
    slowcook_menu_probe_t *pg = slowcook_menu_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_33: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_33, 24, 118);

    /* image_34: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_34, 645, 118);

    /* label_79: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_79, 300, 137);
    lv_obj_set_size(pg->label_79, 64, 32);

    /* temp: 标签 | "180" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_81: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_81, 372, 281);
    lv_obj_set_size(pg->label_81, 38, 32);

    /* label_82: 标签 | "探针温度" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_82, 891, 137);
    lv_obj_set_size(pg->label_82, 125, 32);

    /* label_83: 标签 | "慢煮" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_83, 24, 25);
    lv_obj_set_size(pg->label_83, 144, 32);

    /* label_84: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_84, 996, 39);
    lv_obj_set_size(pg->label_84, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* probetemp: 标签 | "80" | (853,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->probetemp, 853, 249);
    lv_obj_set_size(pg->probetemp, 120, 72);

    /* label_86: 标签 | "℃" | (973,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_86, 973, 281);
    lv_obj_set_size(pg->label_86, 38, 32);

    /* probetempline: 图片 | (891,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->probetempline, 891, 328);

}


/* ==============================================================================
 * slowcook_set 英文布局基准（对应 PAGE_SLOWCOOK_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void slowcook_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SLOWCOOK_SET) return;
    slowcook_set_t *pg = slowcook_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_397: 标签 | "慢煮" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_397, 24, 25);
    lv_obj_set_size(pg->label_397, 144, 32);

    /* label_398: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_398, 165, 157);
    lv_obj_set_size(pg->label_398, 26, 32);

    /* label_399: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_399, 252, 157);
    lv_obj_set_size(pg->label_399, 64, 32);

    /* label_400: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_400, 141, 270);
    lv_obj_set_size(pg->label_400, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_403: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_403, 368, 270);
    lv_obj_set_size(pg->label_403, 30, 32);

    /* label_404: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_404, 64, 32);

    /* image_183: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_183, 164, 376);

    /* label_405: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_405, 232, 385);
    lv_obj_set_size(pg->label_405, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_406: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_406, 830, 154);
    lv_obj_set_size(pg->label_406, 130, 32);

    /* label_407: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_407, 840, 270);
    lv_obj_set_size(pg->label_407, 26, 32);

    /* label_408: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_408, 930, 270);
    lv_obj_set_size(pg->label_408, 26, 32);

    /* label_409: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_409, 831, 381);
    lv_obj_set_size(pg->label_409, 130, 32);

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_411: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_411, 1010, 39);
    lv_obj_set_size(pg->label_411, 85, 36);

    /* onpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat, 983, 144);
    lv_obj_set_size(pg->onpreheat, 135, 53);

    /* ondelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 260);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 372);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * slowcook_set_probe 英文布局基准（对应 PAGE_SLOWCOOK_SET_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void slowcook_set_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SLOWCOOK_SET_PROBE) return;
    slowcook_set_probe_t *pg = slowcook_set_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_87: 标签 | "慢煮" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_87, 24, 25);
    lv_obj_set_size(pg->label_87, 144, 32);

    /* label_88: 标签 | "温" | (165,155) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_88, 165, 155);
    lv_obj_set_size(pg->label_88, 26, 32);

    /* label_89: 标签 | "度：" | (252,155) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_89, 252, 155);
    lv_obj_set_size(pg->label_89, 64, 32);

    /* label_90: 标签 | "探针温度：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_90, 141, 270);
    lv_obj_set_size(pg->label_90, 175, 32);

    /* image_38: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_38, 164, 376);

    /* label_91: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_91, 232, 385);
    lv_obj_set_size(pg->label_91, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_92: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_92, 1010, 39);
    lv_obj_set_size(pg->label_92, 85, 36);

    /* probetemp: 标签 | "80" | (315,254) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->probetemp, 315, 254);
    lv_obj_set_size(pg->probetemp, 83, 53);

    /* label_94: 标签 | "℃" | (369,270) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_94, 369, 270);
    lv_obj_set_size(pg->label_94, 32, 30);

    /* temp: 标签 | "200" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* image_5: 图片 | (839,154) | img: delaytext.png */
    lv_obj_set_pos(pg->image_5, 839, 154);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

}


/* ==============================================================================
 * slowcook_setting 英文布局基准（对应 PAGE_SLOWCOOK_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void slowcook_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SLOWCOOK_SETTING) return;
    slowcook_setting_t *pg = slowcook_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_19: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_19, 0, 0);
    lv_obj_set_size(pg->container_19, 1280, 480);

    /* image_186: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_186, 115, 115);

    /* label_418: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_418, 242, 136);
    lv_obj_set_size(pg->label_418, 73, 30);

    /* label_419: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_419, 454, 136);
    lv_obj_set_size(pg->label_419, 135, 30);

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 213);
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 300, 229);
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 374, 213);
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_424: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_424, 449, 229);
    lv_obj_set_size(pg->label_424, 37, 30);

    /* label_425: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_425, 612, 229);
    lv_obj_set_size(pg->label_425, 37, 30);

    /* houeline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->houeline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 288, 229);
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * slowcook_stop 英文布局基准（对应 PAGE_SLOWCOOK_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void slowcook_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SLOWCOOK_STOP) return;
    slowcook_stop_t *pg = slowcook_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 慢煮 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_191: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_191, 115, 320);

    /* bar_29: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_29, 122, 326);
    lv_obj_set_size(pg->bar_29, 637, 20);

    /* label_429: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_429, 273, 157);
    lv_obj_set_size(pg->label_429, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_192: 图片 | (115,161) | img: slowcookicon.png */
    lv_obj_set_pos(pg->image_192, 115, 161);

}


/* ==============================================================================
 * slowcook_stop_back 英文布局基准（对应 PAGE_SLOWCOOK_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void slowcook_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SLOWCOOK_STOP_BACK) return;
    slowcook_stop_back_t *pg = slowcook_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_193: 图片 | (115,161) | img: slowcookicon.png */
    lv_obj_set_pos(pg->image_193, 115, 161);

    /* status: 标签 | "| 慢煮 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_194: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_194, 115, 320);

    /* bar_30: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_30, 122, 326);
    lv_obj_set_size(pg->bar_30, 637, 20);

    /* label_431: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_431, 273, 157);
    lv_obj_set_size(pg->label_431, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_20: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_20, 0, 0);
    lv_obj_set_size(pg->container_20, 1280, 480);

    /* label_432: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_432, 895, 160);
    lv_obj_set_size(pg->label_432, 252, 36);

    /* label_433: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_433, 895, 198);
    lv_obj_set_size(pg->label_433, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * slowcook_stop_back_probe 英文布局基准（对应 PAGE_SLOWCOOK_STOP_BACK_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void slowcook_stop_back_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SLOWCOOK_STOP_BACK_PROBE) return;
    slowcook_stop_back_probe_t *pg = slowcook_stop_back_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 慢煮 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_45: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_45, 115, 320);

    /* bar_11: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_11, 122, 326);
    lv_obj_set_size(pg->bar_11, 637, 20);

    /* image_46: 图片 | (115,161) | img: slowcookicon.png */
    lv_obj_set_pos(pg->image_46, 115, 161);

    /* label_105: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_105, 235, 60);

    /* container_3: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_3, 0, 0);
    lv_obj_set_size(pg->container_3, 1280, 480);

    /* label_106: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_106, 895, 160);
    lv_obj_set_size(pg->label_106, 252, 36);

    /* label_107: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_107, 895, 198);
    lv_obj_set_size(pg->label_107, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * slowcook_stop_probe 英文布局基准（对应 PAGE_SLOWCOOK_STOP_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void slowcook_stop_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SLOWCOOK_STOP_PROBE) return;
    slowcook_stop_probe_t *pg = slowcook_stop_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_101: 标签 | "| 慢煮 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_101, 274, 232);
    lv_obj_set_size(pg->label_101, 490, 39);

    /* temp: 标签 | "36℃" | (919,156) | 134x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->temp, 919, 156);
    lv_obj_set_size(pg->temp, 134, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_42: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_42, 115, 320);

    /* bar_10: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_10, 122, 326);
    lv_obj_set_size(pg->bar_10, 637, 20);

    /* image_43: 图片 | (115,161) | img: slowcookicon.png */
    lv_obj_set_pos(pg->image_43, 115, 161);

    /* label_103: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_103, 273, 157);
    lv_obj_set_size(pg->label_103, 235, 60);

    /* image_44: 图片 | (1068,180) | img: probeicon.png */
    lv_obj_set_pos(pg->image_44, 1068, 180);

}


/* ==============================================================================
 * somecook 英文布局基准（对应 PAGE_SOMECOOK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void somecook_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SOMECOOK) return;
    somecook_t *pg = somecook_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xffffff | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* step3button: 按钮 | (845,110) | 418x355 | font montserratmedium_16 | bg: stepbg30.png */
    lv_obj_set_pos(pg->step3button, 845, 110);
    lv_obj_set_size(pg->step3button, 418, 355);

    /* step2button: 按钮 | (431,110) | 418x355 | font montserratmedium_16 | bg: stepbg30.png */
    lv_obj_set_pos(pg->step2button, 431, 110);
    lv_obj_set_size(pg->step2button, 418, 355);

    /* step1button: 按钮 | (17,110) | 418x355 | font montserratmedium_16 | bg: stepbg30.png */
    lv_obj_set_pos(pg->step1button, 17, 110);
    lv_obj_set_size(pg->step1button, 418, 355);

    /* image_1: 图片 | (184,138) | img: steptext.png */
    lv_obj_set_pos(pg->image_1, 184, 138);

    /* label_1: 标签 | "多段烹饪" | (24,24) | 103x22 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_1, 24, 24);
    lv_obj_set_size(pg->label_1, 250, 22);
    lv_label_set_text(pg->label_1, "Multi-step Cooking");

    /* plus1: 图片 | (201,262) | img: plus.png */
    lv_obj_set_pos(pg->plus1, 201, 262);

    /* plus2: 图片 | (615,262) | img: plus.png */
    lv_obj_set_pos(pg->plus2, 615, 262);

    /* plus3: 图片 | (1029,262) | img: plus.png */
    lv_obj_set_pos(pg->plus3, 1029, 262);

    /* step1: 容器 | (23,113) | 406x351 */
    lv_obj_set_pos(pg->step1, 23, 113);
    lv_obj_set_size(pg->step1, 406, 351);

    /* modestatus: 标签 | "模式: 上下烧烤" | (26,77) | 264x27 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->modestatus, 26, 77);
    lv_obj_set_size(pg->modestatus, 264, 27);

    /* tempstatus: 标签 | "温度: ↑180°C/↓120℃" | (26,122) | 270x27 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->tempstatus, 26, 122);
    lv_obj_set_size(pg->tempstatus, 270, 27);

    /* timestatus: 标签 | "时间: 1小时20分钟" | (26,168) | 270x27 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->timestatus, 26, 168);
    lv_obj_set_size(pg->timestatus, 270, 27);

    /* edit: 按钮 | (276,270) | 115x58 | font taiwanpearl_regular_30 | bg: editedbg.png */
    lv_obj_set_pos(pg->edit, 276, 270);
    lv_obj_set_size(pg->edit, 115, 58);

    /* delete: 按钮 | (18,270) | 115x58 | font taiwanpearl_regular_30 | bg: editedbg.png */
    lv_obj_set_pos(pg->delete, 18, 270);
    lv_obj_set_size(pg->delete, 115, 58);

    /* step2: 容器 | (437,113) | 406x351 */
    lv_obj_set_pos(pg->step2, 437, 113);
    lv_obj_set_size(pg->step2, 406, 351);

    /* modestatus2: 标签 | "模式: 上下烧烤" | (26,77) | 264x27 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->modestatus2, 26, 77);
    lv_obj_set_size(pg->modestatus2, 264, 27);

    /* tempstatus2: 标签 | "温度: ↑180°C/↓120℃" | (26,122) | 270x27 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->tempstatus2, 26, 122);
    lv_obj_set_size(pg->tempstatus2, 270, 27);

    /* timestatus2: 标签 | "时间: 1小时20分钟" | (26,168) | 270x27 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->timestatus2, 26, 168);
    lv_obj_set_size(pg->timestatus2, 270, 27);

    /* edit2: 按钮 | (276,270) | 115x58 | font taiwanpearl_regular_30 | bg: editedbg.png */
    lv_obj_set_pos(pg->edit2, 276, 270);
    lv_obj_set_size(pg->edit2, 115, 58);

    /* delete2: 按钮 | (18,270) | 115x58 | font taiwanpearl_regular_30 | bg: editedbg.png */
    lv_obj_set_pos(pg->delete2, 18, 270);
    lv_obj_set_size(pg->delete2, 115, 58);

    /* step3: 容器 | (851,113) | 406x351 */
    lv_obj_set_pos(pg->step3, 851, 113);
    lv_obj_set_size(pg->step3, 406, 351);

    /* modestatus3: 标签 | "模式: 上下烧烤" | (26,77) | 264x27 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->modestatus3, 26, 77);
    lv_obj_set_size(pg->modestatus3, 264, 27);

    /* tempstatus3: 标签 | "温度: ↑180°C/↓120℃" | (26,122) | 270x27 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->tempstatus3, 26, 122);
    lv_obj_set_size(pg->tempstatus3, 270, 27);

    /* timestatus3: 标签 | "时间: 1小时20分钟" | (26,168) | 270x27 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->timestatus3, 26, 168);
    lv_obj_set_size(pg->timestatus3, 270, 27);

    /* edit3: 按钮 | (276,270) | 115x58 | font taiwanpearl_regular_30 | bg: editedbg.png */
    lv_obj_set_pos(pg->edit3, 276, 270);
    lv_obj_set_size(pg->edit3, 115, 58);

    /* delete3: 按钮 | (18,270) | 115x58 | font taiwanpearl_regular_30 | bg: editedbg.png */
    lv_obj_set_pos(pg->delete3, 18, 270);
    lv_obj_set_size(pg->delete3, 115, 58);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* surelabel: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->surelabel, 1010, 39);
    lv_obj_set_size(pg->surelabel, 85, 36);

}


/* ==============================================================================
 * somecook_cooking 公共布局（复用结构，以下页面共用：PAGE_SIX_COOKING(第六感烹饪页), PAGE_SOMECOOK_COOKING(多段烹饪页)）
 * 公共部分在此统一调整；各页差异见下方独立函数
 * ============================================================================== */
static void somecook_cooking_common(void)
{
    somecook_cooking_t *pg = somecook_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_12: 标签 | "|                         | 180℃ |…" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_12, 274, 232);
    lv_obj_set_size(pg->label_12, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* image_10: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_10, 115, 320);

    /* bar_1: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_1, 122, 326);
    lv_obj_set_size(pg->bar_1, 637, 20);

    /* cookstatus: 标签 | "烹饪中..." | (273,157) | 262x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->cookstatus, 273, 157);
    lv_obj_set_size(pg->cookstatus, 262, 60);

    /* activestatus: 标签 | "步骤一：发酵" | (289,232) | 161x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->activestatus, 289, 232);
    lv_obj_set_size(pg->activestatus, 161, 39);

    /* container_1: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_1, 0, 0);
    lv_obj_set_size(pg->container_1, 1280, 480);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* text1: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->text1, 895, 160);
    lv_obj_set_size(pg->text1, 252, 36);

    /* text2: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->text2, 895, 198);
    lv_obj_set_size(pg->text2, 252, 36);

}

/* ==============================================================================
 * six_cooking_lang_tune（对应 PAGE_SIX_COOKING 第六感烹饪页）
 * 公共布局调 somecook_cooking_common()，本页差异直接改下方数字
 * ============================================================================== */
void six_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SIX_COOKING) return;
    somecook_cooking_t *pg = somecook_cooking_get(&ui_manager);
    if (!pg) return;

    somecook_cooking_common();

    /* icon: 图片 | (115,161) | img: updown_img.png | 动态定位(默认业务值, 直接改数字) */
    lv_obj_set_pos(pg->icon, 163, 161);

}

/* ==============================================================================
 * somecook_cooking_lang_tune（对应 PAGE_SOMECOOK_COOKING 多段烹饪页）
 * 公共布局调 somecook_cooking_common()，本页差异直接改下方数字
 * ============================================================================== */
void somecook_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SOMECOOK_COOKING) return;
    somecook_cooking_t *pg = somecook_cooking_get(&ui_manager);
    if (!pg) return;

    somecook_cooking_common();

    /* icon: 图片 | (115,161) | img: updown_img.png | 状态切换(默认业务值, 直接改数字) */
    if (g_send.cook_mode == MODE_UNFROZEN)
        lv_obj_set_pos(pg->icon, 149, 161);
    else
        lv_obj_set_pos(pg->icon, 115, 161);

}


/* ==============================================================================
 * special_menu 英文布局基准（对应 PAGE_SPECIAL_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void special_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SPECIAL_MENU) return;
    special_menu_t *pg = special_menu_get(&ui_manager);
    if (!pg) return;

	// lv_obj_set_style_bg_img_src(pg->obj, LVGL_IMAGE_PATH(cookmenuenbg.png), LV_PART_MAIN | LV_STATE_DEFAULT);
	// lv_obj_set_style_bg_img_recolor(pg->obj, lv_color_hex(0xff00ff), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_bg_img_recolor_opa(pg->obj, 80, LV_PART_MAIN | LV_STATE_DEFAULT);
    /* 页面背景: 背景图 bg.jpg | 底色 0x000000 | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* rising_button: 按钮 | (266,268) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->rising_button, 266, 268);
    lv_obj_set_size(pg->rising_button, 248, 193);

    /* corn_button: 按钮 | (516,268) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->corn_button, 516, 268);
    lv_obj_set_size(pg->corn_button, 248, 193);

    /* heat_contain_button: 按钮 | (764,268) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->heat_contain_button, 764, 268);
    lv_obj_set_size(pg->heat_contain_button, 248, 193);

    /* some_cook_button: 按钮 | (1012,267) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->some_cook_button, 1012, 267);
    lv_obj_set_size(pg->some_cook_button, 248, 193);

    /* unfrozen_button: 按钮 | (1012,76) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->unfrozen_button, 1012, 76);
    lv_obj_set_size(pg->unfrozen_button, 248, 193);

    /* slow_cook_button: 按钮 | (764,76) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->slow_cook_button, 764, 76);
    lv_obj_set_size(pg->slow_cook_button, 248, 193);

    /* frozen_cook_button: 按钮 | (516,76) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->frozen_cook_button, 516, 76);
    lv_obj_set_size(pg->frozen_cook_button, 248, 193);

    /* piza_button: 按钮 | (267,75) | 248x193 | font montserratmedium_16 | bg: bottombk.png */
    lv_obj_set_pos(pg->piza_button, 267, 75);
    lv_obj_set_size(pg->piza_button, 248, 193);

    /* air_button: 按钮 | (21,75) | 244x386 | font montserratmedium_16 | bg: updown80bk.png */
    lv_obj_set_pos(pg->air_button, 21, 75);
    lv_obj_set_size(pg->air_button, 244, 386);

    /* air_label: 标签 | "空气炸" | (69,287) | 150x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->air_label, 69, 287);
    lv_obj_set_size(pg->air_label, 150, 32);

    /* fajiao_label: 标签 | "发酵" | (327,405) | 131x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->fajiao_label, 327, 405);
    lv_obj_set_size(pg->fajiao_label, 131, 32);

    /* frozen_cook_label: 标签 | "冷冻烘焙" | (581,212) | 126x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->frozen_cook_label, 555, 212);
    lv_obj_set_size(pg->frozen_cook_label, 200, 32);

    /* corn_label: 标签 | "干果" | (578,405) | 126x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->corn_label, 555, 405);
    lv_obj_set_size(pg->corn_label, 200, 32);

    /* slow_cook_label: 标签 | "慢煮" | (859,212) | 63x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->slow_cook_label, 792, 212);
    lv_obj_set_size(pg->slow_cook_label, 200, 32);

    /* heat_contain_label: 标签 | "保温" | (826,405) | 126x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->heat_contain_label, 788, 405);
    lv_obj_set_size(pg->heat_contain_label, 200, 32);

    /* unfrozen_label: 标签 | "解冻" | (1075,212) | 126x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->unfrozen_label, 1067, 212);
    lv_obj_set_size(pg->unfrozen_label, 126, 32);

    /* some_cook_label: 标签 | "多段烹饪" | (1075,405) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->some_cook_label, 1032, 387);   /* 同事排版微调 */
    lv_obj_set_size(pg->some_cook_label, 200, 64);

    /* special_label: 标签 | "特殊功能" | (24,24) | 97x25 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->special_label, 24, 24);
    lv_obj_set_size(pg->special_label, 260, 25);

    /* piza_label: 标签 | "披萨" | (329,212) | 126x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->piza_label, 329, 212);
    lv_obj_set_size(pg->piza_label, 126, 32);

    /* major_img: 图片 | (94,110) | img: teshuimg.png */
    lv_obj_set_pos(pg->major_img, 94, 110);

}


/* ==============================================================================
 * special_menu_tz 英文布局基准（对应 PAGE_SPECIAL_MENU_TZ ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void special_menu_tz_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SPECIAL_MENU_TZ) return;
    special_menu_tz_t *pg = special_menu_tz_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* slowcook_button: 按钮 | (18,68) | 624x400 | font montserratmedium_16 | bg: div2bk30.png */
    lv_obj_set_pos(pg->slowcook_button, 18, 68);
    lv_obj_set_size(pg->slowcook_button, 624, 400);

    /* special_label: 标签 | "特殊功能" | (24,24) | 97x25 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->special_label, 24, 24);
    lv_obj_set_size(pg->special_label, 97, 25);

    /* major_img: 图片 | (278,188) | img: manzhu.png */
    lv_obj_set_pos(pg->major_img, 278, 188);

    /* slowcook_label: 标签 | "慢煮" | (265,291) | 131x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->slowcook_label, 265, 291);
    lv_obj_set_size(pg->slowcook_label, 131, 32);

}


/* ==============================================================================
 * stepset 英文布局基准（对应 PAGE_STEPSET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void stepset_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_STEPSET) return;
    stepset_t *pg = stepset_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* roller_mode: 滚轮 | (374,248) | 229x176 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->roller_mode, 374, 248);
    lv_obj_set_size(pg->roller_mode, 265, 176);

    /* roller_main: 滚轮 | (63,248) | 229x176 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->roller_main, 63, 248);
    lv_obj_set_size(pg->roller_main, 265, 176);

    /* next: 按钮 | (969,20) | 138x70 | font taiwanpearl_regular_36 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 969, 20);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_1: 图片 | (272,196) | img: modebg.png */
    lv_obj_set_pos(pg->image_1, 272, 196);

    /* label_1: 标签 | "多段烹饪" | (24,24) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_1, 24, 24);
    lv_obj_set_size(pg->label_1, 250, 32);
    lv_label_set_text(pg->label_1, "Multi-step Cooking");

    /* stepname: 标签 | "步骤一" | (596,139) | 144x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->stepname, 596, 139);
    lv_obj_set_size(pg->stepname, 144, 32);

    /* temp: 标签 | "80" | (734,303) | 86x49 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 734, 303);
    lv_obj_set_size(pg->temp, 86, 49);

    /* hour: 标签 | "01" | (937,303) | 59x49 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 937, 303);
    lv_obj_set_size(pg->hour, 59, 49);

    /* min: 标签 | "20" | (1071,303) | 59x49 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 59, 49);

    /* label_7: 标签 | "℃" | (823,319) | 27x24 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_7, 823, 319);
    lv_obj_set_size(pg->label_7, 50, 24);

    /* label_8: 标签 | "时" | (999,319) | 27x24 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_8, 999, 319);
    lv_obj_set_size(pg->label_8, 27, 24);

    /* label_9: 标签 | "分" | (1133,319) | 27x24 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_9, 1133, 319);
    lv_obj_set_size(pg->label_9, 27, 24);

    /* mainline: 图片 | (110,361) | img: underline_135x4.png */
    lv_obj_set_pos(pg->mainline, 110, 361);

    /* modeline4: 图片 | (420,361) | img: underline_135x4.png */
    lv_obj_set_pos(pg->modeline4, 420, 361);

    /* modeline2: 图片 | (451,361) | img: underline_73x4.png */
    lv_obj_set_pos(pg->modeline2, 451, 361);

    /* modeline3: 图片 | (433,361) | img: underline_108x4.png */
    lv_obj_set_pos(pg->modeline3, 433, 361);

    /* templine3: 图片 | (739,361) | img: underline_114x4.png */
    lv_obj_set_pos(pg->templine3, 739, 361);

    /* templine2: 图片 | (767,361) | img: underline_85x4.png */
    lv_obj_set_pos(pg->templine2, 767, 361);

    /* hourline: 图片 | (942,361) | img: underline_84x4.png */
    lv_obj_set_pos(pg->hourline, 942, 361);

    /* minline: 图片 | (1076,361) | img: underline_84x4.png */
    lv_obj_set_pos(pg->minline, 1076, 361);

    /* label_10: 标签 | "下一步" | (984,39) | 111x39 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_10, 984, 39);
    lv_obj_set_size(pg->label_10, 111, 39);

}


/* ==============================================================================
 * strudel_complete 英文布局基准（对应 PAGE_STRUDEL_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void strudel_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_STRUDEL_COMPLETE) return;
    strudel_complete_t *pg = strudel_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_337: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_337, 115, 320);

    /* status: 标签 | "| 果馅卷 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_55: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_55, 122, 326);
    lv_obj_set_size(pg->bar_55, 637, 20);

    /* label_722: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_722, 273, 157);
    lv_obj_set_size(pg->label_722, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_339: 图片 | (115,161) | img: strudelicon.png */
    lv_obj_set_pos(pg->image_339, 115, 161);

    /* image_13: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_13, 902, 160);

}


/* ==============================================================================
 * strudel_cooking 英文布局基准（对应 PAGE_STRUDEL_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void strudel_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_STRUDEL_COOKING) return;
    strudel_cooking_t *pg = strudel_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 果馅卷 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_328: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_328, 115, 320);

    /* bar_52: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_52, 122, 326);
    lv_obj_set_size(pg->bar_52, 637, 20);

    /* label_707: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_707, 273, 157);
    lv_obj_set_size(pg->label_707, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_329: 图片 | (115,161) | img: strudelicon.png */
    lv_obj_set_pos(pg->image_329, 115, 161);

}


/* ==============================================================================
 * strudel_menu 英文布局基准（对应 PAGE_STRUDEL_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void strudel_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_STRUDEL_MENU) return;
    strudel_menu_t *pg = strudel_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_324: 图片 | (24,117) | img: div1bg.png */
    lv_obj_set_pos(pg->image_324, 24, 117);

    /* label_682: 标签 | "烹饪时间" | (578,139) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_682, 578, 139);
    lv_obj_set_size(pg->label_682, 125, 32);

    /* hour: 标签 | "00" | (482,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 482, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_684: 标签 | "时" | (562,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_684, 562, 280);
    lv_obj_set_size(pg->label_684, 38, 32);

    /* label_685: 标签 | "分" | (762,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_685, 762, 280);
    lv_obj_set_size(pg->label_685, 38, 32);

    /* min: 标签 | "30" | (682,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_687: 标签 | "果馅卷" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_687, 24, 25);
    lv_obj_set_size(pg->label_687, 144, 32);

    /* label_688: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_688, 996, 39);
    lv_obj_set_size(pg->label_688, 117, 32);

    /* hourline: 图片 | (480,327) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 480, 327);

    /* minline: 图片 | (680,327) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 680, 327);

}


/* ==============================================================================
 * strudel_set 英文布局基准（对应 PAGE_STRUDEL_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void strudel_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_STRUDEL_SET) return;
    strudel_set_t *pg = strudel_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_689: 标签 | "果馅卷" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_689, 24, 25);
    lv_obj_set_size(pg->label_689, 144, 32);

    /* label_690: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_690, 165, 157);
    lv_obj_set_size(pg->label_690, 26, 32);

    /* label_691: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_691, 252, 157);
    lv_obj_set_size(pg->label_691, 64, 32);

    /* label_692: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_692, 141, 270);
    lv_obj_set_size(pg->label_692, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_695: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_695, 368, 270);
    lv_obj_set_size(pg->label_695, 30, 32);

    /* label_696: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_696, 64, 32);

    /* image_327: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_327, 164, 376);

    /* label_697: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_697, 232, 385);
    lv_obj_set_size(pg->label_697, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_698: 标签 | "延" | (840,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_698, 840, 154);
    lv_obj_set_size(pg->label_698, 26, 32);

    /* label_699: 标签 | "迟" | (930,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_699, 930, 154);
    lv_obj_set_size(pg->label_699, 26, 32);

    /* label_700: 标签 | "自动保温" | (831,270) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_700, 831, 270);
    lv_obj_set_size(pg->label_700, 130, 32);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 260);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "200" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_702: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_702, 1010, 39);
    lv_obj_set_size(pg->label_702, 85, 36);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 260);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * strudel_setting 英文布局基准（对应 PAGE_STRUDEL_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void strudel_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_STRUDEL_SETTING) return;
    strudel_setting_t *pg = strudel_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_31: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_31, 0, 0);
    lv_obj_set_size(pg->container_31, 1280, 480);

    /* image_330: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_330, 115, 115);

    /* label_709: 标签 | "烹饪时间" | (373,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_709, 373, 136);
    lv_obj_set_size(pg->label_709, 135, 30);

    /* hour: 标签 | "00" | (266,212) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 266, 212);
    lv_obj_set_size(pg->hour, 102, 46);

    /* label_711: 标签 | "时" | (340,228) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_711, 340, 228);
    lv_obj_set_size(pg->label_711, 37, 30);

    /* hourline: 图片 | (291,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 291, 269);

    /* min: 标签 | "00" | (483,212) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_713: 标签 | "分" | (558,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_713, 558, 229);
    lv_obj_set_size(pg->label_713, 37, 30);

    /* minline: 图片 | (508,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 508, 269);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * strudel_stop 英文布局基准（对应 PAGE_STRUDEL_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void strudel_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_STRUDEL_STOP) return;
    strudel_stop_t *pg = strudel_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 果馅卷 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_333: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_333, 115, 320);

    /* bar_53: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_53, 122, 326);
    lv_obj_set_size(pg->bar_53, 637, 20);

    /* label_716: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_716, 273, 157);
    lv_obj_set_size(pg->label_716, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_334: 图片 | (115,161) | img: strudelicon.png */
    lv_obj_set_pos(pg->image_334, 115, 161);

}


/* ==============================================================================
 * strudel_stop_back 英文布局基准（对应 PAGE_STRUDEL_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void strudel_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_STRUDEL_STOP_BACK) return;
    strudel_stop_back_t *pg = strudel_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_335: 图片 | (115,161) | img: strudelicon.png */
    lv_obj_set_pos(pg->image_335, 115, 161);

    /* status: 标签 | "| 果馅卷 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_336: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_336, 115, 320);

    /* bar_54: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_54, 122, 326);
    lv_obj_set_size(pg->bar_54, 637, 20);

    /* label_718: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_718, 273, 157);
    lv_obj_set_size(pg->label_718, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_32: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_32, 0, 0);
    lv_obj_set_size(pg->container_32, 1280, 480);

    /* label_719: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_719, 895, 160);
    lv_obj_set_size(pg->label_719, 252, 36);

    /* label_720: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_720, 895, 198);
    lv_obj_set_size(pg->label_720, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * temptip 英文布局基准（对应 PAGE_TEMPTIP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void temptip_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_TEMPTIP) return;
    temptip_t *pg = temptip_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* button_1: 按钮 | (19,113) | 1243x347 | font taiwanpearl_regular_48 | bg: div1bg.png */
    lv_obj_set_pos(pg->button_1, 19, 113);
    lv_obj_set_size(pg->button_1, 1243, 347);

    /* tip1: 标签 | "腔体温度过热，需等待温度降下来。" | (104,215) | 1064x49 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->tip1, 104, 215);
    lv_obj_set_size(pg->tip1, 1064, 49);

    /* tip2: 标签 | "所选功能需要烤箱处于冷却状态，" | (104,286) | 1064x29 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->tip2, 104, 286);
    lv_obj_set_size(pg->tip2, 1064, 29);

    /* tip3: 标签 | "请等待烤箱冷却！" | (104,322) | 1064x29 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->tip3, 104, 322);
    lv_obj_set_size(pg->tip3, 1064, 29);

}


/* ==============================================================================
 * toastcolor 英文布局基准（对应 PAGE_TOASTCOLOR ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void toastcolor_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_TOASTCOLOR) return;
    toastcolor_t *pg = toastcolor_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_9: 图片 | (24,119) | img: div1bg.png */
    lv_obj_set_pos(pg->image_9, 24, 119);

    /* label_23: 标签 | "选择烤色程度" | (529,139) | 216x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_23, 529, 139);
    lv_obj_set_size(pg->label_23, 216, 36);

    /* label_24: 标签 | "面包卷" | (24,24) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_24, 24, 24);
    lv_obj_set_size(pg->label_24, 144, 32);

    /* next: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->next, 983, 22);
    lv_obj_set_size(pg->next, 135, 71);

    /* ---- 场景分支: 三组互斥(g_toast_mode), 运行时仅一组可见 ----
       DEGREE   烤色程度: 面包/蛋糕类 + 探针菜(第六感家禽→烤全鸡/鸭, 肉→猪肉→烤猪里脊肉/烤五花肉)
       MATURITY 成熟度:   探针菜(第六感探针→肉→牛肉→烤牛排)
       WEIGHT   份量:     鸡翅类/烤鳕鱼/烤全鱼/炸薯条/烤玉米/烤红薯 ---- */
    if (g_toast_mode == TOAST_MODE_WEIGHT) {
        /* weight: 标签 | "1600" | (519,248) | 167x77 | font taiwanpearl_regular_72 */
        lv_obj_set_pos(pg->weight, 519, 248);
        lv_obj_set_size(pg->weight, 167, 77);

        /* weighticon: 标签 | "g" | (685,280) | 37x42 | font taiwanpearl_regular_36 */
        lv_obj_set_pos(pg->weighticon, 685, 280);
        lv_obj_set_size(pg->weighticon, 37, 42);

        /* weightline3: 图片 | (569,328) | img: settingline3_141x4.png */
        lv_obj_set_pos(pg->weightline3, 569, 328);

        /* weightline4: 图片 | (530,328) | img: settingline3_179x4.png */
        lv_obj_set_pos(pg->weightline4, 530, 328);
        /* TODO: 英文实测后调整(数字位数多时 weightline4 位置) */
    } else if (g_toast_mode == TOAST_MODE_MATURITY) {
        /* Maturity: 标签 | "全熟" | (531,248) | 218x77 | font taiwanpearl_regular_72 */
        lv_obj_set_pos(pg->Maturity, 531, 248);
        lv_obj_set_size(pg->Maturity, 218, 77);

        /* maturityline3: 图片 | (537,328) | img: settingline3_211x4.png */
        lv_obj_set_pos(pg->maturityline3, 537, 328);

        /* maturityline2: 图片 | (572,328) | img: settingline3_139x4.png */
        lv_obj_set_pos(pg->maturityline2, 572, 328);
        /* TODO: 英文实测后调整(Medium Well 等文本宽度) */
    } else {
        /* DEGREE 烤色程度: 面包/蛋糕类 + 探针菜 */
        /* degree: 标签 | "浅中深" | (590,248) | 100x77 | font taiwanpearl_regular_72 */
        lv_obj_set_pos(pg->degree, 590, 248);
        lv_obj_set_size(pg->degree, 100, 77);

        /* line: 图片 | (606,328) | img: settingline3_73x4.png */
        lv_obj_set_pos(pg->line, 606, 328);
    }

     /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     lv_obj_t *obj = NULL;
     obj = pg->label_24;
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}


/* ==============================================================================
 * top_bbq_complete 英文布局基准（对应 PAGE_TOP_BBQ_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void top_bbq_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_TOP_BBQ_COMPLETE) return;
    top_bbq_complete_t *pg = top_bbq_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_52: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_52, 115, 320);

    /* label_107: 标签 | "| 顶部烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_107, 274, 232);
    lv_obj_set_size(pg->label_107, 490, 39);

    /* bar_9: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_9, 122, 326);
    lv_obj_set_size(pg->bar_9, 637, 20);

    /* label_108: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_108, 273, 157);
    lv_obj_set_size(pg->label_108, 330, 60);   /* 对齐 updown bbq 系 */

    /* button_27: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->button_27, 609, 170);
    lv_obj_set_size(pg->button_27, 50, 43);

    /* image_55: 图片 | (115,161) | img: topicon.png */
    lv_obj_set_pos(pg->image_55, 115, 161);

    /* image_1: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_1, 902, 160);

}


/* ==============================================================================
 * top_bbq_cooking 英文布局基准（对应 PAGE_TOP_BBQ_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void top_bbq_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_TOP_BBQ_COOKING) return;
    top_bbq_cooking_t *pg = top_bbq_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* statu: 标签 | "| 顶部烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->statu, 274, 232);
    lv_obj_set_size(pg->statu, 490, 39);

    /* label_80: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_80, 907, 157);
    lv_obj_set_size(pg->label_80, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_36: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_36, 115, 320);

    /* bar_6: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_6, 122, 326);
    lv_obj_set_size(pg->bar_6, 637, 20);

    /* label_81: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_81, 273, 157);
    lv_obj_set_size(pg->label_81, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* up_img: 图片 | (115,161) | img: topicon.png */
    lv_obj_set_pos(pg->up_img, 115, 161);

}


/* ==============================================================================
 * top_bbq_menu 英文布局基准（对应 PAGE_TOP_BBQ_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void top_bbq_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_TOP_BBQ_MENU) return;
    top_bbq_menu_t *pg = top_bbq_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next_button: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next_button, 981, 22);
    lv_obj_set_size(pg->next_button, 138, 70);

    /* image_28: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_28, 24, 118);

    /* image_29: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_29, 645, 118);

    /* label_42: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_42, 275, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_42, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* temp_label: 标签 | "180" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp_label, 192, 249);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->temp_label, 180, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_44: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_44, 372, 281);
    lv_obj_set_size(pg->label_44, 38, 32);

    /* label_45: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_45, 855, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_45, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* hour_label: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour_label, 788, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->hour_label, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_47: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_47, 872, 280);
    lv_obj_set_size(pg->label_47, 38, 32);

    /* label_48: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_48, 1075, 281);
    lv_obj_set_size(pg->label_48, 38, 32);

    /* min_label: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_pos(pg->min_label, 990, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->min_label, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_50: 标签 | "顶部烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_50, 24, 24);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_50, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_51: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_51, 1007, 41);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_51, 117, 32);   /* 对齐 updown 系英文排版(A) */

    /* templine_long: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine_long, 252, 328);

    /* templine_short: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine_short, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * top_bbq_set 英文布局基准（对应 PAGE_TOP_BBQ_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void top_bbq_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_TOP_BBQ_SET) return;
    top_bbq_set_t *pg = top_bbq_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_52: 标签 | "顶部烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_52, 24, 25);
    lv_obj_set_size(pg->label_52, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_53: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_53, 163, 155);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_53, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_54: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_54, 252, 157);
    lv_obj_set_size(pg->label_54, 64, 32);

    /* label_55: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_55, 150, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_55, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->min, 317, 258);
    lv_obj_set_size(pg->min, 68, 66);   /* 对齐 updown 系英文排版(A) */

    /* label_58: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_58, 368, 270);
    lv_obj_set_size(pg->label_58, 30, 32);

    /* label_59: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->label_59, 371, 271);
    lv_obj_set_size(pg->label_59, 42, 32);   /* 对齐 updown 系英文排版(A) */

    /* image_35: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_35, 164, 376);

    /* label_60: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_60, 232, 385);
    lv_obj_set_size(pg->label_60, 220, 32);   /* 对齐 updown 系英文排版(A) */

    /* sure_button: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure_button, 983, 22);
    lv_obj_set_size(pg->sure_button, 135, 71);

    /* label_61: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_61, 785, 154);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_61, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_62: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_62, 885, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_62, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_63: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_63, 930, 270);
    lv_obj_set_size(pg->label_63, 26, 32);

    /* label_64: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_64, 765, 381);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_64, 200, 32);   /* 对齐 updown 系英文排版(A) */

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* uptemp2: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->uptemp2, 269, 143);
    lv_obj_set_size(pg->uptemp2, 100, 53);

    /* label_71: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_71, 1007, 41);
    lv_obj_set_size(pg->label_71, 85, 36);

    /* onpreheat_button: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat_button, 983, 144);
    lv_obj_set_size(pg->onpreheat_button, 135, 53);

    /* ondelay_button: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay_button, 983, 260);
    lv_obj_set_size(pg->ondelay_button, 135, 53);

    /* oncontain_button: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain_button, 983, 372);
    lv_obj_set_size(pg->oncontain_button, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 355, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 327, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * top_bbq_setting 英文布局基准（对应 PAGE_TOP_BBQ_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void top_bbq_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_TOP_BBQ_SETTING) return;
    top_bbq_setting_t *pg = top_bbq_setting_get(&ui_manager);
    if (!pg) return;
	
    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* time_label: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->time_label, 907, 157);
    lv_obj_set_size(pg->time_label, 257, 60);

    /* container_4: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_4, 0, 0);
    lv_obj_set_size(pg->container_4, 1280, 480);

    /* image_38: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_38, 115, 115);

    /* label_83: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_83, 203, 134);
    lv_obj_set_size(pg->label_83, 150, 30);

    /* label_84: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_84, 444, 134);
    lv_obj_set_size(pg->label_84, 150, 30);

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 217);
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 303, 229);
    lv_obj_set_size(pg->icon3, 37, 30);

    /* line2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->line2, 236, 269);

    /* line3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->line3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 377, 217);
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
	// TODO 位置对齐参考 hour: 标签
    lv_obj_set_size(pg->min, 102, 46);

    /* label_93: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_93, 450, 231);
    lv_obj_set_size(pg->label_93, 37, 30);

    /* label_94: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_94, 612, 231);
    lv_obj_set_size(pg->label_94, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 289, 229);
    lv_obj_set_size(pg->icon2, 37, 30);

    /* surebutton: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->surebutton, 959, 295);
    lv_obj_set_size(pg->surebutton, 129, 83);

}


/* ==============================================================================
 * top_bbq_stop 英文布局基准（对应 PAGE_TOP_BBQ_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void top_bbq_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_TOP_BBQ_STOP) return;
    top_bbq_stop_t *pg = top_bbq_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* statu: 标签 | "| 顶部烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->statu, 274, 232);
    lv_obj_set_size(pg->statu, 490, 39);

    /* time_label: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->time_label, 907, 157);
    lv_obj_set_size(pg->time_label, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_45: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_45, 115, 320);

    /* bar_7: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_7, 122, 326);
    lv_obj_set_size(pg->bar_7, 637, 20);

    /* label_102: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_102, 273, 157);
    lv_obj_set_size(pg->label_102, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_48: 图片 | (115,161) | img: topicon.png */
    lv_obj_set_pos(pg->image_48, 115, 161);

}


/* ==============================================================================
 * top_bbq_stop_back 英文布局基准（对应 PAGE_TOP_BBQ_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void top_bbq_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_TOP_BBQ_STOP_BACK) return;
    top_bbq_stop_back_t *pg = top_bbq_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* up: 图片 | (115,161) | img: topicon.png */
    lv_obj_set_pos(pg->up, 115, 161);

    /* statu: 标签 | "| 顶部烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->statu, 274, 232);
    lv_obj_set_size(pg->statu, 490, 39);

    /* image_49: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_49, 115, 320);

    /* bar_8: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_8, 122, 326);
    lv_obj_set_size(pg->bar_8, 637, 20);

    /* label_104: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_104, 273, 157);
    lv_obj_set_size(pg->label_104, 330, 60);   /* 对齐 updown bbq 系 */   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_5: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_5, 0, 0);
    lv_obj_set_size(pg->container_5, 1280, 480);

    /* label_105: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_105, 252, 36);

    /* label_106: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_106, 895, 198);
    lv_obj_set_size(pg->label_106, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * unfrozen_complete 英文布局基准（对应 PAGE_UNFROZEN_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void unfrozen_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UNFROZEN_COMPLETE) return;
    unfrozen_complete_t *pg = unfrozen_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_216: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_216, 115, 320);

    /* status: 标签 | "| 解冻 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_35: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_35, 122, 326);
    lv_obj_set_size(pg->bar_35, 637, 20);

    /* label_484: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_484, 273, 157);
    lv_obj_set_size(pg->label_484, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_218: 图片 | (149,161) | img: unfrozenicon.png */
    lv_obj_set_pos(pg->image_218, 149, 161);

    /* image_8: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_8, 902, 160);

}


/* ==============================================================================
 * unfrozen_cooking 英文布局基准（对应 PAGE_UNFROZEN_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void unfrozen_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UNFROZEN_COOKING) return;
    unfrozen_cooking_t *pg = unfrozen_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 解冻 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_205: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_205, 115, 320);

    /* bar_32: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_32, 122, 326);
    lv_obj_set_size(pg->bar_32, 637, 20);

    /* label_465: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_465, 273, 157);
    lv_obj_set_size(pg->label_465, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_206: 图片 | (149,161) | img: unfrozenicon.png */
    lv_obj_set_pos(pg->image_206, 149, 161);

}


/* ==============================================================================
 * unfrozen_menu 英文布局基准（对应 PAGE_UNFROZEN_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void unfrozen_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UNFROZEN_MENU) return;
    unfrozen_menu_t *pg = unfrozen_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_198: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_198, 24, 118);

    /* image_199: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_199, 645, 118);

    /* label_436: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_436, 300, 137);
    lv_obj_set_size(pg->label_436, 64, 32);

    /* temp: 标签 | "40" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_438: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_438, 372, 281);
    lv_obj_set_size(pg->label_438, 38, 32);

    /* label_439: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_439, 891, 137);
    lv_obj_set_size(pg->label_439, 125, 32);

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 793, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_441: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_441, 872, 280);
    lv_obj_set_size(pg->label_441, 38, 32);

    /* label_442: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_442, 1075, 281);
    lv_obj_set_size(pg->label_442, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_444: 标签 | "解冻" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_444, 24, 25);
    lv_obj_set_size(pg->label_444, 144, 32);

    /* label_445: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_445, 996, 39);
    lv_obj_set_size(pg->label_445, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * unfrozen_set 英文布局基准（对应 PAGE_UNFROZEN_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void unfrozen_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UNFROZEN_SET) return;
    unfrozen_set_t *pg = unfrozen_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_446: 标签 | "解冻" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_446, 24, 25);
    lv_obj_set_size(pg->label_446, 144, 32);

    /* label_447: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_447, 165, 157);
    lv_obj_set_size(pg->label_447, 26, 32);

    /* label_448: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_448, 252, 157);
    lv_obj_set_size(pg->label_448, 64, 32);

    /* label_449: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_449, 141, 270);
    lv_obj_set_size(pg->label_449, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_452: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_452, 368, 270);
    lv_obj_set_size(pg->label_452, 30, 32);

    /* label_453: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_453, 64, 32);

    /* image_204: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_204, 164, 376);

    /* label_454: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_454, 232, 385);
    lv_obj_set_size(pg->label_454, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_455: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_455, 830, 154);
    lv_obj_set_size(pg->label_455, 130, 32);

    /* label_456: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_456, 840, 270);
    lv_obj_set_size(pg->label_456, 26, 32);

    /* label_457: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_457, 930, 270);
    lv_obj_set_size(pg->label_457, 26, 32);

    /* label_458: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_458, 831, 381);
    lv_obj_set_size(pg->label_458, 130, 32);

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_460: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_460, 1010, 39);
    lv_obj_set_size(pg->label_460, 85, 36);

    /* onpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat, 983, 144);
    lv_obj_set_size(pg->onpreheat, 135, 53);

    /* ondelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 260);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 372);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * unfrozen_setting 英文布局基准（对应 PAGE_UNFROZEN_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void unfrozen_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UNFROZEN_SETTING) return;
    unfrozen_setting_t *pg = unfrozen_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_21: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_21, 0, 0);
    lv_obj_set_size(pg->container_21, 1280, 480);

    /* image_207: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_207, 115, 115);

    /* label_467: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_467, 242, 136);
    lv_obj_set_size(pg->label_467, 73, 30);

    /* label_468: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_468, 454, 136);
    lv_obj_set_size(pg->label_468, 135, 30);

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 213);
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 300, 229);
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 374, 213);
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_473: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_473, 449, 229);
    lv_obj_set_size(pg->label_473, 37, 30);

    /* label_474: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_474, 612, 229);
    lv_obj_set_size(pg->label_474, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 288, 229);
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * unfrozen_stop 英文布局基准（对应 PAGE_UNFROZEN_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void unfrozen_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UNFROZEN_STOP) return;
    unfrozen_stop_t *pg = unfrozen_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 解冻 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_212: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_212, 115, 320);

    /* bar_33: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_33, 122, 326);
    lv_obj_set_size(pg->bar_33, 637, 20);

    /* label_478: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_478, 273, 157);
    lv_obj_set_size(pg->label_478, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_213: 图片 | (149,161) | img: unfrozenicon.png */
    lv_obj_set_pos(pg->image_213, 149, 161);

}


/* ==============================================================================
 * unfrozen_stop_back 英文布局基准（对应 PAGE_UNFROZEN_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void unfrozen_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UNFROZEN_STOP_BACK) return;
    unfrozen_stop_back_t *pg = unfrozen_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_214: 图片 | (149,161) | img: unfrozenicon.png */
    lv_obj_set_pos(pg->image_214, 149, 161);

    /* status: 标签 | "| 解冻 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_215: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_215, 115, 320);

    /* bar_34: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_34, 122, 326);
    lv_obj_set_size(pg->bar_34, 637, 20);

    /* label_480: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_480, 273, 157);
    lv_obj_set_size(pg->label_480, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_22: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_22, 0, 0);
    lv_obj_set_size(pg->container_22, 1280, 480);

    /* label_481: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_481, 895, 160);
    lv_obj_set_size(pg->label_481, 252, 36);

    /* label_482: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_482, 895, 198);
    lv_obj_set_size(pg->label_482, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * updown_bbq_complete 英文布局基准（对应 PAGE_UPDOWN_BBQ_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_COMPLETE) return;
    updown_bbq_complete_t *pg = updown_bbq_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* barbk_img: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->barbk_img, 115, 320);

    /* statu_label: 标签 | "| 上下烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->statu_label, 274, 232);
    lv_obj_set_size(pg->statu_label, 490, 39);

    /* bar_3: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_3, 122, 326);
    lv_obj_set_size(pg->bar_3, 637, 20);

    /* updown_img: 图片 | (115,161) | img: updown_img.png */
    lv_obj_set_pos(pg->updown_img, 115, 161);

    /* complete_label: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->complete_label, 273, 157);
    lv_obj_set_size(pg->complete_label, 330, 60);

    /* image_26: 图片 | (845,160) | img: hotcare.png */
    lv_obj_set_pos(pg->image_26, 845, 160);

    /* little_button: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little_button, 609, 170);
    lv_obj_set_size(pg->little_button, 50, 43);

}


/* ==============================================================================
 * updown_bbq_complete_probe 英文布局基准（对应 PAGE_UPDOWN_BBQ_COMPLETE_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_complete_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_COMPLETE_PROBE) return;
    updown_bbq_complete_probe_t *pg = updown_bbq_complete_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_30: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_30, 115, 320);

    /* label_74: 标签 | "| 上下烧烤 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_74, 274, 232);
    lv_obj_set_size(pg->label_74, 490, 39);

    /* bar_4: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_4, 122, 326);
    lv_obj_set_size(pg->bar_4, 637, 20);

    /* image_31: 图片 | (115,161) | img: updown_img.png */
    lv_obj_set_pos(pg->image_31, 115, 161);

    /* label_75: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_75, 273, 157);
    lv_obj_set_size(pg->label_75, 235, 60);

    /* image_2: 图片 | (845,160) | img: hotcare.png */
    lv_obj_set_pos(pg->image_2, 845, 160);

}


/* ==============================================================================
 * updown_bbq_cooking 英文布局基准（对应 PAGE_UPDOWN_BBQ_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_COOKING) return;
    updown_bbq_cooking_t *pg = updown_bbq_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* updown_label: 标签 | "| 上下烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->updown_label, 274, 232);
    lv_obj_set_size(pg->updown_label, 490, 39);

    /* time_label: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->time_label, 907, 157);
    lv_obj_set_size(pg->time_label, 250, 60);

    /* stop_button: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop_button, 959, 295);
    lv_obj_set_size(pg->stop_button, 129, 83);

    /* barbk_img: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->barbk_img, 115, 320);

    /* bar: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar, 122, 326);
    lv_obj_set_size(pg->bar, 637, 20);

    /* updown_img: 图片 | (115,161) | img: updown_img.png */
    lv_obj_set_pos(pg->updown_img, 115, 161);

    /* label_10: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_10, 273, 157);
    lv_obj_set_size(pg->label_10, 300, 60);

    /* little_button: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little_button, 609, 170);
    lv_obj_set_size(pg->little_button, 50, 43);

}


/* ==============================================================================
 * updown_bbq_cooking_probe 英文布局基准（对应 PAGE_UPDOWN_BBQ_COOKING_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_cooking_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_COOKING_PROBE) return;
    updown_bbq_cooking_probe_t *pg = updown_bbq_cooking_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 上下烧烤 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* temp: 标签 | "36℃" | (919,156) | 134x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->temp, 919, 156);
    lv_obj_set_size(pg->temp, 134, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_22: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_22, 115, 320);

    /* bar_1: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_1, 122, 326);
    lv_obj_set_size(pg->bar_1, 637, 20);

    /* image_23: 图片 | (115,161) | img: updown_img.png */
    lv_obj_set_pos(pg->image_23, 115, 161);

    /* label_66: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_66, 273, 157);
    lv_obj_set_size(pg->label_66, 235, 60);

    /* image_24: 图片 | (1068,180) | img: probeicon.png */
    lv_obj_set_pos(pg->image_24, 1068, 180);

}


/* ==============================================================================
 * updown_bbq_menu 英文布局基准（对应 PAGE_UPDOWN_BBQ_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_MENU) return;
    updown_bbq_menu_t *pg = updown_bbq_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next_button: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next_button, 981, 22);
    lv_obj_set_size(pg->next_button, 138, 70);

    /* bk1_img: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->bk1_img, 24, 118);

    /* bk2_img: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->bk2_img, 645, 118);

    /* label_1: 标签 | "温度" | (300,137) | 60x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_1, 275, 137);
    lv_obj_set_size(pg->label_1, 100, 32);

    /* tempnum_label: 标签 | "180" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->tempnum_label, 192, 249);
    lv_obj_set_size(pg->tempnum_label, 180, 72);

    /* temp_label: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->temp_label, 372, 281);
    lv_obj_set_size(pg->temp_label, 38, 32);

    /* cook_label: 标签 | "烹饪时间" | (891,137) | 120x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->cook_label, 855, 137);
    lv_obj_set_size(pg->cook_label, 180, 32);

    /* hournum_label: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hournum_label, 788, 248);
    lv_obj_set_size(pg->hournum_label, 100, 72);

    /* hour_label: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->hour_label, 872, 280);
    lv_obj_set_size(pg->hour_label, 38, 32);

    /* min_label: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min_label, 38, 32);

    /* minnum_label: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->minnum_label, 990, 248);
    lv_obj_set_size(pg->minnum_label, 100, 72);

    /* updown_label: 标签 | "上下烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->updown_label, 24, 24);
    lv_obj_set_size(pg->updown_label, 160, 32);

    /* next_label: 标签 | "下一步" | (996,39) | 108x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->next_label, 996, 41);
    lv_obj_set_size(pg->next_label, 108, 32);

    /* temeline_long: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->temeline_long, 252, 328);

    /* templine_short: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine_short, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * updown_bbq_menu_low 英文布局基准（对应 PAGE_UPDOWN_BBQ_MENU_LOW ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_menu_low_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_MENU_LOW) return;
    updown_bbq_menu_low_t *pg = updown_bbq_menu_low_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_30: 图片 | (24,118) | img: preheatbg.png */
    lv_obj_set_pos(pg->image_30, 24, 118);

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* label_57: 标签 | "下一步" | (996,39) | 108x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_57, 996, 39);
    lv_obj_set_size(pg->label_57, 108, 32);

    /* label_60: 标签 | "上下烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_60, 24, 25);
    lv_obj_set_size(pg->label_60, 144, 32);

    /* label_7: 标签 | "温度" | (608,139) | 60x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_7, 608, 139);
    lv_obj_set_size(pg->label_7, 60, 32);

    /* temp: 标签 | "180" | (562,248) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 562, 248);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_9: 标签 | "℃" | (682,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_9, 682, 280);
    lv_obj_set_size(pg->label_9, 38, 32);

    /* dir2: 标签 | "↓" | (570,280) | 31x41 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->dir2, 570, 280);
    lv_obj_set_size(pg->dir2, 31, 41);

    /* dir3: 标签 | "↓" | (531,280) | 31x41 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->dir3, 531, 280);
    lv_obj_set_size(pg->dir3, 31, 41);

    /* line3: 图片 | (538,327) | img: underline_181x4.png */
    lv_obj_set_pos(pg->line3, 538, 327);

    /* line2: 图片 | (573,327) | img: underline_146x4.png */
    lv_obj_set_pos(pg->line2, 573, 327);

}


/* ==============================================================================
 * updown_bbq_menu_probe 英文布局基准（对应 PAGE_UPDOWN_BBQ_MENU_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_menu_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_MENU_PROBE) return;
    updown_bbq_menu_probe_t *pg = updown_bbq_menu_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_3: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_3, 24, 118);

    /* image_4: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_4, 645, 118);

    /* label_4: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_4, 286, 137);
    lv_obj_set_size(pg->label_4, 100, 32);

    /* temp: 标签 | "180" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 192, 249);
    lv_obj_set_size(pg->temp, 180, 72);

    /* label_6: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_6, 372, 281);
    lv_obj_set_size(pg->label_6, 50, 32);

    /* label_7: 标签 | "探针温度" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_7, 860, 137);
    lv_obj_set_size(pg->label_7, 200, 32);

    /* label_12: 标签 | "上下烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_12, 24, 25);
    lv_obj_set_size(pg->label_12, 200, 32);

    /* label_13: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_13, 1010, 41);
    lv_obj_set_size(pg->label_13, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_img_set_src(pg->templine3, LVGL_IMAGE_PATH(underline_179x4.png));
    lv_obj_set_pos(pg->templine3, 244, 328);
    lv_obj_set_size(pg->templine3, 165, 4);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* probetemp: 标签 | "80" | (853,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->probetemp, 792, 249);
    lv_obj_set_size(pg->probetemp, 180, 72);

    /* label_15: 标签 | "℃" | (973,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_15, 973, 281);
    lv_obj_set_size(pg->label_15, 50, 32);

    /* probeline2: 图片 | (891,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->probeline2, 889, 328);

}


/* ==============================================================================
 * updown_bbq_menu_top 英文布局基准（对应 PAGE_UPDOWN_BBQ_MENU_TOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_menu_top_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_MENU_TOP) return;
    updown_bbq_menu_top_t *pg = updown_bbq_menu_top_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_21: 图片 | (24,118) | img: preheatbg.png */
    lv_obj_set_pos(pg->image_21, 24, 118);

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* label_50: 标签 | "下一步" | (996,39) | 108x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_50, 996, 39);
    lv_obj_set_size(pg->label_50, 108, 32);

    /* label_49: 标签 | "上下烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_49, 24, 25);
    lv_obj_set_size(pg->label_49, 144, 32);

    /* label_1: 标签 | "温度" | (608,139) | 60x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_1, 608, 139);
    lv_obj_set_size(pg->label_1, 60, 32);

    /* temp: 标签 | "180" | (562,248) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 562, 248);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_3: 标签 | "℃" | (682,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_3, 682, 280);
    lv_obj_set_size(pg->label_3, 38, 32);

    /* line2: 图片 | (573,327) | img: underline_146x4.png */
    lv_obj_set_pos(pg->line2, 573, 327);

    /* line3: 图片 | (538,327) | img: underline_181x4.png */
    lv_obj_set_pos(pg->line3, 538, 327);

    /* dir3: 标签 | "↑" | (531,280) | 31x41 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->dir3, 531, 280);
    lv_obj_set_size(pg->dir3, 31, 41);

    /* dir2: 标签 | "↑" | (570,280) | 31x41 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->dir2, 570, 280);
    lv_obj_set_size(pg->dir2, 31, 41);

}


/* ==============================================================================
 * updown_bbq_set 英文布局基准（对应 PAGE_UPDOWN_BBQ_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_SET) return;
    updown_bbq_set_t *pg = updown_bbq_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* updown_label: 标签 | "上下烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->updown_label, 24, 25);
    lv_obj_set_size(pg->updown_label, 160, 32);

    /* wen_label: 标签 | "温" | (165,155) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->wen_label, 163, 155);
    lv_obj_set_size(pg->wen_label, 100, 32);

    /* du_label: 标签 | "度：" | (252,155) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->du_label, 252, 155);
    lv_obj_set_size(pg->du_label, 64, 32);

    /* cooktime_label: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->cooktime_label, 150, 270);
    lv_obj_set_size(pg->cooktime_label, 175, 32);

    /* hour_label: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour_label, 307, 254);
    lv_obj_set_size(pg->hour_label, 62, 53);

    /* min_label: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->min_label, 317, 258);
	lv_obj_set_size(pg->min_label, 68, 66);

    /* shi_label: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->shi_label, 368, 270);
    lv_obj_set_size(pg->shi_label, 30, 32);

    /* fen_label: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
	// lv_obj_set_pos(pg->fen_label, 371, 271);
    lv_obj_set_size(pg->fen_label, 42, 32);

    /* place_img: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->place_img, 164, 376);

    /* place_label: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->place_label, 232, 385);
    lv_obj_set_size(pg->place_label, 220, 32);

    /* sure_button: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure_button, 983, 22);
    lv_obj_set_size(pg->sure_button, 135, 71);

    /* preheat_label: 标签 | "快速预热" | (840,154) | 120x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->preheat_label, 785, 154);
    lv_obj_set_size(pg->preheat_label, 180, 32);

    /* yan_label: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->yan_label, 885, 270);
    lv_obj_set_size(pg->yan_label, 100, 32);

    /* chi_label: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->chi_label, 930, 270);
    lv_obj_set_size(pg->chi_label, 26, 32);

    /* downtemp_button: 按钮 | (476,140) | 170x61 | font montserratmedium_16 | bg: tembk.png */
    lv_obj_set_pos(pg->downtemp_button, 428, 140);
    lv_obj_set_size(pg->downtemp_button, 170, 61);

    /* uptemp_button: 按钮 | (308,140) | 170x61 | font montserratmedium_16 | bg: tembk.png */
    lv_obj_set_pos(pg->uptemp_button, 260, 140);
    lv_obj_set_size(pg->uptemp_button, 170, 61);

    /* contain_label: 标签 | "自动保温" | (841,381) | 120x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->contain_label, 765, 381);
    lv_obj_set_size(pg->contain_label, 200, 32);

    /* preheat_button: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->preheat_button, 983, 144);
    lv_obj_set_size(pg->preheat_button, 135, 53);

    /* delay_button: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->delay_button, 983, 260);
    lv_obj_set_size(pg->delay_button, 135, 53);

    /* contain_button: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->contain_button, 983, 372);
    lv_obj_set_size(pg->contain_button, 135, 53);

    /* down3_dir_label: 标签 | "↓" | (493,159) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->down3_dir_label, 445, 159);
    lv_obj_set_size(pg->down3_dir_label, 26, 32);

    /* down3_tempnum_label: 标签 | "100" | (510,143) | 87x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->down3_tempnum_label, 461, 147);
    lv_obj_set_size(pg->down3_tempnum_label, 87, 53);

    /* down3_icon_label: 标签 | "℃" | (600,159) | 26x28 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->down3_icon_label, 521, 159);
    lv_obj_set_size(pg->down3_icon_label, 60, 28);

    /* up2_dir_label: 标签 | "↑" | (342,159) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->up2_dir_label, 285, 159);
    lv_obj_set_size(pg->up2_dir_label, 26, 32);

    /* up2_tempnum_label: 标签 | "80" | (365,143) | 59x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->up2_tempnum_label, 310, 147);
    lv_obj_set_size(pg->up2_tempnum_label, 59, 53);

    /* up2_icon_label: 标签 | "℃" | (422,159) | 26x28 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->up2_icon_label, 340, 159);
    lv_obj_set_size(pg->up2_icon_label, 60, 28);

    /* sure_label: 标签 | "确 定" | (1010,39) | 80x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->sure_label, 1000, 42);
    lv_obj_set_size(pg->sure_label, 100, 36);

    /* preheat_on_button: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->preheat_on_button, 983, 144);
    lv_obj_set_size(pg->preheat_on_button, 135, 53);

    /* delay_on_button: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: switchbg30.png */
    lv_obj_set_pos(pg->delay_on_button, 983, 260);
    lv_obj_set_size(pg->delay_on_button, 135, 53);

    /* contain_on_button: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->contain_on_button, 983, 372);
    lv_obj_set_size(pg->contain_on_button, 135, 53);

    /* up3_icon_label: 标签 | "℃" | (432,159) | 26x28 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->up3_icon_label, 361, 159);
    lv_obj_set_size(pg->up3_icon_label, 50, 28);

    /* down2_icon_label: 标签 | "℃" | (590,159) | 26x28 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->down2_icon_label, 518, 159);
    lv_obj_set_size(pg->down2_icon_label, 50, 28);

    /* down2_tempnum_label: 标签 | "80" | (504,143) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->down2_tempnum_label, 452, 147);
    lv_obj_set_size(pg->down2_tempnum_label, 83, 53);

    /* down2_dir_label: 标签 | "↓" | (510,159) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->down2_dir_label, 455, 159);
    lv_obj_set_size(pg->down2_dir_label, 26, 32);

    /* up3_dir_label: 标签 | "↑" | (325,159) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->up3_dir_label, 273, 159);
    lv_obj_set_size(pg->up3_dir_label, 26, 32);

    /* up3_tempnum_label: 标签 | "100" | (342,143) | 87x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->up3_tempnum_label, 291, 147);
    lv_obj_set_size(pg->up3_tempnum_label, 87, 53);

}


/* ==============================================================================
 * updown_bbq_set_probe 英文布局基准（对应 PAGE_UPDOWN_BBQ_SET_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_set_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_SET_PROBE) return;
    updown_bbq_set_probe_t *pg = updown_bbq_set_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_19: 标签 | "上下烧烤" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_19, 24, 25);
    lv_obj_set_size(pg->label_19, 144, 32);

    /* label_20: 标签 | "温" | (165,155) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_20, 165, 155);
    lv_obj_set_size(pg->label_20, 26, 32);

    /* label_21: 标签 | "度：" | (252,155) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_21, 252, 155);
    lv_obj_set_size(pg->label_21, 64, 32);

    /* label_22: 标签 | "探针温度：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_22, 141, 270);
    lv_obj_set_size(pg->label_22, 175, 32);

    /* image_11: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_11, 164, 376);

    /* label_27: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_27, 232, 385);
    lv_obj_set_size(pg->label_27, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_38: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_38, 1010, 39);
    lv_obj_set_size(pg->label_38, 85, 36);

    /* probetemp: 标签 | "80" | (315,254) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->probetemp, 315, 254);
    lv_obj_set_size(pg->probetemp, 83, 53);

    /* probeicon2: 标签 | "℃" | (369,270) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->probeicon2, 369, 270);
    lv_obj_set_size(pg->probeicon2, 32, 30);

    /* label_1: 标签 | "迟" | (930,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_1, 930, 154);
    lv_obj_set_size(pg->label_1, 26, 32);

    /* label_2: 标签 | "延" | (840,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_2, 840, 154);
    lv_obj_set_size(pg->label_2, 26, 32);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* image_2: 图片 | (839,154) | img: delaytext.png */
    lv_obj_set_pos(pg->image_2, 839, 154);

    /* temp: 标签 | "200" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

}


/* ==============================================================================
 * updown_bbq_setting 英文布局基准（对应 PAGE_UPDOWN_BBQ_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_SETTING) return;
    updown_bbq_setting_t *pg = updown_bbq_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* time_label: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->time_label, 907, 157);
    lv_obj_set_size(pg->time_label, 250, 60);

    /* container_3: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_3, 0, 0);
    lv_obj_set_size(pg->container_3, 1280, 480);

    /* settingbk_img: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->settingbk_img, 115, 115);

    /* temp_label: 标签 | "温度" | (255,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->temp_label, 226, 136);
    lv_obj_set_size(pg->temp_label, 100, 30);

    /* cook_label: 标签 | "烹饪时间" | (523,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->cook_label, 467, 136);
    lv_obj_set_size(pg->cook_label, 250, 30);

    /* dirup3_label: 标签 | "↑" | (147,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->dirup3_label, 140, 229);
    lv_obj_set_size(pg->dirup3_label, 37, 30);

    /* icon3_label1: 标签 | "℃" | (248,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3_label1, 248, 229);
    lv_obj_set_size(pg->icon3_label1, 37, 30);

    /* tempup_label: 标签 | "80" | (161,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->tempup_label, 155, 217);
    lv_obj_set_size(pg->tempup_label, 102, 46);

    /* longup_templine_img: 图片 | (149,269) | img: settingline1.png */
    lv_obj_set_pos(pg->longup_templine_img, 149, 269);

    /* tempdown_label: 标签 | "180" | (311,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->tempdown_label, 306, 217);
    lv_obj_set_size(pg->tempdown_label, 102, 46);

    /* dirdown3_label: 标签 | "↓" | (297,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->dirdown3_label, 292, 229);
    lv_obj_set_size(pg->dirdown3_label, 37, 30);

    /* icon3_label2: 标签 | "℃" | (398,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3_label2, 398, 229);
    lv_obj_set_size(pg->icon3_label2, 37, 30);

    /* longdown_templine_img: 图片 | (299,269) | img: settingline1.png */
    lv_obj_set_pos(pg->longdown_templine_img, 299, 269);

    /* hour_label: 标签 | "00" | (450,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour_label, 451, 217);
    lv_obj_set_size(pg->hour_label, 102, 46);

    /* min_label: 标签 | "00" | (599,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min_label, 102, 46);

    /* shi_label: 标签 | "时" | (524,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->shi_label, 524, 229);
    lv_obj_set_size(pg->shi_label, 37, 30);

    /* fen_label: 标签 | "分" | (674,229) | 37x30 | font taiwanpearl_regular_30 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->fen_label, 37, 30);

    /* hourline_img: 图片 | (473,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline_img, 473, 269);

    /* minline_label: 图片 | (622,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline_label, 622, 269);

    /* icon2_label1: 标签 | "℃" | (236,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2_label1, 236, 229);
    lv_obj_set_size(pg->icon2_label1, 37, 30);

    /* dirup2_label: 标签 | "↑" | (161,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->dirup2_label, 161, 229);
    lv_obj_set_size(pg->dirup2_label, 37, 30);

    /* shortup_templine_img: 图片 | (156,269) | img: settingline2.png */
    lv_obj_set_pos(pg->shortup_templine_img, 156, 269);

    /* shordown_templine_img: 图片 | (306,269) | img: settingline2.png */
    lv_obj_set_pos(pg->shordown_templine_img, 306, 269);

    /* dirdown2_label: 标签 | "↓" | (311,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->dirdown2_label, 311, 229);
    lv_obj_set_size(pg->dirdown2_label, 37, 30);

    /* icon2_label2: 标签 | "℃" | (386,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2_label2, 386, 229);
    lv_obj_set_size(pg->icon2_label2, 37, 30);

    /* sure_button: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure_button, 959, 295);
    lv_obj_set_size(pg->sure_button, 129, 83);

}


/* ==============================================================================
 * updown_bbq_stop 英文布局基准（对应 PAGE_UPDOWN_BBQ_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_STOP) return;
    updown_bbq_stop_t *pg = updown_bbq_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* statu_label: 标签 | "| 上下烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->statu_label, 274, 232);
    lv_obj_set_size(pg->statu_label, 490, 39);

    /* time_label: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->time_label, 907, 157);
    lv_obj_set_size(pg->time_label, 250, 60);

    /* start_button: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start_button, 959, 295);
    lv_obj_set_size(pg->start_button, 129, 83);

    /* image_2: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_2, 115, 320);

    /* bar_1: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_1, 122, 326);
    lv_obj_set_size(pg->bar_1, 637, 20);

    /* image_3: 图片 | (115,161) | img: updown_img.png */
    lv_obj_set_pos(pg->image_3, 115, 161);

    /* label_4: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_4, 273, 157);
    lv_obj_set_size(pg->label_4, 300, 60);

    /* little_button: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little_button, 609, 170);
    lv_obj_set_size(pg->little_button, 50, 43);

}


/* ==============================================================================
 * updown_bbq_stop_back 英文布局基准（对应 PAGE_UPDOWN_BBQ_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_STOP_BACK) return;
    updown_bbq_stop_back_t *pg = updown_bbq_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* ---- 场景分支: 本页被两处复用 ----
       原生:   上下烧烤流程停止/暂停返回(status "| 上下烧烤 | 180℃ | 时间 |")
       六感:   六感预约取消(delay_cancel_to_stop_back, g_delay_source_page==PAGE_DESCRIPTIONMENU)
               status "| 菜名 | 成熟度/颜色 |"(由 nav_stop.c 六感分支设置) ---- */
    /* statu_label: 标签 | "| 上下烧烤 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->statu_label, 274, 232);
    if (g_delay_source_page == PAGE_DESCRIPTIONMENU) {
        /* 六感场景: 文本格式不同 */
        lv_obj_set_size(pg->statu_label, 490, 39);
        /* TODO: 英文实测后调整(如 "Roast Leg of Lamb | Mid | Medium Well" 宽度) */
    } else {
        lv_obj_set_size(pg->statu_label, 490, 39);
    }

    /* image_6: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_6, 115, 320);

    /* bar_2: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_2, 122, 326);
    lv_obj_set_size(pg->bar_2, 637, 20);

    /* image_7: 图片 | (115,161) | img: updown_img.png | 场景条件定位(业务分支设置, tune不设) */
    /* 位置由业务场景分支控制, 微调按文件头模板 */

    /* label_8: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_8, 273, 157);
    lv_obj_set_size(pg->label_8, 330, 60);

    /* littal_button: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->littal_button, 609, 170);
    lv_obj_set_size(pg->littal_button, 50, 43);

    /* container_1: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_1, 0, 0);
    lv_obj_set_size(pg->container_1, 1280, 480);

    /* label_10: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_10, 895, 160);
    lv_obj_set_size(pg->label_10, 252, 36);

    /* label_11: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_11, 895, 198);
    lv_obj_set_size(pg->label_11, 252, 36);

    /* sure_button: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure_button, 959, 295);
    lv_obj_set_size(pg->sure_button, 129, 83);

}


/* ==============================================================================
 * updown_bbq_stop_back_probe 英文布局基准（对应 PAGE_UPDOWN_BBQ_STOP_BACK_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_stop_back_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_STOP_BACK_PROBE) return;
    updown_bbq_stop_back_probe_t *pg = updown_bbq_stop_back_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 上下烧烤 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_28: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_28, 115, 320);

    /* bar_3: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_3, 122, 326);
    lv_obj_set_size(pg->bar_3, 637, 20);

    /* image_29: 图片 | (115,161) | img: updown_img.png */
    lv_obj_set_pos(pg->image_29, 115, 161);

    /* label_71: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_71, 273, 157);
    lv_obj_set_size(pg->label_71, 235, 60);

    /* container_3: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_3, 0, 0);
    lv_obj_set_size(pg->container_3, 1280, 480);

    /* label_72: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_72, 895, 160);
    lv_obj_set_size(pg->label_72, 252, 36);

    /* label_73: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_73, 895, 198);
    lv_obj_set_size(pg->label_73, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * updown_bbq_stop_probe 英文布局基准（对应 PAGE_UPDOWN_BBQ_STOP_PROBE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void updown_bbq_stop_probe_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_UPDOWN_BBQ_STOP_PROBE) return;
    updown_bbq_stop_probe_t *pg = updown_bbq_stop_probe_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 上下烧烤 | 180℃ | 80℃" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* temp: 标签 | "36℃" | (919,156) | 134x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->temp, 919, 156);
    lv_obj_set_size(pg->temp, 134, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_25: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_25, 115, 320);

    /* bar_2: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_2, 122, 326);
    lv_obj_set_size(pg->bar_2, 637, 20);

    /* image_26: 图片 | (115,161) | img: updown_img.png */
    lv_obj_set_pos(pg->image_26, 115, 161);

    /* label_69: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_69, 273, 157);
    lv_obj_set_size(pg->label_69, 235, 60);

    /* image_27: 图片 | (1068,180) | img: probeicon.png */
    lv_obj_set_pos(pg->image_27, 1068, 180);

}


/* ==============================================================================
 * vegetablemenu 英文布局基准（对应 PAGE_VEGETABLEMENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void vegetablemenu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_VEGETABLEMENU) return;
    vegetablemenu_t *pg = vegetablemenu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0x000000 | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* bt1: 按钮 | (18,76) | 313x386 | font taiwanpearl_regular_30 | bg: div7_1_30.png */
    lv_obj_set_pos(pg->bt1, 18, 76);
    lv_obj_set_size(pg->bt1, 313, 386);

    /* button_11: 按钮 | (330,76) | 310x193 | font taiwanpearl_regular_30 | bg: div7_2_30.png */
    lv_obj_set_pos(pg->button_11, 330, 76);
    lv_obj_set_size(pg->button_11, 310, 193);

    /* label_7: 标签 | "蔬菜" | (24,24) | 97x25 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_7, 24, 24);
    lv_obj_set_size(pg->label_7, 97, 25);

    /* button_15: 按钮 | (640,76) | 310x193 | font taiwanpearl_regular_30 | bg: div7_2_30.png */
    lv_obj_set_pos(pg->button_15, 640, 76);
    lv_obj_set_size(pg->button_15, 310, 193);

    /* button_16: 按钮 | (950,76) | 310x193 | font taiwanpearl_regular_30 | bg: div7_2_30.png */
    lv_obj_set_pos(pg->button_16, 950, 76);
    lv_obj_set_size(pg->button_16, 310, 193);

    /* button_17: 按钮 | (330,269) | 310x193 | font taiwanpearl_regular_30 | bg: div7_2_30.png */
    lv_obj_set_pos(pg->button_17, 330, 269);
    lv_obj_set_size(pg->button_17, 310, 193);

    /* button_18: 按钮 | (640,269) | 310x193 | font taiwanpearl_regular_30 | bg: div7_2_30.png */
    lv_obj_set_pos(pg->button_18, 640, 269);
    lv_obj_set_size(pg->button_18, 310, 193);

    /* button_19: 按钮 | (950,269) | 310x193 | font taiwanpearl_regular_30 | bg: div7_2_30.png */
    lv_obj_set_pos(pg->button_19, 950, 269);
    lv_obj_set_size(pg->button_19, 310, 193);

     /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     lv_obj_t *obj = NULL;
     obj = pg->label_7;
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}


/* ==============================================================================
 * waitmenu_24 英文布局基准（对应 PAGE_WAITMENU_24 ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void waitmenu_24_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WAITMENU_24) return;
    waitmenu_24_t *pg = waitmenu_24_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0x000000 | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* time_label: 标签 | "12:00" | (421,123) | 438x127 | font taiwanpearl_regular_128 */
    lv_obj_set_pos(pg->time_label, 421, 123);
    lv_obj_set_size(pg->time_label, 438, 127);

    /* week_label: 标签 | "星期一二三四五六日, 2025年12月15日" | (373,259) | 539x51 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->week_label, 373, 259);
    lv_obj_set_size(pg->week_label, 539, 51);

    /* label_13: 标签 | "Label" | (1096,40) | 100x32 | font montserratmedium_16 */
    lv_obj_set_pos(pg->label_13, 1096, 40);
    lv_obj_set_size(pg->label_13, 100, 32);

     /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
     lv_obj_t *obj = lv_screen_active();
     //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_standby_24_En.png), 0);
 
     obj = pg->time_label;
     // lv_obj_set_style_text_font(obj, c_)
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
     lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
 
     obj = pg->week_label;
     lv_obj_set_style_text_font(obj, &c_aktivgroteskmedium_48, 0);
     lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
     lv_obj_align(obj, LV_ALIGN_CENTER, 0, 47);
     lv_label_set_text(obj, "Sunday, June 1st, 2025");
}


/* ==============================================================================
 * waterclean_complete 英文布局基准（对应 PAGE_WATER_CLEAN_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void waterclean_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WATER_CLEAN_COMPLETE) return;
    waterclean_complete_t *pg = waterclean_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_9: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_9, 115, 320);

    /* label_12: 标签 | "| 水解自清洁 | 10分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_12, 274, 232);
    lv_obj_set_size(pg->label_12, 490, 39);

    /* bar_4: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_4, 122, 326);
    lv_obj_set_size(pg->bar_4, 637, 20);

    /* image_10: 图片 | (167,161) | img: watercleanicon.png */
    lv_obj_set_pos(pg->image_10, 167, 161);

    /* label_13: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_13, 273, 157);
    lv_obj_set_size(pg->label_13, 235, 60);

    /* image_12: 图片 | (902,160) | img: tips.png */
    lv_obj_set_pos(pg->image_12, 902, 160);

}


/* ==============================================================================
 * waterclean_cooking 英文布局基准（对应 PAGE_WATER_CLEAN_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void waterclean_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WATER_CLEAN_COOKING) return;
    waterclean_cooking_t *pg = waterclean_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_6: 标签 | "| 水解自清洁 | 10分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_6, 274, 232);
    lv_obj_set_size(pg->label_6, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_5: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_5, 115, 320);

    /* bar_2: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_2, 122, 326);
    lv_obj_set_size(pg->bar_2, 637, 20);

    /* image_6: 图片 | (167,161) | img: watercleanicon.png */
    lv_obj_set_pos(pg->image_6, 167, 161);

    /* label_8: 标签 | "清洁中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_8, 273, 157);
    lv_obj_set_size(pg->label_8, 235, 60);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_8;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->label_6;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->timelabel;
    lv_obj_set_pos(obj, 902, 162);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->stop;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * waterclean_set 英文布局基准（对应 PAGE_WATER_CLEAN_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void waterclean_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WATER_CLEAN_SET) return;
    waterclean_set_t *pg = waterclean_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_1: 图片 | (117,155) | img: waterbg.png */
    lv_obj_set_pos(pg->image_1, 117, 155);

    /* image_2: 图片 | (167,161) | img: watercleanicon.png */
    lv_obj_set_pos(pg->image_2, 167, 161);

}


/* ==============================================================================
 * waterclean_stop 英文布局基准（对应 PAGE_WATER_CLEAN_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void waterclean_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WATER_CLEAN_STOP) return;
    waterclean_stop_t *pg = waterclean_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_9: 标签 | "| 水解自清洁 | 10分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_9, 274, 232);
    lv_obj_set_size(pg->label_9, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_7: 图片 | (115,320) | img: bar.png | 场景条件定位(业务分支设置, tune不设) */
    /* 位置由业务场景分支控制, 微调按文件头模板 */

    /* bar_3: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_3, 122, 326);
    lv_obj_set_size(pg->bar_3, 637, 20);

    /* image_8: 图片 | (167,161) | img: watercleanicon.png */
    lv_obj_set_pos(pg->image_8, 167, 161);

    /* label_11: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_11, 273, 157);
    lv_obj_set_size(pg->label_11, 235, 60);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_11;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->label_9;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->timelabel;
    lv_obj_set_pos(obj, 902, 162);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->start;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * waterclean_stop_back 英文布局基准（对应 PAGE_WATER_CLEAN_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void waterclean_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WATER_CLEAN_STOP_BACK) return;
    waterclean_stop_back_t *pg = waterclean_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 水解自清洁 | 10分钟 |" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_1: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_1, 115, 320);

    /* bar_1: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_1, 122, 326);
    lv_obj_set_size(pg->bar_1, 637, 20);

    /* image_2: 图片 | (167,161) | img: watercleanicon.png */
    lv_obj_set_pos(pg->image_2, 167, 161);

    /* label_6: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_6, 273, 157);
    lv_obj_set_size(pg->label_6, 235, 60);

    /* container_1: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_1, 0, 0);
    lv_obj_set_size(pg->container_1, 1280, 480);

    /* label_8: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_8, 895, 198);
    lv_obj_set_size(pg->label_8, 252, 36);

    /* label_7: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_7, 895, 160);
    lv_obj_set_size(pg->label_7, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Eng modify ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    lv_obj_t *obj = lv_screen_active();
    //lv_obj_set_style_bg_image_src(obj, LVGL_IMAGE_PATH(sku3_bg_work_cnt_En.png), 0);

    obj = pg->label_6;
    lv_obj_set_pos(obj, 272, 161);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->status;
    lv_obj_set_pos(obj, 275, 232);
    lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    obj = pg->sure;
    lv_obj_set_pos(obj, 958, 293);
}


/* ==============================================================================
 * west_complete 英文布局基准（对应 PAGE_WEST_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void west_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WEST_COMPLETE) return;
    west_complete_t *pg = west_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_90: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_90, 115, 320);

    /* status: 标签 | "| 西式塔 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_11: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_11, 122, 326);
    lv_obj_set_size(pg->bar_11, 637, 20);

    /* label_192: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_192, 273, 157);
    lv_obj_set_size(pg->label_192, 235, 60);

    /* image_91: 图片 | (845,160) | img: hotcare.png */
    lv_obj_set_pos(pg->image_91, 845, 160);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_92: 图片 | (115,161) | img: westicon.png */
    lv_obj_set_pos(pg->image_92, 115, 161);

}


/* ==============================================================================
 * west_cooking 英文布局基准（对应 PAGE_WEST_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void west_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WEST_COOKING) return;
    west_cooking_t *pg = west_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 西式塔 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_79: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_79, 115, 320);

    /* bar_8: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_8, 122, 326);
    lv_obj_set_size(pg->bar_8, 637, 20);

    /* label_173: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_173, 273, 157);
    lv_obj_set_size(pg->label_173, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_80: 图片 | (115,161) | img: westicon.png */
    lv_obj_set_pos(pg->image_80, 115, 161);

}


/* ==============================================================================
 * west_menu 英文布局基准（对应 PAGE_WEST_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void west_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WEST_MENU) return;
    west_menu_t *pg = west_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_72: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_72, 24, 118);

    /* image_73: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_73, 645, 118);

    /* label_144: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_144, 300, 137);
    lv_obj_set_size(pg->label_144, 64, 32);

    /* temp: 标签 | "170" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 252, 249);
    lv_obj_set_size(pg->temp, 120, 72);

    /* label_146: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_146, 372, 281);
    lv_obj_set_size(pg->label_146, 38, 32);

    /* label_147: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_147, 891, 137);
    lv_obj_set_size(pg->label_147, 125, 32);

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 793, 248);
    lv_obj_set_size(pg->hour, 84, 72);

    /* label_149: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_149, 872, 280);
    lv_obj_set_size(pg->label_149, 38, 32);

    /* label_150: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_150, 1075, 281);
    lv_obj_set_size(pg->label_150, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 84, 72);

    /* label_152: 标签 | "西式塔" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_152, 24, 25);
    lv_obj_set_size(pg->label_152, 144, 32);

    /* label_153: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_153, 996, 39);
    lv_obj_set_size(pg->label_153, 117, 32);

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * west_set 英文布局基准（对应 PAGE_WEST_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void west_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WEST_SET) return;
    west_set_t *pg = west_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_154: 标签 | "西式塔" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_154, 24, 25);
    lv_obj_set_size(pg->label_154, 144, 32);

    /* label_155: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_155, 165, 157);
    lv_obj_set_size(pg->label_155, 26, 32);

    /* label_156: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_156, 252, 157);
    lv_obj_set_size(pg->label_156, 64, 32);

    /* label_157: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_157, 141, 270);
    lv_obj_set_size(pg->label_157, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 58, 53);

    /* label_160: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_160, 368, 270);
    lv_obj_set_size(pg->label_160, 30, 32);

    /* label_161: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->label_161, 64, 32);

    /* image_78: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_78, 164, 376);

    /* label_162: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_162, 232, 385);
    lv_obj_set_size(pg->label_162, 148, 32);

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_164: 标签 | "延" | (840,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_164, 840, 154);
    lv_obj_set_size(pg->label_164, 26, 32);

    /* label_165: 标签 | "迟" | (930,154) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_165, 930, 154);
    lv_obj_set_size(pg->label_165, 26, 32);

    /* label_166: 标签 | "自动保温" | (831,270) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_166, 831, 270);
    lv_obj_set_size(pg->label_166, 130, 32);

    /* offdelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 144);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 260);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "170" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 315, 141);
    lv_obj_set_size(pg->temp, 83, 53);

    /* label_168: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_168, 1010, 39);
    lv_obj_set_size(pg->label_168, 85, 36);

    /* ondelay: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 144);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 260);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 396, 157);
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 369, 157);
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * west_setting 英文布局基准（对应 PAGE_WEST_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void west_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WEST_SETTING) return;
    west_setting_t *pg = west_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* container_9: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_9, 0, 0);
    lv_obj_set_size(pg->container_9, 1280, 480);

    /* image_81: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_81, 115, 115);

    /* label_175: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_175, 242, 136);
    lv_obj_set_size(pg->label_175, 73, 30);

    /* label_176: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_176, 454, 136);
    lv_obj_set_size(pg->label_176, 135, 30);

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 213);
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 300, 229);
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 374, 213);
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_181: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_181, 449, 229);
    lv_obj_set_size(pg->label_181, 37, 30);

    /* label_182: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_182, 612, 229);
    lv_obj_set_size(pg->label_182, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 288, 229);
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * west_stop 英文布局基准（对应 PAGE_WEST_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void west_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WEST_STOP) return;
    west_stop_t *pg = west_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 西式塔 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 235, 60);

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_86: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_86, 115, 320);

    /* bar_9: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_9, 122, 326);
    lv_obj_set_size(pg->bar_9, 637, 20);

    /* label_186: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_186, 273, 157);
    lv_obj_set_size(pg->label_186, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_87: 图片 | (115,161) | img: westicon.png */
    lv_obj_set_pos(pg->image_87, 115, 161);

}


/* ==============================================================================
 * west_stop_back 英文布局基准（对应 PAGE_WEST_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void west_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WEST_STOP_BACK) return;
    west_stop_back_t *pg = west_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_88: 图片 | (115,161) | img: westicon.png */
    lv_obj_set_pos(pg->image_88, 115, 161);

    /* status: 标签 | "| 西式塔 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_89: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_89, 115, 320);

    /* bar_10: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_10, 122, 326);
    lv_obj_set_size(pg->bar_10, 637, 20);

    /* label_188: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_188, 273, 157);
    lv_obj_set_size(pg->label_188, 235, 60);

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* container_10: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_10, 0, 0);
    lv_obj_set_size(pg->container_10, 1280, 480);

    /* label_189: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_189, 895, 160);
    lv_obj_set_size(pg->label_189, 252, 36);

    /* label_190: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_190, 895, 198);
    lv_obj_set_size(pg->label_190, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * windchange_bbq_complete 英文布局基准（对应 PAGE_WINDCHANGE_BBQ_COMPLETE ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void windchange_bbq_complete_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WINDCHANGE_BBQ_COMPLETE) return;
    windchange_bbq_complete_t *pg = windchange_bbq_complete_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_159: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_159, 115, 320);

    /* status: 标签 | "| 热风对流 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* bar_33: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_33, 122, 326);
    lv_obj_set_size(pg->bar_33, 637, 20);

    /* label_356: 标签 | "已完成" | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_356, 273, 157);
    lv_obj_set_size(pg->label_356, 330, 60);   /* 对齐 updown bbq 系 */

    /* image_160: 图片 | (845,160) | img: hotcare.png */
    lv_obj_set_pos(pg->image_160, 845, 160);

    /* button_119: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->button_119, 609, 170);
    lv_obj_set_size(pg->button_119, 50, 43);

    /* image_161: 图片 | (115,161) | img: windchange.png */
    lv_obj_set_pos(pg->image_161, 115, 161);

}


/* ==============================================================================
 * windchange_bbq_cooking 英文布局基准（对应 PAGE_WINDCHANGE_BBQ_COOKING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void windchange_bbq_cooking_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WINDCHANGE_BBQ_COOKING) return;
    windchange_bbq_cooking_t *pg = windchange_bbq_cooking_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热风对流 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* stop: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->stop, 959, 295);
    lv_obj_set_size(pg->stop, 129, 83);

    /* image_153: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_153, 115, 320);

    /* bar_30: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_30, 122, 326);
    lv_obj_set_size(pg->bar_30, 637, 20);

    /* label_347: 标签 | "烹饪中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_347, 273, 157);
    lv_obj_set_size(pg->label_347, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_154: 图片 | (115,161) | img: windchange.png */
    lv_obj_set_pos(pg->image_154, 115, 161);

}


/* ==============================================================================
 * windchange_bbq_menu 英文布局基准（对应 PAGE_WINDCHANGE_BBQ_MENU ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void windchange_bbq_menu_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WINDCHANGE_BBQ_MENU) return;
    windchange_bbq_menu_t *pg = windchange_bbq_menu_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* next: 按钮 | (981,22) | 138x70 | font montserratmedium_16 | bg: nextbk.png */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* image_146: 图片 | (24,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_146, 24, 118);

    /* image_147: 图片 | (645,118) | img: div2bk.png */
    lv_obj_set_pos(pg->image_147, 645, 118);

    /* label_319: 标签 | "温度" | (300,137) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_319, 275, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_319, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* temp: 标签 | "180" | (252,249) | 120x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->temp, 192, 249);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->temp, 180, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_321: 标签 | "℃" | (372,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_321, 372, 281);
    lv_obj_set_size(pg->label_321, 38, 32);

    /* label_322: 标签 | "烹饪时间" | (891,137) | 125x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_322, 855, 137);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_322, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* hour: 标签 | "00" | (793,248) | 84x72 | font taiwanpearl_regular_72 */
    lv_obj_set_pos(pg->hour, 788, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->hour, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_324: 标签 | "时" | (872,280) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_324, 872, 280);
    lv_obj_set_size(pg->label_324, 38, 32);

    /* label_325: 标签 | "分" | (1075,281) | 38x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_325, 1075, 281);
    lv_obj_set_size(pg->label_325, 38, 32);

    /* min: 标签 | "30" | (995,248) | 84x72 | font taiwanpearl_regular_72 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_pos(pg->min, 990, 248);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->min, 100, 72);   /* 对齐 updown 系英文排版(A) */

    /* label_327: 标签 | "热风对流" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_327, 24, 24);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_327, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_328: 标签 | "下一步" | (996,39) | 117x32 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_328, 1007, 41);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_328, 117, 32);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */

    /* templine3: 图片 | (252,328) | img: underline.png */
    lv_obj_set_pos(pg->templine3, 252, 328);

    /* templine2: 图片 | (289,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->templine2, 289, 328);

    /* hourline: 图片 | (790,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->hourline, 790, 328);

    /* minline: 图片 | (993,328) | img: underline_120x4.png */
    lv_obj_set_pos(pg->minline, 993, 328);

}


/* ==============================================================================
 * windchange_bbq_set 英文布局基准（对应 PAGE_WINDCHANGE_BBQ_SET ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void windchange_bbq_set_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WINDCHANGE_BBQ_SET) return;
    windchange_bbq_set_t *pg = windchange_bbq_set_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* label_329: 标签 | "热风对流" | (24,25) | 144x32 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->label_329, 24, 25);
    lv_obj_set_size(pg->label_329, 160, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_330: 标签 | "温" | (165,157) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_330, 163, 155);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_330, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_331: 标签 | "度：" | (252,157) | 64x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_331, 252, 157);
    lv_obj_set_size(pg->label_331, 64, 32);

    /* label_332: 标签 | "烹饪时间：" | (141,270) | 175x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_332, 150, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_332, 175, 32);

    /* hour: 标签 | "01" | (307,254) | 62x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 307, 254);
    lv_obj_set_size(pg->hour, 62, 53);

    /* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->min, 317, 258);
    lv_obj_set_size(pg->min, 68, 66);   /* 对齐 updown 系英文排版(A) */

    /* label_335: 标签 | "时" | (368,270) | 30x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_335, 368, 270);
    lv_obj_set_size(pg->label_335, 30, 32);

    /* label_336: 标签 | "分钟" | (448,269) | 64x32 | font taiwanpearl_regular_30 | 动态定位(默认业务值, 直接改数字) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    // lv_obj_set_pos(pg->label_336, 371, 271);
    lv_obj_set_size(pg->label_336, 42, 32);   /* 对齐 updown 系英文排版(A) */

    /* image_152: 图片 | (164,376) | img: place_1.png */
    lv_obj_set_pos(pg->image_152, 164, 376);

    /* label_337: 标签 | "烤架位置 3" | (232,385) | 148x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_337, 232, 385);
    lv_obj_set_size(pg->label_337, 220, 32);   /* 对齐 updown 系英文排版(A) */

    /* sure: 按钮 | (983,22) | 135x71 | font taiwanpearl_regular_36 | bg: surebk.png */
    lv_obj_set_pos(pg->sure, 983, 22);
    lv_obj_set_size(pg->sure, 135, 71);

    /* label_338: 标签 | "快速预热" | (830,154) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_338, 785, 154);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_338, 180, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_339: 标签 | "延" | (840,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_339, 885, 270);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_339, 100, 32);   /* 对齐 updown 系英文排版(A) */

    /* label_340: 标签 | "迟" | (930,270) | 26x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_340, 930, 270);
    lv_obj_set_size(pg->label_340, 26, 32);

    /* label_341: 标签 | "自动保温" | (831,381) | 130x32 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_341, 765, 381);   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_341, 200, 32);   /* 对齐 updown 系英文排版(A) */

    /* offpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offpreheat, 983, 144);
    lv_obj_set_size(pg->offpreheat, 135, 53);

    /* offdelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offdelay, 983, 260);
    lv_obj_set_size(pg->offdelay, 135, 53);

    /* offcontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: off.png */
    lv_obj_set_pos(pg->offcontain, 983, 372);
    lv_obj_set_size(pg->offcontain, 135, 53);

    /* temp: 标签 | "180" | (315,141) | 83x53 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 269, 143);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->temp, 100, 53);   /* 对齐 top_bbq 系排版 */

    /* label_343: 标签 | "确 定" | (1010,39) | 85x36 | font taiwanpearl_regular_36 */
    lv_obj_set_pos(pg->label_343, 1007, 41);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */
    lv_obj_set_size(pg->label_343, 85, 36);   /* 对齐 top_bbq 系排版 */   /* 对齐 updown 系英文排版(A) */

    /* onpreheat: 按钮 | (983,144) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->onpreheat, 983, 144);
    lv_obj_set_size(pg->onpreheat, 135, 53);

    /* ondelay: 按钮 | (983,260) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->ondelay, 983, 260);
    lv_obj_set_size(pg->ondelay, 135, 53);

    /* oncontain: 按钮 | (983,372) | 135x53 | font montserratmedium_16 | bg: on1.png */
    lv_obj_set_pos(pg->oncontain, 983, 372);
    lv_obj_set_size(pg->oncontain, 135, 53);

    /* icon3: 标签 | "℃" | (396,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 355, 157);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon3, 32, 30);

    /* icon2: 标签 | "℃" | (369,157) | 32x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 327, 157);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon2, 32, 30);

}


/* ==============================================================================
 * windchange_bbq_setting 英文布局基准（对应 PAGE_WINDCHANGE_BBQ_SETTING ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void windchange_bbq_setting_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WINDCHANGE_BBQ_SETTING) return;
    windchange_bbq_setting_t *pg = windchange_bbq_setting_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 257, 60);   /* 对齐 top_bbq 系排版 */

    /* container_4: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_4, 0, 0);
    lv_obj_set_size(pg->container_4, 1280, 480);

    /* image_16: 图片 | (115,115) | img: settingbk.png */
    lv_obj_set_pos(pg->image_16, 115, 115);

    /* label_32: 标签 | "温度" | (242,136) | 73x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_32, 203, 134);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_32, 150, 30);   /* 对齐 top_bbq 系排版 */

    /* label_33: 标签 | "烹饪时间" | (454,136) | 135x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_33, 444, 134);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_33, 150, 30);   /* 对齐 top_bbq 系排版 */

    /* temp: 标签 | "180" | (213,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->temp, 213, 217);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->temp, 102, 46);

    /* icon3: 标签 | "℃" | (300,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon3, 303, 229);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon3, 37, 30);

    /* templine2: 图片 | (236,269) | img: settingline3_87x4.png */
    lv_obj_set_pos(pg->templine2, 236, 269);

    /* templine3: 图片 | (223,269) | img: settingline2.png */
    lv_obj_set_pos(pg->templine3, 223, 269);

    /* hour: 标签 | "00" | (374,213) | 102x46 | font taiwanpearl_regular_48 */
    lv_obj_set_pos(pg->hour, 377, 217);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->hour, 102, 46);

    /* min: 标签 | "00" | (537,213) | 102x46 | font taiwanpearl_regular_48 | 动态定位(需微调见文件头模板) */
    /* 位置由业务动态控制, 微调按文件头模板 */
    lv_obj_set_size(pg->min, 102, 46);

    /* label_38: 标签 | "时" | (449,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_38, 450, 231);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_38, 37, 30);

    /* label_39: 标签 | "分" | (612,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_39, 612, 231);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->label_39, 37, 30);

    /* hourline: 图片 | (398,269) | img: settingline3.png */
    lv_obj_set_pos(pg->hourline, 398, 269);

    /* minline: 图片 | (560,269) | img: settingline3.png */
    lv_obj_set_pos(pg->minline, 560, 269);

    /* icon2: 标签 | "℃" | (288,229) | 37x30 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->icon2, 289, 229);   /* 对齐 top_bbq 系排版 */
    lv_obj_set_size(pg->icon2, 37, 30);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}


/* ==============================================================================
 * windchange_bbq_stop 英文布局基准（对应 PAGE_WINDCHANGE_BBQ_STOP ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void windchange_bbq_stop_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WINDCHANGE_BBQ_STOP) return;
    windchange_bbq_stop_t *pg = windchange_bbq_stop_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* status: 标签 | "| 热风对流 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* timelabel: 标签 | "00:19:59" | (907,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->timelabel, 907, 157);
    lv_obj_set_size(pg->timelabel, 250, 60);   /* 对齐 updown 系英文排版(A) */

    /* start: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->start, 959, 295);
    lv_obj_set_size(pg->start, 129, 83);

    /* image_155: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_155, 115, 320);

    /* bar_31: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_31, 122, 326);
    lv_obj_set_size(pg->bar_31, 637, 20);

    /* label_350: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_350, 273, 157);
    lv_obj_set_size(pg->label_350, 300, 60);   /* 对齐 updown 系英文排版(A) */

    /* little: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->little, 609, 170);
    lv_obj_set_size(pg->little, 50, 43);

    /* image_156: 图片 | (115,161) | img: windchange.png */
    lv_obj_set_pos(pg->image_156, 115, 161);

}


/* ==============================================================================
 * windchange_bbq_stop_back 英文布局基准（对应 PAGE_WINDCHANGE_BBQ_STOP_BACK ）
 * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）
 * ============================================================================== */
void windchange_bbq_stop_back_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_WINDCHANGE_BBQ_STOP_BACK) return;
    windchange_bbq_stop_back_t *pg = windchange_bbq_stop_back_get(&ui_manager);
    if (!pg) return;

    /* 页面背景: 背景图 bg.jpg | 底色 0xfcfcfc | opa 255（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */

    /* image_157: 图片 | (115,161) | img: windchange.png */
    lv_obj_set_pos(pg->image_157, 115, 161);

    /* status: 标签 | "| 热风对流 | 180℃ | 1小时20分钟" | (274,232) | 490x39 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->status, 274, 232);
    lv_obj_set_size(pg->status, 490, 39);

    /* image_158: 图片 | (115,320) | img: bar.png */
    lv_obj_set_pos(pg->image_158, 115, 320);

    /* bar_32: 进度条 | (122,326) | 637x20 */
    lv_obj_set_pos(pg->bar_32, 122, 326);
    lv_obj_set_size(pg->bar_32, 637, 20);

    /* label_352: 标签 | "暂停中..." | (273,157) | 235x60 | font taiwanpearl_regular_60 */
    lv_obj_set_pos(pg->label_352, 273, 157);
    lv_obj_set_size(pg->label_352, 330, 60);   /* 对齐 updown bbq 系 */   /* 对齐 updown 系英文排版(A) */

    /* button_117: 按钮 | (609,170) | 50x43 | font montserratmedium_16 | bg: little.png */
    lv_obj_set_pos(pg->button_117, 609, 170);
    lv_obj_set_size(pg->button_117, 50, 43);

    /* container_13: 容器 | (0,0) | 1280x480 */
    lv_obj_set_pos(pg->container_13, 0, 0);
    lv_obj_set_size(pg->container_13, 1280, 480);

    /* label_353: 标签 | "是否结束当前任务" | (895,160) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_353, 895, 160);
    lv_obj_set_size(pg->label_353, 252, 36);

    /* label_354: 标签 | "回到主页" | (895,198) | 252x36 | font taiwanpearl_regular_30 */
    lv_obj_set_pos(pg->label_354, 895, 198);
    lv_obj_set_size(pg->label_354, 252, 36);

    /* sure: 按钮 | (959,295) | 129x83 | font taiwanpearl_regular_36 | bg: stopbk1.png */
    lv_obj_set_pos(pg->sure, 959, 295);
    lv_obj_set_size(pg->sure, 129, 83);

}

/* ============ 排版微调函数注册表（页面 → 函数 → 动态偏移 dx/dy） ============
/* dx/dy: 定时器重写对象(如 bartemp)的整体平移偏移，中文模式/0 = 零影响 */
/* ==============================================================================
 * sixset2 英文布局基准（对应 PAGE_SIXSET2 ）
 * 复用场景: 二维选择页 —— 份量×程度(带皮土豆/千层面/卡内罗尼) vs 成熟度×程度(牛肉/羊腿/羊排)
 * 分支标志: six_chick_is_matdeg()(熟度模式,中间区=maturity组) / 其余=weight组
 * ============================================================================== */
void sixset2_lang_tune(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_SIXSET2) return;
    sixset2_t *pg = sixset2_get(&ui_manager);
    if (!pg) return;

    /* next: 按钮 | (981,22) | 138x70 (公共) */
    lv_obj_set_pos(pg->next, 981, 22);
    lv_obj_set_size(pg->next, 138, 70);

    /* label_18: 左上角菜名 | (24,25) | 144x32 (公共;文本按菜动态) */
    lv_obj_set_pos(pg->label_18, 24, 25);
    lv_obj_set_size(pg->label_18, 144, 32);

    /* degree 组(右侧,常显): label_13 标题 + degree 单字 + degreeline */
    lv_obj_set_pos(pg->label_13, 855, 137);
    lv_obj_set_size(pg->label_13, 191, 32);
    lv_obj_set_pos(pg->degree, 915, 249);
    lv_obj_set_size(pg->degree, 84, 72);
    lv_obj_set_pos(pg->degreeline, 918, 328);

    if (six_chick_is_matdeg()) {
        /* 熟度模式: 中间区 = name1("成熟度") + maturity + maturityline2/3; weight 组隐藏不排 */
        lv_obj_set_pos(pg->name1, 277, 137);
        lv_obj_set_size(pg->name1, 104, 32);
        lv_obj_set_pos(pg->maturity, 216, 249);
        lv_obj_set_size(pg->maturity, 228, 72);
        lv_obj_set_pos(pg->maturityline3, 223, 328);
        lv_obj_set_pos(pg->maturityline2, 262, 328);
    } else {
        /* 份量模式: 中间区 = name1("份量/种类") + weight + weighticon(g) + weightline3/4 */
        lv_obj_set_pos(pg->name1, 277, 137);
        lv_obj_set_size(pg->name1, 104, 32);
        lv_obj_set_pos(pg->weight, 152, 248);
        lv_obj_set_size(pg->weight, 228, 72);
        lv_obj_set_pos(pg->weighticon, 380, 280);
        lv_obj_set_pos(pg->weightline4, 223, 328);
        lv_obj_set_pos(pg->weightline3, 264, 328);
    }
    /* TODO: 英文实测后在此分支内微调坐标/字号(如 maturity 文本加宽/居中) */
}


const struct { page_id_t page; lang_tune_fn fn; int dx, dy; } s_tune_tab[] = {
    { PAGE_AIR_COMPLETE, air_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_AIR_COOKING, air_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_AIR_MENU, air_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_AIR_SET, air_set_lang_tune, 0, 0 },   /*  */
    { PAGE_AIR_SETTING, air_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_AIR_STOP, air_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_AIR_STOP_BACK, air_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_BOTTOM_BBQ_COMPLETE, bottom_bbq_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_BOTTOM_BBQ_COMPLETE_PROBE, bottom_bbq_complete_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_BOTTOM_BBQ_COOKING, bottom_bbq_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_BOTTOM_BBQ_COOKING_PROBE, bottom_bbq_cooking_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_BOTTOM_BBQ_MENU, bottom_bbq_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_BOTTOM_BBQ_MENU_PROBE, bottom_bbq_menu_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_BOTTOM_BBQ_SET, bottom_bbq_set_lang_tune, 0, 0 },   /*  */
    { PAGE_BOTTOM_BBQ_SET_PROBE, bottom_bbq_set_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_BOTTOM_BBQ_SETTING, bottom_bbq_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_BOTTOM_BBQ_STOP, bottom_bbq_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_BOTTOM_BBQ_STOP_BACK, bottom_bbq_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_BOTTOM_BBQ_STOP_BACK_PROBE, bottom_bbq_stop_back_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_BOTTOM_BBQ_STOP_PROBE, bottom_bbq_stop_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_BREAD6MENU, bread6menu_lang_tune, 0, 0 },   /*  */
    { PAGE_BREAD_COMPLETE, bread_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_BREAD_COOKING, bread_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_BREAD_MENU, bread_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_BREAD_SET, bread_set_lang_tune, 0, 0 },   /*  */
    { PAGE_BREAD_SETTING, bread_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_BREAD_STOP, bread_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_BREAD_STOP_BACK, bread_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_CAKE6MENU, cake6menu_lang_tune, 0, 0 },   /*  */
    { PAGE_CENTRAL_BBQ_COMPLETE, central_bbq_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_CENTRAL_BBQ_COOKING, central_bbq_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_CENTRAL_BBQ_MENU, central_bbq_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_CENTRAL_BBQ_SET, central_bbq_set_lang_tune, 0, 0 },   /*  */
    { PAGE_CENTRAL_BBQ_SETTING, central_bbq_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_CENTRAL_BBQ_STOP, central_bbq_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_CENTRAL_BBQ_STOP_BACK, central_bbq_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_CHICK6MENU, chick6menu_lang_tune, 0, 0 },   /*  */
    { PAGE_CHICKENCOOKING, chickencooking_lang_tune, 0, 0 },   /*  */
    { PAGE_CHICKENMENU, chickenmenu_lang_tune, 0, 0 },   /*  */
    { PAGE_CHICKMENUTZ, chickmenutz_lang_tune, 0, 0 },   /*  */
    { PAGE_CHIP_COMPLETE, chip_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_CHIP_COOKING, chip_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_CHIP_MENU, chip_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_CHIP_SET, chip_set_lang_tune, 0, 0 },   /*  */
    { PAGE_CHIP_SETTING, chip_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_CHIP_STOP, chip_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_CHIP_STOP_BACK, chip_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_CLEAN_MENU, clean_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_COLOR_COOKING, color_cookoing_lang_tune, 0, 0 },   /*  */
    { PAGE_COLOR_STOP, color_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_COLOR_STOP_BACK, color_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_COLOR_COOKING_COMPLETE, colorcooking_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_COOK4_MENU, cook4menu_lang_tune, 0, 0 },   /*  */
    { PAGE_COOK_MENU_TZ, cook_menu_tz_lang_tune, 0, 0 },   /*  */
    { PAGE_COOKIE_COMPLETE, cookie_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_COOKIE_COOKING, cookie_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_COOKIE_MENU, cookie_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_COOKIE_SET, cookie_set_lang_tune, 0, 0 },   /*  */
    { PAGE_COOKIE_SETTING, cookie_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_COOKIE_STOP, cookie_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_COOKIE_STOP_BACK, cookie_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_COOKMENU, cookmenu_lang_tune, 0, 0 },   /*  */
    { PAGE_CORN_COMPLETE, corn_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_CORN_COOKING, corn_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_CORN_MENU, corn_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_CORN_SET, corn_set_lang_tune, 0, 0 },   /*  */
    { PAGE_CORN_SETTING, corn_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_CORN_STOP, corn_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_CORN_STOP_BACK, corn_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_CUSTOM_COMPLETE, custom_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_CUSTOM_COOKING, custom_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_CUSTOM_MENU, custom_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_CUSTOM_SET, custom_set_lang_tune, 0, 0 },   /*  */
    { PAGE_CUSTOM_SETTING, custom_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_CUSTOM_STOP, custom_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_CUSTOM_STOP_BACK, custom_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_DELAYCOOKING, delaycooking_lang_tune, 0, 0 },   /* 预约烹饪页 */
    { PAGE_DELAYSET, delayset_lang_tune, 0, 0 },   /*  */
    { PAGE_DESCRIPTIONMENU, descriptionmenu_lang_tune, 0, 0 },   /*  */
    { PAGE_DUCK6MENU, duckmenu_lang_tune, 0, 0 },   /*  */
    { PAGE_EXTRA_COLOR, extra_color_lang_tune, 0, 0 },   /*  */
    { PAGE_FROZEN_COOK, frozencookpage_lang_tune, 0, 0 },   /*  */
    { PAGE_HEATCONTAIN_COMPLETE, heatcontain_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_HEATCONTAIN_COOKING, heatcontain_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_HEATCONTAIN_MENU, heatcontain_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_HEATCONTAIN_SET, heatcontain_set_lang_tune, 0, 0 },   /*  */
    { PAGE_HEATCONTAIN_SETTING, heatcontain_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_HEATCONTAIN_STOP, heatcontain_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_HEATCONTAIN_STOP_BACK, heatcontain_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_HOT_BBQ_COMPLETE, hot_bbq_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_HOT_BBQ_COMPLETE_PROBE, hot_bbq_complete_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_HOT_BBQ_COOKING, hot_bbq_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_HOT_BBQ_COOKING_PROBE, hot_bbq_cooking_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_HOT_BBQ_MENU, hot_bbq_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_HOT_BBQ_MENU_PROBE, hot_bbq_menu_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_HOT_BBQ_SET, hot_bbq_set_lang_tune, 0, 0 },   /*  */
    { PAGE_HOT_BBQ_SET_PROBE, hot_bbq_set_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_HOT_BBQ_SETTING, hot_bbq_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_HOT_BBQ_STOP, hot_bbq_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_HOT_BBQ_STOP_BACK, hot_bbq_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_HOT_BBQ_STOP_BACK_PROBE, hot_bbq_stop_back_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_HOT_BBQ_STOP_PROBE, hot_bbq_stop_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEAN_MENU, hotclean_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANHIGH_COMPLETE, hotcleanhigh_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANHIGH_COOKING, hotcleanhigh_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANHIGH_COOLING, hotcleanhigh_cooling_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANHIGH_SET, hotcleanhigh_set_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANHIGH_STOP, hotcleanhigh_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANHIGH_STOP_BACK, hotcleanhigh_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANMIDDLE_COMPLETE, hotcleanmiddle_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANMIDDLE_COOKING, hotcleanmiddle_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANMIDDLE_COOLING, hotcleanmiddle_cooling_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANMIDDLE_SET, hotcleanmiddle_set_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANMIDDLE_STOP, hotcleanmiddle_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANMIDDLE_STOP_BACK, hotcleanmiddle_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANSAVE_COMPLETE, hotcleansave_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANSAVE_COOKING, hotcleansave_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANSAVE_COOLING, hotcleansave_cooling_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANSAVE_SET, hotcleansave_set_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANSAVE_STOP, hotcleansave_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTCLEANSAVE_STOP_BACK, hotcleansave_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTWIND_BBQ_COMPLETE, hotwind_bbq_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTWIND_BBQ_COOKING, hotwind_bbq_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTWIND_BBQ_MENU, hotwind_bbq_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTWIND_BBQ_SET, hotwind_bbq_set_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTWIND_BBQ_SETTING, hotwind_bbq_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTWIND_BBQ_STOP, hotwind_bbq_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_HOTWIND_BBQ_STOP_BACK, hotwind_bbq_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_LASAGNA_COMPLETE, lasagna_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_LASAGNA_COOKING, lasagna_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_LASAGNA_MENU, lasagna_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_LASAGNA_SET, lasagna_set_lang_tune, 0, 0 },   /*  */
    { PAGE_LASAGNA_SETTING, lasagna_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_LASAGNA_STOP, lasagna_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_LASAGNA_STOP_BACK, lasagna_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_MAJOR_MENU, major_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_MAJOR_MENU_TZ, major_menu_tz_lang_tune, 0, 0 },   /*  */
    { PAGE_MENU_COOK_COMPLETE, menu_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_MENU_COOK_COOKING, menu_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_MENU_COOK_MENU, menu_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_MENU_COOK_SET, menu_set_lang_tune, 0, 0 },   /*  */
    { PAGE_MENU_COOK_SETTING, menu_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_MENU_COOK_STOP, menu_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_MENU_COOK_STOP_BACK, menu_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA3_COMPLETE, pizza3_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA3_COOKING, pizza3_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA3_MENU, pizza3_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA3_SET, pizza3_set_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA3_SETTING, pizza3_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA3_STOP, pizza3_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA3_STOP_BACK, pizza3_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_2_COMPLETE, pizza_2_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_2_COOKING, pizza_2_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_2_MENU, pizza_2_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_2_SET, pizza_2_set_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_2_SETTING, pizza_2_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_2_STOP, pizza_2_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_2_STOP_BACK, pizza_2_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_COMPLETE, pizza_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_COOKING, pizza_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_MENU, pizza_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_SET, pizza_set_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_SETTING, pizza_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_STOP, pizza_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_PIZZA_STOP_BACK, pizza_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_PREHEAT_STOP_BACK, preheat_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_PREHEAT_COMPLETE, preheatcomplete_lang_tune, 0, 0 },   /*  */
    { PAGE_PREHEAT_COOKING, preheatcooking_lang_tune, 0, 0 },   /*  */
    { PAGE_COLOR_MENU, color_menu_lang_tune, 0, 0 },   /* 额外上色设置页（复用预热菜单结构） */
    { PAGE_PREHEAT_MENU, preheat_menu_lang_tune, 0, 0 },   /* 预热菜单页 */
    { PAGE_PREHEAT_STOP, preheatstop_lang_tune, 0, 0 },   /*  */
    { PAGE_PROBENEEDTIP, probeneedtip_lang_tune, 0, 0 },   /*  */
    { PAGE_PROBETIP, probetip_lang_tune, 0, 0 },   /*  */
    { PAGE_RISING_COMPLETE, rising_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_RISING_COOKING, rising_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_RISING_MENU, rising_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_RISING_SET, rising_set_lang_tune, 0, 0 },   /*  */
    { PAGE_RISING_SETTING, rising_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_RISING_STOP, rising_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_RISING_STOP_BACK, rising_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_RISINGPAGE, risingpage_lang_tune, 0, 0 },   /*  */
    { PAGE_SAVE_BBQ_COMPLETE, save_bbq_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_SAVE_BBQ_COOKING, save_bbq_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_SAVE_BBQ_MENU, save_bbq_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_SAVE_BBQ_SET, save_bbq_set_lang_tune, 0, 0 },   /*  */
    { PAGE_SAVE_BBQ_SETTING, save_bbq_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_SAVE_BBQ_STOP, save_bbq_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_SAVE_BBQ_STOP_BACK, save_bbq_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_SCREEN_SET, screen_SET_lang_tune, 0, 0 },   /*  */
    { PAGE_SIXMENU, sixmenu_lang_tune, 0, 0 },   /*  */
    { PAGE_SIXMENUTZ, sixmenutz_lang_tune, 0, 0 },   /*  */
    { PAGE_SIXSET2, sixset2_lang_tune, 0, 0 },   /* 六感双维选择页(复用:份量/熟度两模式) */
    { PAGE_SIXOP3PAGE, sixop3page_lang_tune, 0, 0 },   /*  */
    { PAGE_SLOWCOOK_COMPLETE, slowcook_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_SLOWCOOK_COMPLETE_PROBE, slowcook_complete_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_SLOWCOOK_COOKING, slowcook_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_SLOWCOOK_COOKING_PROBE, slowcook_cooking_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_SLOWCOOK_MENU, slowcook_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_SLOWCOOK_MENU_PROBE, slowcook_menu_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_SLOWCOOK_SET, slowcook_set_lang_tune, 0, 0 },   /*  */
    { PAGE_SLOWCOOK_SET_PROBE, slowcook_set_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_SLOWCOOK_SETTING, slowcook_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_SLOWCOOK_STOP, slowcook_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_SLOWCOOK_STOP_BACK, slowcook_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_SLOWCOOK_STOP_BACK_PROBE, slowcook_stop_back_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_SLOWCOOK_STOP_PROBE, slowcook_stop_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_SOMECOOK, somecook_lang_tune, 0, 0 },   /*  */
    { PAGE_SIX_COOKING, six_cooking_lang_tune, 0, 0 },   /* 第六感烹饪页 */
    { PAGE_SOMECOOK_COOKING, somecook_cooking_lang_tune, 0, 0 },   /* 多段烹饪页 */
    { PAGE_SPECIAL_MENU, special_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_SPECIAL_MENU_TZ, special_menu_tz_lang_tune, 0, 0 },   /*  */
    { PAGE_STEPSET, stepset_lang_tune, 0, 0 },   /*  */
    { PAGE_STRUDEL_COMPLETE, strudel_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_STRUDEL_COOKING, strudel_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_STRUDEL_MENU, strudel_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_STRUDEL_SET, strudel_set_lang_tune, 0, 0 },   /*  */
    { PAGE_STRUDEL_SETTING, strudel_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_STRUDEL_STOP, strudel_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_STRUDEL_STOP_BACK, strudel_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_TEMPTIP, temptip_lang_tune, 0, 0 },   /*  */
    { PAGE_TOASTCOLOR, toastcolor_lang_tune, 0, 0 },   /*  */
    { PAGE_TOP_BBQ_COMPLETE, top_bbq_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_TOP_BBQ_COOKING, top_bbq_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_TOP_BBQ_MENU, top_bbq_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_TOP_BBQ_SET, top_bbq_set_lang_tune, 0, 0 },   /*  */
    { PAGE_TOP_BBQ_SETTING, top_bbq_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_TOP_BBQ_STOP, top_bbq_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_TOP_BBQ_STOP_BACK, top_bbq_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_UNFROZEN_COMPLETE, unfrozen_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_UNFROZEN_COOKING, unfrozen_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_UNFROZEN_MENU, unfrozen_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_UNFROZEN_SET, unfrozen_set_lang_tune, 0, 0 },   /*  */
    { PAGE_UNFROZEN_SETTING, unfrozen_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_UNFROZEN_STOP, unfrozen_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_UNFROZEN_STOP_BACK, unfrozen_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_COMPLETE, updown_bbq_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_COMPLETE_PROBE, updown_bbq_complete_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_COOKING, updown_bbq_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_COOKING_PROBE, updown_bbq_cooking_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_MENU, updown_bbq_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_MENU_LOW, updown_bbq_menu_low_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_MENU_PROBE, updown_bbq_menu_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_MENU_TOP, updown_bbq_menu_top_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_SET, updown_bbq_set_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_SET_PROBE, updown_bbq_set_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_SETTING, updown_bbq_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_STOP, updown_bbq_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_STOP_BACK, updown_bbq_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_STOP_BACK_PROBE, updown_bbq_stop_back_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_UPDOWN_BBQ_STOP_PROBE, updown_bbq_stop_probe_lang_tune, 0, 0 },   /*  */
    { PAGE_VEGETABLEMENU, vegetablemenu_lang_tune, 0, 0 },   /*  */
    { PAGE_WAITMENU_24, waitmenu_24_lang_tune, 0, 0 },   /*  */
    { PAGE_WATER_CLEAN_COMPLETE, waterclean_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_WATER_CLEAN_COOKING, waterclean_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_WATER_CLEAN_SET, waterclean_set_lang_tune, 0, 0 },   /*  */
    { PAGE_WATER_CLEAN_STOP, waterclean_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_WATER_CLEAN_STOP_BACK, waterclean_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_WEST_COMPLETE, west_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_WEST_COOKING, west_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_WEST_MENU, west_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_WEST_SET, west_set_lang_tune, 0, 0 },   /*  */
    { PAGE_WEST_SETTING, west_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_WEST_STOP, west_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_WEST_STOP_BACK, west_stop_back_lang_tune, 0, 0 },   /*  */
    { PAGE_WINDCHANGE_BBQ_COMPLETE, windchange_bbq_complete_lang_tune, 0, 0 },   /*  */
    { PAGE_WINDCHANGE_BBQ_COOKING, windchange_bbq_cooking_lang_tune, 0, 0 },   /*  */
    { PAGE_WINDCHANGE_BBQ_MENU, windchange_bbq_menu_lang_tune, 0, 0 },   /*  */
    { PAGE_WINDCHANGE_BBQ_SET, windchange_bbq_set_lang_tune, 0, 0 },   /*  */
    { PAGE_WINDCHANGE_BBQ_SETTING, windchange_bbq_setting_lang_tune, 0, 0 },   /*  */
    { PAGE_WINDCHANGE_BBQ_STOP, windchange_bbq_stop_lang_tune, 0, 0 },   /*  */
    { PAGE_WINDCHANGE_BBQ_STOP_BACK, windchange_bbq_stop_back_lang_tune, 0, 0 },   /*  */
};
const int s_tune_tab_n = (int)(sizeof(s_tune_tab) / sizeof(s_tune_tab[0]));
