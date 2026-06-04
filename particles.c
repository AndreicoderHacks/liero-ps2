#include "liero.h"

// ============================================================
//  PARTICLES.C — Particule vizuale (explozie, fum, etc.)
// ============================================================

void particle_spawn(GameState *gs, int x, int y, int vx, int vy,
                    u64 color, int life) {
    int i;
    for (i = 0; i < MAX_PARTICLES; i++) {
        if (!gs->particles[i].alive) {
            gs->particles[i].alive = 1;
            gs->particles[i].x     = x << 4;   // fixed point *16
            gs->particles[i].y     = y << 4;
            gs->particles[i].vx    = vx;
            gs->particles[i].vy    = vy;
            gs->particles[i].life  = life;
            gs->particles[i].color = color;
            return;
        }
    }
}

void particle_tickAll(GameState *gs) {
    int i;
    for (i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &gs->particles[i];
        if (!p->alive) continue;

        p->vy += GRAVITY;   // gravitatie
        p->x  += p->vx;
        p->y  += p->vy;

        // Coliziune cu teren
        int px = p->x >> 4;
        int py = p->y >> 4;
        if (world_isSolid(&gs->world, px, py)) {
            p->vx = p->vx * -1 / 2;
            p->vy = p->vy * -1 / 2;
        }

        p->life--;
        if (p->life <= 0) p->alive = 0;
    }
}
