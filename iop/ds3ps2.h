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
	DS3PS2_SET_LED,			//(slot, n)
	DS3PS2_SET_RUMBLE,		//(slot, power_r, time_r, power_l, time_l)
	DS3PS2_SEND_LEDSRUMBLE,	//(slot, void)
	DS3PS2_GET_INPUT,		//(slot, struct)
	DS3PS2_SLOT_CONNECTED	//(slot)
};

struct ds3_input {
	unsigned char HID_data;
	unsigned char unk0;
	struct {
		unsigned char left   : 1;
		unsigned char down   : 1;
		unsigned char right  : 1;
		unsigned char up     : 1;
		unsigned char start  : 1;
		unsigned char R3     : 1;
		unsigned char L3     : 1;
		unsigned char select : 1;

		unsigned char square   : 1;
		unsigned char cross    : 1;
		unsigned char circle   : 1;
		unsigned char triangle : 1;
		unsigned char R1       : 1;
		unsigned char L1       : 1;
		unsigned char R2       : 1;
		unsigned char L2       : 1;

		unsigned char not_used : 7;
		unsigned char PS       : 1;
	};
	unsigned char unk1;
	unsigned char leftX;
	unsigned char leftY;
	unsigned char rightX;
	unsigned char rightY;

	unsigned int unk2;

	struct {
		unsigned char up_sens;
		unsigned char right_sens;
		unsigned char down_sens;
		unsigned char left_sens;
	};

	struct {
		unsigned char L2_sens;
		unsigned char R2_sens;
		unsigned char L1_sens;
		unsigned char R1_sens;
	};

	struct {
		unsigned char triangle_sens;
		unsigned char circle_sens;
		unsigned char cross_sens;
		unsigned char square_sens;
	};

	unsigned short unk3;
	unsigned char  unk4;

	unsigned char status;
	unsigned char power_rating;
	unsigned char comm_status;
	unsigned int  unk5;
	unsigned int  unk6;
	unsigned char unk7;
	struct {
		unsigned short accelX;
		unsigned short accelY;
		unsigned short accelZ;
		union {
			unsigned short gyroZ;
			unsigned short roll;
		};
	};
} __attribute__((packed, aligned(32)));

#define DS3PS2_INPUT_LEN (sizeof(struct ds3_input))

#endif
