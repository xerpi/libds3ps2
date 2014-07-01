#ifndef DS3PS2_H
#define DS3PS2_H

#define DS3PS2_BIND_RPC_ID 0xD2372200

#define DS3_VID 0x054C
#define DS3_PID 0x0268
#define DS3PS3_INPUT_LEN 49

#define DS3PS3_MAX_SLOTS 2
enum DS3PS3_SLOTS {
    DS3PS3_SLOT_1,
    DS3PS3_SLOT_2
};

enum ds3ps2_commands {
    DS3PS2_SET_LED,              //(slot, n)
    DS3PS2_SET_RUMBLE,           //(slot, power_r, time_r, power_l, time_l)
    DS3PS2_SEND_LEDSRUMBLE,      //(slot, void)
    DS3PS2_GET_INPUT,            //(slot, struct)
    DS3PS2_SLOT_CONNECTED        //(slot)
};

#endif
