#include "liero.h"

// ============================================================
//  PLAYER.C — Fizica jucator + controls
// ============================================================

void player_init(Player *p, int x, int y, int padIndex) {
    memset(p, 0, sizeof(Player));
    p->alive        = 1;
    p->x            = x << 4;   // fixed point *16
    p->y            = y << 4;
    p->health       = 100;
    p->maxHealth    = 100;
    p->dir          = (padIndex == 0) ? 1 : 0;  // P1 priveste dreapta, P2 stanga
    p->aimAngle     = (padIndex == 0) ? 0 : 128; // 0=dreapta, 128=stanga
    p->selectedWeapon = WEAPON_PISTOL;
    p->padIndex     = padIndex;
    p->fireCooldown = 0;
    p->onGround     = 0;
}

// ------------------------------------------------------------
//  Coliziune punct cu teren (in spatiu pixel)
// ------------------------------------------------------------
static int collide(GameState *gs, int px, int py) {
    return world_isSolid(&gs->world, px, py);
}

// Hitbox jucator: 6x10 pixeli centrat pe (px, py)
#define PL_W  6
#define PL_H 10

static int player_onGround(GameState *gs, Player *pl) {
    int px = pl->x >> 4;
    int py = pl->y >> 4;
    // Verificam un pixel sub picioare
    return collide(gs, px - PL_W/2, py + PL_H/2 + 1) ||
           collide(gs, px,           py + PL_H/2 + 1) ||
           collide(gs, px + PL_W/2, py + PL_H/2 + 1);
}

static int player_blocked(GameState *gs, Player *pl, int nx, int ny) {
    int px = nx >> 4;
    int py = ny >> 4;
    return collide(gs, px - PL_W/2, py - PL_H/2) ||
           collide(gs, px + PL_W/2, py - PL_H/2) ||
           collide(gs, px - PL_W/2, py + PL_H/2) ||
           collide(gs, px + PL_W/2, py + PL_H/2);
}

// ------------------------------------------------------------
//  player_tick
// ------------------------------------------------------------
void player_tick(GameState *gs, int idx) {
    Player *pl = &gs->players[idx];
    if (!pl->alive) return;

    int pad = pl->padIndex;

    // ---- Miscare orizontala (D-pad sau analog stang) ----
    int moveX = 0;
    if (input_held(gs, pad, PAD_LEFT))  moveX = -1;
    if (input_held(gs, pad, PAD_RIGHT)) moveX =  1;

    // Analog stang depaseste 40 ca threshold
    if (gs->input.analogLX[pad] < -40) moveX = -1;
    if (gs->input.analogLX[pad] >  40) moveX =  1;

    pl->vx = moveX * (PLAYER_SPEED << 4);

    // ---- Salt (X / Cruce sau analog stang sus) ----
    pl->onGround = player_onGround(gs, pl);
    if (pl->onGround) {
        if (input_pressed(gs, pad, PAD_CROSS) ||
            gs->input.analogLY[pad] < -60) {
            pl->vy = JUMP_FORCE << 4;
        }
    }

    // ---- Gravitatie ----
    pl->vy += GRAVITY << 2;  // mai putin gravity ca sa fie playabil
    if (pl->vy > 80) pl->vy = 80;  // terminal velocity

    // ---- Aplicare miscare X ----
    int nx = pl->x + pl->vx;
    if (!player_blocked(gs, pl, nx, pl->y)) {
        pl->x = nx;
    } else {
        pl->vx = 0;
    }

    // ---- Aplicare miscare Y ----
    int ny = pl->y + pl->vy;
    if (!player_blocked(gs, pl, pl->x, ny)) {
        pl->y = ny;
    } else {
        if (pl->vy > 0) pl->vy = 0;   // aterizare
        else pl->vy = 0;               // lovit tavan
    }

    // ---- Tintire cu analog drept ----
    int rx = gs->input.analogRX[pad];
    int ry = gs->input.analogRY[pad];
    if (rx*rx + ry*ry > 40*40) {
        // Convertim analog drept la unghi 0-255
        // atan2 aproximativ: folosim octanti
        // rx, ry sunt in -128..127
        // Unghi 0 = dreapta (rx>0, ry=0), creste orar
        int angle = 0;
        int ax = rx < 0 ? -rx : rx;
        int ay = ry < 0 ? -ry : ry;

        if (ax > ay) {
            // aproape de orizontal
            angle = (ay * 32) / (ax + 1);   // 0-32
            if (rx > 0 && ry >= 0) angle = angle;
            else if (rx > 0 && ry <  0) angle = 256 - angle;
            else if (rx < 0 && ry >= 0) angle = 128 - angle;
            else                         angle = 128 + angle;
        } else {
            // aproape de vertical
            angle = 64 - (ax * 32) / (ay + 1);
            if (rx >= 0 && ry > 0) angle = angle;
            else if (rx <  0 && ry > 0) angle = 128 - angle;
            else if (rx >= 0 && ry < 0) angle = 256 - angle;
            else                         angle = 128 + angle;
        }
        pl->aimAngle = angle & 0xFF;
    } else {
        // Fara analog: tintire cu L1/R1 sau directional
        if (input_held(gs, pad, PAD_L1)) pl->aimAngle = (pl->aimAngle - 4) & 0xFF;
        if (input_held(gs, pad, PAD_R1)) pl->aimAngle = (pl->aimAngle + 4) & 0xFF;
    }

    // ---- Schimb arma (L2/R2) ----
    if (input_pressed(gs, pad, PAD_L2)) {
        pl->selectedWeapon = (pl->selectedWeapon - 1 + WEAPON_COUNT) % WEAPON_COUNT;
    }
    if (input_pressed(gs, pad, PAD_R2)) {
        pl->selectedWeapon = (pl->selectedWeapon + 1) % WEAPON_COUNT;
    }

    // ---- Foc (R1 sau Patrat) ----
    if (input_held(gs, pad, PAD_SQUARE) ||
        input_held(gs, pad, PAD_R1)) {
        weapon_fire(gs, idx);
    }

    // ---- Cooldown arma ----
    if (pl->fireCooldown > 0) pl->fireCooldown--;

    // ---- Hurt timer ----
    if (pl->hurtTimer > 0) pl->hurtTimer--;
}

// ------------------------------------------------------------
//  player_hurt
// ------------------------------------------------------------
void player_hurt(GameState *gs, int idx, int damage) {
    Player *pl = &gs->players[idx];
    if (!pl->alive) return;
    if (pl->hurtTimer > 0) return;  // invincibil temporar

    pl->health -= damage;
    pl->hurtTimer = 20;

    // Particule de sange
    int px = pl->x >> 4;
    int py = pl->y >> 4;
    int i;
    for (i = 0; i < 6; i++) {
        int vx = -16 + (i * 6);
        int vy = -20 + (i * 4);
        particle_spawn(gs, px, py, vx, vy, COL_RED, 15);
    }

    if (pl->health <= 0) {
        pl->health = 0;
        pl->alive  = 0;
    }
}
