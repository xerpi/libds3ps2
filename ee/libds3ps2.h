#ifndef LIBDS3PS2_H
#define LIBDS3PS2_H

#include <kernel.h>
#include <ds3ps2.h>

#ifdef __cplusplus
extern "C" {
#endif


int ds3ps2_init();
int ds3ps2_slot_connected(int slot);
int ds3ps2_set_led(int slot, u8 n);
int ds3ps2_set_rumble(int slot, u8 power_r, u8 time_r, u8 power_l, u8 time_l);
int ds3ps2_send_ledsrumble(int slot);
int ds3ps2_get_input(int slot, u8 *buffer);


#ifdef __cplusplus
}
#endif

#endif
