#include <gsKit.h>
#include <dmaKit.h>
#include <malloc.h>
#include <stdio.h>
#include <math.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <gsToolkit.h>
#include <libds3ps2.h>

void video_init();
void clear_screen();
void flip_screen();
GSGLOBAL *gsGlobal;
GSFONTM *gsFontM;
u64 White, Black, FontColor, Red, Blue, CircleColor;
#define font_print(x, y, text) \
	gsKit_fontm_print_scaled(gsGlobal, gsFontM, x, y, 3, 0.5f, FontColor, text)

#define DEG2RAD(x) ((x)*0.01745329251)
void draw_circle(GSGLOBAL *gsGlobal, float x, float y, float radius, u64 color, u8 filled);
int print_data(int y, struct ds3_input *data);
void correct_data(struct ds3_input *data);


int main(void)
{
	video_init();
	int ret = SifLoadModule("mass:/ds3ps2.irx", 0, NULL);
	if (ret < 0) {
		char *txt = "Could not find 'mass:/ds3ps2.irx'";
		while (1) {
			clear_screen();
			font_print(5, 10, txt);
			flip_screen();
		}
	}
	struct ds3_input ds3_1, ds3_2;
	memset(&ds3_1, 0x0, sizeof(struct ds3_input));
	memset(&ds3_2, 0x0, sizeof(struct ds3_input));
	ds3ps2_init();
	float pos_x = gsGlobal->Width/2, pos_y = gsGlobal->Height/2;
	char text_connected[64];
	int led = 1, old_r1 = 0;
	while (!(ds3_1.PS && ds3_1.start)) {
		clear_screen();

		ds3ps2_get_input(DS3PS2_SLOT_1, (void*)&ds3_1);
		ds3ps2_get_input(DS3PS2_SLOT_2, (void*)&ds3_2);
			if (ds3_1.L1) {pos_x = gsGlobal->Width/2, pos_y = gsGlobal->Height/2;}
		if (ds3_1.R1 && !old_r1) {
			led++;
			if (led > 7) led = 0;
			ds3ps2_set_led(DS3PS2_SLOT_1, led);
			ds3ps2_send_ledsrumble(DS3PS2_SLOT_1);
		}
		old_r1 = ds3_1.R1;

			#define THRESHOLD 5.0f
		if (fabs(ds3_1.accelY) > THRESHOLD)
			pos_y -= ds3_1.accelY;
		if (fabs(ds3_1.gyroZ) > THRESHOLD)
			pos_x -= ds3_1.gyroZ/5.0f;
			draw_circle(gsGlobal, pos_x, pos_y, 19, CircleColor, 0);
		draw_circle(gsGlobal, pos_x, pos_y, 18, CircleColor, 0);
		draw_circle(gsGlobal, pos_x, pos_y, 17, CircleColor, 0);
		draw_circle(gsGlobal, pos_x, pos_y, 16, CircleColor, 0);
			sprintf(text_connected, "connected: SLOT_1 %i	SLOT_2 %i", ds3ps2_slot_connected(DS3PS2_SLOT_1),
			ds3ps2_slot_connected(DS3PS2_SLOT_2));
		font_print(5, 10, text_connected);
	
		int y = print_data(30, &ds3_1);
		print_data(y+10, &ds3_2);

		flip_screen();
	}

	return 0;
}

int print_data(int y, struct ds3_input *data)
{
	char text[256];
	int x = 5;
	sprintf(text,"PS: %01X  START: %01X  SELECT: %01X  /\\: %01X  []: %01X  O: %01X  X: %01X", \
			data->PS, data->start, data->select, data->triangle, \
			data->square, data->circle, data->cross);
	font_print(x, y+=30, text);

	sprintf(text,"L3: %01X  R3: %01X  L1: %01X  L2: %01X  R1: %01X  R2: %01X", \
			 data->L3, data->R3, data->L1, data->L2, data->R1, data->R2);
	font_print(x, y+=30, text);

	sprintf(text,"UP: %i  DOWN: %i  RIGHT: %i  LEFT: %i LX: %02X  LY: %02X  RX: %02X  RY: %02X", \
			data->up, data->down, data->right, data->left,
			data->leftX, data->leftY, data->rightX, data->rightY);
	font_print(x, y+=30, text);

	sprintf(text,"aX: %04X  aY: %04X  aZ: %04X  Zgyro: %04X", \
			data->accelX, data->accelY, data->accelZ, data->gyroZ);
	font_print(x, y+=30, text);

	sprintf(text,"L1 predata: %02X  L2 predata: %02X  R1 predata: %02X  R2 predata: %02X", \
			data->L1_sens, data->L2_sens, data->R1_sens, data->R2_sens);
	font_print(x, y+=30, text);

	sprintf(text,"/\\ predata: %02X  [] predata: %02X  O predata: %02X  X predata: %02X",
			data->triangle_sens, data->square_sens, data->circle_sens, data->cross_sens);
	font_print(x, y+=30, text);

	sprintf(text,"UP: %02X DOWN: %02X  RIGHT: %02X  LEFT: %02X", \
			data->up_sens, data->down_sens, data->right_sens, data->left_sens);
	font_print(x, y+=30, text);
	return y;
}

void draw_circle(GSGLOBAL *gsGlobal, float x, float y, float radius, u64 color, u8 filled)
{	
	float v[37*2];
	int a;
	float ra;
	for (a = 0; a < 36; a++) {
		ra = DEG2RAD(a*10);
		v[a*2] = cos(ra) * radius + x;
		v[a*2+1] = sin(ra) * radius + y;
	}
	if (!filled) {
		v[72] = radius + x;
		v[73] = y;
	}
	if (filled) gsKit_prim_triangle_fan(gsGlobal, v, 36, 3, color);
	else		gsKit_prim_line_strip(gsGlobal, v, 37, 3, color);
}

void video_init()
{
	gsGlobal = gsKit_init_global();
	gsFontM = gsKit_init_fontm();

	dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
	D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);

	// Initialize the DMAC
	dmaKit_chan_init(DMA_CHANNEL_GIF);
	dmaKit_chan_init(DMA_CHANNEL_FROMSPR);
	dmaKit_chan_init(DMA_CHANNEL_TOSPR);

	Black = GS_SETREG_RGBAQ(0x00,0x00,0x00,0xFF,0x00);
	White = GS_SETREG_RGBAQ(0xFF,0xFF,0xFF,0x00,0x00);
	Red = CircleColor = GS_SETREG_RGBAQ(0xFF,0x00,0x00,0x00,0x00);
	Blue = GS_SETREG_RGBAQ(0x00,0x00,0xFF,0x00,0x00);
	FontColor = GS_SETREG_RGBAQ(0x2F,0x20,0x20,0xFF,0x00);

	gsGlobal->PrimAlphaEnable = GS_SETTING_ON;
	gsKit_init_screen(gsGlobal);
	gsKit_fontm_upload(gsGlobal, gsFontM);
	gsFontM->Spacing = 0.75f;
	gsKit_mode_switch(gsGlobal, GS_ONESHOT);
}

void clear_screen()
{
	gsKit_clear(gsGlobal, White);
}

void flip_screen()
{
	gsKit_sync_flip(gsGlobal);
	gsKit_queue_exec(gsGlobal);
}
