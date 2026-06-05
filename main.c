#include "liero.h"

// ============================================================
//  LIERO PS2 - main.c
// ============================================================

static GameState gs;

void game_init(GameState *g) {
    memset(g, 0, sizeof(GameState));
    world_generate(&g->world);
    player_init(&g->players[0], 100, 100, 0);
    player_init(&g->players[1], 500, 100, 1);
    g->players[0].selectedWeapon = WEAPON_PISTOL;
    g->players[1].selectedWeapon = WEAPON_PISTOL;
    g->projectileCount = 0;
    g->particleCount   = 0;
    g->tickCount       = 0;
    g->state           = STATE_PLAYING;
    g->roundTimer      = ROUND_TIME;
}

void game_tick(GameState *g) {
    if (g->state != STATE_PLAYING) return;
    g->tickCount++;
    player_tick(g, 0);
    player_tick(g, 1);
    projectile_tickAll(g);
    particle_tickAll(g);
    if (g->roundTimer > 0) {
        g->roundTimer--;
    } else {
        g->state = STATE_ROUND_END;
    }
    if (!g->players[0].alive) { g->winner = 1; g->state = STATE_ROUND_END; }
    if (!g->players[1].alive) { g->winner = 0; g->state = STATE_ROUND_END; }
}

int main(void) {
    SifInitRpc(0);

    // DMA init PRIMA — obligatoriu pe hardware real
    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    // Init GS cu Interlace + Field — fix pentru PS2 real
    GSGLOBAL *gsGlobal        = gsKit_init_global();
    gsGlobal->Mode            = GS_MODE_NTSC;
    gsGlobal->Interlace       = GS_INTERLACE_ON;
    gsGlobal->Field           = GS_FIELD_EVEN;
    gsGlobal->Width           = SCREEN_W;
    gsGlobal->Height          = SCREEN_H;
    gsGlobal->PSM             = GS_PSM_CT32;
    gsGlobal->PSMZ            = GS_PSMZ_16;
    gsGlobal->ZBuffering      = GS_SETTING_OFF;
    gsGlobal->DoubleBuffering = GS_SETTING_ON;
    gsGlobal->PrimAlphaEnable = GS_SETTING_ON;

    gsKit_init_screen(gsGlobal);
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);

    input_init();
    game_init(&gs);

    while (1) {
        input_update(&gs);

        if (gs.state == STATE_ROUND_END) {
            if (input_pressed(&gs, 0, PAD_START) ||
                input_pressed(&gs, 1, PAD_START)) {
                game_init(&gs);
            }
        }

        game_tick(&gs);
        game_render(&gs, gsGlobal);

        gsKit_queue_exec(gsGlobal);
        gsKit_sync_flip(gsGlobal);
    }

    return 0;
}
