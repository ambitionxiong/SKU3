# 页面速查表（268 页 → tune 函数）

> 使用方法：先找功能（如"空气炸"），再找页面类型，tune 函数名即 `nav_lang_tune.c` 中的函数。
> 命名规律：`功能名_页面类型_lang_tune`（menu/set/setting/cooking/stop/stop_back/complete）。
> 探针版本加 `_probe`；`PAGE_SIX_COOKING` 与 `PAGE_SOMECOOK_COOKING` 共用 `somecook_cooking_lang_tune`。

## 一、标准烧烤/烘焙模式组（menu → set → cooking → setting → stop → stop_back → complete）

| 功能（中文） | 页面 | page_id | tune 函数 |
|-------------|------|---------|-----------|
| 空气炸 | 菜单 | PAGE_AIR_MENU | air_menu_lang_tune |
| 空气炸 | 设置 | PAGE_AIR_SET | air_set_lang_tune |
| 空气炸 | 烹饪 | PAGE_AIR_COOKING | air_cooking_lang_tune |
| 空气炸 | 运行时设置 | PAGE_AIR_SETTING | air_setting_lang_tune |
| 空气炸 | 暂停 | PAGE_AIR_STOP | air_stop_lang_tune |
| 空气炸 | 确认退出 | PAGE_AIR_STOP_BACK | air_stop_back_lang_tune |
| 空气炸 | 完成 | PAGE_AIR_COMPLETE | air_complete_lang_tune |
| 底部烧烤 | 菜单/设置/烹饪/… | PAGE_BOTTOM_BBQ_* | bottom_bbq_*_lang_tune（+_probe 探针版）|
| 顶部烧烤 | 菜单/设置/… | PAGE_TOP_BBQ_* | top_bbq_*_lang_tune |
| 上下烧烤 | 菜单/设置/… | PAGE_UPDOWN_BBQ_* | updown_bbq_*_lang_tune（+_probe）|
| 上下烧烤(上层) | 菜单 | PAGE_UPDOWN_BBQ_MENU_TOP | updown_bbq_menu_top_lang_tune |
| 上下烧烤(下层) | 菜单 | PAGE_UPDOWN_BBQ_MENU_LOW | updown_bbq_menu_low_lang_tune |
| 热风烧烤 | 菜单/设置/… | PAGE_HOT_BBQ_* | hot_bbq_*_lang_tune（+_probe）|
| 热风 | 菜单/设置/… | PAGE_HOTWIND_BBQ_* | hotwind_bbq_*_lang_tune |
| 节能热风 | 菜单/设置/… | PAGE_SAVE_BBQ_* | save_bbq_*_lang_tune |
| 集中烧烤 | 菜单/设置/… | PAGE_CENTRAL_BBQ_* | central_bbq_*_lang_tune |
| 热风对流 | 菜单/设置/… | PAGE_WINDCHANGE_BBQ_* | windchange_bbq_*_lang_tune |
| 面包 | 菜单/设置/… | PAGE_BREAD_* | bread_*_lang_tune（菜单还有 bread6menu：PAGE_BREAD6MENU → bread6menu_lang_tune）|
| 饼干 | 菜单/设置/… | PAGE_COOKIE_* | cookie_*_lang_tune（另有 COOK4：PAGE_COOK4_MENU → cook4menu_lang_tune）|
| 披萨 | 菜单/设置/… | PAGE_PIZZA_* | pizza_*_lang_tune |
| 披萨3 | 菜单/设置/… | PAGE_PIZZA3_* | pizza3_*_lang_tune |
| 披萨2 | 菜单/设置/… | PAGE_PIZZA_2_* | pizza_2_*_lang_tune |
| 菜单(菜谱) | 菜单/设置/… | PAGE_MENU_COOK_* | menu_*_lang_tune |
| 千层面 | 菜单/设置/… | PAGE_LASAGNA_* | lasagna_*_lang_tune |
| 干果 | 菜单/设置/… | PAGE_CORN_* | corn_*_lang_tune |
| 炸薯条 | 菜单/设置/… | PAGE_CHIP_* | chip_*_lang_tune |
| 自定义 | 菜单/设置/… | PAGE_CUSTOM_* | custom_*_lang_tune |
| 发酵 | 菜单/设置/… | PAGE_RISING_* | rising_*_lang_tune（另有发酵确认 risingpage：PAGE_RISINGPAGE → risingpage_lang_tune）|
| 慢煮 | 菜单/设置/… | PAGE_SLOWCOOK_* | slowcook_*_lang_tune（+_probe）|
| 解冻 | 菜单/设置/… | PAGE_UNFROZEN_* | unfrozen_*_lang_tune |
| 保温 | 菜单/设置/… | PAGE_HEATCONTAIN_* | heatcontain_*_lang_tune |
| 果馅卷 | 菜单/设置/… | PAGE_STRUDEL_* | strudel_*_lang_tune |
| 西式塔 | 菜单/设置/… | PAGE_WEST_* | west_*_lang_tune |

## 二、主菜单 / 功能入口

| 功能（中文） | 页面 | page_id | tune 函数 |
|-------------|------|---------|-----------|
| 待机页 | 开机首页 | PAGE_WAITMENU_24 | waitmenu_24_lang_tune |
| 主菜单 | 菜单 | PAGE_MAJOR_MENU | major_menu_lang_tune |
| 主菜单(探针) | 菜单 | PAGE_MAJOR_MENU_TZ | major_menu_tz_lang_tune |
| 烹饪功能 | 菜单 | PAGE_COOKMENU | cookmenu_lang_tune |
| 烹饪功能(探针) | 菜单 | PAGE_COOK_MENU_TZ | cook_menu_tz_lang_tune |
| 特殊功能 | 菜单 | PAGE_SPECIAL_MENU | special_menu_lang_tune |
| 特殊功能(探针) | 菜单 | PAGE_SPECIAL_MENU_TZ | special_menu_tz_lang_tune |
| 菜谱说明页 | 描述 | PAGE_DESCRIPTIONMENU | descriptionmenu_lang_tune |
| 预约设置 | 预约 | PAGE_DELAYSET | delayset_lang_tune |
| 额外上色 | 烤色程度 | PAGE_EXTRA_COLOR | extra_color_lang_tune |

## 三、第六感家族（鸡/鸭/肉/海鲜/面包蛋糕）

| 功能（中文） | 页面 | page_id | tune 函数 |
|-------------|------|---------|-----------|
| 第六感 | 分类菜单 | PAGE_SIXMENU | sixmenu_lang_tune |
| 第六感(探针) | 分类菜单 | PAGE_SIXMENUTZ | sixmenutz_lang_tune |
| 面包类 | 六感面包菜单 | PAGE_BREAD6MENU | bread6menu_lang_tune |
| 蛋糕类 | 六感蛋糕菜单 | PAGE_CAKE6MENU | cake6menu_lang_tune |
| 家禽类 | 六感鸡鸭菜单 | PAGE_CHICK6MENU | chick6menu_lang_tune |
| 家禽类(探针) | 六感菜单 | PAGE_CHICKMENUTZ | chickmenutz_lang_tune |
| 鸡 | 菜单 | PAGE_CHICKENMENU | chickenmenu_lang_tune |
| 鸭 | 菜单 | PAGE_DUCK6MENU | duckmenu_lang_tune |
| 蔬菜类 | 菜单 | PAGE_VEGETABLEMENU | vegetablemenu_lang_tune |
| 烤鸡/鸭 | 烹饪 | PAGE_CHICKENCOOKING | chickencooking_lang_tune |
| 烤色/重量 | 选择页 | PAGE_TOASTCOLOR | toastcolor_lang_tune |
| 六感选项 | 选择页 | PAGE_SIXOP3PAGE | sixop3page_lang_tune |
| 第六感烹饪 | 烹饪（与多段共用结构）| PAGE_SIX_COOKING | **six_cooking_lang_tune** |
| 多段烹饪 | 烹饪（与六感共用结构）| PAGE_SOMECOOK_COOKING | somecook_cooking_lang_tune |
| 额外上色 | 设置页（复用预热菜单结构）| PAGE_COLOR_MENU | **color_menu_lang_tune** |
| 预热 | 菜单（与额外上色共用结构）| PAGE_PREHEAT_MENU | preheatmenu_lang_tune |
| 预约烹饪 | 预约（29 模式共用）| PAGE_DELAYCOOKING | **delaycooking_lang_tune** |

## 四、多段烹饪 / 预热 / 清洁 / 系统

| 功能（中文） | 页面 | page_id | tune 函数 |
|-------------|------|---------|-----------|
| 多段烹饪 | 主页 | PAGE_SOMECOOK | somecook_lang_tune |
| 多段烹饪 | 烹饪 | PAGE_SOMECOOK_COOKING | somecook_cooking_lang_tune（与六感共用）|
| 多段步骤 | 步骤设置 | PAGE_STEPSET | stepset_lang_tune |
| 预热 | 菜单/烹饪/暂停/完成 | PAGE_PREHEAT_* | preheatmenu / preheatcooking / preheatstop / preheat_stop_back / preheatcomplete _lang_tune |
| 热解自清洁 | 菜单 | PAGE_HOTCLEAN_MENU | hotclean_menu_lang_tune |
| 热解自清洁(高档) | 烹饪/冷却/… | PAGE_HOTCLEANHIGH_* | hotcleanhigh_*_lang_tune |
| 热解自清洁(中档) | 烹饪/冷却/… | PAGE_HOTCLEANMIDDLE_* | hotcleanmiddle_*_lang_tune |
| 热解自清洁(节能) | 烹饪/冷却/… | PAGE_HOTCLEANSAVE_* | hotcleansave_*_lang_tune |
| 水清洁 | 设置/烹饪/… | PAGE_WATER_CLEAN_* | waterclean_*_lang_tune |
| 冷冻烘焙 | 菜单 | PAGE_FROZEN_COOK | frozencookpage_lang_tune |
| 设置 | 设置页（独立覆盖层）| PAGE_SCREEN_SET | screen_SET_lang_tune |
| 探针提示 | 提示 | PAGE_PROBETIP | probetip_lang_tune |
| 探针需插入 | 提示 | PAGE_PROBENEEDTIP | probeneedtip_lang_tune |
| 腔温过热 | 提示 | PAGE_TEMPTIP | temptip_lang_tune |
| 额外上色烹饪 | 烹饪/暂停/完成 | PAGE_COLOR_COOKING / PAGE_COLOR_STOP / PAGE_COLOR_COOKING_COMPLETE | color_cookoing / color_stop / colorcooking_complete _lang_tune |

## 五、速查：页面类型后缀 → 函数名

```
任意页面 tune 函数 = <功能前缀> + <页面类型后缀> + _lang_tune
前缀例子：air / bread / pizza / slowcook / preheat / hotcleanhigh ...
后缀：_menu _set _setting _cooking _stop _stop_back _complete（探针加 _probe）
特例（生成文件名拼写）：color_cookoing（少个 k）、preheatcomplete（无下划线）、
  screen_SET（大写）、waitmenu_24、somecook_cooking（双页共用）
```