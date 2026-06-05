#include "liero.h"

// ============================================================
//  MENU.C — Meniu principal, selectie arme, pauza
// ============================================================

// Lista completa de arme disponibile pentru selectie
static const char *weapon_list[WEAPON_COUNT] = {
    "PISTOL",
    "SHOTGUN",
    "BAZOOKA",
    "GRENADE",
    "MINIGUN",
};

// ------------------------------------------------------------
//  Desenam o casuta de selectie arme pentru un jucator
// ------------------------------------------------------------
static void draw_weapon_select(GSGLOBAL *g, GameState *gs, int playerIdx) {
    MenuState *m = &gs->menu;
    int i;

    // Background
    int bx = (playerIdx == 0) ? 40 : SCREEN_W/2 + 20;
    int by = 80;
    int bw = SCREEN_W/2 - 60;
    int bh = 300;

    draw_rect(g, bx, by, bw, bh, GS_SETREG_RGBAQ(20, 20, 40, 0x80, 0));
    draw_rect(g, bx, by, bw, 2,  COL_WHITE);
    draw_rect(g, bx, by+bh-2, bw, 2, COL_WHITE);
    draw_rect(g, bx, by, 2, bh, COL_WHITE);
    draw_rect(g, bx+bw-2, by, 2, bh, COL_WHITE);

    // Titlu
    u64 titleCol = (playerIdx == 0) ? COL_P1 : COL_P2;
    char title[16];
    if (playerIdx == 0) {
        title[0]='P'; title[1]='1'; title[2]=' ';
        title[3]='A'; title[4]='R'; title[5]='M'; title[6]='E';
        title[7]='\0';
    } else {
        title[0]='P'; title[1]='2'; title[2]=' ';
        title[3]='A'; title[4]='R'; title[5]='M'; title[6]='E';
        title[7]='\0';
    }
    draw_text(g, title, bx + 10, by + 8, titleCol);

    // Lista arme disponibile
    for (i = 0; i < WEAPON_COUNT; i++) {
        int ix = bx + 14;
        int iy = by + 30 + i * 20;

        // Cursor
        int isCursor = (m->weaponCursor[playerIdx] == i);
        if (isCursor) {
            draw_rect(g, bx + 8, iy - 2, bw - 16, 18,
                      GS_SETREG_RGBAQ(60, 60, 100, 0x80, 0));
            draw_text(g, ">", bx + 8, iy, COL_YELLOW);
        }

        // Arma selectata sau nu
        int isSelected = 0;
        int j;
        for (j = 0; j < WEAPONS_PER_PLAYER; j++) {
            if (m->selectedWeapons[playerIdx][j] == i &&
                m->weaponSlotFilled[playerIdx][j]) {
                isSelected = 1;
                break;
            }
        }

        u64 col = isSelected ? COL_GREEN : COL_WHITE;
        draw_text(g, weapon_list[i], ix, iy, col);
    }

    // Sloturi selectate (jos)
    draw_text(g, "SELECTATE:", bx + 10, by + 150, COL_YELLOW);
    for (i = 0; i < WEAPONS_PER_PLAYER; i++) {
        int sx = bx + 10 + i * 56;
        int sy = by + 168;
        draw_rect(g, sx, sy, 50, 18,
                  GS_SETREG_RGBAQ(40, 40, 60, 0x80, 0));
        if (m->weaponSlotFilled[playerIdx][i]) {
            draw_text(g, weapon_list[m->selectedWeapons[playerIdx][i]],
                      sx + 2, sy + 2, titleCol);
        } else {
            draw_text(g, "---", sx + 2, sy + 2, COL_GRAY);
        }
    }

    // Numar selectate
    int filled = 0;
    for (i = 0; i < WEAPONS_PER_PLAYER; i++)
        if (m->weaponSlotFilled[playerIdx][i]) filled++;

    if (filled >= WEAPONS_PER_PLAYER) {
        draw_text(g, "START >", bx + 10, by + 200, COL_GREEN);
    } else {
        draw_text(g, "ALEGE 5 ARME", bx + 10, by + 200, COL_GRAY);
    }
}

// ------------------------------------------------------------
//  menu_render
// ------------------------------------------------------------
void menu_render(GSGLOBAL *g, GameState *gs) {
    MenuState *m = &gs->menu;

    gsKit_clear(g, GS_SETREG_RGBAQ(10, 10, 25, 0x80, 0));

    if (m->phase == MENU_PHASE_MAIN) {
        // Titlu
        draw_text(g, "LIERO PS2", SCREEN_W/2 - 60, 60, COL_YELLOW);
        draw_text(g, "BY THY",    SCREEN_W/2 - 36, 82, COL_GRAY);

        // Optiuni meniu
        const char *opts[] = { "PLAY", "QUIT" };
        int i;
        for (i = 0; i < 2; i++) {
            int oy = 160 + i * 40;
            if (m->mainCursor == i) {
                draw_rect(g, SCREEN_W/2 - 70, oy - 4, 140, 24,
                          GS_SETREG_RGBAQ(40, 40, 80, 0x80, 0));
                draw_text(g, ">", SCREEN_W/2 - 66, oy, COL_YELLOW);
            }
            draw_text(g, opts[i], SCREEN_W/2 - 36, oy,
                      m->mainCursor == i ? COL_WHITE : COL_GRAY);
        }

        draw_text(g, "UP/DOWN=NAVIGA  CROSS=SELECT",
                  SCREEN_W/2 - 168, SCREEN_H - 30, COL_DARK_GRAY);

    } else if (m->phase == MENU_PHASE_P1_SELECT) {
        draw_text(g, "P1 ALEGE ARMELE", SCREEN_W/2 - 90, 20, COL_P1);
        draw_text(g, "CROSS=ADAUGA  CIRCLE=STERGE  START=GATA",
                  40, 50, COL_GRAY);
        draw_weapon_select(g, gs, 0);

    } else if (m->phase == MENU_PHASE_P2_SELECT) {
        draw_text(g, "P2 ALEGE ARMELE", SCREEN_W/2 - 90, 20, COL_P2);
        draw_text(g, "CROSS=ADAUGA  CIRCLE=STERGE  START=GATA",
                  40, 50, COL_GRAY);
        draw_weapon_select(g, gs, 1);
    }
}

// ------------------------------------------------------------
//  menu_tick — input pentru meniu
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
                // Play — reseteaza selectia si merge la P1 select
                int p, i;
                for (p = 0; p < MAX_PLAYERS; p++) {
                    for (i = 0; i < WEAPONS_PER_PLAYER; i++) {
                        m->selectedWeapons[p][i] = 0;
                        m->weaponSlotFilled[p][i] = 0;
                    }
                    m->weaponCursor[p] = 0;
                }
                m->phase = MENU_PHASE_P1_SELECT;
            } else {
                // Quit — nu facem nimic pe PS2, doar revenim la main
                m->mainCursor = 0;
            }
        }

    } else if (m->phase == MENU_PHASE_P1_SELECT ||
               m->phase == MENU_PHASE_P2_SELECT) {

        int pidx = (m->phase == MENU_PHASE_P1_SELECT) ? 0 : 1;

        // Navigare lista
        if (input_pressed(gs, pidx, PAD_UP))
            m->weaponCursor[pidx] = (m->weaponCursor[pidx] - 1 + WEAPON_COUNT) % WEAPON_COUNT;
        if (input_pressed(gs, pidx, PAD_DOWN))
            m->weaponCursor[pidx] = (m->weaponCursor[pidx] + 1) % WEAPON_COUNT;

        // Adauga arma (Cross)
        if (input_pressed(gs, pidx, PAD_CROSS)) {
            int filled = 0, i;
            for (i = 0; i < WEAPONS_PER_PLAYER; i++)
                if (m->weaponSlotFilled[pidx][i]) filled++;

            if (filled < WEAPONS_PER_PLAYER) {
                // Gasim primul slot liber
                for (i = 0; i < WEAPONS_PER_PLAYER; i++) {
                    if (!m->weaponSlotFilled[pidx][i]) {
                        m->selectedWeapons[pidx][i] = m->weaponCursor[pidx];
                        m->weaponSlotFilled[pidx][i] = 1;
                        break;
                    }
                }
            }
        }

        // Sterge ultima arma (Circle)
        if (input_pressed(gs, pidx, PAD_CIRCLE)) {
            int i;
            for (i = WEAPONS_PER_PLAYER - 1; i >= 0; i--) {
                if (m->weaponSlotFilled[pidx][i]) {
                    m->weaponSlotFilled[pidx][i] = 0;
                    break;
                }
            }
        }

        // Start/Gata — doar daca are 5 arme selectate
        if (input_pressed(gs, pidx, PAD_START)) {
            int filled = 0, i;
            for (i = 0; i < WEAPONS_PER_PLAYER; i++)
                if (m->weaponSlotFilled[pidx][i]) filled++;

            if (filled >= WEAPONS_PER_PLAYER) {
                if (m->phase == MENU_PHASE_P1_SELECT) {
                    m->phase = MENU_PHASE_P2_SELECT;
                } else {
                    // Ambii au ales — pornim jocul
                    gs->state = STATE_PLAYING;
                    game_start(gs);
                }
            }
        }
    }
}

// ------------------------------------------------------------
//  pause_render
// ------------------------------------------------------------
void pause_render(GSGLOBAL *g, GameState *gs) {
    // Semi-transparent overlay
    draw_rect(g, SCREEN_W/2 - 100, SCREEN_H/2 - 80, 200, 160,
              GS_SETREG_RGBAQ(10, 10, 30, 0x80, 0));
    draw_rect(g, SCREEN_W/2 - 100, SCREEN_H/2 - 80, 200, 2,  COL_WHITE);
    draw_rect(g, SCREEN_W/2 - 100, SCREEN_H/2 + 80, 200, 2,  COL_WHITE);
    draw_rect(g, SCREEN_W/2 - 100, SCREEN_H/2 - 80, 2, 160,  COL_WHITE);
    draw_rect(g, SCREEN_W/2 + 98,  SCREEN_H/2 - 80, 2, 160,  COL_WHITE);

    draw_text(g, "PAUZA", SCREEN_W/2 - 30, SCREEN_H/2 - 64, COL_YELLOW);

    const char *opts[] = {
        "CONTINUA",
        "VOL +",
        "VOL -",
        "RESTART",
        "MENIU PRINCIPAL"
    };
    int i;
    for (i = 0; i < PAUSE_OPTS; i++) {
        int oy = SCREEN_H/2 - 40 + i * 22;
        if (gs->menu.pauseCursor == i) {
            draw_rect(g, SCREEN_W/2 - 96, oy - 2, 192, 20,
                      GS_SETREG_RGBAQ(40, 40, 80, 0x80, 0));
            draw_text(g, ">", SCREEN_W/2 - 92, oy, COL_YELLOW);
        }
        draw_text(g, opts[i], SCREEN_W/2 - 72, oy,
                  gs->menu.pauseCursor == i ? COL_WHITE : COL_GRAY);
    }
}

// ------------------------------------------------------------
//  pause_tick
// ------------------------------------------------------------
void pause_tick(GameState *gs) {
    MenuState *m = &gs->menu;

    if (input_pressed(gs, 0, PAD_UP) || input_pressed(gs, 1, PAD_UP))
        m->pauseCursor = (m->pauseCursor - 1 + PAUSE_OPTS) % PAUSE_OPTS;
    if (input_pressed(gs, 0, PAD_DOWN) || input_pressed(gs, 1, PAD_DOWN))
        m->pauseCursor = (m->pauseCursor + 1) % PAUSE_OPTS;

    if (input_pressed(gs, 0, PAD_CROSS) || input_pressed(gs, 1, PAD_CROSS)) {
        switch (m->pauseCursor) {
            case 0: // Continua
                gs->state = STATE_PLAYING;
                break;
            case 1: // Vol +
                if (gs->volume < 10) gs->volume++;
                break;
            case 2: // Vol -
                if (gs->volume > 0)  gs->volume--;
                break;
            case 3: // Restart
                gs->state = STATE_PLAYING;
                game_start(gs);
                break;
            case 4: // Meniu principal
                gs->state = STATE_MENU;
                m->phase  = MENU_PHASE_MAIN;
                break;
        }
    }

    // Start = continua rapid
    if (input_pressed(gs, 0, PAD_START) || input_pressed(gs, 1, PAD_START))
        gs->state = STATE_PLAYING;
}
