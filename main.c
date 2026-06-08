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
    // FIX: world_generate in loc de teren plat
    world_generate(&g->world);
    memset(g->projectiles, 0, sizeof(g->projectiles));
    memset(g->particles,   0, sizeof(g->particles));
    g->projectileCount = 0;
    g->particleCount   = 0;
    g->tickCount       = 0;
    g->roundTimer      = ROUND_TIME;

    // Spawn jucatori deasupra terenului
    player_init(&g->players[0], 120, 40, 0);
    player_init(&g->players[1], 520, 40, 1);

    // FIX: copiem armele din meniu corect
    for (i = 0; i < WEAPONS_PER_PLAYER; i++) {
        g->players[0].weapons[i] = g->menu.selectedWeapons[0][i];
        g->players[1].weapons[i] = g->menu.selectedWeapons[1][i];
    }
    g->players[0].weaponCount    = WEAPONS_PER_PLAYER;
    g->players[1].weaponCount    = WEAPONS_PER_PLAYER;
    // FIX: selectedWeapon = index in weapons[], nu weapon ID
    g->players[0].selectedWeapon = 0;
    g->players[1].selectedWeapon = 0;

    g->state = STATE_PLAYING;
}

void game_tick(GameState *g) {
    // FIX: START nu face nimic in meniu
    if (g->state == STATE_MENU) {
        menu_tick(g);
        return;
    }

    if (g->state == STATE_PAUSED) {
        // FIX: START din pauza = continua
        if (input_pressed(g, 0, PAD_START) || input_pressed(g, 1, PAD_START)) {
            g->state = STATE_PLAYING;
            return;
        }
        pause_tick(g);
        return;
    }

    if (g->state == STATE_ROUND_END) {
        if (input_pressed(g, 0, PAD_START) || input_pressed(g, 1, PAD_START)) {
            g->state      = STATE_MENU;
            g->menu.phase = MENU_PHASE_MAIN;
        }
        return;
    }

    if (g->state != STATE_PLAYING) return;

    // FIX: START in joc = pauza
    if (input_pressed(g, 0, PAD_START) || input_pressed(g, 1, PAD_START)) {
        g->state = STATE_PAUSED;
        g->menu.pauseCursor = 0;
        return;
    }

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
    // sfx_init dezactivat — cauzeaza crash
    game_init(&gs);

    while (1) {
        input_update(&gs);
        game_tick(&gs);
        game_render(&gs, gsGlobal);
        gsKit_queue_exec(gsGlobal);
        gsKit_sync_flip(gsGlobal);
    }

    return 0;
}
