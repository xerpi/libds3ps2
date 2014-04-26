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

u8 buffer[DS3PS3_INPUT_LEN] __attribute__((aligned(64)));

int main(void)
{
    u64 White, Black, FontColor, Red;
    GSGLOBAL *gsGlobal = gsKit_init_global();
    GSFONTM *gsFontM = gsKit_init_fontm();

    dmaKit_init(D_CTRL_RELE_OFF,D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
    D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);

    // Initialize the DMAC
    dmaKit_chan_init(DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_FROMSPR);
    dmaKit_chan_init(DMA_CHANNEL_TOSPR);

    Black = GS_SETREG_RGBAQ(0x00,0x00,0x00,0xFF,0x00);
    White = GS_SETREG_RGBAQ(0xFF,0xFF,0xFF,0x00,0x00);
    Red = GS_SETREG_RGBAQ(0xFF,0x00,0x00,0x00,0x00);
    FontColor = GS_SETREG_RGBAQ(0x8F,0x80,0x80,0x80,0x00);

    gsGlobal->PrimAlphaEnable = GS_SETTING_ON;
    gsKit_init_screen(gsGlobal);
    gsKit_fontm_upload(gsGlobal, gsFontM);
    gsFontM->Spacing = 0.95f;
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);
    
    
    SifLoadModule("mass:/ds3ps2.irx", 0, NULL);
    char text[16];
    
    ds3ps2_init();
    ds3ps2_set_led(1);
    ds3ps2_send_ledsrumble();

    int i;
    int led = 0;
    int cnt = 0;
    while (1) {
        gsKit_clear(gsGlobal, White);
        
        memset(buffer, 0x0, sizeof(buffer));
        ds3ps2_get_input(buffer);

        int x = 10, y = 50;
        for (i = 0; i < 64; ++i) {
            sprintf(text, " %X\n", buffer[i]);
            gsKit_fontm_print_scaled(gsGlobal, gsFontM, x, y, 3, 0.85f, FontColor, text);
            x += 56;
            if (i%10 == 0) {y += 56; x = 10;}
        }
    
        cnt++;
        if (cnt > 15) {
            led++;
            if (led > 7) led = 0;
            cnt = 0;
            ds3ps2_set_led(led);
            ds3ps2_send_ledsrumble();
        }
    
        char s[16];
        sprintf(s, "led: %i", led);
        gsKit_fontm_print_scaled(gsGlobal, gsFontM, 200, 50, 3, 0.85f, FontColor, s);
    
        gsKit_sync_flip(gsGlobal);
        gsKit_queue_exec(gsGlobal);
    }

    return 0;
}


