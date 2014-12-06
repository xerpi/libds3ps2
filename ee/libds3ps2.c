#include <kernel.h>
#include <sifrpc.h>
#include "libds3ps2.h"

static SifRpcClientData_t ds3ps2if __attribute__((aligned(64)));
static u8 ds3ps2_buffer[5] __attribute__((aligned(64)));
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

int ds3ps2_slot_connected(int slot)
{
	unsigned char connected = slot;
	SifCallRpc(&ds3ps2if, DS3PS2_SLOT_CONNECTED, 0,
		&connected, 1, &connected, 1, NULL, NULL);
	return connected;
}

int ds3ps2_set_led(int slot, u8 n)
{
	ds3ps2_buffer[0] = slot;
	ds3ps2_buffer[1] = n;
	return SifCallRpc(&ds3ps2if, DS3PS2_SET_LED, 0,
		ds3ps2_buffer, 2, NULL, 0, NULL, NULL);
}

int ds3ps2_set_rumble(int slot, u8 power_r, u8 time_r, u8 power_l, u8 time_l)
{
	ds3ps2_buffer[0] = slot;
	ds3ps2_buffer[1] = power_r;
	ds3ps2_buffer[2] = time_r;
	ds3ps2_buffer[3] = power_l;
	ds3ps2_buffer[4] = time_l;
	return SifCallRpc(&ds3ps2if, DS3PS2_SET_RUMBLE, 0,
		ds3ps2_buffer, 5, NULL, 0, NULL, NULL);
}

int ds3ps2_send_ledsrumble(int slot)
{
	ds3ps2_buffer[0] = slot;
	return SifCallRpc(&ds3ps2if, DS3PS2_SEND_LEDSRUMBLE, 0,
		ds3ps2_buffer, 1, NULL, 0, NULL, NULL);
}

int ds3ps2_get_input(int slot, u8 *buffer)
{
	buffer[0] = slot;
	return SifCallRpc(&ds3ps2if, DS3PS2_GET_INPUT, 0, buffer, 49, buffer, 49, NULL, NULL);
}
