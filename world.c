#include "liero.h"

// ============================================================
//  WORLD.C — Teren destructibil
//  Lumea e un bitmap de pixeli: 1 = solid, 0 = aer
// ============================================================

// RNG simplu (duplicat local ca sa nu depinda de utils.c inca)
static unsigned int w_rng_state = 0x12345678;
static int w_rng(void) {
    w_rng_state ^= w_rng_state << 13;
    w_rng_state ^= w_rng_state >> 17;
    w_rng_state ^= w_rng_state << 5;
    return (int)(w_rng_state & 0x7FFFFFFF);
}
static int w_rng_range(int lo, int hi) {
    return lo + (w_rng() % (hi - lo + 1));
}

// ---- Culori teren ----
// Codificam culoarea ca index simplu, render.c o transforma in GS color
#define TCOL_DIRT   1
#define TCOL_ROCK   2
#define TCOL_SAND   3
#define TCOL_MUD    4

// ------------------------------------------------------------
//  Noise Perlin 1D simplificat — folosit pentru suprafata
// ------------------------------------------------------------
static int heightmap[WORLD_W];

static void generate_heightmap(void) {
    // Octave de zgomot adunat
    int base[WORLD_W];
    int i, oct;

    for (i = 0; i < WORLD_W; i++) base[i] = 0;

    // Octava 1: valuri mari (frecventa mica)
    int amp = 60, freq = WORLD_W / 4;
    for (oct = 0; oct < 4; oct++) {
        int prev = w_rng_range(-amp, amp);
        int j = 0;
        while (j < WORLD_W) {
            int next = w_rng_range(-amp, amp);
            int steps = freq;
            if (j + steps > WORLD_W) steps = WORLD_W - j;
            for (i = 0; i < steps; i++) {
                base[j + i] += prev + (next - prev) * i / freq;
            }
            prev = next;
            j += steps;
        }
        amp  /= 2;
        freq /= 2;
        if (freq < 1) freq = 1;
    }

    // Normalizam la o banda: suprafata intre 60 si 200
    int mid = WORLD_H / 2;
    for (i = 0; i < WORLD_W; i++) {
        heightmap[i] = mid + base[i] / 4;
        if (heightmap[i] < 40)           heightmap[i] = 40;
        if (heightmap[i] > WORLD_H - 40) heightmap[i] = WORLD_H - 40;
    }
}

// ------------------------------------------------------------
//  world_generate
// ------------------------------------------------------------
void world_generate(World *w) {
    int x, y;

    w_rng_state = 0xDEADBEEF;

    memset(w->solid, 0, sizeof(w->solid));
    memset(w->color, 0, sizeof(w->color));

    generate_heightmap();

    for (x = 0; x < WORLD_W; x++) {
        int surf = heightmap[x];

        for (y = 0; y < WORLD_H; y++) {
            if (y < surf) {
                // Aer
                w->solid[y * WORLD_W + x] = 0;
                w->color[y * WORLD_W + x] = 0;
            } else {
                // Teren solid
                w->solid[y * WORLD_W + x] = 1;

                // Culoare in functie de adancime
                int depth = y - surf;
                if (depth < 4) {
                    w->color[y * WORLD_W + x] = TCOL_SAND;
                } else if (depth < 20) {
                    w->color[y * WORLD_W + x] = TCOL_DIRT;
                } else if (depth < 50) {
                    w->color[y * WORLD_W + x] = TCOL_MUD;
                } else {
                    w->color[y * WORLD_W + x] = TCOL_ROCK;
                }
            }
        }
    }

    // Adaugam pesteri (cercuri goale random in subsol)
    int caves = 30 + w_rng_range(0, 20);
    int c;
    for (c = 0; c < caves; c++) {
        int cx = w_rng_range(20, WORLD_W - 20);
        int cy = w_rng_range(WORLD_H / 2, WORLD_H - 20);
        int r  = w_rng_range(8, 25);
        world_destroy(w, cx, cy, r);
    }

    // Adaugam coloane de roca tare la margini (nu pot fi distruse)
    for (y = 0; y < WORLD_H; y++) {
        for (x = 0; x < 4; x++) {
            w->solid[y * WORLD_W + x]                  = 1;
            w->solid[y * WORLD_W + (WORLD_W - 1 - x)]  = 1;
            w->color[y * WORLD_W + x]                  = TCOL_ROCK;
            w->color[y * WORLD_W + (WORLD_W - 1 - x)]  = TCOL_ROCK;
        }
    }
    // Podea si tavan
    for (x = 0; x < WORLD_W; x++) {
        for (y = 0; y < 4; y++) {
            w->solid[y * WORLD_W + x]                  = 0; // tavan = aer
        }
        for (y = WORLD_H - 4; y < WORLD_H; y++) {
            w->solid[y * WORLD_W + x]                  = 1;
            w->color[y * WORLD_W + x]                  = TCOL_ROCK;
        }
    }
}

// ------------------------------------------------------------
//  world_isSolid
// ------------------------------------------------------------
int world_isSolid(World *w, int x, int y) {
    if (x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) return 1;
    return w->solid[y * WORLD_W + x];
}

// ------------------------------------------------------------
//  world_destroy — explozie circulara
// ------------------------------------------------------------
void world_destroy(World *w, int cx, int cy, int radius) {
    int x, y;
    int r2 = radius * radius;

    for (y = cy - radius; y <= cy + radius; y++) {
        if (y < 0 || y >= WORLD_H) continue;
        for (x = cx - radius; x <= cx + radius; x++) {
            if (x < 0 || x >= WORLD_W) continue;
            int dx = x - cx, dy = y - cy;
            if (dx*dx + dy*dy <= r2) {
                w->solid[y * WORLD_W + x] = 0;
                w->color[y * WORLD_W + x] = 0;
            }
        }
    }
}

// ------------------------------------------------------------
//  world_setPixel
// ------------------------------------------------------------
void world_setPixel(World *w, int x, int y, int solid, u8 color) {
    if (x < 0 || x >= WORLD_W || y < 0 || y >= WORLD_H) return;
    w->solid[y * WORLD_W + x] = (u8)solid;
    w->color[y * WORLD_W + x] = color;
}
