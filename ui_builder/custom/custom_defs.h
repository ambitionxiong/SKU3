#ifndef CUSTOM_DEFS_H
#define CUSTOM_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ui_objects.h"
#if LV_USE_AIC_SIMULATOR
#include <stddef.h>
typedef size_t rt_size_t;
#else
#include <rtthread.h>
#endif
#include <stdint.h>

// ===== 屏幕 ID 枚举 =====
typedef enum {
    ScreenHome = 1,
    Screen1,
    Temper_Screen_id_COUNT
} Temper_Screen_id_Index;

// ===== 协议索引 =====
// 发送（显示板→电源板 24字节）—— 校验和在 BUF[23]，累加前 23 字节
#define Send_data_Data_Sum          23

// 按键值（接收数据 BUF[15]）
#define KEY1                  1    // 开关机键（长按触发省电）
#define KEY_MENU              3    // 进入主菜单
#define KEY_EXTRA_COLOR       5    // 进入额外上色
#define KEY_CLEAN             7    // 进入清洁菜单
#define KEY_BACK              21   // 返回键（触控板或按键板）
#define KEY_ENCODER_CW       31   // 编码器顺时针旋转
#define KEY_ENCODER_CCW      41   // 编码器逆时针旋转
#define KEY_ENCODER_PRESS    51   // 编码器按下确认

// 接收（电源板→显示板 18字节）
#define Receive_data_Data_Sum       17
#define Receive_data_QiangTi_Temp_H     3
#define Receive_data_QiangTi_Temp_L     4
#define Receive_data_ZhengFaQi_Temp_H   5
#define Receive_data_ZhengFaQi_Temp_L   6
#define Receive_data_Probe_Temp_H       7
#define Receive_data_Probe_Temp_L       8
#define Receive_data_ZhengFaQi_Time_hour      9
#define Receive_data_ZhengFaQi_Time_minute    10
#define Receive_data_Power_ALL_State         11
#define Receive_data_Power_ALL_Error         12
#define Receive_data_Power_Versions          13
#define Receive_data_CiKongGuan_Temp         14
#define Receive_data_Touch_Key               15

// ===== 设置参数结构体 =====
typedef struct {
    int8_t Set_Lock;           // 童锁           0关    1开
    int8_t Set_KeepWarm;       // 自动保温       0关    1开
    int8_t Set_FanCooling;     // 通风冷却       0温度控制 1时间控制
    int8_t Set_6th;            // 第六感         0全部  1无猪肉
    int8_t Set_Light;          // 炉灯           0开 1持续15秒 2仅开门时开 3关
    int8_t Set_TempUnit;       // 温度单位       0℃    1℉
    int8_t Set_VolumeFlag;     // 声音标志       0关   1开
    int8_t Set_VolumeHintTime; // 提示音持续时间 0一次 1 5秒 2 10秒 3 30秒
    int8_t Set_VolumeKey;      // 按键音大小
    int8_t Set_VolumeWelcome;  // 开机欢迎曲     0关   1开
    int8_t Set_Brightness;     // 屏幕亮度
    int8_t Set_TimeType;       // 时制           0 24小时 1 12小时
    int8_t Set_StandbyTime;    // 待机时间显示   0开 1关 2夜间模式
    int8_t Set_Language;       // 语言           0英语 1繁体 2简体
    int8_t Set_Power;          // 电源管理       0 16A 1 13A
    int8_t Set_DemoMode;       // 演示模式       0关   1开
} Setting_Data;

extern volatile Setting_Data SET_Data;

// ===== 串口数组 =====
extern uint8_t uart_data_receive[18];
extern uint8_t uart_data_send[24];

// ===== 8568 时钟 IC =====
extern int selected_year;
extern int selected_month;
extern int selected_day;
extern int selected_weekday;
extern int selected_hour;
extern int selected_min;
extern int selected_sec;

// ===== 消息结构体 =====
struct ui_msg {
    uint8_t *data_ptr;
    uint32_t data_size;
};

struct tx_msg {
    uint8_t *data_ptr;
    uint32_t data_size;
};

// ===== 函数声明 =====
extern void get_screen_id(lv_obj_t *screen);
extern Temper_Screen_id_Index screen_id;
#ifndef LV_USE_AIC_SIMULATOR
extern void backlight_set_level(int level);
extern int recv_lv_ui_msg(struct ui_msg *lv_ui_msg);
extern void free_ui_msg(struct ui_msg *msg);
extern void send_uart_param_array(uint8_t *data, rt_size_t length);
extern int test_uart(int argc, char *argv[]);
#endif

#ifdef __cplusplus
}
#endif

// ===== 可编辑字段描述 =====
typedef struct {
    lv_obj_t *label;
    lv_obj_t *ind_short;   // temp<100 时用 / 小时/分钟用这个
    lv_obj_t *ind_long;    // temp≥100 时用（对小时/分钟=NULL）
    int *value;
    int min;
    int max;
    int step;
    const char *fmt;     // 显示格式："%d" 或 "%02d"
} edit_field_t;

static inline int is_door_open(void) {
    return (uart_data_receive[Receive_data_Power_ALL_State] & (1 << 1)) != 0;
}

static inline int is_probe_inserted(void) {
    return (uart_data_receive[Receive_data_Power_ALL_State] & (1 << 2)) != 0;
}

#endif // CUSTOM_DEFS_H
