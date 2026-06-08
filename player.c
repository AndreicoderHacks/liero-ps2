#include "liero.h"

void player_init(Player *p, int x, int y, int padIndex) {
    memset(p, 0, sizeof(Player));
    p->alive        = 1;
    p->x            = x << 4;
    p->y            = y << 4;
    p->health       = 100;
    p->maxHealth    = 100;
    p->dir          = (padIndex == 0) ? 1 : 0;
    p->aimAngle     = (padIndex == 0) ? 0 : 128;
    // FIX: selectedWeapon = index in weapons[], default 0
    p->selectedWeapon = 0;
    p->padIndex     = padIndex;
    p->fireCooldown = 0;
    p->onGround     = 0;
}

#define PL_W  6
#define PL_H 10

static int player_onGround(GameState *gs, Player *pl) {
    int px = pl->x >> 4;
    int py = pl->y >> 4;
    return world_isSolid(&gs->world, px - PL_W/2, py + PL_H/2 + 1) ||
           world_isSolid(&gs->world, px,           py + PL_H/2 + 1) ||
           world_isSolid(&gs->world, px + PL_W/2,  py + PL_H/2 + 1);
}

static int player_blocked(GameState *gs, Player *pl, int nx, int ny) {
    int px = nx >> 4;
    int py = ny >> 4;
    return world_isSolid(&gs->world, px - PL_W/2, py - PL_H/2) ||
           world_isSolid(&gs->world, px + PL_W/2, py - PL_H/2) ||
           world_isSolid(&gs->world, px - PL_W/2, py + PL_H/2) ||
           world_isSolid(&gs->world, px + PL_W/2, py + PL_H/2);
}

void player_tick(GameState *gs, int idx) {
    Player *pl = &gs->players[idx];
    if (!pl->alive) return;

    int pad = pl->padIndex;

    // Miscare orizontala
    int moveX = 0;
    if (input_held(gs, pad, PAD_LEFT))  moveX = -1;
    if (input_held(gs, pad, PAD_RIGHT)) moveX =  1;
    if (gs->input.analogLX[pad] < 0) moveX = -1;
    if (gs->input.analogLX[pad] > 0) moveX =  1;
    pl->vx = moveX * (PLAYER_SPEED << 4);

    // Salt
    pl->onGround = player_onGround(gs, pl);
    if (pl->onGround) {
        if (input_pressed(gs, pad, PAD_CROSS) ||
            gs->input.analogLY[pad] < 0) {
            pl->vy = JUMP_FORCE << 4;
        }
    }

    // Gravitatie
    pl->vy += GRAVITY << 2;
    if (pl->vy > 80) pl->vy = 80;

    // Miscare X
    int nx = pl->x + pl->vx;
    if (!player_blocked(gs, pl, nx, pl->y)) pl->x = nx;
    else pl->vx = 0;

    // Miscare Y
    int ny = pl->y + pl->vy;
    if (!player_blocked(gs, pl, pl->x, ny)) pl->y = ny;
    else pl->vy = 0;

    // Tintire analog drept
    int rx = gs->input.analogRX[pad];
    int ry = gs->input.analogRY[pad];
    if (rx*rx + ry*ry > 40*40) {
        int ax = rx < 0 ? -rx : rx;
        int ay = ry < 0 ? -ry : ry;
        int angle = 0;
        if (ax > ay) {
            angle = (ay * 32) / (ax + 1);
            if      (rx > 0 && ry >= 0) angle = angle;
            else if (rx > 0 && ry <  0) angle = 256 - angle;
            else if (rx < 0 && ry >= 0) angle = 128 - angle;
            else                         angle = 128 + angle;
        } else {
            angle = 64 - (ax * 32) / (ay + 1);
            if      (rx >= 0 && ry > 0) angle = angle;
            else if (rx <  0 && ry > 0) angle = 128 - angle;
            else if (rx >= 0 && ry < 0) angle = 256 - angle;
            else                         angle = 128 + angle;
        }
        pl->aimAngle = angle & 0xFF;
    } else {
        if (input_held(gs, pad, PAD_L1)) pl->aimAngle = (pl->aimAngle - 4) & 0xFF;
        if (input_held(gs, pad, PAD_R1)) pl->aimAngle = (pl->aimAngle + 4) & 0xFF;
    }

    // FIX: schimb arma = index in weapons[], nu weapon ID direct
    if (input_pressed(gs, pad, PAD_L2)) {
        pl->selectedWeapon = (pl->selectedWeapon - 1 + pl->weaponCount) % pl->weaponCount;
    }
    if (input_pressed(gs, pad, PAD_R2)) {
        pl->selectedWeapon = (pl->selectedWeapon + 1) % pl->weaponCount;
    }

    // Foc
    if (input_held(gs, pad, PAD_SQUARE) ||
        input_held(gs, pad, PAD_R1)) {
        weapon_fire(gs, idx);
    }

    if (pl->fireCooldown > 0) pl->fireCooldown--;
    if (pl->hurtTimer > 0)    pl->hurtTimer--;
}

void player_hurt(GameState *gs, int idx, int damage) {
    Player *pl = &gs->players[idx];
    if (!pl->alive) return;
    if (pl->hurtTimer > 0) return;

    pl->health -= damage;
    pl->hurtTimer = 20;

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
