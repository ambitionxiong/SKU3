#include "protocol.h"
#include "nav.h"
#include "custom_defs.h"

send_state_t g_send = {
    .iface_status = IFACE_SETTING,
    .remaining_ms = -1,
};

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
    uart_data_send[SEND_COOK_FLAG]    = 0;
    uart_data_send[SEND_MICRO_POWER]  = 0;

    uart_data_send[SEND_TEMP_H]       = (uint8_t)(g_send.set_temp >> 8);
    uart_data_send[SEND_TEMP_L]       = (uint8_t)(g_send.set_temp & 0xFF);
    uart_data_send[SEND_PROBE_TEMP]   = 0;

    uart_data_send[SEND_TIME_HOUR]    = (uint8_t)h;
    uart_data_send[SEND_TIME_MIN]     = (uint8_t)m;
    uart_data_send[SEND_TIME_SEC]     = (uint8_t)s;
    uart_data_send[SEND_AUTO_MENU_ID] = 0;

    uart_data_send[SEND_BUZZER]       = g_send.buzzer_req;
    g_send.buzzer_req = 0;

    uart_data_send[SEND_SETTINGS1]    = 0;
    uart_data_send[SEND_LAMP_STATE]   = 0;
    uart_data_send[SEND_SETTINGS2]    = 0;

    uart_data_send[SEND_TEMP_LOWER_H] = (uint8_t)(g_send.set_temp_lower >> 8);
    uart_data_send[SEND_TEMP_LOWER_L] = (uint8_t)(g_send.set_temp_lower & 0xFF);
    uart_data_send[SEND_STEAM_LEVEL]  = 0;
    uart_data_send[SEND_MOTOR_CTRL]   = 0;
    uart_data_send[SEND_SPECIAL_CMD]  = 0;

    uart_data_send[SEND_CHECKSUM] = CalculateChecksum(uart_data_send, SEND_CHECKSUM);
}

#ifdef LV_USE_AIC_SIMULATOR
static lv_timer_t *sim_scan_timer = NULL;

static void sim_scan_cb(lv_timer_t *t)
{
    uart_send_fill();
}

void protocol_init(void)
{
    if (!sim_scan_timer)
        sim_scan_timer = lv_timer_create(sim_scan_cb, 100, NULL);
}
#endif
