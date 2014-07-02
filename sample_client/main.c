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


#define DEG2RAD(x) ((x)*0.01745329251)
void draw_circle(GSGLOBAL *gsGlobal, float x, float y, float radius, u64 color, u8 filled);
int print_data(int y, struct SS_GAMEPAD *data);
void correct_data(struct SS_GAMEPAD *data);


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
    
    struct SS_GAMEPAD ds3_1, ds3_2;
    ds3ps2_init();
    
    float pos_x = gsGlobal->Width/2, pos_y = gsGlobal->Height/2;
    char text_connected[64];
    int led = 1, old_r1 = 0;
    
    while (!(ds3_1.buttons.PS && ds3_1.buttons.start)) {
        clear_screen();
        
        memset(&ds3_1, 0x0, sizeof(struct SS_GAMEPAD));
        ds3ps2_get_input(DS3PS2_SLOT_1, (void*)&ds3_1);
        correct_data(&ds3_1);
        
        memset(&ds3_2, 0x0, sizeof(struct SS_GAMEPAD));
        ds3ps2_get_input(DS3PS2_SLOT_2, (void*)&ds3_2);
        correct_data(&ds3_2);
        
        if (ds3_1.buttons.L1) {pos_x = gsGlobal->Width/2, pos_y = gsGlobal->Height/2;}
        if (ds3_1.buttons.R1 && !old_r1) {
            led++;
            if (led > 7) led = 0;
            ds3ps2_set_led(DS3PS2_SLOT_1, led);
            ds3ps2_send_ledsrumble(DS3PS2_SLOT_1);
        }
        old_r1 = ds3_1.buttons.R1;

        
        #define THRESHOLD 5.0f
        if (fabs(ds3_1.motion.acc_y) > THRESHOLD)
            pos_y -= ds3_1.motion.acc_y;
        if (fabs(ds3_1.motion.z_gyro) > THRESHOLD)
            pos_x -= ds3_1.motion.z_gyro/5.0f;
        
        draw_circle(gsGlobal, pos_x, pos_y, 19, CircleColor, 0);
        draw_circle(gsGlobal, pos_x, pos_y, 18, CircleColor, 0);
        draw_circle(gsGlobal, pos_x, pos_y, 17, CircleColor, 0);
        draw_circle(gsGlobal, pos_x, pos_y, 16, CircleColor, 0);
        
        sprintf(text_connected, "connected: SLOT_1 %i   SLOT_2 %i", ds3ps2_slot_connected(DS3PS2_SLOT_1),
            ds3ps2_slot_connected(DS3PS2_SLOT_2));
        font_print(5, 10, text_connected);
     
        int y = print_data(30, &ds3_1);
        print_data(y+10, &ds3_2);

        flip_screen();
    }

    return 0;
}

#define swap16(x) (((x&0xFF)<<8)|((x>>8)&0xFF))
#define zeroG 511.5
void correct_data(struct SS_GAMEPAD *data)
{
    data->motion.acc_x = swap16(data->motion.acc_x) - zeroG;
    data->motion.acc_y = swap16(data->motion.acc_y) - zeroG;
    data->motion.acc_z = swap16(data->motion.acc_z) - zeroG;
    data->motion.z_gyro = swap16(data->motion.z_gyro) - zeroG;
}

int print_data(int y, struct SS_GAMEPAD *data)
{ 
    char text[256];
    int x = 5;
    
    sprintf(text,"PS: %i   START: %i   SELECT: %i   /\\: %i   []: %i   O: %i   X: %i", \
            data->buttons.PS, data->buttons.start, data->buttons.select, data->buttons.triangle, \
            data->buttons.square, data->buttons.circle, data->buttons.cross);
    font_print(x, y+=30, text);

    sprintf(text,"L3: %i   R3: %i   L1: %i   L2: %i   R1: %i   R2: %i", \
             data->buttons.L3, data->buttons.R3, data->buttons.L1, data->buttons.L2, data->buttons.R1, data->buttons.R2);
    font_print(x, y+=30, text);

    sprintf(text,"UP: %i   DOWN: %i   RIGHT: %i   LEFT: %i   LX: %i   LY: %i   RX: %i   RY: %i", \
            data->buttons.up, data->buttons.down, data->buttons.right, data->buttons.left,
            data->left_analog.x, data->left_analog.y, data->right_analog.x, data->right_analog.y);
    font_print(x, y+=30, text);

    sprintf(text,"aX: %i   aY: %i   aZ: %i   Zgyro: %i", \
            data->motion.acc_x, data->motion.acc_y, data->motion.acc_z, data->motion.z_gyro);
    font_print(x, y+=30, text);

    sprintf(text,"L1 predata: %i   L2 predata: %i   R1 predata: %i   R2 predata: %i", \
            data->shoulder_sens.L1, data->shoulder_sens.L2, data->shoulder_sens.R1, data->shoulder_sens.R2);
    font_print(x, y+=30, text);

    sprintf(text,"/\\ predata: %i   [] predata: %i   O predata: %i   X predata: %i",
            data->button_sens.triangle, data->button_sens.square, data->button_sens.circle, data->button_sens.cross);
    font_print(x, y+=30, text);

    sprintf(text,"UP: %i   DOWN: %i   RIGHT: %i   LEFT: %i", \
            data->dpad_sens.up, data->dpad_sens.down, data->dpad_sens.right, data->dpad_sens.left);
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
    else        gsKit_prim_line_strip(gsGlobal, v, 37, 3, color);
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
