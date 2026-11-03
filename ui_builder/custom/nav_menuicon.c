/*
 * nav_menuicon.c - major/cook/six/special 及探针三菜单 EN 版独立图标+24px居中排版 (2026-10-10)
 *
 * 用户定版规则:图标与文字垂直间距 25px(墨迹间距),图标+文字作为整体在瓦片内垂直居中,
 * 图标水平居中;两行文字瓦片的图标与同排单行瓦片对齐(icon_rel=35),
 * 两行文字块的中心对齐单行文字的中心(label 顶=瓦顶+120)。
 * label 盒顶到墨迹顶约 7px(c_aktivgroteskmedium_24 度量)。
 * 按钮默认主题 padding 会把子控件坐标原点推偏 ~20px(实测右移),
 * 故加子控件的瓦片按钮统一 pad_all=0(背景图不受 padding 影响)。
 * sixmenu 子 label 生成层烙有 ALIGN_CENTER,LVGL9 里 align 样式优先于 set_pos,
 * 必须用 lv_obj_align(TOP_LEFT) 重设对齐再给 y(否则 y 变成自中心的偏移,文字跑到按钮外)。
 * EN 模式: 藏长条图标图(menuimg/cookmenuicon/six_en),换 21 张单瓦片素材+文字 label 化;
 * 简繁直接 return(长条图照旧,显示零变化)。
 */
#include "nav.h"
#include "protocol.h"   /* g_send/IFACE_*:探针小标按界面状态取舍(2026-11-03) */

#define MENU_INK_OFF   7    /* 24px label 盒顶到墨迹顶 */
#define MENU_GAP       25   /* 图标底到文字墨迹顶 */

/* 瓦片按钮加独立图标:清按钮 padding(否则子控件原点偏 ~20px),水平居中,y=相对瓦片顶 */
static lv_obj_t *menu_icon_add(lv_obj_t *btn, const char *src, int tile_w, int art_w, int y_rel)
{
    lv_obj_set_style_pad_all(btn, 0, LV_PART_MAIN);
    lv_obj_t *ic = lv_img_create(btn);
    lv_img_set_src(ic, src);
    lv_obj_clear_flag(ic, LV_OBJ_FLAG_CLICKABLE);   /* 不挡按钮点击/焦点 */
    lv_obj_set_pos(ic, (tile_w - art_w) / 2, y_rel);
    return ic;
}

/* 屏幕级瓦片文字:24px + 瓦片等宽盒子水平居中(生成层无 align 烙印,set_pos 即绝对坐标) */
static void menu_label_fit(lv_obj_t *lbl, int tile_x, int label_y_abs, int tile_w, int h)
{
    if (!lbl) return;
    lv_obj_set_pos(lbl, tile_x, label_y_abs);
    lv_obj_set_size(lbl, tile_w, h);
    lv_obj_set_style_text_font(lbl, &c_aktivgroteskmedium_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
}

/* sixmenu 瓦片子 label(生成层空串+ALIGN_CENTER 烙印):TOP_LEFT 重设对齐+填文案+24px 白字居中
 * text 由调用点给:普通瓦片传 tr(key),两行瓦片传带 \n 的定版文案(换行在 & 后;
 * 词典值被子菜单标题共用,不能动词典) */
static void six_tile_label(lv_obj_t *btn, const char *text, int y_rel, int h, int tile_w)
{
    lv_obj_t *lbl = lv_obj_get_child(btn, 0);
    if (!lbl || !lv_obj_check_type(lbl, &lv_label_class)) return;
    lv_label_set_text(lbl, text);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 0, y_rel);   /* 先清 CENTER 烙印,y 才是真坐标 */
    lv_obj_set_size(lbl, tile_w, h);
    lv_obj_set_style_text_font(lbl, &c_aktivgroteskmedium_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
}

void nav_menuicon_refit(void)
{
    if (depth <= 0 || !is_english()) return;   /* 仅英文;简繁长条图照旧 */

    page_id_t pg = page_stack[depth - 1];

    if (pg == PAGE_MAJOR_MENU) {
        major_menu_t *mg = major_menu_get(&ui_manager);
        if (!mg) return;
        if (mg->major_img) lv_obj_add_flag(mg->major_img, LV_OBJ_FLAG_HIDDEN);
        /* 瓦片 439x417, 图 81 高: icon_rel=147, label_abs=60+147+81+18=306 */
        if (mg->cook_button)    menu_icon_add(mg->cook_button, LVGL_IMAGE_PATH(cookingfunction_icon.png), 439, 60, 147);
        if (mg->cook4_button)   menu_icon_add(mg->cook4_button, LVGL_IMAGE_PATH(cook4_icon.png), 439, 123, 147);
        if (mg->special_button) menu_icon_add(mg->special_button, LVGL_IMAGE_PATH(specialfunction_icon.png), 439, 122, 147);
        menu_label_fit(mg->cook_label, 7, 306, 439, 44);
        menu_label_fit(mg->cook4_label, 421, 306, 439, 44);
        menu_label_fit(mg->special_label, 835, 306, 439, 44);
    }
    else if (pg == PAGE_COOKMENU) {
        cookmenu_t *ck = cookmenu_get(&ui_manager);
        if (!ck) return;
        if (ck->mainimg_1) lv_obj_add_flag(ck->mainimg_1, LV_OBJ_FLAG_HIDDEN);
        /* 竖瓦片 244x386: icon_rel=131, label_abs=75+131+99=305 */
        if (ck->up_down_button) menu_icon_add(ck->up_down_button, LVGL_IMAGE_PATH(conventional_icon.png), 244, 101, 131);
        menu_label_fit(ck->up_down_labal, 21, 305, 244, 44);
        /* 行1 248x193: icon_rel=35 同排对齐; 单行 label_abs=210(y76)/209(y75), 两行中心对齐单行中心 label_abs=196 */
        if (ck->top_bbq_button)  menu_icon_add(ck->top_bbq_button, LVGL_IMAGE_PATH(grill_icon.png), 248, 101, 35);
        if (ck->hot_bbq_button)  menu_icon_add(ck->hot_bbq_button, LVGL_IMAGE_PATH(turbogrill_icon.png), 248, 101, 35);
        if (ck->hot_wind_button) menu_icon_add(ck->hot_wind_button, LVGL_IMAGE_PATH(forceair_icon.png), 248, 101, 35);
        if (ck->save_button)     menu_icon_add(ck->save_button, LVGL_IMAGE_PATH(ecoforceair_icon.png), 248, 103, 35);
        menu_label_fit(ck->hot_bbq_labal, 267, 209, 248, 44);
        menu_label_fit(ck->hotwind_bbq_labal, 516, 210, 248, 44);
        menu_label_fit(ck->hot_wind_labal, 764, 210, 248, 44);
        menu_label_fit(ck->save_labal, 1012, 196, 248, 58);
        /* 行2 248x193: 单行 label_abs=402; 两行(Convection Bake) label_abs=388 */
        if (ck->bottom_button)      menu_icon_add(ck->bottom_button, LVGL_IMAGE_PATH(bottomheat_icon.png), 248, 101, 35);
        if (ck->central_button)     menu_icon_add(ck->central_button, LVGL_IMAGE_PATH(halfgrill_icon.png), 248, 100, 35);
        if (ck->windchange_buttonn) menu_icon_add(ck->windchange_buttonn, LVGL_IMAGE_PATH(conventionalbake_icon.png), 248, 101, 35);
        if (ck->preheater_button)   menu_icon_add(ck->preheater_button, LVGL_IMAGE_PATH(preheat_icon.png), 248, 61, 35);
        menu_label_fit(ck->bottom_bbq_labal, 266, 402, 248, 44);
        menu_label_fit(ck->central_labal, 516, 402, 248, 44);
        menu_label_fit(ck->wind_change_labal, 764, 388, 248, 58);
        menu_label_fit(ck->preheater_labal, 1012, 402, 248, 44);
        /* 规范图两行拆行(词典值被收藏页/事件模块共用,不能动词典,这里直设) */
        if (ck->save_labal)        lv_label_set_text(ck->save_labal, "Eco\nForced Air");
        if (ck->wind_change_labal) lv_label_set_text(ck->wind_change_labal, "Convection\nBake");
        /* 页标题: 24px 单行放下,恢复单行高度(保持左对齐,字体走 pass2 映射) */
        if (ck->pengren_labal) {
            lv_obj_set_pos(ck->pengren_labal, 24, 24);
            lv_obj_set_size(ck->pengren_labal, 320, 30);
        }
    }
    else if (pg == PAGE_SPECIAL_MENU) {
        /* 特色菜单: 图标尺寸不一(multistage 仅 25 高),按用户定版——
         * 参照瓦片(frozen_bake 行1 / rising 行2)按组规则落位,本排其余图标中心对齐参照图标中心;
         * 整排文字取参照瓦片 label_y 统一;两行文字块中心对齐单行中心 */
        special_menu_t *sp = special_menu_get(&ui_manager);
        if (!sp) return;
        if (sp->major_img) lv_obj_add_flag(sp->major_img, LV_OBJ_FLAG_HIDDEN);
        /* 空气炸竖瓦片 244x386: icon_rel=131(abs 206), label_abs=305 */
        if (sp->air_button) menu_icon_add(sp->air_button, LVGL_IMAGE_PATH(airfry_icon.png), 244, 101, 131);
        menu_label_fit(sp->air_label, 21, 305, 244, 44);
        /* 行1 参照 frozen_cook(516,76): icon_rel=35(abs 111,中心 151.5), label_abs=210;
         * piza 瓦 y75 中心对齐 abs=111 → rel=36 */
        if (sp->frozen_cook_button) menu_icon_add(sp->frozen_cook_button, LVGL_IMAGE_PATH(frozenbake_icon.png), 248, 101, 35);
        if (sp->piza_button)        menu_icon_add(sp->piza_button, LVGL_IMAGE_PATH(pizzasspecial_icon.png), 248, 101, 36);
        if (sp->slow_cook_button)   menu_icon_add(sp->slow_cook_button, LVGL_IMAGE_PATH(slowcooking_icon.png), 248, 104, 35);
        if (sp->unfrozen_button)    menu_icon_add(sp->unfrozen_button, LVGL_IMAGE_PATH(defroast_icon.png), 248, 74, 35);
        menu_label_fit(sp->piza_label, 267, 210, 248, 44);
        menu_label_fit(sp->frozen_cook_label, 516, 210, 248, 44);
        menu_label_fit(sp->slow_cook_label, 764, 210, 248, 44);
        menu_label_fit(sp->unfrozen_label, 1012, 210, 248, 44);
        /* 行2 参照 rising(266,268): icon_rel=35(abs 303,中心 343.5), label_abs=402;
         * multistage 图 25 高中心对齐 abs=331 → rel=64(瓦 y267); 多段两行 label_abs=387 */
        if (sp->rising_button)       menu_icon_add(sp->rising_button, LVGL_IMAGE_PATH(rising_icon.png), 248, 101, 35);
        if (sp->corn_button)         menu_icon_add(sp->corn_button, LVGL_IMAGE_PATH(dehydration_icon.png), 248, 78, 35);
        if (sp->heat_contain_button) menu_icon_add(sp->heat_contain_button, LVGL_IMAGE_PATH(keepwarm_icon.png), 248, 140, 35);
        if (sp->some_cook_button)    menu_icon_add(sp->some_cook_button, LVGL_IMAGE_PATH(multistage_icon.png), 248, 101, 64);
        menu_label_fit(sp->fajiao_label, 266, 402, 248, 44);
        menu_label_fit(sp->corn_label, 516, 402, 248, 44);
        menu_label_fit(sp->heat_contain_label, 764, 402, 248, 44);
        menu_label_fit(sp->some_cook_label, 1012, 387, 248, 58);
        /* 多段烹饪词典首个命中即两行版(Multi-step\nCooking),直设保准(收藏页共用词典值不改) */
        if (sp->some_cook_label) lv_label_set_text(sp->some_cook_label, "Multi-step\nCooking");
        /* 页标题: 24px 单行放下(保持左对齐) */
        if (sp->special_label) {
            lv_obj_set_pos(sp->special_label, 24, 24);
            lv_obj_set_size(sp->special_label, 320, 30);
        }
    }
    else if (pg == PAGE_SIXMENU) {
        sixmenu_t *sm = sixmenu_get(&ui_manager);
        if (!sm) return;
        if (sm->image_1) lv_obj_add_flag(sm->image_1, LV_OBJ_FLAG_HIDDEN);
        /* 面包竖瓦片 244x386 图 76 高: icon_rel=134, label_rel=134+76+18=228 */
        if (sm->bread) {
            menu_icon_add(sm->bread, LVGL_IMAGE_PATH(bread_icon.png), 244, 175, 134);
            six_tile_label(sm->bread, tr("面包"), 134 + 76 + MENU_GAP - MENU_INK_OFF, 44, 244);
        }
        /* 行1 248x193 y76: icon_rel=35; 单行 label_rel=35+art_h+18 */
        if (sm->cake) {
            menu_icon_add(sm->cake, LVGL_IMAGE_PATH(cakepastries_icon.png), 248, 136, 35);
            six_tile_label(sm->cake, tr("蛋糕/糕点"), 35 + 80 + MENU_GAP - MENU_INK_OFF, 44, 248);
        }
        if (sm->chick) {
            menu_icon_add(sm->chick, LVGL_IMAGE_PATH(poultry_icon.png), 248, 136, 35);
            six_tile_label(sm->chick, tr("家禽"), 35 + 81 + MENU_GAP - MENU_INK_OFF, 44, 248);
        }
        if (sm->meat) {
            menu_icon_add(sm->meat, LVGL_IMAGE_PATH(meat_icon.png), 248, 158, 35);
            six_tile_label(sm->meat, tr("肉"), 35 + 81 + MENU_GAP - MENU_INK_OFF, 44, 248);
        }
        if (sm->fish) {
            menu_icon_add(sm->fish, LVGL_IMAGE_PATH(fishseafood_icon.png), 248, 161, 35);
            six_tile_label(sm->fish, tr("鱼/海鲜"), 35 + 81 + MENU_GAP - MENU_INK_OFF, 44, 248);
        }
        /* 行2 248x193 y268: 单行 label_rel=134; 蔬菜/砂锅两行: 图标对齐同排(icon_rel=35),
         * 两行文字中心对齐单行中心(label_rel=120, h58) */
        if (sm->vegetable) {
            menu_icon_add(sm->vegetable, LVGL_IMAGE_PATH(vegetables_icon.png), 248, 90, 35);
            six_tile_label(sm->vegetable, "Vegetables &\nSide Dishes", 120, 58, 248);
        }
        if (sm->pizza6) {
            menu_icon_add(sm->pizza6, LVGL_IMAGE_PATH(pizzas_icon.png), 248, 129, 35);
            six_tile_label(sm->pizza6, tr("披萨"), 35 + 81 + MENU_GAP - MENU_INK_OFF, 44, 248);
        }
        if (sm->pasta) {
            menu_icon_add(sm->pasta, LVGL_IMAGE_PATH(casseroles_icon.png), 248, 102, 35);
            six_tile_label(sm->pasta, "Casseroles &\nBaked Pasta", 120, 58, 248);
        }
        if (sm->snack) {
            menu_icon_add(sm->snack, LVGL_IMAGE_PATH(snack_icon.png), 248, 118, 35);
            six_tile_label(sm->snack, tr("零食"), 35 + 81 + MENU_GAP - MENU_INK_OFF, 44, 248);
        }
    }
    else if (pg == PAGE_MAJOR_MENU_TZ) {
        /* 探针主菜单: 双瓦片 642x390 y73, 图 81 高: icon_rel=(390-123)/2=134,
         * label_abs=73+134+81+18=306 */
        major_menu_tz_t *mg = major_menu_tz_get(&ui_manager);
        if (!mg) return;
        if (mg->major_img) lv_obj_add_flag(mg->major_img, LV_OBJ_FLAG_HIDDEN);
        if (mg->cook_button)    menu_icon_add(mg->cook_button, LVGL_IMAGE_PATH(cookingfunction_icon.png), 642, 60, 134);
        if (mg->special_button) menu_icon_add(mg->special_button, LVGL_IMAGE_PATH(specialfunction_icon.png), 642, 122, 134);
        menu_label_fit(mg->cook_label, 9, 306, 642, 44);
        menu_label_fit(mg->special_label, 630, 306, 642, 44);
    }
    else if (pg == PAGE_COOK_MENU_TZ) {
        /* 探针烹调菜单: 三瓦片 439x417 y60, icon_rel=147, label_abs=60+147+81+18=306 */
        cook_menu_tz_t *tz = cook_menu_tz_get(&ui_manager);
        if (!tz) return;
        if (tz->major_img) lv_obj_add_flag(tz->major_img, LV_OBJ_FLAG_HIDDEN);
        if (tz->updown_button)  menu_icon_add(tz->updown_button, LVGL_IMAGE_PATH(conventional_icon.png), 439, 101, 147);
        if (tz->hot_bbq_button) menu_icon_add(tz->hot_bbq_button, LVGL_IMAGE_PATH(turbogrill_icon.png), 439, 101, 147);
        if (tz->bottom_button)  menu_icon_add(tz->bottom_button, LVGL_IMAGE_PATH(bottomheat_icon.png), 439, 101, 147);
        menu_label_fit(tz->updown_label, 7, 306, 439, 44);
        menu_label_fit(tz->hot_label, 421, 306, 439, 44);
        menu_label_fit(tz->bottom_label, 835, 306, 439, 44);
    }
    else if (pg == PAGE_SPECIAL_MENU_TZ) {
        /* 探针特色菜单: 单瓦片 624x400 y68, icon_rel=(400-123)/2=139,
         * label_abs=68+139+81+18=306 */
        special_menu_tz_t *sp = special_menu_tz_get(&ui_manager);
        if (!sp) return;
        if (sp->major_img) lv_obj_add_flag(sp->major_img, LV_OBJ_FLAG_HIDDEN);
        if (sp->slowcook_button) menu_icon_add(sp->slowcook_button, LVGL_IMAGE_PATH(slowcooking_icon.png), 624, 104, 139);
        menu_label_fit(sp->slowcook_label, 18, 306, 624, 44);
    }
    else if (pg == PAGE_SIXMENUTZ) {
        /* 探针第六感菜单: 双瓦片 chick 638x387 y75 / meat 637x389 y74,
         * icon_rel=(瓦高-123)/2=132/133, label_abs 均为 306 */
        sixmenutz_t *mz = sixmenutz_get(&ui_manager);
        if (!mz) return;
        if (mz->image_3) lv_obj_add_flag(mz->image_3, LV_OBJ_FLAG_HIDDEN);
        if (mz->chick) menu_icon_add(mz->chick, LVGL_IMAGE_PATH(poultry_icon.png), 638, 136, 132);
        if (mz->meat)  menu_icon_add(mz->meat, LVGL_IMAGE_PATH(meat_icon.png), 637, 158, 133);
        menu_label_fit(mz->label_8, 12, 306, 638, 44);
        menu_label_fit(mz->label_9, 633, 306, 637, 44);
    }
}

/* ============ cook4menu/冷冻烘焙 两页卡片恢复"图标+标签"样式(2026-10-15) ============ */

/* COOK 4 四卡: 生成层本有整套原版图标+标签控件——独立图标 image_47~50
 * (cookieimg/westimg/pizaimg/menuimgg 101x81,原位已与瓦片对心)+独立标签
 * label_84~87(与内联文字同文案,树遍历照常翻译),导出时一并烙了 HIDDEN
 * 换成按键内联文字;这里翻回: 藏内联文字,亮原图标+标签。标签盒 100 宽左
 * 对齐放 EN 长词会裁边,放宽到瓦片宽 340 并文字居中。EN 走小字规则:
 * 24px aktiv+图标+文字整体瓦内垂直居中(墨迹间距 25px,盒顶补偿 7),
 * 简繁维持原版 30px/原位。全语言幂等,跑在 lang_on_page_built 末位
 * (EN tune 的 label_84~87 行先执行,被这里覆盖) */
void nav_cook4_card_icons(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_COOK4_MENU) return;
    cook4menu_t *pg = cook4menu_get(&ui_manager);
    if (!pg || !pg->obj) return;

    lv_obj_t *btns[4] = { pg->cookie, pg->west, pg->piza, pg->menu };
    lv_obj_t *imgs[4] = { pg->image_47, pg->image_48, pg->image_49, pg->image_50 };
    lv_obj_t *labs[4] = { pg->label_84, pg->label_85, pg->label_86, pg->label_87 };
    /* 2026-11-03 用户定:西式塔与披萨两卡换位(左→右 饼干|披萨|西式塔|菜单)。
       标签 x 直接按换位后的瓦位给(85=西式塔 走 625,86=披萨 走 315),
       按键/图标在下面循环后单独搬——三件套整块跟着瓦走,不会出现"字在左图在右" */
    const int cx[4]   = { 5, 625, 315, 936 };
    int lab_y = 290, icon_y = 0;
    if (is_english()) {
        icon_y = 206; lab_y = 305;   /* EN: 瓦 340x406 y65, 群组 81+25+17 居中 */
    }
    for (int i = 0; i < 4; i++) {
        if (btns[i]) {
            lv_obj_t *in = lv_obj_get_child(btns[i], 0);
            if (in && lv_obj_check_type(in, &lv_label_class))
                lv_obj_add_flag(in, LV_OBJ_FLAG_HIDDEN);
        }
        if (imgs[i]) {
            lv_obj_clear_flag(imgs[i], LV_OBJ_FLAG_HIDDEN);
            if (icon_y) lv_obj_set_y(imgs[i], icon_y);
        }
        if (labs[i]) {
            lv_obj_clear_flag(labs[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(labs[i], cx[i], lab_y);
            lv_obj_set_size(labs[i], 340, 32);
            lv_obj_set_style_text_align(labs[i], LV_TEXT_ALIGN_CENTER,
                                        LV_PART_MAIN | LV_STATE_DEFAULT);
            if (is_english())
                lv_obj_set_style_text_font(labs[i], &c_aktivgroteskmedium_24,
                                           LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}

/* 探针小标(tanzhen.png 40x26):探针态下烹饪前各菜单页标题右侧的一枚标记。
 * 2026-11-03 用户定:1)位置照探针主菜单(major_menu_tz 的 tz_img 96,24)——横向贴
 * 标题文字右缘 8px、纵向与标题同 y;2)逐页显式登记本页"页对象+标题标签",不做树扫描
 * (扫描会挑错标签,加错页)。标题盒是 97x25 定值,英文长标题会溢出盒子,故横向按实测
 * 文字宽度算,不能用盒宽。探针主菜单生成层已自带该图,本表不列它(防双标)。
 * 表里没有的页(烹饪/停止/完成/待机/设置覆盖层)一律不加。 */
#define PB_ENTRY(pid, type, getter, field) \
    case pid: { type *p = getter(&ui_manager); if (p) { page = p->obj; title = p->field; } } break;

void nav_probe_badge_refit(void)
{
    static lv_obj_t *s_badge = NULL;
    static lv_obj_t *s_page = NULL;

    if (depth <= 0 || !is_probe_inserted()) return;

    lv_obj_t *page = NULL, *title = NULL;
    switch (page_stack[depth - 1]) {
    PB_ENTRY(PAGE_COOK_MENU_TZ,          cook_menu_tz_t,          cook_menu_tz_get,          cook_label)
    PB_ENTRY(PAGE_SPECIAL_MENU_TZ,       special_menu_tz_t,       special_menu_tz_get,       special_label)
    PB_ENTRY(PAGE_SIXMENUTZ,             sixmenutz_t,             sixmenutz_get,             label_7)
    PB_ENTRY(PAGE_CHICKMENUTZ,           chickmenutz_t,           chickmenutz_get,           label_4)
    PB_ENTRY(PAGE_SIXOP3PAGE,            sixop3page_t,            sixop3page_get,            name)
    PB_ENTRY(PAGE_DESCRIPTIONMENU,       descriptionmenu_t,       descriptionmenu_get,       label_19)
    PB_ENTRY(PAGE_TOASTCOLOR,            toastcolor_t,            toastcolor_get,            label_24)
    PB_ENTRY(PAGE_UPDOWN_BBQ_MENU_PROBE, updown_bbq_menu_probe_t, updown_bbq_menu_probe_get, label_12)
    PB_ENTRY(PAGE_UPDOWN_BBQ_SET_PROBE,  updown_bbq_set_probe_t,  updown_bbq_set_probe_get,  label_19)
    PB_ENTRY(PAGE_HOT_BBQ_MENU_PROBE,    hot_bbq_menu_probe_t,    hot_bbq_menu_probe_get,    label_5)
    PB_ENTRY(PAGE_HOT_BBQ_SET_PROBE,     hot_bbq_set_probe_t,     hot_bbq_set_probe_get,     label_9)
    PB_ENTRY(PAGE_BOTTOM_BBQ_MENU_PROBE, bottom_bbq_menu_probe_t, bottom_bbq_menu_probe_get, label_50)
    PB_ENTRY(PAGE_BOTTOM_BBQ_SET_PROBE,  bottom_bbq_set_probe_t,  bottom_bbq_set_probe_get,  label_54)
    PB_ENTRY(PAGE_SLOWCOOK_MENU_PROBE,   slowcook_menu_probe_t,   slowcook_menu_probe_get,   label_83)
    PB_ENTRY(PAGE_SLOWCOOK_SET_PROBE,    slowcook_set_probe_t,    slowcook_set_probe_get,    label_87)
    default: return;
    }
    if (!page || !title || !lv_obj_is_valid(page) || !lv_obj_is_valid(title)) return;

    /* 标挂在本页上;换页后旧标随页销毁,这里只清指针,不在切屏途中 del */
    if (s_badge && (!lv_obj_is_valid(s_badge) || s_page != page)) {
        s_badge = NULL;
        s_page = NULL;
    }
    if (!s_badge) {
        s_badge = lv_img_create(page);
        lv_img_set_src(s_badge, LVGL_IMAGE_PATH(tanzhen.png));
        lv_obj_clear_flag(s_badge, LV_OBJ_FLAG_CLICKABLE);
        s_page = page;
    }

    const char *t = lv_label_get_text(title);
    int tw = lv_txt_get_width(t, (uint32_t)strlen(t),
                              lv_obj_get_style_text_font(title, LV_PART_MAIN),
                              lv_obj_get_style_text_letter_space(title, LV_PART_MAIN));
    lv_obj_clear_flag(s_badge, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(s_badge, lv_obj_get_x(title) + tw + 8, lv_obj_get_y(title));
}

/* 探针二级菜单(家禽/牛肉/羊肉/猪肉共用 chickmenutz 页) EN 小字规则:
 * 24px aktiv + 图标+文字整体瓦内垂直居中(墨迹间距 25px)。瓦 638x387 y75/y74,
 * 图 68x41: 图 rel=(387-83)/2=152, label 盒顶=152+41+18(25-7);
 * tune 的 align_to 大字版先跑,被这里覆盖。简繁不动(生成层 30px 原位),仅英文执行 */
void nav_chickmenutz_en_refit(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_CHICKMENUTZ) return;
    if (!is_english()) return;
    chickmenutz_t *pg = chickmenutz_get(&ui_manager);
    if (!pg || !pg->obj) return;

    if (pg->image_1) lv_obj_set_pos(pg->image_1, 296, 227);
    if (pg->image_2) lv_obj_set_pos(pg->image_2, 917, 226);
    if (pg->label_5) {
        lv_obj_set_size(pg->label_5, 638, LV_SIZE_CONTENT);
        lv_obj_set_pos(pg->label_5, 12, 286);
        lv_obj_set_style_text_font(pg->label_5, &c_aktivgroteskmedium_24,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (pg->label_6) {
        lv_obj_set_size(pg->label_6, 638, LV_SIZE_CONTENT);
        lv_obj_set_pos(pg->label_6, 633, 285);
        lv_obj_set_style_text_font(pg->label_6, &c_aktivgroteskmedium_24,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

/* 冷冻烘焙六卡: 生成层没有单卡图标控件——原版是 928x338 简体位图长条图
 * frozencookfr(繁體换 _tw),文字烙在图里无法随 EN 显示,395px 节距也对不上
 * 414px 卡片栅格,故藏掉;逐卡铺图标+文字,烹饪页同套图标偏大,等比缩到高 80
 * (126x101→100x80 预缩素材 *_s80.png,设备端不跑 transform 缩放)。
 * 排版沿用定版规则: 图标水平居中,图标+文字整体卡内垂直居中,墨迹间距 25px;
 * EN 标签按小字规则用 24px(MENU_INK_OFF=7),简繁维持 30px taiwanpearl
 * (盒顶补偿 5),图标/标签 y 随字号分两组。全语言幂等 */
void nav_frozen_card_icons(void)
{
    if (depth <= 0 || page_stack[depth - 1] != PAGE_FROZEN_COOK) return;
    frozencookpage_t *pg = frozencookpage_get(&ui_manager);
    if (!pg || !pg->obj) return;

    if (pg->image_279) lv_obj_add_flag(pg->image_279, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *btns[6] = { pg->Lasagna, pg->Strudel, pg->bread,
                          pg->pizza3,  pg->chip,   pg->custom };
    const char *src[6] = {
        LVGL_IMAGE_PATH(lasagnaicon_s80.png), LVGL_IMAGE_PATH(strudelicon_s80.png),
        LVGL_IMAGE_PATH(breadicon_1_s80.png), LVGL_IMAGE_PATH(pizza3icon_s80.png),
        LVGL_IMAGE_PATH(chipicon_s80.png),    LVGL_IMAGE_PATH(customicon_s80.png),
    };
    int icon_y = 43, lab_y = 143;   /* 简繁 30px: 群组 80+25+29 卡内居中 */
    if (is_english()) {
        icon_y = 49; lab_y = 147;   /* EN 24px 小字: 群组 80+25+17 */
    }
    for (int i = 0; i < 6; i++) {
        if (!btns[i]) continue;
        menu_icon_add(btns[i], src[i], 437, 100, icon_y);
        lv_obj_t *in = lv_obj_get_child(btns[i], 0);
        if (in && lv_obj_check_type(in, &lv_label_class)) {
            lv_obj_align(in, LV_ALIGN_TOP_MID, 0, lab_y);
            if (is_english())
                lv_obj_set_style_text_font(in, &c_aktivgroteskmedium_24,
                                           LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}
