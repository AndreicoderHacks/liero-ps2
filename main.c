#include "liero.h"

// TEST MINIMAL — doar gsKit init + dreptunghi rosu pe ecran
// Daca merge => problema e in game_init/world_generate
// Daca nu merge => problema e in gsKit init

int main(void) {
    SifInitRpc(0);

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    GSGLOBAL *gsGlobal        = gsKit_init_global();
    gsGlobal->Mode            = GS_MODE_NTSC;
    gsGlobal->Interlace       = GS_INTERLACED;
    gsGlobal->Field           = GS_FIELD;
    gsGlobal->Width           = 640;
    gsGlobal->Height          = 448;
    gsGlobal->PSM             = GS_PSM_CT32;
    gsGlobal->PSMZ            = GS_PSMZ_16;
    gsGlobal->ZBuffering      = GS_SETTING_OFF;
    gsGlobal->DoubleBuffering = GS_SETTING_ON;
    gsGlobal->PrimAlphaEnable = GS_SETTING_OFF;

    gsKit_init_screen(gsGlobal);
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);

    while (1) {
        gsKit_clear(gsGlobal, GS_SETREG_RGBAQ(0, 0, 40, 0x80, 0));

        // Dreptunghi rosu mare in centru
        gsKit_prim_sprite(gsGlobal,
            200.0f, 150.0f,
            440.0f, 300.0f,
            1, GS_SETREG_RGBAQ(255, 0, 0, 0x80, 0));

        gsKit_queue_exec(gsGlobal);
        gsKit_sync_flip(gsGlobal);
    }

    return 0;
}
