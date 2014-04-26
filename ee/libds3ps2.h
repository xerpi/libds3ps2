#ifndef LIBDS3PS2_H
#define LIBDS3PS2_H

#include <ds3ps2.h>
#include <kernel.h>

#ifdef __cplusplus
extern "C" {
#endif


int ds3ps2_init();
int ds3ps2_set_led(u8 n);
int ds3ps2_set_rumble(u8 power_r, u8 time_r, u8 power_l, u8 time_l);
int ds3ps2_send_ledsrumble();
int ds3ps2_get_input(u8 *buffer);



#ifdef __cplusplus
}
#endif

#endif
