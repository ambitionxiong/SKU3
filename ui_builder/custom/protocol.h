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
#define IFACE_DELAY_RESERVE 6   // 延时预约

/* ===== 机器设置状态字节(移植自同事 screen_SET.c,BUF[15]/BUF[16]/BUF[17]) =====
 * Machine_Set_num_1 → SEND_SETTINGS1(15);Machine_Set_num → SEND_SETTINGS2(17),
 * 其 BIT4-5(炉灯态 0-3)同时映射到 SEND_LAMP_STATE(16)。宏直接操作状态字节 */
#define Send_MachineState_1_SoundMute			0x01		// BIT0: 声音总开关 (1=静音)
#define Send_MachineState_1_KeyTone_BIT1		0x02		// BIT1: 按键音音量低位
#define Send_MachineState_1_KeyTone_BIT2		0x04		// BIT2: 按键音音量中位
#define Send_MachineState_1_KeyTone_BIT3		0x08		// BIT3: 按键音音量高位
#define Send_MachineState_1_BootTone_ON_or_OFF	0x10		// BIT4: 打开开机音 (1=关闭)

// 声音总开关
#define MSt_SoundMute_ON						Machine_Set_num_1 &= ~Send_MachineState_1_SoundMute          // 声音：开
#define MSt_SoundMute_OFF						Machine_Set_num_1 |=  Send_MachineState_1_SoundMute          // 声音：关
// 按键音音量设置（反逻辑：000=7档最大，111=0档无按键音）
#define MSt_KeyTone_Level7						Machine_Set_num_1 &= ~(Send_MachineState_1_KeyTone_BIT1 |  Send_MachineState_1_KeyTone_BIT2 | Send_MachineState_1_KeyTone_BIT3)	// 000=7档（最大）
#define MSt_KeyTone_Level6						Machine_Set_num_1  =  (Machine_Set_num_1 & ~(0x0E))		|  Send_MachineState_1_KeyTone_BIT1
#define MSt_KeyTone_Level5						Machine_Set_num_1  =  (Machine_Set_num_1 & ~(0x0E))		|  Send_MachineState_1_KeyTone_BIT2
#define MSt_KeyTone_Level4						Machine_Set_num_1  =  (Machine_Set_num_1 & ~(0x0E))		| (Send_MachineState_1_KeyTone_BIT1 | Send_MachineState_1_KeyTone_BIT2)	// 011=4档
#define MSt_KeyTone_Level3						Machine_Set_num_1  =  (Machine_Set_num_1 & ~(0x0E))		|  Send_MachineState_1_KeyTone_BIT3						// 100=3档
#define MSt_KeyTone_Level2						Machine_Set_num_1  =  (Machine_Set_num_1 & ~(0x0E))		| (Send_MachineState_1_KeyTone_BIT1 | Send_MachineState_1_KeyTone_BIT3)	// 101=2档
#define MSt_KeyTone_Level1						Machine_Set_num_1  =  (Machine_Set_num_1 & ~(0x0E))		| (Send_MachineState_1_KeyTone_BIT2 | Send_MachineState_1_KeyTone_BIT3)	// 110=1档
#define MSt_KeyTone_Level0						Machine_Set_num_1 |=  (Send_MachineState_1_KeyTone_BIT1 |  Send_MachineState_1_KeyTone_BIT2 | Send_MachineState_1_KeyTone_BIT3)	// 111=0档（无按键音）
// 开机音开关
#define MSt_BootTone_ON							Machine_Set_num_1 &= ~Send_MachineState_1_BootTone_ON_or_OFF         // 开机音—打开
#define MSt_BootTone_OFF						Machine_Set_num_1 |=  Send_MachineState_1_BootTone_ON_or_OFF         // 开机音—关闭

#define Send_MachineState_AutoKeepWarm          0x01
#define Send_MachineState_CoolingFanRun         0x02        //0 Temp controlled  1 Time controlled
#define Send_MachineState_Power_xxA             0x08        //16A电流
#define Send_MachineState_LightState_ON_15S     0x10        //炉灯亮15s
#define Send_MachineState_LightState_DOOR       0x20        //炉灯仅在开门时亮
#define Send_MachineState_LightState_OFF        0x30        //炉灯关

#define MSt_AutoKeepWarm_EN                     Machine_Set_num |=  Send_MachineState_AutoKeepWarm          //自动保温—开
#define MSt_AutoKeepWarm_UN                     Machine_Set_num &= ~Send_MachineState_AutoKeepWarm          //自动保温—关
#define MSt_CoolingFanRun_of_TimeControlled     Machine_Set_num |=  Send_MachineState_CoolingFanRun         //风扇—时间 控制
#define MSt_CoolingFanRun_of_TempControlled     Machine_Set_num &= ~Send_MachineState_CoolingFanRun         //风扇—温度 控制
#define MSt_Power_16A                           Machine_Set_num |=  Send_MachineState_Power_xxA             //16A—电流
#define MSt_Power_13A                           Machine_Set_num &= ~Send_MachineState_Power_xxA             //13A—电流
#define MSt_LightState_ON                       Machine_Set_num &= ~(Send_MachineState_LightState_ON_15S | Send_MachineState_LightState_DOOR | Send_MachineState_LightState_OFF)        //炉灯—开
#define MSt_LightState_ON_15S                   Machine_Set_num = (Machine_Set_num & ~(0x30)) | Send_MachineState_LightState_ON_15S   //炉灯—亮15s
#define MSt_LightState_DOOR                     Machine_Set_num = (Machine_Set_num & ~(0x30)) | Send_MachineState_LightState_DOOR     //炉灯—仅在开门时亮
#define MSt_LightState_OFF                      Machine_Set_num = (Machine_Set_num & ~(0x30)) | Send_MachineState_LightState_OFF        //炉灯—关

extern uint8_t Machine_Set_num_1;   /* 机器设置字节1(声音/按键音/开机音) → BUF[15] */
extern uint8_t Machine_Set_num;     /* 机器设置字节2(保温/风扇/功率/炉灯) → BUF[17],BIT4-5→BUF[16] */

// ===== 烹调模式 (BUF[4]) =====
#define MODE_NONE           0
#define MODE_UPDOWN_BBQ     3
#define MODE_TOP_BBQ        4
#define MODE_BOTTOM_BBQ     5
#define MODE_HOT_BBQ        8
#define MODE_HOTWIND_BBQ    2  // 热风对流 Forced Air
#define MODE_WINDCHANGE_BBQ 25 // 加强热风对流 Conventional Bake
#define MODE_CENTRAL_BBQ    26 // 小面积烧烤 Half width Grill
#define MODE_SAVE_BBQ       27 // 节能热风 Eco Forced Air
#define MODE_COOK4          33 // 多层烹调
#define MODE_PIZZA_2         6 // 披萨
#define MODE_FROZEN_BAKE    34 // 冷食速烹
#define MODE_SLOWCOOK       35 // 慢速烹调
#define MODE_UNFROZEN       40 // 烧烤解冻
#define MODE_RISING         12 // 发酵
#define MODE_CORN           36 // 脱水/干果
#define MODE_HEATCONTAIN    11 // 保温
#define MODE_AIR            13 // 空气炸
#define MODE_PREHEAT        28 // 快速预热
#define MODE_WATER_CLEAN    20 // 自清洁
#define MODE_HOT_CLEAN      41 // 高温清洁PYRO
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
