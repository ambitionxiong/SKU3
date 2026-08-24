#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成 nav_lang_tune.c：全页面英文排版基准
数据来自生成文件（中文布局原值）：pos/size/font/align 调用 + 标签文本/图片源/背景图 注释
用法: python3 gen_tune.py <项目根> <映射json> <输出文件>"""
import re, sys, json, os, glob

ROOT = sys.argv[1]
MAPPING = json.load(open(sys.argv[2]))
OUT = sys.argv[3]

def extract_obj_bg(fn):
    """提取页面根对象(scr->obj)的背景: bg图名/bg色/opa"""
    if not os.path.exists(fn):
        return None
    src = open(fn, encoding='utf-8', errors='replace').read()
    bg = re.search(r'bg_img_src\(scr->obj, LVGL_IMAGE_PATH\((\S+?)\)', src)
    color = re.search(r'bg_color\(scr->obj, lv_color_hex\((0x[0-9a-fA-F]+)\)', src)
    opa = re.search(r'bg_opa\(scr->obj, (\d+)', src)
    return {
        'bg': bg.group(1) if bg else None,
        'color': color.group(1) if color else None,
        'opa': opa.group(1) if opa else None,
    }

def clean_txt(t):
    """注释用文本清洗：\n→⏎、引号转义、超长截断"""
    t = t.replace('\\n', '⏎').replace('\\"', '"').replace('"', "'")
    if len(t) > 34:
        t = t[:34] + '…'
    return t

def collect_dynamic_objs():
    """锁定对象 = 定时器回调内 set_pos 的对象 ∪ 业务坐标与生成值不同的对象。
    原因: tune 在页面显示后执行, 若业务按状态切换/改动位置(如 set_hour
    切换 min 位置), tune 固定值会破坏状态布局——这类对象只能由注册表
    dx/dy 偏移调整, 不能 tune 直接改。screen_SET(create 初始化=生成值)除外。"""
    timers = set()
    for fn in glob.glob(f"{ROOT}/ui_builder/custom/*.c"):
        if fn.endswith('nav_lang_tune.c') or fn.endswith('nav_lang.c') or fn.endswith('i18n.c'):
            continue
        src = open(fn, encoding='utf-8', errors='replace').read()
        for m in re.finditer(r'lv_timer_create\((\w+)\s*,', src):
            timers.add(m.group(1))
    dyn = set()
    for fn in glob.glob(f"{ROOT}/ui_builder/custom/*.c"):
        if fn.endswith('nav_lang_tune.c') or fn.endswith('nav_lang.c') or fn.endswith('i18n.c'):
            continue
        src = open(fn, encoding='utf-8', errors='replace').read()
        func = "?"
        for line in src.split('\n'):
            m = re.match(r'\s*(?:static\s+)?(?:void|int|bool|uint8_t|const\s+char\s*\*|char\s*\*)\s+(\w+)\s*\(', line)
            if m:
                func = m.group(1)
            if func in timers:
                m2 = re.match(r'lv_obj_set_pos\(\w+->(\w+)\s*,', line.strip())
                if m2:
                    dyn.add(m2.group(1))
    # 业务 set_pos 坐标集合（排除 screen_SET.c 的 create 初始化）
    biz = {}
    for fn in glob.glob(f"{ROOT}/ui_builder/custom/*.c"):
        if 'screen_SET' in fn or fn.endswith('nav_lang_tune.c') or fn.endswith('nav_lang.c') or fn.endswith('i18n.c'):
            continue
        src = open(fn, encoding='utf-8', errors='replace').read()
        # 兼容已加偏移的形式: N + lang_dyn_dx()
        for m in re.finditer(r'lv_obj_set_pos\(\w+->(\w+)\s*,\s*(\d+)(?:\s*\+\s*lang_dyn_dx\(\))?\s*,\s*(\d+)(?:\s*\+\s*lang_dyn_dy\(\))?\s*\)', src):
            biz.setdefault(m.group(1), set()).add((m.group(2), m.group(3)))
    # 生成文件坐标
    gen = {}
    for fn in glob.glob(f"{ROOT}/ui_builder/*.c"):
        src = open(fn, encoding='utf-8', errors='replace').read()
        for m in re.finditer(r'lv_obj_set_pos\(scr->(\w+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)', src):
            gen.setdefault(m.group(1), set()).add((m.group(2), m.group(3)))
    for obj, coords in biz.items():
        if len(coords) >= 2 or coords != gen.get(obj, coords):
            dyn.add(obj)
    return dyn

DYNAMIC = collect_dynamic_objs()

def extract_block_data(fn):
    """解析生成文件，返回 [{name, type, pos, size, font, align, img, bg, txt}]（按成员）"""
    if not os.path.exists(fn):
        return []
    src = open(fn, encoding='utf-8', errors='replace').read()
    parts = re.split(r'\n\s*//\s*Init\s+scr->(\w+)', src)
    objs = []
    for i in range(1, len(parts), 2):
        name, body = parts[i], parts[i+1]
        if name == 'obj':
            continue
        m = re.search(r'scr->%s\s*=\s*lv_(\w+)_create' % re.escape(name), body)
        otype = m.group(1) if m else '?'
        pos = re.search(r'lv_obj_set_pos\s*\(\s*scr->%s\s*,\s*(\d+)\s*,\s*(\d+)\s*\)' % re.escape(name), body)
        size = re.search(r'lv_obj_set_size\s*\(\s*scr->%s\s*,\s*(\d+)\s*,\s*(\d+)\s*\)' % re.escape(name), body)
        font = re.search(r'lv_obj_set_style_text_font\s*\(\s*scr->%s\s*,\s*&(c_\w+)\s*,' % re.escape(name), body)
        align = re.search(r'lv_obj_align\s*\(\s*scr->%s\s*,\s*(\w+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*\)' % re.escape(name), body)
        img = re.search(r'lv_img_set_src\s*\(\s*scr->%s\s*,\s*LVGL_IMAGE_PATH\((\S+?)\)' % re.escape(name), body)
        bg = re.search(r'lv_obj_set_style_bg_img_src\s*\(\s*scr->%s\s*,\s*LVGL_IMAGE_PATH\((\S+?)\)' % re.escape(name), body)
        txt = None
        if otype in ('label', 'btn'):
            tm = re.search(r'lv_label_set_text\s*\(\s*scr->%s\s*,\s*"((?:[^"\\]|\\.)*)"' % re.escape(name), body)
            if tm:
                txt = clean_txt(tm.group(1))
        objs.append({
            'name': name,
            'type': otype,
            'pos': pos.groups() if pos else None,
            'size': size.groups() if size else None,
            'font': font.group(1) if font else None,
            'align': align.groups() if align else None,
            'img': img.group(1) if img else None,
            'bg': bg.group(1) if bg else None,
            'txt': txt,
        })
    return objs

TYPE_CN = {
    'obj': '容器', 'btn': '按钮', 'label': '标签', 'img': '图片', 'roller': '滚轮',
    'bar': '进度条', 'container': '容器', 'arc': '弧', 'switch': '开关',
    'chart': '图表', 'textarea': '输入框', 'dropdown': '下拉', 'checkbox': '复选',
    'led': '指示灯', 'line': '线', 'list': '列表', 'table': '表格',
    'canvas': '画布', 'scale': '刻度', 'spinbox': '数字框', 'spinner': '加载',
    'keyboard': '键盘', 'menu': '菜单', 'msgbox': '消息框', 'tileview': '瓦片',
}

def gen_function(base, pages):
    fname = f"{base}_lang_tune"
    L = []
    L.append(f"/* {'='*78}")
    L.append(f" * {base} 英文布局基准（对应 {', '.join(sorted(pages))}）")
    L.append(f" * 数据 = 中文布局原值；改数值即改英文版布局（仅英文模式执行）")
    L.append(f" * {'='*78} */")
    L.append(f"void {fname}(void)")
    L.append(f"{{")
    if len(pages) == 1:
        L.append(f"    if (depth <= 0 || page_stack[depth - 1] != {sorted(pages)[0]}) return;")
    else:
        L.append(f"    if (depth <= 0) return;")
        cond = " && ".join(f"page_stack[depth - 1] != {p}" for p in sorted(pages))
        L.append(f"    if ({cond}) return;")
    L.append(f"    {base}_t *pg = {base}_get(&ui_manager);")
    L.append(f"    if (!pg) return;")
    L.append(f"")
    objs = extract_block_data(f"{ROOT}/ui_builder/{base}.c") or extract_block_data(f"{ROOT}/ui_builder/custom/{base}.c")
    objbg = extract_obj_bg(f"{ROOT}/ui_builder/{base}.c") or extract_obj_bg(f"{ROOT}/ui_builder/custom/{base}.c")
    if objbg and (objbg['bg'] or objbg['color'] or objbg['opa']):
        parts = []
        if objbg['bg']: parts.append(f"背景图 {objbg['bg']}")
        if objbg['color']: parts.append(f"底色 {objbg['color']}")
        if objbg['opa']: parts.append(f"opa {objbg['opa']}")
        L.append(f"    /* 页面背景: {' | '.join(parts)}（设置于 scr->obj 根部；换背景改生成文件或自行加 set_style_bg_img_src） */")
        L.append(f"")
    for o in objs:
        desc = []
        if o['type'] in TYPE_CN:
            desc.append(TYPE_CN[o['type']])
        else:
            desc.append(o['type'])
        if o['txt'] is not None:
            desc.append(f"\"{o['txt']}\"")
        if o['pos']:
            desc.append(f"({o['pos'][0]},{o['pos'][1]})")
        if o['size']:
            desc.append(f"{o['size'][0]}x{o['size'][1]}")
        if o['font']:
            desc.append(f"font {o['font'].replace('c_', '')}")
        if o['align']:
            desc.append(f"align {o['align'][0]}({o['align'][1]},{o['align'][2]})")
        if o['img']:
            desc.append(f"img: {o['img']}")
        if o['bg']:
            desc.append(f"bg: {o['bg']}")
        dynamic = o['name'] in DYNAMIC
        if dynamic:
            desc.append(f"动态定位(需微调见下方模板)")
        L.append(f"    /* {o['name']}: {' | '.join(desc)} */")
        if o['pos'] and not dynamic:
            L.append(f"    lv_obj_set_pos(pg->{o['name']}, {o['pos'][0]}, {o['pos'][1]});")
        if o['size']:
            L.append(f"    lv_obj_set_size(pg->{o['name']}, {o['size'][0]}, {o['size'][1]});")
        if o['align'] and not dynamic:
            L.append(f"    lv_obj_align(pg->{o['name']}, {o['align'][0]}, {o['align'][1]}, {o['align'][2]});")
        if not (o['pos'] or o['size'] or o['align']):
            L.append(f"    /* (无位置/尺寸设置) */")
        L.append(f"")
    L.append(f"}}")
    L.append(f"")
    return '\n'.join(L), len(objs)

out = []
out.append("/* =====================================================================")
out.append(" * 英文排版调整层：同事编辑区（nav_lang_tune.c）— 全页面布局基准")
out.append(" *")
out.append(" * 每个函数对应一个页面，内含该页面全部对象的布局基准数据：")
out.append(" *   标签文本 \"...\" / 坐标 (x,y) / 尺寸 WxH / 字号 font / 对齐 align")
out.append(" *   / 图片源 img / 背景图 bg（值来自生成文件中文布局；img/bg 仅注释）")
out.append(" *")
out.append(" * 使用：改数值即可调整英文版布局（仅英文模式执行，中文模式零影响）")
out.append(" *   lv_obj_set_pos(pg->xxx, 新x, 新y)      —— 移动")
out.append(" *   lv_obj_set_size(pg->xxx, 新w, 新h)     —— 改尺寸")
out.append(" *   字号：英文文本建议 aktivgroteskmedium_XX（树遍历已自动映射）")
out.append(" *   换图：注释里的 img:/bg: 是当前图名，需换图自行处理（本文件不改图）")
out.append(" *")
out.append(" * 动态定位对象（注释标注'动态定位'）：位置由业务代码按状态切换")
out.append(" *   （set_hour 有无小时 / cook_mode 模式 / 进度），本文件默认不设位置。")
out.append(" *   需要微调时在此函数里加判断（tune 执行时状态变量已是显示值）：")
out.append(" *")
out.append(" *     以 set 页分钟+单位标签为例：")
out.append(" *     if (set_hour == 0) {                       // 无小时")
out.append(" *         lv_obj_set_pos(pg->min, 312 + 8, 254);")
out.append(" *         lv_obj_set_pos(pg->label_306, 365 + 8, 269);")
out.append(" *     } else {                                   // 有小时")
out.append(" *         lv_obj_set_pos(pg->min, 395 + 8, 254);")
out.append(" *         lv_obj_set_pos(pg->label_306, 448 + 8, 269);")
out.append(" *     }")
out.append(" *     // icon 按模式： if (g_send.cook_mode == MODE_UNFROZEN) ...")
out.append(" *")
out.append(" *   bartemp（进度条跟随，定时器每秒重写）：判断无效，只能用")
out.append(" *   注册表 dx/dy 整体偏移（见文件末尾注册表）。")
out.append(" *")
out.append(" * 注册表在文件末尾：页面 → 函数 一一对应；新增页面照格式加一行")
out.append(" * 详见 docs/I18N_GUIDE_zh.md")
out.append(" * ===================================================================== */")
out.append("#include \"nav_lang.h\"")
out.append("#include \"i18n.h\"")
out.append("#include \"protocol.h\"   /* g_send 状态变量（icon/模式判断用） */")
out.append("#include \"screen_SET.h\"   /* 独立页面结构体（不挂 ui_manager） */")
out.append("")
out.append("typedef void (*lang_tune_fn)(void);")
out.append("")

total_objs = 0
funcs = []
for base in sorted(MAPPING):
    pages = MAPPING[base]
    func, nobj = gen_function(base, pages)
    funcs.append(func)
    total_objs += nobj

out.append("\n\n".join(funcs))

out.append("/* ============ 排版微调函数注册表（页面 → 函数 → 动态偏移 dx/dy） ============")
out.append("/* dx/dy: 定时器重写对象(如 bartemp)的整体平移偏移，中文模式/0 = 零影响 */")
out.append("const struct { page_id_t page; lang_tune_fn fn; int dx, dy; } s_tune_tab[] = {")
for base in sorted(MAPPING):
    for p in sorted(MAPPING[base]):
        out.append(f"    {{ {p}, {base}_lang_tune, 0, 0 }},")
out.append("};")
out.append("const int s_tune_tab_n = (int)(sizeof(s_tune_tab) / sizeof(s_tune_tab[0]));")
out.append("")

open(OUT, 'w', encoding='utf-8').write('\n'.join(out))
print(f"生成完成: {len(MAPPING)} 个函数, {total_objs} 个对象, 输出 {OUT}")