#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

// ===== 发送缓冲区索引 (display→power, 24 bytes) =====
#define SEND_HEADER          0
#define SEND_FRAME1          1
#define SEND_FRAME2          2
#define SEND_IFACE_STATUS    3
#define SEND_COOK_MODE       4
#define SEND_COOK_FLAG       5
#define SEND_MICRO_POWER     6
#define SEND_TEMP_H          7
#define SEND_TEMP_L          8
#define SEND_PROBE_TEMP      9
#define SEND_TIME_HOUR       10
#define SEND_TIME_MIN        11
#define SEND_TIME_SEC        12
#define SEND_AUTO_MENU_ID    13
#define SEND_BUZZER          14
#define SEND_SETTINGS1       15
#define SEND_LAMP_STATE      16
#define SEND_SETTINGS2       17
#define SEND_TEMP_LOWER_H    18
#define SEND_TEMP_LOWER_L    19
#define SEND_STEAM_LEVEL     20
#define SEND_MOTOR_CTRL      21
#define SEND_SPECIAL_CMD     22
#define SEND_CHECKSUM        23

// ===== 界面状态 (BUF[3]) =====
#define IFACE_STANDBY    0
#define IFACE_SETTING    1
#define IFACE_COOKING    2
#define IFACE_PAUSE      3
#define IFACE_COMPLETE   4
#define IFACE_SLEEP      5

// ===== 烹调模式 (BUF[4]) =====
#define MODE_NONE           0
#define MODE_UPDOWN_BBQ     3
#define MODE_TOP_BBQ        4
#define MODE_BOTTOM_BBQ     5
#define MODE_HOT_BBQ        8
#define MODE_EXTRA_COLOR    38

// ===== 蜂鸣器声音 (BUF[14]) =====
#define BUZZER_NONE           0
#define BUZZER_POWER_ON       1
#define BUZZER_KEY_VALID      2
#define BUZZER_KEY_INVALID    3
#define BUZZER_PREHEAT_DONE   4
#define BUZZER_PAUSE          5
#define BUZZER_COOK_DONE      6
#define BUZZER_ALARM          7
#define BUZZER_POWER_OFF      8
#define BUZZER_NO_WATER       9
#define BUZZER_ENCODER        11

// ===== 发送状态结构体 =====
// uart_send_fill() 只读此结构体，不访问其他全局变量
typedef struct {
    uint8_t iface_status;    // BUF[3] 界面状态
    uint8_t cook_mode;       // BUF[4] 烹调模式
    uint8_t cook_flag;       // BUF[5] 本机固定0
    uint8_t micro_power;     // BUF[6] 本机固定0
    uint16_t set_temp;       // BUF[7-8] 上腔温度
    uint8_t probe_temp;      // BUF[9] 本机固定0
    int remaining_ms;        // → BUF[10-12] h/m/s，-1=无剩余时间
    uint8_t auto_menu_id;    // BUF[13] 本机固定0
    uint8_t buzzer_req;      // BUF[14] 蜂鸣器（一次性，发完清0）
    uint8_t settings1;       // BUF[15] 暂固定0
    uint8_t lamp_state;      // BUF[16] 暂固定0
    uint8_t settings2;       // BUF[17] 暂固定0
    uint16_t set_temp_lower; // BUF[18-19] 下腔温度
    uint8_t steam_level;     // BUF[20] 本机固定0
    uint8_t motor_ctrl;      // BUF[21] 本机固定0
    uint8_t special_cmd;     // BUF[22] 本机固定0
} send_state_t;

extern send_state_t g_send;

// ===== 函数声明 =====
uint8_t CalculateChecksum(uint8_t *data, int length);
void uart_send_fill(void);
#ifdef LV_USE_AIC_SIMULATOR
void protocol_init(void);
#endif

#endif
