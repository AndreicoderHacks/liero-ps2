#include "liero.h"

// ============================================================
//  MENU.C — Meniu principal, selectie arme, pauza
// ============================================================

static const char *weapon_list[WEAPON_COUNT] = {
    "PISTOL", "SHOTGUN", "BAZOOKA", "GRENADE", "MINIGUN",
};

// ------------------------------------------------------------
//  draw_weapon_select
// ------------------------------------------------------------
static void draw_weapon_select(GSGLOBAL *g, GameState *gs, int pidx) {
    MenuState *m = &gs->menu;
    int i;

    int bx = (pidx == 0) ? 40 : SCREEN_W/2 + 20;
    int by = 80;
    int bw = SCREEN_W/2 - 60;
    int bh = 260;

    draw_rect(g, bx, by, bw, bh, GS_SETREG_RGBAQ(20, 20, 40, 0x80, 0));
    draw_rect(g, bx,       by,       bw, 2,  COL_WHITE);
    draw_rect(g, bx,       by+bh-2,  bw, 2,  COL_WHITE);
    draw_rect(g, bx,       by,       2,  bh, COL_WHITE);
    draw_rect(g, bx+bw-2,  by,       2,  bh, COL_WHITE);

    u64 titleCol = (pidx == 0) ? COL_P1 : COL_P2;
    draw_text(g, (pidx == 0) ? "P1 ARME" : "P2 ARME", bx+10, by+8, titleCol);

    // Lista arme
    for (i = 0; i < WEAPON_COUNT; i++) {
        int iy = by + 30 + i * 22;
        int isCursor = (m->weaponCursor[pidx] == i);

        if (isCursor) {
            draw_rect(g, bx+8, iy-2, bw-16, 20,
                      GS_SETREG_RGBAQ(60, 60, 100, 0x80, 0));
            draw_text(g, ">", bx+8, iy, COL_YELLOW);
        }

        // Verde daca e selectata
        int isSelected = 0, j;
        for (j = 0; j < WEAPONS_PER_PLAYER; j++) {
            if (m->weaponSlotFilled[pidx][j] &&
                m->selectedWeapons[pidx][j] == i) {
                isSelected = 1; break;
            }
        }
        draw_text(g, weapon_list[i], bx+22, iy,
                  isSelected ? COL_GREEN : COL_WHITE);
    }

    // Sloturi selectate
    draw_text(g, "SELECTATE:", bx+10, by+148, COL_YELLOW);
    for (i = 0; i < WEAPONS_PER_PLAYER; i++) {
        int sx = bx + 10 + i * 52;
        int sy = by + 166;
        draw_rect(g, sx, sy, 48, 18, GS_SETREG_RGBAQ(40,40,60,0x80,0));
        if (m->weaponSlotFilled[pidx][i])
            draw_text(g, weapon_list[m->selectedWeapons[pidx][i]],
                      sx+2, sy+2, titleCol);
        else
            draw_text(g, "---", sx+2, sy+2, COL_GRAY);
    }

    // Status
    int filled = 0;
    for (i = 0; i < WEAPONS_PER_PLAYER; i++)
        if (m->weaponSlotFilled[pidx][i]) filled++;

    if (filled >= WEAPONS_PER_PLAYER)
        draw_text(g, "START = GATA!", bx+10, by+200, COL_GREEN);
    else {
        // Afisam cate mai trebuie
        char buf[] = "MAI ALEGE X";
        buf[9] = '0' + (WEAPONS_PER_PLAYER - filled);
        draw_text(g, buf, bx+10, by+200, COL_GRAY);
    }
}

// ------------------------------------------------------------
//  menu_render
// ------------------------------------------------------------
void menu_render(GSGLOBAL *g, GameState *gs) {
    MenuState *m = &gs->menu;

    gsKit_clear(g, GS_SETREG_RGBAQ(10, 10, 25, 0x80, 0));

    if (m->phase == MENU_PHASE_MAIN) {
        draw_text(g, "LIERO PS2",  SCREEN_W/2 - 54, 60,  COL_YELLOW);
        draw_text(g, "BY THY",     SCREEN_W/2 - 36, 82,  COL_GRAY);

        const char *opts[] = { "PLAY", "QUIT" };
        int i;
        for (i = 0; i < 2; i++) {
            int oy = 160 + i * 40;
            if (m->mainCursor == i) {
                draw_rect(g, SCREEN_W/2-70, oy-4, 140, 24,
                          GS_SETREG_RGBAQ(40,40,80,0x80,0));
                draw_text(g, ">", SCREEN_W/2-66, oy, COL_YELLOW);
            }
            draw_text(g, opts[i], SCREEN_W/2-36, oy,
                      m->mainCursor == i ? COL_WHITE : COL_GRAY);
        }
        draw_text(g, "UP/DOWN=NAVIGA  CROSS=SELECT",
                  SCREEN_W/2-168, SCREEN_H-30, COL_DARK_GRAY);

    } else if (m->phase == MENU_PHASE_P1_SELECT ||
               m->phase == MENU_PHASE_P2_SELECT) {
        int pidx = (m->phase == MENU_PHASE_P1_SELECT) ? 0 : 1;
        draw_text(g, (pidx==0) ? "P1 ALEGE ARMELE" : "P2 ALEGE ARMELE",
                  SCREEN_W/2-90, 20,
                  (pidx==0) ? COL_P1 : COL_P2);
        draw_text(g, "CROSS=ADAUGA  CIRCLE=STERGE  START=GATA",
                  40, 50, COL_GRAY);
        draw_weapon_select(g, gs, pidx);
    }
}

// ------------------------------------------------------------
//  menu_tick
// ------------------------------------------------------------
void menu_tick(GameState *gs) {
    MenuState *m = &gs->menu;

    if (m->phase == MENU_PHASE_MAIN) {
        if (input_pressed(gs, 0, PAD_UP))
            m->mainCursor = (m->mainCursor - 1 + 2) % 2;
        if (input_pressed(gs, 0, PAD_DOWN))
            m->mainCursor = (m->mainCursor + 1) % 2;

        if (input_pressed(gs, 0, PAD_CROSS)) {
            if (m->mainCursor == 0) {
                int p, i;
                for (p = 0; p < MAX_PLAYERS; p++) {
                    for (i = 0; i < WEAPONS_PER_PLAYER; i++) {
                        m->selectedWeapons[p][i]  = 0;
                        m->weaponSlotFilled[p][i] = 0;
                    }
                    m->weaponCursor[p] = 0;
                }
                m->phase = MENU_PHASE_P1_SELECT;
                sfx_play(gs, SFX_SELECT);
            }
        }

    } else if (m->phase == MENU_PHASE_P1_SELECT ||
               m->phase == MENU_PHASE_P2_SELECT) {

        int pidx = (m->phase == MENU_PHASE_P1_SELECT) ? 0 : 1;

        if (input_pressed(gs, pidx, PAD_UP)) {
            m->weaponCursor[pidx] = (m->weaponCursor[pidx] - 1 + WEAPON_COUNT) % WEAPON_COUNT;
            sfx_play(gs, SFX_NAVIGATE);
        }
        if (input_pressed(gs, pidx, PAD_DOWN)) {
            m->weaponCursor[pidx] = (m->weaponCursor[pidx] + 1) % WEAPON_COUNT;
            sfx_play(gs, SFX_NAVIGATE);
        }

        if (input_pressed(gs, pidx, PAD_CROSS)) {
            int filled = 0, i;
            for (i = 0; i < WEAPONS_PER_PLAYER; i++)
                if (m->weaponSlotFilled[pidx][i]) filled++;

            if (filled < WEAPONS_PER_PLAYER) {
                for (i = 0; i < WEAPONS_PER_PLAYER; i++) {
                    if (!m->weaponSlotFilled[pidx][i]) {
                        m->selectedWeapons[pidx][i]  = m->weaponCursor[pidx];
                        m->weaponSlotFilled[pidx][i] = 1;
                        sfx_play(gs, SFX_SELECT);
                        break;
                    }
                }
            }
        }

        if (input_pressed(gs, pidx, PAD_CIRCLE)) {
            int i;
            for (i = WEAPONS_PER_PLAYER - 1; i >= 0; i--) {
                if (m->weaponSlotFilled[pidx][i]) {
                    m->weaponSlotFilled[pidx][i] = 0;
                    sfx_play(gs, SFX_BACK);
                    break;
                }
            }
        }

        if (input_pressed(gs, pidx, PAD_START)) {
            int filled = 0, i;
            for (i = 0; i < WEAPONS_PER_PLAYER; i++)
                if (m->weaponSlotFilled[pidx][i]) filled++;

            if (filled >= WEAPONS_PER_PLAYER) {
                sfx_play(gs, SFX_SELECT);
                if (m->phase == MENU_PHASE_P1_SELECT)
                    m->phase = MENU_PHASE_P2_SELECT;
                else
                    game_start(gs);
            }
        }
    }
}

// ------------------------------------------------------------
//  pause_render
// ------------------------------------------------------------
void pause_render(GSGLOBAL *g, GameState *gs) {
    // Box pauza — NU facem gsKit_clear, jocul e in fundal
    int bx = SCREEN_W/2 - 100;
    int by = SCREEN_H/2 - 80;
    int bw = 200, bh = 160;

    draw_rect(g, bx,      by,      bw, bh, GS_SETREG_RGBAQ(10,10,30,0x80,0));
    draw_rect(g, bx,      by,      bw, 2,  COL_WHITE);
    draw_rect(g, bx,      by+bh-2, bw, 2,  COL_WHITE);
    draw_rect(g, bx,      by,      2,  bh, COL_WHITE);
    draw_rect(g, bx+bw-2, by,      2,  bh, COL_WHITE);

    draw_text(g, "PAUZA", SCREEN_W/2-30, by+8, COL_YELLOW);

    const char *opts[] = {
        "CONTINUA", "VOL +", "VOL -", "RESTART", "MENIU PRINCIPAL"
    };
    int i;
    for (i = 0; i < PAUSE_OPTS; i++) {
        int oy = by + 30 + i * 22;
        if (gs->menu.pauseCursor == i) {
            draw_rect(g, bx+4, oy-2, bw-8, 20,
                      GS_SETREG_RGBAQ(40,40,80,0x80,0));
            draw_text(g, ">", bx+6, oy, COL_YELLOW);
        }
        draw_text(g, opts[i], bx+20, oy,
                  gs->menu.pauseCursor == i ? COL_WHITE : COL_GRAY);
    }

    // Afisam volumul curent
    char vbuf[] = "VOL: X";
    vbuf[5] = '0' + gs->volume;
    draw_text(g, vbuf, bx+10, by+bh-20, COL_GRAY);
}

// ------------------------------------------------------------
//  pause_tick
// ------------------------------------------------------------
void pause_tick(GameState *gs) {
    MenuState *m = &gs->menu;

    if (input_pressed(gs, 0, PAD_UP) || input_pressed(gs, 1, PAD_UP)) {
        m->pauseCursor = (m->pauseCursor - 1 + PAUSE_OPTS) % PAUSE_OPTS;
        sfx_play(gs, SFX_NAVIGATE);
    }
    if (input_pressed(gs, 0, PAD_DOWN) || input_pressed(gs, 1, PAD_DOWN)) {
        m->pauseCursor = (m->pauseCursor + 1) % PAUSE_OPTS;
        sfx_play(gs, SFX_NAVIGATE);
    }

    if (input_pressed(gs, 0, PAD_CROSS) || input_pressed(gs, 1, PAD_CROSS)) {
        switch (m->pauseCursor) {
            case 0: gs->state = STATE_PLAYING; break;
            case 1: if (gs->volume < 9) { gs->volume++; sfx_set_volume(gs); } break;
            case 2: if (gs->volume > 0) { gs->volume--; sfx_set_volume(gs); } break;
            case 3:
                gs->state = STATE_PLAYING;
                game_start(gs);
                break;
            case 4:
                gs->state     = STATE_MENU;
                m->phase      = MENU_PHASE_MAIN;
                break;
        }
        sfx_play(gs, SFX_SELECT);
    }

    if (input_pressed(gs, 0, PAD_START) || input_pressed(gs, 1, PAD_START))
        gs->state = STATE_PLAYING;
}
