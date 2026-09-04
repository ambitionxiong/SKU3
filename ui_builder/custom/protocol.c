/*
 * protocol.c - 通讯协议封装
 * uart_send_fill: 将 g_send 状态结构打包为 24 字节发送帧(含校验和)，
 *   供显示→电源板通讯。CalculateChecksum: 累加校验。
 * 模拟器下另有 sim_scan 定时扫描(模拟按键/探针/温度输入)。
 */
#include "protocol.h"
#include "nav.h"
#include "custom_defs.h"

send_state_t g_send = {
    .iface_status = IFACE_SETTING,
    .remaining_ms = -1,
};

/* 机器设置状态字节(设置覆盖层改动,通信帧 BUF[15]/16/17 持续上报)。
 * 默认:声音开/按键音7档/开机音开;自动保温关/风扇温度控制/13A/炉灯常亮 */
uint8_t Machine_Set_num_1 = 0x00;
uint8_t Machine_Set_num   = 0x00;

uint8_t CalculateChecksum(uint8_t *data, int length)
{
    uint8_t checksum = 0;
    for (int i = 0; i < length; i++)
        checksum += data[i];
    return checksum;
}

void uart_send_fill(void)
{
    int h = 0, m = 0, s = 0;
    if (g_send.remaining_ms >= 0) {
        int sec = g_send.remaining_ms / 1000;
        h = sec / 3600;
        m = (sec % 3600) / 60;
        s = sec % 60;
    }

    uart_data_send[SEND_HEADER]       = 0xAA;
    uart_data_send[SEND_FRAME1]       = 0xA5;
    uart_data_send[SEND_FRAME2]       = 0x33;
    uart_data_send[SEND_IFACE_STATUS] = g_send.iface_status;
    uart_data_send[SEND_COOK_MODE]    = g_send.cook_mode;
    uart_data_send[SEND_COOK_FLAG]    = g_send.cook_flag;
    uart_data_send[SEND_MICRO_POWER]  = 0;

    uart_data_send[SEND_TEMP_H]       = (uint8_t)(g_send.set_temp >> 8);
    uart_data_send[SEND_TEMP_L]       = (uint8_t)(g_send.set_temp & 0xFF);
    uart_data_send[SEND_PROBE_TEMP]   = g_send.probe_temp;

    uart_data_send[SEND_TIME_HOUR]    = (uint8_t)h;
    uart_data_send[SEND_TIME_MIN]     = (uint8_t)m;
    uart_data_send[SEND_TIME_SEC]     = (uint8_t)s;
    uart_data_send[SEND_AUTO_MENU_ID] = 0;

    uart_data_send[SEND_BUZZER]       = g_send.buzzer_req;
    g_send.buzzer_req = 0;

    uart_data_send[SEND_SETTINGS1]    = Machine_Set_num_1;
    uart_data_send[SEND_LAMP_STATE]   = (uint8_t)((Machine_Set_num >> 4) & 0x03);   /* 炉灯态 0-3,联调时与板端确认 */
    uart_data_send[SEND_SETTINGS2]    = (uint8_t)(Machine_Set_num | (g_keepwarm_active ? Send_MachineState_AutoKeepWarm : 0x00));   /* BIT0: 设置位 | 运行中保温 */

    uart_data_send[SEND_TEMP_LOWER_H] = (uint8_t)(g_send.set_temp_lower >> 8);
    uart_data_send[SEND_TEMP_LOWER_L] = (uint8_t)(g_send.set_temp_lower & 0xFF);
    uart_data_send[SEND_STEAM_LEVEL]  = 0;
    uart_data_send[SEND_MOTOR_CTRL]   = 0;
    uart_data_send[SEND_SPECIAL_CMD]  = 0;

    uart_data_send[SEND_CHECKSUM] = CalculateChecksum(uart_data_send, SEND_CHECKSUM);

    /* 调试:设置字节变化时打印一次(实机验证 BUF[15]/BUF[17] 发送值) */
    static uint8_t s_last_set1 = 0xFF, s_last_set2 = 0xFF;
    if (Machine_Set_num_1 != s_last_set1 || Machine_Set_num != s_last_set2) {
        s_last_set1 = Machine_Set_num_1;
        s_last_set2 = Machine_Set_num;
        printf("[send] set1=0x%02X set2=0x%02X\n", s_last_set1, s_last_set2);
    }
}

#ifdef LV_USE_AIC_SIMULATOR
static lv_timer_t *sim_scan_timer = NULL;

static void sim_scan_cb(lv_timer_t *t)
{
    nav_key1_hold_check();
    uart_send_fill();
}

void protocol_init(void)
{
    if (!sim_scan_timer)
        sim_scan_timer = lv_timer_create(sim_scan_cb, 100, NULL);
}
#endif
