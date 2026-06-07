#include "liero.h"

static GameState gs;

void game_init(GameState *g) {
    memset(&g->state,      0, sizeof(int));
    memset(&g->volume,     0, sizeof(int));
    memset(&g->menu,       0, sizeof(MenuState));
    memset(&g->input,      0, sizeof(InputState));
    memset(&g->winner,     0, sizeof(int));
    memset(&g->tickCount,  0, sizeof(int));
    memset(&g->roundTimer, 0, sizeof(int));
    g->projectileCount = 0;
    g->particleCount   = 0;
    g->state           = STATE_MENU;
    g->volume          = 7;
    g->menu.phase      = MENU_PHASE_MAIN;
}

void game_start(GameState *g) {
    int i;
    memset(g->world.solid, 0, WORLD_W * WORLD_H);
    memset(g->world.color, 0, WORLD_W * WORLD_H);
    int solid_start = (WORLD_H / 2) * WORLD_W;
    int solid_size  = WORLD_W * (WORLD_H / 2);
    memset(&g->world.solid[solid_start], 1, solid_size);
    memset(&g->world.color[solid_start], 2, solid_size);
    memset(g->projectiles, 0, sizeof(g->projectiles));
    memset(g->particles,   0, sizeof(g->particles));
    g->projectileCount = 0;
    g->particleCount   = 0;
    g->tickCount       = 0;
    g->roundTimer      = ROUND_TIME;
    player_init(&g->players[0], 100, WORLD_H/2 - 20, 0);
    player_init(&g->players[1], 500, WORLD_H/2 - 20, 1);
    for (i = 0; i < WEAPONS_PER_PLAYER; i++) {
        g->players[0].weapons[i] = g->menu.selectedWeapons[0][i];
        g->players[1].weapons[i] = g->menu.selectedWeapons[1][i];
    }
    g->players[0].weaponCount    = WEAPONS_PER_PLAYER;
    g->players[1].weaponCount    = WEAPONS_PER_PLAYER;
    g->players[0].selectedWeapon = 0;
    g->players[1].selectedWeapon = 0;
    g->state = STATE_PLAYING;
}

void game_tick(GameState *g) {
    // Pauza
    if (input_pressed(g, 0, PAD_START) || input_pressed(g, 1, PAD_START)) {
        if (g->state == STATE_PLAYING) {
            g->state = STATE_PAUSED;
            g->menu.pauseCursor = 0;
            return;
        } else if (g->state == STATE_PAUSED) {
            g->state = STATE_PLAYING;
            return;
        } else if (g->state == STATE_ROUND_END) {
            g->state      = STATE_MENU;
            g->menu.phase = MENU_PHASE_MAIN;
            return;
        }
    }

    if (g->state == STATE_PAUSED) {
        pause_tick(g);
        return;
    }

    if (g->state == STATE_MENU) {
        menu_tick(g);
        return;
    }

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
}

int main(void) {
    SifInitRpc(0);

    // Exact ca Minicraft
    GSGLOBAL *gsGlobal        = gsKit_init_global();
    gsGlobal->Mode            = GS_MODE_NTSC;
    gsGlobal->Width           = SCREEN_W;
    gsGlobal->Height          = SCREEN_H;
    gsGlobal->PSM             = GS_PSM_CT32;
    gsGlobal->PSMZ            = GS_PSMZ_16;
    gsGlobal->ZBuffering      = GS_SETTING_OFF;
    gsGlobal->DoubleBuffering = GS_SETTING_ON;
    gsGlobal->PrimAlphaEnable = GS_SETTING_OFF;

    dmaKit_init(D_CTRL_RELE_OFF, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC,
                D_CTRL_STD_OFF, D_CTRL_RCYC_8, 1 << DMA_CHANNEL_GIF);
    dmaKit_chan_init(DMA_CHANNEL_GIF);

    gsKit_init_screen(gsGlobal);
    gsKit_mode_switch(gsGlobal, GS_ONESHOT);

    input_init();
    // sfx_init(); // dezactivat
    game_init(&gs);

    // Exact ca Minicraft — loop simplu fara switch
    while (1) {
        input_update(&gs);
        game_tick(&gs);
        game_render(&gs, gsGlobal);
        gsKit_queue_exec(gsGlobal);
        gsKit_sync_flip(gsGlobal);
    }

    return 0;
}
