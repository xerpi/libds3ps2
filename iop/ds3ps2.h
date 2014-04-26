#ifndef DS3PS2_H
#define DS3PS2_H

#define DS3PS2_BIND_RPC_ID 0xD2372200

#define DS3_VID 0x054C
#define DS3_PID 0x0268
#define DS3PS3_INPUT_LEN 49

enum ds3ps2_commands {
    DS3PS2_SET_LED,              //(n)
    DS3PS2_SET_RUMBLE,           //(power_r, time_r, power_l, time_l)
    DS3PS2_SEND_LEDSRUMBLE,      //(void)
    DS3PS2_GET_INPUT             //(struct)
};

#endif
