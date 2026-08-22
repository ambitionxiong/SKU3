# 架构入门指南（烤箱显示模组）

> 给新协作者的快速上手文档。目标：看懂项目结构、知道改哪里、别改哪里。

## 1. 目录角色（最关键）

| 目录 | 内容 | 能不能手改 |
|------|------|-----------|
| `ui_builder/custom/` | 手写导航/逻辑/i18n 层（nav_*.c、i18n.c、protocol.h 等） | ✅ **只改这里** |
| `ui_builder/*.c`（根部） | AiUIBuilder 生成的页面 UI（air_menu.c、major_menu.c 等），含全部控件布局坐标 | ⚠️ **不要手改**，会被 `sync_ui_from_base.bat` 覆盖 |
| `simulator/` | SDL2 模拟器（main.c 按键模拟）+ LVGL 源码 | 少改（数字键 6 语言切换钩子在此） |
| `ui_builder/font/` | 字库（taiwanpearl 中文 / aktivgrotesk 英文） | 只加不改 |

## 2. 代码模块地图（custom/）

| 文件 | 职责 |
|------|------|
| `nav.c` | 壳注释（记录拆分布局） |
| `nav_core.c` | 全局变量、页面栈 `page_push()`、焦点组创建、公共 helper |
| `nav_pop.c` | `page_pop()`——返回上级页面时按父页重建 UI（巨型 switch） |
| `nav_key.c` | `process_key()` 按键分发（编码器/功能键/BACK） |
| `nav_keyio.c` | 输入状态机、长按重复 |
| `nav_events.c` | `bind_events()` 事件绑定、菜单入口回调、`mode_display_name()` 模式名表 |
| `nav_cooktimer.c` | `cooking_timer_cb()`——烹饪计时、状态文本、进度条（单体大函数） |
| `nav_jump.c` | 入口跳转（主菜单→各功能） |
| `nav_stop.c` | 暂停/恢复、stop_back 确认退出 |
| `nav_system.c` | `nav_init()`、500ms 系统定时器、topflag 时钟 |
| `nav_lang.c` | **英文排版调整层**（见下方 i18n 机制） |
| `i18n.c` | 翻译表（418 条中→英）+ `tr()` 查表 + 树遍历翻译 |
| `nav_<组名>.c`（24 个） | 每个标准组一套 7 页流程（menu→set→cooking→setting→stop→stop_back→complete） |
| `nav_six_*.c` | 第六感家族（菜谱/烤鸡/面包蛋糕等） |
| `nav_preheat.c` 等 | 独立功能页 |

## 3. 页面栈模型

```
页面跳转：jump_to_xxx() → page_push(页ID) → 建 UI → 完成
页面返回：KEY_BACK → process_key → page_pop() → 按父页重建 → 完成
```

- 栈数组 `page_stack[]`，`depth` = 栈高，`page_stack[depth-1]` = 当前页
- 标准组七页生命周期：`menu(设置温度时间) → set(预热/延时/保温) → cooking(烹饪) → setting(改值) / stop(暂停) → stop_back(确认退出) → complete(完成)`
- 24 个标准组文件结构完全同构（同一模板的拷贝）：每组 7 个 `jump_to_xxx_页()` + 7 个 `xxx_rebuild_页()` + 私有回调

## 4. 模拟器运行与键位

- 构建：VS Code + CMake（AiUIBuilder 工具链），产物 `build/main.exe`，1280×480 SDL 窗口
- 键位：`Tab`=菜单、`Esc`=BACK、`方向键`=编码器、`空格`=确认、`6`=**中英切换**、`0`=门、`8/9/-`=探针、`4`=预热、`2`=六感、`7`=清洁

## 5. i18n 机制速览（详见 I18N_GUIDE.md）

```
页面进入/重建（语言无关）
  → 根文件 create() 写中文默认值
  → custom 动态串 tr() 包装处写文本（中文模式原样）
  → 英文模式：nav_lang 树遍历替换静态文本 + 切 aktivgrotesk 字体
  → 英文模式：调当前页排版微调函数（若注册）
```

## 6. 已知命名陷阱（不是 bug，别改）

- `color_cookoing.c`（cooking 拼错）、`hot_wind_labal`（labal 拼错）、`windchange_buttonn`——被代码引用，改名需连带
- `screen_SET.c`（生成器大写风格）与 `nav_screen_set.c`（custom 风格）是同一页
- 探针版本菜单文件带 `_tz` 后缀
- printf 调试日志均为英文，无中文干扰

## 7. 常见任务入口

| 任务 | 改哪里 |
|------|--------|
| 改界面布局坐标 | 根文件（⚠️ 会被 sync 覆盖，见 sync_ui_from_base.bat 流程） |
| 改导航逻辑 | custom/nav_*.c |
| 加翻译词条 | custom/i18n.c 翻译表 |
| 调英文版排版 | custom/nav_lang.c 注册表 + 页面函数 |