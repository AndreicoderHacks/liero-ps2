#include "liero.h"

static GameState gs;

void game_init(GameState *g) {
    memset(&g->input, 0, sizeof(InputState));
    g->state  = STATE_MENU;
    g->volume = 7;
}

void game_start(GameState *g) { (void)g; }
void game_tick(GameState *g)  { (void)g; }

int main(void) {
    SifInitRpc(0);

    GSGLOBAL *g        = gsKit_init_global();
    g->Mode            = GS_MODE_NTSC;
    g->Width           = 640;
    g->Height          = 448;
    g->PSM             = GS_PSM_CT32;
    g->PSMZ            = GS_PSMZ_16;
    g->ZBuffering      = GS_SETTING_OFF;
    g->DoubleBuffering = GS_SETTING_ON;
    g->PrimAlphaEnable = GS_SETTING_OFF;

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    gsKit_init_screen(g);
    gsKit_mode_switch(g, GS_ONESHOT);

    input_init();
    game_init(&gs);

    int frame = 0;

    while (1) {
        input_update(&gs);
        frame++;

        gsKit_clear(g, GS_SETREG_RGBAQ(10, 10, 30, 0x80, 0));

        // ---- Cub care flicare rosu/alb la fiecare 30 frame-uri ----
        u64 cubCol = (frame / 30) % 2 == 0
            ? GS_SETREG_RGBAQ(255, 0, 0, 0x80, 0)
            : GS_SETREG_RGBAQ(255, 255, 255, 0x80, 0);
        gsKit_prim_sprite(g, 8.0f, 8.0f, 28.0f, 28.0f, 1, cubCol);

        // ---- Afisam valoarea raw hex a butoanelor pad 0 ----
        u32 raw = gs.input.current[0];

        // Titlu
        draw_text(g, "PAD0 RAW:", 40, 20, GS_SETREG_RGBAQ(255,255,0,0x80,0));

        // Afisam fiecare bit ca un patrat mic
        // 16 butoane = 16 patrate
        int b;
        for (b = 0; b < 16; b++) {
            int bx = 40 + b * 22;
            int by = 50;
            u64 bc = (raw & (1 << b))
                ? GS_SETREG_RGBAQ(0, 255, 0, 0x80, 0)   // verde = apasat
                : GS_SETREG_RGBAQ(80, 80, 80, 0x80, 0);  // gri = liber
            gsKit_prim_sprite(g, (float)bx, (float)by,
                                 (float)(bx+18), (float)(by+18), 1, bc);
        }

        // Label butoane
        const char *labels[] = {
            "SEL","L3","R3","STA","UP","RT","DN","LT",
            "L2","R2","L1","R1","TRI","CIR","CRO","SQR"
        };
        for (b = 0; b < 16; b++) {
            int bx = 40 + b * 22;
            draw_text(g, labels[b], bx, 72,
                      GS_SETREG_RGBAQ(180,180,180,0x80,0));
        }

        // Valoare numerica hex
        // Afisam raw ca 4 cifre hex
        char hex[10];
        hex[0] = '0'; hex[1] = 'x';
        hex[2] = "0123456789ABCDEF"[(raw >> 12) & 0xF];
        hex[3] = "0123456789ABCDEF"[(raw >>  8) & 0xF];
        hex[4] = "0123456789ABCDEF"[(raw >>  4) & 0xF];
        hex[5] = "0123456789ABCDEF"[(raw >>  0) & 0xF];
        hex[6] = 0;
        draw_text(g, hex, 40, 100,
                  GS_SETREG_RGBAQ(255, 200, 0, 0x80, 0));

        // Pad state
        int state = padGetState(0, 0);
        char st[4];
        st[0] = 'S'; st[1] = ':';
        st[2] = '0' + (state % 10);
        st[3] = 0;
        draw_text(g, st, 40, 120,
                  GS_SETREG_RGBAQ(100, 200, 255, 0x80, 0));

        // Frame counter
        char fc[8];
        fc[0] = 'F'; fc[1] = ':';
        fc[2] = '0' + ((frame / 100) % 10);
        fc[3] = '0' + ((frame /  10) % 10);
        fc[4] = '0' + ((frame      ) % 10);
        fc[5] = 0;
        draw_text(g, fc, 40, 140,
                  GS_SETREG_RGBAQ(150, 150, 150, 0x80, 0));

        gsKit_queue_exec(g);
        gsKit_sync_flip(g);
    }

    return 0;
}
