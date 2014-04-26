#include "libds3ps2.h"
#include <kernel.h>
#include <sifrpc.h>

static SifRpcClientData_t ds3ps2if __attribute__((aligned(64)));
static u8 ds3ps2_buffer[4]         __attribute__((aligned(64)));
static int ds3ps2_initialized = 0;

int ds3ps2_init()
{
    if (ds3ps2_initialized) return 0;
    
    ds3ps2if.server = NULL;
    
    do {
        if (SifBindRpc(&ds3ps2if, DS3PS2_BIND_RPC_ID, 0) < 0) {
            return -1;
        }
        nopdelay();
    } while (!ds3ps2if.server);
    
    ds3ps2_initialized = 1;
    return 1;
}

int ds3ps2_set_led(u8 n)
{
    ds3ps2_buffer[0] = n;
    return SifCallRpc(&ds3ps2if, DS3PS2_SET_LED, 0,
        ds3ps2_buffer, 1, ds3ps2_buffer, 1, NULL, NULL);
}

int ds3ps2_set_rumble(u8 power_r, u8 time_r, u8 power_l, u8 time_l)
{
    ds3ps2_buffer[0] = power_r;
    ds3ps2_buffer[1] = time_r;
    ds3ps2_buffer[2] = power_l;
    ds3ps2_buffer[3] = time_l;
    return SifCallRpc(&ds3ps2if, DS3PS2_SET_RUMBLE, 0,
        ds3ps2_buffer, 4, ds3ps2_buffer, 4, NULL, NULL);
}

int ds3ps2_send_ledsrumble()
{
    return SifCallRpc(&ds3ps2if, DS3PS2_SEND_LEDSRUMBLE, 0,
        NULL, 0, NULL, 0, NULL, NULL);
}

int ds3ps2_get_input(u8 *buffer)
{
    return SifCallRpc(&ds3ps2if, DS3PS2_GET_INPUT, 0, buffer, 49, buffer, 49, NULL, NULL);
}
