#ifndef DS3PS2_H
#define DS3PS2_H

#define DS3PS2_BIND_RPC_ID 0xD2372200

#define DS3_VID 0x054C
#define DS3_PID 0x0268

#define DS3PS2_MAX_SLOTS 2
enum DS3PS2_SLOTS {
    DS3PS2_SLOT_1,
    DS3PS2_SLOT_2
};

enum ds3ps2_commands {
    DS3PS2_SET_LED,              //(slot, n)
    DS3PS2_SET_RUMBLE,           //(slot, power_r, time_r, power_l, time_l)
    DS3PS2_SEND_LEDSRUMBLE,      //(slot, void)
    DS3PS2_GET_INPUT,            //(slot, struct)
    DS3PS2_SLOT_CONNECTED        //(slot)
};

struct SS_BUTTONS
{
    u8 select   : 1;
    u8 L3       : 1;
    u8 R3       : 1;
    u8 start    : 1;
    u8 up       : 1;
    u8 right    : 1;
    u8 down     : 1;
    u8 left     : 1;


    u8 L2       : 1;
    u8 R2       : 1;
    u8 L1       : 1;
    u8 R1       : 1;
    u8 triangle : 1;
    u8 circle   : 1;
    u8 cross    : 1;
    u8 square   : 1;

    u8 PS       : 1;
    u8 not_used : 7;
};

struct SS_ANALOG
{
    u8 x;
    u8 y;
};

struct SS_DPAD_SENSITIVE
{
    u8 up;
    u8 right;
    u8 down;
    u8 left;
};

struct SS_SHOULDER_SENSITIVE
{
    u8 L2;
    u8 R2;
    u8 L1;
    u8 R1;
};

struct SS_BUTTON_SENSITIVE
{
    u8 triangle;
    u8 circle;
    u8 cross;
    u8 square;
};

struct SS_MOTION
{
    s16 acc_x;
    s16 acc_y;
    s16 acc_z;
    s16 z_gyro;
};

struct SS_GAMEPAD
{
    u8                             hid_data;
    u8                             unk0;
    struct SS_BUTTONS              buttons;
    u8                             unk1;
    struct SS_ANALOG               left_analog;
    struct SS_ANALOG               right_analog;
    u32                            unk2;
    struct SS_DPAD_SENSITIVE       dpad_sens;
    struct SS_SHOULDER_SENSITIVE   shoulder_sens;
    struct SS_BUTTON_SENSITIVE     button_sens;
    u16                            unk3;
    u8                             unk4;
    u8                             status;
    u8                             power_rating;
    u8                             comm_status;
    u32                            unk5;
    u32                            unk6;
    u8                             unk7;
    struct SS_MOTION               motion;
} __attribute__((packed, aligned(32)));

#define DS3PS2_INPUT_LEN (sizeof(struct SS_GAMEPAD))

#endif
