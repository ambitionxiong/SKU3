#ifndef NAV_FAVORITES_H
#define NAV_FAVORITES_H

#include "nav.h"
#include "protocol.h"

/* ===================== 收藏功能数据层 =====================
 * 全部收藏相关结构体/全局变量集中于此文件。
 * 收藏页 UI 与业务逻辑见 screen_favorites.c（独立页面，不依赖 ui_manager）
 * 导航/快照/保存/启动逻辑见 nav_favorites.c
 * ========================================================== */

/* 收藏内部模式标识（非协议 MODE_xxx，仅用于收藏数据结构内区分类型） */
#define FAV_MODE_SIX       100   /* 六感菜单收藏 */
#define FAV_MODE_MULTI     101   /* 多段烹饪收藏 */

/* 六感菜谱数量（收藏探针温度数组大小用） */
#define FAV_FUNLIST_COUNT  50

/* ---------- 多段烹饪设置结构体 ---------- */
typedef struct
{
    int8_t Func_Class;          /* 一级父类功能 */
    int8_t Func_Subclass;       /* 二级子类功能 */
    int16_t Func_Temp;          /* 设定温度 */
    int8_t Func_Hour;           /* 设定小时 */
    int8_t Func_Minute;         /* 设定分钟 */
    int8_t Func_Steam;          /* 是否有蒸汽辅助 */
    int8_t PengTiaoMode_num;    /* 通信：烹调模式 */
} Fun_Multi_step;

typedef struct
{
    Fun_Multi_step Func_Value_step_1;   /* 步骤一参数 */
    Fun_Multi_step Func_Value_step_2;   /* 步骤二参数 */
    Fun_Multi_step Func_Value_step_3;   /* 步骤三参数 */
    int8_t Func_num_1;                  /* 步骤一功能号 */
    int8_t Func_num_2;                  /* 步骤二功能号 */
    int8_t Func_num_3;                  /* 步骤三功能号 */
    uint8_t has_step_num;               /* 有哪些步骤存在 */
} Fun_Multi_SUM_Value;

/* ---------- 收藏值结构体 ---------- */
typedef struct
{
    uint16_t temperature;       /* 温度 */
    int8_t Func_Hour;           /* 设定小时 */
    int8_t Func_Minute;         /* 设定分钟 */
    int8_t Func_Steam;          /* 是否有蒸汽辅助 */
    int8_t Probe_temp;          /* 探针模式温度 */
    int8_t PengTiaoMode_name;   /* 烹调模式名称（FAV_MODE_SIX/FAV_MODE_MULTI/MODE_xxx） */
    uint8_t PengTiao_Mode;      /* 烹调烹饪模式（通信模式号） */
    int8_t Six_Cook_Fun;        /* Six菜单：菜谱编号 */
    int8_t Six_KaoSe;           /* Six菜单：烤色 */
    int8_t Six_FaJiao;          /* Six菜单：发酵 */
    int8_t Six_KG;              /* Six菜单：克重 */
    int8_t Six_Maturity;        /* Six菜单：成熟度 */
    int16_t source_page;        /* 来源页（区分 MODE_FROZEN_BAKE/COOK4 共用模式号的子类） */
    int16_t temp_down;          /* 下腔温度（上下烧烤） */
} Fun_favorites_Value;

typedef struct
{
    uint8_t has_favorites_byte;             /* 8位状态字节 */
    Fun_favorites_Value favorites_val[8];   /* 8个收藏的状态和参数 */
} SUM_favorites_Value;

/* 收藏卡片三行标签文本 */
typedef struct
{
    char L1_Lb_str[30];
    char L2_Lb_str[30];
    char L3_Lb_str[30];
} Fav_lb_str;

/* ---------- 收藏页 UI 结构（独立页面，不依赖 ui_manager） ---------- */
typedef struct
{
    lv_obj_t *obj;
    lv_group_t *group;              /* 创建焦点组 */
    lv_group_t *group_sub_1;        /* 创建焦点组 */
    lv_group_t *group_sub_2;        /* 创建焦点组 */
    lv_group_t *group_sub_3;        /* 创建焦点组 */
    lv_group_t *group_sub_4;        /* 创建焦点组 */
    lv_obj_t *obj_null;             /* 空焦点对象，用于处于非默认组时聚焦 */
    lv_obj_t *obj_null_1;           /* 空焦点对象，用于处于非默认组时聚焦 */
    lv_obj_t *obj_null_2;           /* 空焦点对象，用于处于非默认组时聚焦 */
    lv_obj_t *obj_null_3;           /* 空焦点对象，用于处于非默认组时聚焦 */
    lv_obj_t *obj_null_4;           /* 空焦点对象，用于处于非默认组时聚焦 */
    lv_obj_t *fav_title_Lb;
    lv_obj_t *favorites_box_1_Btn;
    lv_obj_t *favorites_box_2_Btn;
    lv_obj_t *favorites_box_3_Btn;
    lv_obj_t *favorites_box_4_Btn;
    lv_obj_t *fav_page_1_No;
    lv_obj_t *fav_page_2_No;
    lv_obj_t *fav_page_1_yes;
    lv_obj_t *fav_page_2_yes;
    lv_obj_t *plus_sign_1;
    lv_obj_t *plus_sign_2;
    lv_obj_t *plus_sign_3;
    lv_obj_t *plus_sign_4;
    lv_obj_t *favo_Cont_1;
    lv_obj_t *favo_LB_Cont_1;
    lv_obj_t *favo_Title_Lb_1;
    lv_obj_t *favo_L1_Lb_1;
    lv_obj_t *favo_Func_Temp_Lb_1;
    lv_obj_t *favo_L2_Lb_1;
    lv_obj_t *favo_L3_Lb_1;
    lv_obj_t *favo_Start_Btn_1;
    lv_obj_t *favo_Delete_Btn_1;
    lv_obj_t *favo_Cont_2;
    lv_obj_t *favo_LB_Cont_2;
    lv_obj_t *favo_Title_Lb_2;
    lv_obj_t *favo_L1_Lb_2;
    lv_obj_t *favo_Func_Temp_Lb_2;
    lv_obj_t *favo_L2_Lb_2;
    lv_obj_t *favo_L3_Lb_2;
    lv_obj_t *favo_Start_Btn_2;
    lv_obj_t *favo_Delete_Btn_2;
    lv_obj_t *favo_Cont_3;
    lv_obj_t *favo_LB_Cont_3;
    lv_obj_t *favo_Title_Lb_3;
    lv_obj_t *favo_L1_Lb_3;
    lv_obj_t *favo_Func_Temp_Lb_3;
    lv_obj_t *favo_L2_Lb_3;
    lv_obj_t *favo_L3_Lb_3;
    lv_obj_t *favo_Start_Btn_3;
    lv_obj_t *favo_Delete_Btn_3;
    lv_obj_t *favo_Cont_4;
    lv_obj_t *favo_LB_Cont_4;
    lv_obj_t *favo_Title_Lb_4;
    lv_obj_t *favo_L1_Lb_4;
    lv_obj_t *favo_Func_Temp_Lb_4;
    lv_obj_t *favo_L2_Lb_4;
    lv_obj_t *favo_L3_Lb_4;
    lv_obj_t *favo_Start_Btn_4;
    lv_obj_t *favo_Delete_Btn_4;
    lv_obj_t *Favorites_full_delete_1_Btn;
    lv_obj_t *Favorites_full_delete_2_Btn;
    lv_obj_t *Favorites_full_delete_3_Btn;
    lv_obj_t *Favorites_full_delete_4_Btn;
} screen_favorites_t;

/* ---------- 全局变量 ---------- */
extern SUM_favorites_Value Func_favorites_Value;        /* 普通收藏集 */
extern SUM_favorites_Value Func_favorites_Value_Probe;  /* 探针收藏集 */
extern SUM_favorites_Value Func_favorites_Value_NULL;   /* 空收藏模板 */
extern SUM_favorites_Value *Fav_Cur;                    /* 当前收藏集指针 */
extern Fun_Multi_SUM_Value Favorites_Value;
extern Fun_Multi_SUM_Value Func_SUM_Value_step;   /* g_steps 的收藏转换载体 */             /* 多段收藏参数 */
extern Fav_lb_str Fav_line_lb_str;
extern uint8_t favorites_how_many;                      /* 收藏数量 */
extern int8_t favorites_choose_which;                   /* 选中第几个收藏 1~8 */
extern uint8_t fav_succeed_no_repetitive;               /* 收藏成功且不重复标志 */
extern int8_t Del_Fav_create_flag;                      /* 删除界面标志 */

/* 收藏输入参数（进入 cooking 时快照，运行中 setting 改动不参与收藏） */
extern uint8_t input_Cooking_Mode;
extern uint16_t input_Temp;
extern int8_t input_Hour;
extern int8_t input_Minute;
extern int8_t input_Is_Steam;
extern int8_t input_Mode_name;
extern int8_t input_Six_num;
extern uint16_t input_Temp_Conventional_Dowm;
extern uint8_t input_Temp_probe[FAV_FUNLIST_COUNT];

/* 进入 cooking 时的参数快照（fav_snapshot_save 填充，收藏保存时读取） */
extern int fav_init_temp;
extern int fav_init_hour;
extern int fav_init_min;
extern int fav_init_probe_temp;
extern int fav_init_temp_up;
extern int fav_init_temp_down;

/* 页面实例 */
extern screen_favorites_t g_fav_screen;

/* ---------- 函数声明 ---------- */
void fav_snapshot_save(void);                        /* 进入 cooking 时调用，保存初始参数 */
void favorites_save_current(void);                   /* 收藏当前运行参数（KEY_FAV complete 态调用） */
void favorites_start_selected(void);                 /* 启动选中的收藏（直接进入运行） */
void Add_favorites(void);
void Add_favorites_of_Multi(Fun_Multi_SUM_Value Fav_Val);
void Delete_favorites(uint16_t favorites_id);
void Clear_all_favorites(void);
bool Favorites_Check_Exists(void);
void Favorites_Cover_Func(void);
bool Favorites_Check_Is_full(void);
void Fav_Start(void);
void Fav_Select_By_Probe(void);
void screen_favorites_create(void);                  /* 独立页面创建（无 ui_manager 依赖） */
void FAV_screen_Refresh_FirstPage(void);
void FAV_screen_Refresh_SecondPage(void);
void encoder_favorites_action(char key);
void return_favorites_action(void);
void favo_safety_group_delete(void);
const char *fav_mode_name(const Fun_favorites_Value *fav);   /* 按收藏值返回模式名（含子类） */
void jump_to_favorites(void);                                /* 进入收藏页 */
void favorites_rebuild(page_id_t child);                     /* 返回收藏页重建 */
void somecook_cooking_start(void);                           /* 多段启动（nav_somecook_cooking.c 导出） */

#endif /* NAV_FAVORITES_H */