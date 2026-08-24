# i18n 开发指南：英文版翻译与排版（操作教程）

> 本文档面向负责「英文版界面」的协作者。核心三句话：
> **翻译在 `i18n.c` 一处加词条；排版在 `nav_lang_tune.c` 每页函数里直接改数字；中文版完全不受影响。**

---

## 1. 整体机制（30 秒理解）

```
模拟器按数字键 6 切英文（再按一次切回中文）
页面进入/重建时（所有页面统一走 lang_scr_load_anim 包装出口）：
  1. 生成代码写中文默认值（ui_builder/ 根部文件，别碰）
  2. 动态文本（倒计时/状态条/按钮/菜名）→ tr() 查表替换英文（已包装 675+ 处）
     —— 顺带自动完成 ℃ → °C 转换
  3. nav_lang 树遍历：遍历当前屏所有 label
     a) 文本命中翻译表 → 替换为英文（精确匹配；状态条走模糊匹配解析）
     b) 字体 taiwanpearl → aktivgrotesk（不含中文的文本才切）
     c) roller 选项逐行翻译
  4. 当前页排版函数（nav_lang_tune.c）→ 执行（坐标/字号/对齐，改数字即生效）
切回中文：tr() 原样返回 + 树遍历早退 → 页面完全是中文，机制零执行
```

**关键**：语言切换只作用于「文本内容 + 字体」两个显示维度，
**不触碰按键分发、定时器、状态机、协议字段（g_send）、焦点组**——所有业务逻辑与语言无关（详见 §7）。

---

## 2. 翻译词条怎么加

文件：`custom/i18n.c` 的 `s_table[]`（当前 463+ 条）

```c
static const i18n_t s_table[] = {
    { "空气炸",           "Air Fry" },          // 纯文本
    { "| 空气炸 | %d℃ | %02d分钟",
      "| Air Fry | %d℃ | %02d min" },          // 格式串（占位符必须一一对应！）
    ...
};
```

规则：
- 查表是**整串精确匹配**（`tr("中文")` → 找完全一样的 zh）
- 格式串条目：`%d`/`%s` 占位符顺序和数量必须与英文版完全一致（数量不一致会显示错乱）
- 换行符 `\n` 直接写进字符串，英文版可自行断行
- **℃ 不用管**：英文模式下所有文本里的 ℃ 自动变 °C（`tr()` 统一转换，无需词条）
- 词条来源参考 `SKU123456中英繁对照表2026-5-9.xlsx`（sku3_最新资料 目录）
- **半角标点**：英文值里请用半角 `,` `:`（全角 `，` `：` 是中文残留，勿用于英文）

### 验证一个词条有没有生效

```
模拟器按 6 → 进入对应页面 → 看到中文即词条缺失或没包 tr()
```

---

## 3. 怎么找到要改的页面（最重要）

### 3.1 页面速查表

**完整 268 页索引见 `docs/PAGE_INDEX_zh.md`**（功能中文名 → 生成文件 → 页面 ID → tune 函数）。

### 3.2 命名规律（最快）

`tune 函数名 = 功能名_页面类型_lang_tune`，页面类型固定：

| 页面类型 | 后缀 | 例 |
|---------|------|-----|
| 菜单 | `_menu` | `air_menu_lang_tune` |
| 设置 | `_set` | `air_set_lang_tune` |
| 运行时设置 | `_setting` | `air_setting_lang_tune` |
| 烹饪 | `_cooking` | `air_cooking_lang_tune` |
| 暂停 | `_stop` | `air_stop_lang_tune` |
| 确认退出 | `_stop_back` | `air_stop_back_lang_tune` |
| 完成 | `_complete` | `air_complete_lang_tune` |

例："空气炸设置页" → `air_set_lang_tune`；"预热完成页" → `preheatcomplete_lang_tune`。
探针版加 `_probe`（如 `slowcook_set_probe_lang_tune`）。

### 3.3 三种辅助查找

- **页面 ID 反查**：nav.h 枚举 `PAGE_XXX` → 打开 nav_lang_tune.c 末尾注册表搜 `{ PAGE_XXX,` → 行尾即函数名
- **模拟器日志**：切页时终端打印 `[nav] jump: ...` → 跳转函数名（`jump_to_air_set`）即页面线索
- **中文标题搜**：grep 页面上的中文（如"烹饪时间"）在 `ui_builder/` 生成文件 → 文件 base 即函数名前缀

---

## 4. 排版调整（主战场）：nav_lang_tune.c

文件：`custom/nav_lang_tune.c`（自动生成的 267 个页面函数，**每个函数已含全部对象数据**）。
找到页面函数后，**直接改数字即可**。三类对象的改法不同：

### 4.1 静态对象（按钮/标题/背景/图片/设置页）

tune 里已生成 set_pos/set_size，直接改数字：

```c
void special_menu_lang_tune(void)
{
    ...
    /* special_label: 标签 | "特殊功能" | (24,24) | 97x25 | font taiwanpearl_regular_24 */
    lv_obj_set_pos(pg->special_label, 24, 24);        // ← 改这里
    lv_obj_set_size(pg->special_label, 97, 25);
    // 字号：lv_obj_set_style_text_font(pg->xxx, &c_aktivgroteskmedium_30, LV_PART_MAIN | 0);
    // 颜色：lv_obj_set_style_text_color(pg->xxx, lv_color_hex(0xff0000), LV_PART_MAIN | 0);
}
```

注释含义：`类型 | "显示内容" | 坐标(x,y) | 尺寸WxH | 字号 | 图片源img | 背景图bg`。
每函数开头有页面背景注释（`bg.jpg` 等，全项目统一）。

### 4.2 状态切换对象（min 数字/单位标签/icon——tune 里已有判断，改数字即可）

这些对象位置由业务按状态二选一（set_hour 有无小时 / cook_mode 模式）。
**tune 里已生成带默认业务值的判断代码**，直接改数字：

```c
/* min: 标签 | "20" | (395,254) | 58x53 | font taiwanpearl_regular_48 | 状态切换(默认业务值, 直接改数字) */
if (set_hour == 0)
    lv_obj_set_pos(pg->min, 312, 254);       // 无小时（居中）
else
    lv_obj_set_pos(pg->min, 395, 254);       // 有小时（右移）
```

想整体平移就把两组数字都加偏移；想改字号直接加 set_style_text_font。
**不要删掉判断结构改成单值**——那会让另一状态错位（见常见问题）。

个别对象因多业务共用（如 somecook_cooking 页 icon：第六感固定 163 / 多段烹饪按模式 149/115），
tune 里只留注释 `位置由业务动态控制, 微调按文件头模板`——需要微调时按文件头模板手写判断
（判断 `g_send.cook_mode == MODE_UNFROZEN` 等）。

### 4.3 定时器重写对象（bartemp——只能用注册表 dx/dy）

bartemp（进度条温度指示）每秒按进度重写位置，tune 里改了 1 秒后被覆盖，**判断也无效**。
调整方式：**文件末尾注册表改 dx/dy**（该页 bartemp 整体平移）：

```c
const struct { page_id_t page; lang_tune_fn fn; int dx, dy; } s_tune_tab[] = {
    { PAGE_AIR_SET, air_set_lang_tune, 0, 0 },
    { PAGE_PREHEAT_STOP_BACK, preheat_stop_back_lang_tune, 8, 0 },   // ← 该页 bartemp 右移 8px
    ...
};
```

dx=横向偏移、dy=纵向偏移；中文模式恒为 0（零影响）。

### 4.4 工作流（标准流程）

```
1. 模拟器按 6 切英文
2. 进到目标页 → 看到文字溢出/错位/字体不对
3. 查 docs/PAGE_INDEX_zh.md 或按命名规律找到 tune 函数
4. 改数字（set_pos / set_size / set_style_text_font / set_text）
5. Windows 侧重新编译模拟器（build 目录 make 或 VS 构建）
6. 重进页面看效果；按 6 切回中文确认不受影响
```

---

## 5. 字体

| 用途 | 字体 | 字号 |
|------|------|------|
| 中文默认 | taiwanpearl_regular | 23/24/30/36/48/60/72/128 |
| 英文 | aktivgroteskmedium | 24/30/36/48/60/72/128（树遍历自动切换）|

- 英文文本自动映射同字号 aktivgrotesk（nav_lang.c else-if 链）
- 切换条件是「文本不含中文」：含中文残留的 label 保持 taiwanpearl（防方块）
- aktivgrotesk 只含 ASCII + °↑↓（字体已含 U+00B0/U+2191/U+2193）：
  - ℃ 统一由 tr() 转成 °C
  - 文本含中文/未翻译残留 → 不切字体，taiwanpearl 显示（不方块）
- 字号 23 无英文对应档，保留 taiwanpearl
- 某处英文想整体调小：该页 tune 函数 `set_style_text_font` 指定更小字号

---

## 6. 语言切换与验证

- **切换**：模拟器按 **6**（main.c 钩子：`g_lang_en` 取反 + 立即刷新当前屏）
- **验证清单**：
  - 各功能页：温度 `230°C`（非 ℃）、状态条 `| Air Fry | 180°C | 1h 20min`
  - 倒计时走动 10 秒+：状态/按钮/菜名保持英文（动态刷新不被打回）
  - set 页：小时=0 与有小时两种布局位置正确
  - 预热完成：bartemp 停在 100% 位置（679）
  - 六感全流程：菜名/烤色/发酵小结/确认提示英文
  - 中文模式：全部中文，布局原样

---

## 7. 语言机制与业务逻辑隔离（重要）

语言切换设计**只影响显示**，已验证不影响任何事件逻辑：

| 业务层 | 是否被语言机制触碰 |
|--------|-------------------|
| 按键分发（process_key / nav_handle_key）| ❌ 语言键在 main.c 直接处理，不进业务分发 |
| 定时器（cooking_timer_cb 等）| ❌ 不创建/删除/修改 |
| 状态机（g_six_phase / s_chick_* / 模式切换）| ❌ |
| 协议字段（g_send）| ❌ 无任何文本赋值 |
| 焦点组 / 事件回调 | ❌ |
| 文本比较/逻辑判断 | ❌ 业务代码零 strcmp（唯一 strcmp 在树遍历自身）|

原理：`is_english()` 标志只被 i18n/nav_lang 层读取；tr() 只在显示路径返回文本；
tune 函数只改样式属性。**不要在业务逻辑里加 is_english() 判断**。

---

## 8. 常见问题

| 现象 | 原因 | 解法 |
|------|------|------|
| 某词条仍是中文 | 词条不在翻译表 | i18n.c 加一行 `{ "中文", "English" }` |
| 倒计时一动就变中文 | 动态写入点漏包 tr()（跨行三元/变量参数）| 查 custom 代码对应 set_text 行，包 tr()；词条确认存在 |
| 英文文字溢出 | 英文比中文长 | 该页 tune 缩小字号/移动位置 |
| 切英文后当前页没变 | 需重进页面 | 切英文后退出再进入目标页 |
| 格式串翻译错乱 | 占位符数量/顺序不一致 | 检查 i18n.c 该条目 `%d`/`%s` 与中文版一致 |
| 温度显示 ℃ 而非 °C | 该处文本没走 tr() | 动态写入包 tr()；静态由树遍历自动处理 |
| set 页分钟位置不随小时切换 | tune 里写死单值覆盖了状态 | 用生成的 if/else 判断结构改数字（见 4.2）|
| 动态标签位置 1 秒被打回 | 定时器重写（bartemp）| 用注册表 dx/dy（见 4.3）|
| 方块字符 | 字体缺该字符（aktivgrotesk 只含 ASCII+°↑↓）| 文本含中文残留保持 taiwanpearl；或确认字体包含所需符号 |
| 某些按钮文字仍是中文 | 文本写在根部生成文件 | 树遍历会自动覆盖；若没覆盖，确认页面重建触发（page_push/pop）|
| screen_SET 页位置 | 独立覆盖层页（custom/screen_SET.c）| tune 里 screen_SET_lang_tune 直接改（已解锁）|

---

## 9. 别做的事

- ❌ 不要手改 `ui_builder/*.c` 根部文件（sync 脚本会覆盖）
- ❌ 不要动业务逻辑（状态机/定时器/g_send/页面栈）
- ❌ 不要给 printf 调试日志加翻译
- ❌ 不要在中文路径加英文判断（`is_english()` 只应出现在 i18n/nav_lang 层）
- ❌ 不要把状态切换对象的 if/else 改成单值（会破坏另一状态布局）
- ❌ 不要直接改 `nav_lang_tune.c` 的文件结构（它是生成的；只改函数里的数值/注释）