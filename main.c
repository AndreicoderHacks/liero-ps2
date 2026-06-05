#include "liero.h"

static GameState gs;

// ------------------------------------------------------------
//  game_init — initializare completa (prima data)
// ------------------------------------------------------------
void game_init(GameState *g) {
    memset(g, 0, sizeof(GameState));
    g->state        = STATE_MENU;
    g->volume       = 7;
    g->menu.phase   = MENU_PHASE_MAIN;
}

// ------------------------------------------------------------
//  game_start — porneste runda cu armele alese din meniu
// ------------------------------------------------------------
void game_start(GameState *g) {
    int i;

    // Teren plat simplu
    int x, y;
    memset(&g->world, 0, sizeof(World));
    for (y = 0; y < WORLD_H; y++) {
        for (x = 0; x < WORLD_W; x++) {
            if (y > WORLD_H / 2) {
                g->world.solid[y * WORLD_W + x] = 1;
                g->world.color[y * WORLD_W + x] = 2;
            }
        }
    }

    // Spawn jucatori
    player_init(&g->players[0], 100, WORLD_H/2 - 20, 0);
    player_init(&g->players[1], 500, WORLD_H/2 - 20, 1);

    // Copiem armele alese din meniu in player
    for (i = 0; i < WEAPONS_PER_PLAYER; i++) {
        g->players[0].weapons[i] = g->menu.selectedWeapons[0][i];
        g->players[1].weapons[i] = g->menu.selectedWeapons[1][i];
    }
    g->players[0].weaponCount    = WEAPONS_PER_PLAYER;
    g->players[1].weaponCount    = WEAPONS_PER_PLAYER;
    g->players[0].selectedWeapon = 0;
    g->players[1].selectedWeapon = 0;

    g->projectileCount = 0;
    g->particleCount   = 0;
    g->tickCount       = 0;
    g->roundTimer      = ROUND_TIME;
    g->state           = STATE_PLAYING;
}

// ------------------------------------------------------------
//  game_tick
// ------------------------------------------------------------
void game_tick(GameState *g) {
    if (g->state != STATE_PLAYING) return;

    g->tickCount++;
    player_tick(g, 0);
    player_tick(g, 1);
    projectile_tickAll(g);
    particle_tickAll(g);

    if (g->roundTimer > 0) g->roundTimer--;
    else g->state = STATE_ROUND_END;

    if (!g->players[0].alive) { g->winner = 1; g->state = STATE_ROUND_END; }
    if (!g->players[1].alive) { g->winner = 0; g->state = STATE_ROUND_END; }

    // Start = pauza in joc
    if (input_pressed(g, 0, PAD_START) || input_pressed(g, 1, PAD_START)) {
        g->state = STATE_PAUSED;
        g->menu.pauseCursor = 0;
    }
}

// ------------------------------------------------------------
//  main
// ------------------------------------------------------------
int main(void) {
    SifInitRpc(0);

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    GSGLOBAL *gsGlobal        = gsKit_init_global();
    gsGlobal->Mode            = GS_MODE_NTSC;
    gsGlobal->Interlace       = GS_INTERLACED;
    gsGlobal->Field           = GS_FIELD;
    gsGlobal->Width           = SCREEN_W;
    gsGlobal->Height          = SCREEN_H;
    gsGlobal->PSM             = GS_PSM_CT32;
    gsGlobal->PSMZ            = GS_PSMZ_16;
    gsGlobal->ZBuffering      = GS_SETTING_OFF;
    gsGlobal->DoubleBuffering = GS_SETTING_ON;
    gsGlobal->PrimAlphaEnable = GS_SETTING_OFF;

    gsKit_init_screen(gsGlobal);
    gsKit_mode_switch(gsGlobal, GS_PERSISTENT);

    input_init();
    sfx_init();
    game_init(&gs);

    while (1) {
        input_update(&gs);

        switch (gs.state) {
            case STATE_MENU:
                menu_tick(&gs);
                menu_render(gsGlobal, &gs);
                break;

            case STATE_PLAYING:
                game_tick(&gs);
                game_render(&gs, gsGlobal);
                break;

            case STATE_PAUSED:
                pause_tick(&gs);
                game_render(&gs, gsGlobal);   // jocul in fundal
                pause_render(gsGlobal, &gs);
                break;

            case STATE_ROUND_END:
                game_render(&gs, gsGlobal);
                // Start = inapoi la meniu
                if (input_pressed(&gs, 0, PAD_START) ||
                    input_pressed(&gs, 1, PAD_START)) {
                    gs.state      = STATE_MENU;
                    gs.menu.phase = MENU_PHASE_MAIN;
                }
                break;
        }

        gsKit_queue_exec(gsGlobal);
        gsKit_sync_flip(gsGlobal);
    }

    return 0;
}
