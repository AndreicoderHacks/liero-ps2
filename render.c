#include "liero.h"

// ============================================================
//  RENDER.C — Render lume, jucatori, HUD
// ============================================================

// ---- Font 5x7 minimal (ASCII 32-90) ----
// Fiecare caracter = 5 bytes, fiecare byte = un rand de 5 pixeli (bitmask)
static const u8 font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // ' '
    {0x00,0x00,0x5F,0x00,0x00}, // '!'
    {0x00,0x07,0x00,0x07,0x00}, // '"'
    {0x14,0x7F,0x14,0x7F,0x14}, // '#'
    {0x24,0x2A,0x7F,0x2A,0x12}, // '$'
    {0x23,0x13,0x08,0x64,0x62}, // '%'
    {0x36,0x49,0x55,0x22,0x50}, // '&'
    {0x00,0x05,0x03,0x00,0x00}, // '\''
    {0x00,0x1C,0x22,0x41,0x00}, // '('
    {0x00,0x41,0x22,0x1C,0x00}, // ')'
    {0x14,0x08,0x3E,0x08,0x14}, // '*'
    {0x08,0x08,0x3E,0x08,0x08}, // '+'
    {0x00,0x50,0x30,0x00,0x00}, // ','
    {0x08,0x08,0x08,0x08,0x08}, // '-'
    {0x00,0x60,0x60,0x00,0x00}, // '.'
    {0x20,0x10,0x08,0x04,0x02}, // '/'
    {0x3E,0x51,0x49,0x45,0x3E}, // '0'
    {0x00,0x42,0x7F,0x40,0x00}, // '1'
    {0x42,0x61,0x51,0x49,0x46}, // '2'
    {0x21,0x41,0x45,0x4B,0x31}, // '3'
    {0x18,0x14,0x12,0x7F,0x10}, // '4'
    {0x27,0x45,0x45,0x45,0x39}, // '5'
    {0x3C,0x4A,0x49,0x49,0x30}, // '6'
    {0x01,0x71,0x09,0x05,0x03}, // '7'
    {0x36,0x49,0x49,0x49,0x36}, // '8'
    {0x06,0x49,0x49,0x29,0x1E}, // '9'
    {0x00,0x36,0x36,0x00,0x00}, // ':'
    {0x00,0x56,0x36,0x00,0x00}, // ';'
    {0x08,0x14,0x22,0x41,0x00}, // '<'
    {0x14,0x14,0x14,0x14,0x14}, // '='
    {0x00,0x41,0x22,0x14,0x08}, // '>'
    {0x02,0x01,0x51,0x09,0x06}, // '?'
    {0x32,0x49,0x79,0x41,0x3E}, // '@'
    {0x7E,0x11,0x11,0x11,0x7E}, // 'A'
    {0x7F,0x49,0x49,0x49,0x36}, // 'B'
    {0x3E,0x41,0x41,0x41,0x22}, // 'C'
    {0x7F,0x41,0x41,0x22,0x1C}, // 'D'
    {0x7F,0x49,0x49,0x49,0x41}, // 'E'
    {0x7F,0x09,0x09,0x09,0x01}, // 'F'
    {0x3E,0x41,0x49,0x49,0x7A}, // 'G'
    {0x7F,0x08,0x08,0x08,0x7F}, // 'H'
    {0x00,0x41,0x7F,0x41,0x00}, // 'I'
    {0x20,0x40,0x41,0x3F,0x01}, // 'J'
    {0x7F,0x08,0x14,0x22,0x41}, // 'K'
    {0x7F,0x40,0x40,0x40,0x40}, // 'L'
    {0x7F,0x02,0x0C,0x02,0x7F}, // 'M'
    {0x7F,0x04,0x08,0x10,0x7F}, // 'N'
    {0x3E,0x41,0x41,0x41,0x3E}, // 'O'
    {0x7F,0x09,0x09,0x09,0x06}, // 'P'
    {0x3E,0x41,0x51,0x21,0x5E}, // 'Q'
    {0x7F,0x09,0x19,0x29,0x46}, // 'R'
    {0x46,0x49,0x49,0x49,0x31}, // 'S'
    {0x01,0x01,0x7F,0x01,0x01}, // 'T'
    {0x3F,0x40,0x40,0x40,0x3F}, // 'U'
    {0x1F,0x20,0x40,0x20,0x1F}, // 'V'
    {0x3F,0x40,0x38,0x40,0x3F}, // 'W'
    {0x63,0x14,0x08,0x14,0x63}, // 'X'
    {0x07,0x08,0x70,0x08,0x07}, // 'Y'
    {0x61,0x51,0x49,0x45,0x43}, // 'Z'
};

// ---- Culori teren dupa index ----
static u64 terrain_color(u8 col_idx) {
    switch (col_idx) {
        case 1: return COL_DIRT;
        case 2: return COL_GRAY;
        case 3: return COL_BROWN;
        case 4: return GS_SETREG_RGBAQ(80, 50, 20, 0x80, 0);
        default: return COL_DARK_GRAY;
    }
}

// ------------------------------------------------------------
//  draw_rect
// ------------------------------------------------------------
void draw_rect(GSGLOBAL *g, int x, int y, int w, int h, u64 color) {
    gsKit_prim_sprite(g,
        (float)x,       (float)y,
        (float)(x + w), (float)(y + h),
        1, color);
}

// ------------------------------------------------------------
//  draw_char — 5x7 font, 2x scale
// ------------------------------------------------------------
void draw_char(GSGLOBAL *g, char c, int x, int y, u64 color) {
    if (c < 32 || c > 90) c = '?';
    int idx = c - 32;
    int col, row;
    for (col = 0; col < 5; col++) {
        u8 bits = font5x7[idx][col];
        for (row = 0; row < 7; row++) {
            if (bits & (1 << row)) {
                draw_rect(g, x + col*2, y + row*2, 2, 2, color);
            }
        }
    }
}

void draw_text(GSGLOBAL *g, const char *str, int x, int y, u64 color) {
    int i = 0;
    while (str[i]) {
        char c = str[i];
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
        draw_char(g, c, x + i * 12, y, color);
        i++;
    }
}

// ---- Numere rapide (evita sprintf) ----
static void draw_int(GSGLOBAL *g, int val, int x, int y, u64 color) {
    char buf[12];
    int i = 0;
    if (val < 0) { buf[i++] = '-'; val = -val; }
    if (val == 0) { buf[i++] = '0'; }
    else {
        char tmp[10]; int t = 0;
        while (val > 0) { tmp[t++] = '0' + (val % 10); val /= 10; }
        while (t > 0) buf[i++] = tmp[--t];
    }
    buf[i] = '\0';
    draw_text(g, buf, x, y, color);
}

// ------------------------------------------------------------
//  render_world — deseneaza terenul vizibil pe ecran
//  Camera centrata pe mijlocul celor 2 jucatori
// ------------------------------------------------------------
static void render_world(GSGLOBAL *g, GameState *gs, int camX, int camY) {
    int sx, sy;
    int startX = camX - SCREEN_W / 2;
    int startY = camY - SCREEN_H / 2;
    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;

    // Desenam linie cu linie, fiecare pixel = 2x2 pe ecran
    // Citim din 2 in 2 pixeli => de 4x mai putine draw calls
    for (sy = 0; sy < SCREEN_H; sy += 2) {
        int wy = startY + (sy >> 1);
        if (wy >= WORLD_H) break;

        int run_start = -1;
        u64 run_col   = 0;

        for (sx = 0; sx <= SCREEN_W; sx += 2) {
            int wx    = startX + (sx >> 1);
            int solid = 0;
            u64 col   = 0;

            if (sx < SCREEN_W && wx < WORLD_W) {
                int idx = wy * WORLD_W + wx;
                if (gs->world.solid[idx]) {
                    solid = 1;
                    col   = terrain_color(gs->world.color[idx]);
                }
            }

            if (solid && run_start < 0) {
                run_start = sx;
                run_col   = col;
            } else if (run_start >= 0 && (!solid || col != run_col)) {
                gsKit_prim_sprite(g,
                    (float)run_start, (float)sy,
                    (float)sx,        (float)(sy + 2),
                    1, run_col);
                run_start = solid ? sx : -1;
                run_col   = col;
            }
        }
    }
}

// ------------------------------------------------------------
//  render_player
// ------------------------------------------------------------
static void render_player(GSGLOBAL *g, Player *pl, int camX, int camY,
                           int idx) {
    if (!pl->alive) return;

    int px = (pl->x >> 4) - camX + SCREEN_W / 2;
    int py = (pl->y >> 4) - camY + SCREEN_H / 2;

    // Cliping simplu
    if (px < -10 || px > SCREEN_W + 10) return;
    if (py < -10 || py > SCREEN_H + 10) return;

    u64 col = (idx == 0) ? COL_P1 : COL_P2;

    // Corp: dreptunghi 6x10
    if (pl->hurtTimer > 0 && (pl->hurtTimer % 4) < 2) {
        col = COL_WHITE;  // flash la hurt
    }
    draw_rect(g, px - 3, py - 5, 6, 10, col);

    // Cap: dreptunghi mic deasupra
    draw_rect(g, px - 2, py - 9, 4, 4, col);

    // Linie de tintire (4 pixeli in directia armei)
    static const short stab[256] = {
          0,   6,  13,  19,  25,  31,  38,  44,
         50,  56,  62,  68,  74,  80,  86,  92,
         98, 104, 109, 115, 121, 126, 132, 137,
        142, 147, 152, 157, 162, 167, 171, 176,
        180, 185, 189, 193, 197, 201, 205, 208,
        212, 215, 219, 222, 225, 228, 231, 234,
        236, 238, 241, 243, 245, 247, 248, 250,
        251, 252, 253, 254, 255, 255, 256, 256,
        256, 256, 256, 255, 255, 254, 253, 252,
        251, 250, 248, 247, 245, 243, 241, 238,
        236, 234, 231, 228, 225, 222, 219, 215,
        212, 208, 205, 201, 197, 193, 189, 185,
        180, 176, 171, 167, 162, 157, 152, 147,
        142, 137, 132, 126, 121, 115, 109, 104,
         98,  92,  86,  80,  74,  68,  62,  56,
         50,  44,  38,  31,  25,  19,  13,   6,
          0,  -6, -13, -19, -25, -31, -38, -44,
        -50, -56, -62, -68, -74, -80, -86, -92,
        -98,-104,-109,-115,-121,-126,-132,-137,
       -142,-147,-152,-157,-162,-167,-171,-176,
       -180,-185,-189,-193,-197,-201,-205,-208,
       -212,-215,-219,-222,-225,-228,-231,-234,
       -236,-238,-241,-243,-245,-247,-248,-250,
       -251,-252,-253,-254,-255,-255,-256,-256,
       -256,-256,-256,-255,-255,-254,-253,-252,
       -251,-250,-248,-247,-245,-243,-241,-238,
       -236,-234,-231,-228,-225,-222,-219,-215,
       -212,-208,-205,-201,-197,-193,-189,-185,
       -180,-176,-171,-167,-162,-157,-152,-147,
       -142,-137,-132,-126,-121,-115,-109,-104,
        -98, -92, -86, -80, -74, -68, -62, -56,
        -50, -44, -38, -31, -25, -19, -13,  -6,
    };
    int ax = (stab[(pl->aimAngle + 64) & 0xFF] * 8) >> 8;
    int ay = (stab[pl->aimAngle & 0xFF]         * 8) >> 8;
    draw_rect(g, px + ax - 1, py + ay - 1, 2, 2, COL_YELLOW);
}

// ------------------------------------------------------------
//  render_projectiles
// ------------------------------------------------------------
static void render_projectiles(GSGLOBAL *g, GameState *gs,
                                int camX, int camY) {
    int i;
    for (i = 0; i < MAX_PROJECTILES; i++) {
        Projectile *pr = &gs->projectiles[i];
        if (!pr->alive) continue;
        int px = (pr->x >> 4) - camX + SCREEN_W / 2;
        int py = (pr->y >> 4) - camY + SCREEN_H / 2;
        if (px < 0 || px >= SCREEN_W) continue;
        if (py < 0 || py >= SCREEN_H) continue;
        draw_rect(g, px - 1, py - 1, 3, 3, pr->color);
    }
}

// ------------------------------------------------------------
//  render_particles
// ------------------------------------------------------------
static void render_particles(GSGLOBAL *g, GameState *gs,
                              int camX, int camY) {
    int i;
    for (i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &gs->particles[i];
        if (!p->alive) continue;
        int px = (p->x >> 4) - camX + SCREEN_W / 2;
        int py = (p->y >> 4) - camY + SCREEN_H / 2;
        if (px < 0 || px >= SCREEN_W) continue;
        if (py < 0 || py >= SCREEN_H) continue;
        draw_rect(g, px, py, 2, 2, p->color);
    }
}

// ------------------------------------------------------------
//  render_hud
// ------------------------------------------------------------
static void render_hud(GSGLOBAL *g, GameState *gs) {
    // P1 HUD - stanga sus
    draw_text(g, "P1", 8, 8, COL_P1);
    draw_rect(g, 8, 24, 100, 8, COL_DARK_GRAY);
    draw_rect(g, 8, 24, gs->players[0].health, 8, COL_P1);
    draw_text(g, weapon_getName(gs->players[0].selectedWeapon), 8, 36, COL_WHITE);

    // P2 HUD - dreapta sus
    draw_text(g, "P2", SCREEN_W - 30, 8, COL_P2);
    draw_rect(g, SCREEN_W - 108, 24, 100, 8, COL_DARK_GRAY);
    draw_rect(g, SCREEN_W - 108, 24, gs->players[1].health, 8, COL_P2);
    draw_text(g, weapon_getName(gs->players[1].selectedWeapon),
              SCREEN_W - 108, 36, COL_WHITE);

    // Timer runda - centru sus
    int secs = gs->roundTimer / 60;
    draw_int(g, secs, SCREEN_W / 2 - 12, 8, COL_YELLOW);
}

// ------------------------------------------------------------
//  render_round_end
// ------------------------------------------------------------
static void render_round_end(GSGLOBAL *g, GameState *gs) {
    draw_rect(g, SCREEN_W/2 - 80, SCREEN_H/2 - 20, 160, 40, COL_BLACK);
    if (gs->winner == 0)
        draw_text(g, "P1 WINS", SCREEN_W/2 - 42, SCREEN_H/2 - 10, COL_P1);
    else
        draw_text(g, "P2 WINS", SCREEN_W/2 - 42, SCREEN_H/2 - 10, COL_P2);
    draw_text(g, "START TO PLAY AGAIN", SCREEN_W/2 - 114, SCREEN_H/2 + 14, COL_WHITE);
}

// ------------------------------------------------------------
//  game_render — entry point
// ------------------------------------------------------------
void game_render(GameState *gs, GSGLOBAL *g) {
    gsKit_clear(g, COL_SKY);

    // Camera: mijlocul dintre cei 2 jucatori
    int camX = ((gs->players[0].x + gs->players[1].x) >> 1) >> 4;
    int camY = ((gs->players[0].y + gs->players[1].y) >> 1) >> 4;

    // Clamp camera
    if (camX < SCREEN_W / 2)           camX = SCREEN_W / 2;
    if (camX > WORLD_W - SCREEN_W / 2) camX = WORLD_W - SCREEN_W / 2;
    if (camY < SCREEN_H / 2)           camY = SCREEN_H / 2;
    if (camY > WORLD_H - SCREEN_H / 2) camY = WORLD_H - SCREEN_H / 2;

    render_world(g, gs, camX, camY);
    gsKit_queue_exec(g);   // flush dupa teren
    render_particles(g, gs, camX, camY);
    render_projectiles(g, gs, camX, camY);
    render_player(g, &gs->players[0], camX, camY, 0);
    render_player(g, &gs->players[1], camX, camY, 1);
    render_hud(g, gs);

    if (gs->state == STATE_ROUND_END) {
        render_round_end(g, gs);
    }
}
