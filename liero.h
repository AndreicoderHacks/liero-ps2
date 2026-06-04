#ifndef LIERO_H
#define LIERO_H

// ============================================================
//  LIERO PS2
//  Header central — structuri, constante, declaratii functii
// ============================================================

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>
#include <gsKit.h>
#include <dmaKit.h>
#include <gsToolkit.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

// ---- PS2 Controller button masks ----
#ifndef PAD_SELECT
#define PAD_SELECT      0x0001
#define PAD_L3          0x0002
#define PAD_R3          0x0004
#define PAD_START       0x0008
#define PAD_UP          0x0800
#define PAD_RIGHT       0x0400
#define PAD_DOWN        0x0200
#define PAD_LEFT        0x0100
#define PAD_L2          0x0040
#define PAD_R2          0x0020
#define PAD_L1          0x0010
#define PAD_R1          0x0080
#define PAD_TRIANGLE    0x1000
#define PAD_CIRCLE      0x2000
#define PAD_CROSS       0x4000
#define PAD_SQUARE      0x8000
#endif

// ============================================================
//  SCREEN & WORLD
// ============================================================

#define SCREEN_W        640
#define SCREEN_H        448

// Lumea e un bitmap de pixeli solizi/goi
// 1 = solid (pamant), 0 = gol (aer)
#define WORLD_W         640
#define WORLD_H         360

// ============================================================
//  GAME CONSTANTS
// ============================================================

#define MAX_PLAYERS     2
#define MAX_PROJECTILES 64
#define MAX_PARTICLES   256

#define GRAVITY         1        // pixeli/tick^2 (fixed point /16)
#define PLAYER_SPEED    3
#define JUMP_FORCE      -10

#define ROUND_TIME      (60 * 60)   // 60 secunde la 60fps

// ============================================================
//  GAME STATES
// ============================================================

#define STATE_MENU       0
#define STATE_PLAYING    1
#define STATE_ROUND_END  2
#define STATE_PAUSED     3

// ============================================================
//  ARME
// ============================================================

#define WEAPON_PISTOL    0
#define WEAPON_SHOTGUN   1
#define WEAPON_BAZOOKA   2
#define WEAPON_GRENADE   3
#define WEAPON_MINIGUN   4
#define WEAPON_COUNT     5

// ============================================================
//  CULORI (GS format RGBA, alpha 0x80 = opac)
// ============================================================

#define COL_BLACK       GS_SETREG_RGBAQ(0,   0,   0,   0x80, 0)
#define COL_WHITE       GS_SETREG_RGBAQ(255, 255, 255, 0x80, 0)
#define COL_RED         GS_SETREG_RGBAQ(220, 40,  40,  0x80, 0)
#define COL_GREEN       GS_SETREG_RGBAQ(40,  200, 40,  0x80, 0)
#define COL_BLUE        GS_SETREG_RGBAQ(40,  40,  220, 0x80, 0)
#define COL_YELLOW      GS_SETREG_RGBAQ(255, 220, 30,  0x80, 0)
#define COL_ORANGE      GS_SETREG_RGBAQ(255, 130, 20,  0x80, 0)
#define COL_GRAY        GS_SETREG_RGBAQ(130, 130, 130, 0x80, 0)
#define COL_DARK_GRAY   GS_SETREG_RGBAQ(60,  60,  60,  0x80, 0)
#define COL_BROWN       GS_SETREG_RGBAQ(120, 70,  30,  0x80, 0)
#define COL_DIRT        GS_SETREG_RGBAQ(100, 60,  20,  0x80, 0)
#define COL_SKY         GS_SETREG_RGBAQ(20,  20,  40,  0x80, 0)

// Culori jucatori
#define COL_P1          GS_SETREG_RGBAQ(255, 80,  80,  0x80, 0)   // rosu
#define COL_P2          GS_SETREG_RGBAQ(80,  80,  255, 0x80, 0)   // albastru

// ============================================================
//  STRUCTURI DE DATE
// ============================================================

// ---- Lumea (teren destructibil) ----
typedef struct {
    // Fiecare bit = 1 pixel: 1 solid, 0 gol
    // 640 * 360 = 230400 bytes ~ 225 KB — incape in RAM PS2
    u8 solid[WORLD_W * WORLD_H];
    u8 color[WORLD_W * WORLD_H];   // culoarea fiecarui pixel de teren
} World;

// ---- Proiectil ----
typedef struct {
    int   alive;
    int   x, y;          // pozitie (fixed point *16)
    int   vx, vy;        // viteza (fixed point *16)
    int   owner;         // 0 = P1, 1 = P2
    int   weapon;        // tipul armei (pentru damage + explozie)
    int   timer;         // grenade countdown
    u64   color;
} Projectile;

// ---- Particula ----
typedef struct {
    int   alive;
    int   x, y;
    int   vx, vy;
    int   life;
    u64   color;
} Particle;

// ---- Jucator ----
typedef struct {
    int   alive;
    int   x, y;          // pozitie (fixed point *16)
    int   vx, vy;        // viteza (fixed point *16)
    int   health;
    int   maxHealth;
    int   dir;           // 0 = stanga, 1 = dreapta
    int   aimAngle;      // unghi tintire (0-255, mapat la 0-360)
    int   selectedWeapon;
    int   fireCooldown;
    int   onGround;
    int   padIndex;      // 0 = pad1, 1 = pad2
    int   hurtTimer;
} Player;

// ---- Input ----
typedef struct {
    u32   current[MAX_PLAYERS];
    u32   prev[MAX_PLAYERS];
    int   analogLX[MAX_PLAYERS];   // -128 .. 127
    int   analogLY[MAX_PLAYERS];
    int   analogRX[MAX_PLAYERS];
    int   analogRY[MAX_PLAYERS];
} InputState;

// ---- GameState (tot ce tine de joc) ----
typedef struct {
    int         state;
    int         tickCount;
    int         roundTimer;
    int         winner;       // 0 sau 1, valid cand state == STATE_ROUND_END

    World       world;

    Player      players[MAX_PLAYERS];

    Projectile  projectiles[MAX_PROJECTILES];
    int         projectileCount;

    Particle    particles[MAX_PARTICLES];
    int         particleCount;

    InputState  input;
} GameState;

// ============================================================
//  DECLARATII FUNCTII
// ============================================================

// main.c
void game_init(GameState *gs);
void game_tick(GameState *gs);
void game_render(GameState *gs, GSGLOBAL *gsGlobal);

// world.c
void world_generate(World *w);
int  world_isSolid(World *w, int x, int y);
void world_destroy(World *w, int cx, int cy, int radius);
void world_setPixel(World *w, int x, int y, int solid, u8 color);

// player.c
void player_init(Player *p, int x, int y, int padIndex);
void player_tick(GameState *gs, int idx);
void player_hurt(GameState *gs, int idx, int damage);

// weapon.c
void weapon_fire(GameState *gs, int playerIdx);
int  weapon_getCooldown(int weapon);
int  weapon_getDamage(int weapon);
int  weapon_getExplosionRadius(int weapon);
const char* weapon_getName(int weapon);

// projectile.c
void projectile_spawn(GameState *gs, int x, int y, int vx, int vy,
                      int owner, int weapon);
void projectile_tickAll(GameState *gs);
void projectile_explode(GameState *gs, int idx);

// render.c
void game_render(GameState *gs, GSGLOBAL *g);
void draw_rect(GSGLOBAL *g, int x, int y, int w, int h, u64 color);
void draw_text(GSGLOBAL *g, const char *str, int x, int y, u64 color);
void draw_char(GSGLOBAL *g, char c, int x, int y, u64 color);

// input.c
void input_init(void);
void input_update(GameState *gs);
int  input_pressed(GameState *gs, int player, u32 button);
int  input_held(GameState *gs, int player, u32 button);

// particles.c
void particle_spawn(GameState *gs, int x, int y, int vx, int vy,
                    u64 color, int life);
void particle_tickAll(GameState *gs);

// utils
int  rng_next(void);
void rng_seed(unsigned int s);
int  rng_range(int min, int max);

#endif // LIERO_H
