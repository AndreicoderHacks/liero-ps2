#include "liero.h"

// ============================================================
//  PROJECTILE.C — Fizica proiectile + coliziuni
// ============================================================

void projectile_spawn(GameState *gs, int x, int y, int vx, int vy,
                      int owner, int weapon) {
    int i;
    for (i = 0; i < MAX_PROJECTILES; i++) {
        if (!gs->projectiles[i].alive) {
            Projectile *p   = &gs->projectiles[i];
            p->alive        = 1;
            p->x            = x << 4;   // fixed point *16
            p->y            = y << 4;
            p->vx           = vx;
            p->vy           = vy;
            p->owner        = owner;
            p->weapon       = weapon;
            p->timer        = 120;      // grenade fuse: 2 secunde

            // Culoare dupa arma
            switch (weapon) {
                case WEAPON_PISTOL:  p->color = COL_YELLOW;  break;
                case WEAPON_SHOTGUN: p->color = COL_WHITE;   break;
                case WEAPON_BAZOOKA: p->color = COL_ORANGE;  break;
                case WEAPON_GRENADE: p->color = COL_GREEN;   break;
                case WEAPON_MINIGUN: p->color = COL_YELLOW;  break;
                default:             p->color = COL_WHITE;   break;
            }
            return;
        }
    }
}

// ------------------------------------------------------------
//  Explozie: distruge teren + particule + damage jucatori
// ------------------------------------------------------------
void projectile_explode(GameState *gs, int idx) {
    Projectile *pr = &gs->projectiles[idx];
    int cx = pr->x >> 4;
    int cy = pr->y >> 4;
    int radius = weapon_getExplosionRadius(pr->weapon);
    int damage = weapon_getDamage(pr->weapon);

    // Distruge teren
    if (radius > 0) {
        world_destroy(&gs->world, cx, cy, radius);
    }

    // Damage jucatori in raza
    int p;
    for (p = 0; p < MAX_PLAYERS; p++) {
        if (!gs->players[p].alive) continue;
        int px = gs->players[p].x >> 4;
        int py = gs->players[p].y >> 4;
        int dx = px - cx, dy = py - cy;
        int dist2 = dx*dx + dy*dy;
        int r = (radius > 0) ? radius : 8;
        if (dist2 <= r * r) {
            player_hurt(gs, p, damage);
        }
    }

    // Particule de explozie
    int i;
    for (i = 0; i < 20; i++) {
        int angle = (i * 256) / 20;
        // Viteze random in cerc
        int speed = 16 + (i % 4) * 8;
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
        int vx = (stab[(angle + 64) & 0xFF] * speed) >> 8;
        int vy = (stab[angle & 0xFF]         * speed) >> 8;
        u64 col = (i % 2) ? COL_ORANGE : COL_YELLOW;
        particle_spawn(gs, cx, cy, vx, vy, col, 20 + i * 2);
    }

    pr->alive = 0;
}

// ------------------------------------------------------------
//  projectile_tickAll
// ------------------------------------------------------------
void projectile_tickAll(GameState *gs) {
    int i;
    for (i = 0; i < MAX_PROJECTILES; i++) {
        Projectile *pr = &gs->projectiles[i];
        if (!pr->alive) continue;

        // Gravitatie pentru bazooka si grenade
        if (pr->weapon == WEAPON_BAZOOKA ||
            pr->weapon == WEAPON_GRENADE) {
            pr->vy += GRAVITY * 2;
        }

        // Fuse grenade
        if (pr->weapon == WEAPON_GRENADE) {
            pr->timer--;
            if (pr->timer <= 0) {
                projectile_explode(gs, i);
                continue;
            }
        }

        // Miscare pas cu pas (subpixel)
        pr->x += pr->vx;
        pr->y += pr->vy;

        int px = pr->x >> 4;
        int py = pr->y >> 4;

        // Coliziune cu teren
        if (world_isSolid(&gs->world, px, py)) {
            if (pr->weapon == WEAPON_BAZOOKA ||
                pr->weapon == WEAPON_GRENADE) {
                projectile_explode(gs, i);
            } else {
                // Glont normal: distruge 1-2px teren si dispare
                world_destroy(&gs->world, px, py, 2);
                pr->alive = 0;
            }
            continue;
        }

        // Coliziune cu jucatori
        int p;
        for (p = 0; p < MAX_PLAYERS; p++) {
            if (p == pr->owner) continue;
            if (!gs->players[p].alive) continue;
            int plx = gs->players[p].x >> 4;
            int ply = gs->players[p].y >> 4;
            int dx = px - plx, dy = py - ply;
            if (dx*dx + dy*dy < 6*6) {
                if (pr->weapon == WEAPON_BAZOOKA) {
                    projectile_explode(gs, i);
                } else {
                    player_hurt(gs, p, weapon_getDamage(pr->weapon));
                    // Particule mici la hit
                    particle_spawn(gs, px, py,  8, -8, COL_RED, 10);
                    particle_spawn(gs, px, py, -8, -8, COL_RED, 10);
                    pr->alive = 0;
                }
                break;
            }
        }

        // Out of bounds
        if (px < 0 || px >= WORLD_W || py < 0 || py >= WORLD_H) {
            pr->alive = 0;
        }
    }
}
