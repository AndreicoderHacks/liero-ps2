#include "liero.h"

// ============================================================
//  LIERO PS2 - main.c
//  Game init, main loop, tick dispatch
// ============================================================

static GameState gs;

// ------------------------------------------------------------
//  game_init
// ------------------------------------------------------------
void game_init(GameState *g) {
    memset(g, 0, sizeof(GameState));

    // Generare teren
    world_generate(&g->world);

    // Spawn jucatori
    player_init(&g->players[0], 100, 100, 0);  // P1 - stanga
    player_init(&g->players[1], 500, 100, 1);  // P2 - dreapta

    // Arme default: ambii incep cu pistol
    g->players[0].selectedWeapon = WEAPON_PISTOL;
    g->players[1].selectedWeapon = WEAPON_PISTOL;

    g->projectileCount = 0;
    g->particleCount   = 0;
    g->tickCount       = 0;
    g->state           = STATE_PLAYING;
    g->roundTimer      = ROUND_TIME;
}

// ------------------------------------------------------------
//  game_tick
// ------------------------------------------------------------
void game_tick(GameState *g) {
    if (g->state != STATE_PLAYING) return;

    g->tickCount++;

    // Tick jucatori
    player_tick(g, 0);
    player_tick(g, 1);

    // Tick proiectile
    projectile_tickAll(g);

    // Tick particule
    particle_tickAll(g);

    // Timer runda
    if (g->roundTimer > 0) {
        g->roundTimer--;
    } else {
        g->state = STATE_ROUND_END;
    }

    // Check win condition
    if (!g->players[0].alive) {
        g->winner = 1;
        g->state  = STATE_ROUND_END;
    }
    if (!g->players[1].alive) {
        g->winner = 0;
        g->state  = STATE_ROUND_END;
    }
}

// ------------------------------------------------------------
//  main
// ------------------------------------------------------------
int main(void) {
    SifInitRpc(0);

    // Init GS
    GSGLOBAL *gsGlobal = gsKit_init_global();
    gsGlobal->Mode            = GS_MODE_NTSC;
    gsGlobal->Width           = SCREEN_W;
    gsGlobal->Height          = SCREEN_H;
    gsGlobal->PSM             = GS_PSM_CT32;
    gsGlobal->PSMZ            = GS_PSMZ_16;
    gsGlobal->ZBuffering      = GS_SETTING_OFF;
    gsGlobal->DoubleBuffering = GS_SETTING_ON;
    gsGlobal->PrimAlphaEnable = GS_SETTING_ON;

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    gsKit_init_screen(gsGlobal);
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);

    input_init();
    game_init(&gs);

    while (1) {
        input_update(&gs);

        // R pe ambii jucatori = restart runda
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
