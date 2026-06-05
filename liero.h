#ifndef LIERO_H
#define LIERO_H

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
#define WORLD_W         640
#define WORLD_H         360

// ============================================================
//  GAME CONSTANTS
// ============================================================
#define MAX_PLAYERS         2
#define MAX_PROJECTILES     64
#define MAX_PARTICLES       256
#define WEAPONS_PER_PLAYER  5

#define GRAVITY         1
#define PLAYER_SPEED    3
#define JUMP_FORCE      -10
#define ROUND_TIME      (60 * 60)

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
//  MENIU
// ============================================================
#define MENU_PHASE_MAIN       0
#define MENU_PHASE_P1_SELECT  1
#define MENU_PHASE_P2_SELECT  2

#define PAUSE_OPTS      5

// ============================================================
//  CULORI
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
#define COL_P1          GS_SETREG_RGBAQ(255, 80,  80,  0x80, 0)
#define COL_P2          GS_SETREG_RGBAQ(80,  80,  255, 0x80, 0)

// ============================================================
//  STRUCTURI
// ============================================================

typedef struct {
    u8 solid[WORLD_W * WORLD_H];
    u8 color[WORLD_W * WORLD_H];
} World;

typedef struct {
    int   alive;
    int   x, y;
    int   vx, vy;
    int   owner;
    int   weapon;
    int   timer;
    u64   color;
} Projectile;

typedef struct {
    int   alive;
    int   x, y;
    int   vx, vy;
    int   life;
    u64   color;
} Particle;

typedef struct {
    int   alive;
    int   x, y;
    int   vx, vy;
    int   health;
    int   maxHealth;
    int   dir;
    int   aimAngle;
    int   weapons[WEAPONS_PER_PLAYER];   // armele alese in meniu
    int   weaponCount;
    int   selectedWeapon;                // index in weapons[]
    int   fireCooldown;
    int   onGround;
    int   padIndex;
    int   hurtTimer;
} Player;

typedef struct {
    u32   current[MAX_PLAYERS];
    u32   prev[MAX_PLAYERS];
    int   analogLX[MAX_PLAYERS];
    int   analogLY[MAX_PLAYERS];
    int   analogRX[MAX_PLAYERS];
    int   analogRY[MAX_PLAYERS];
} InputState;

typedef struct {
    int   phase;                                        // MENU_PHASE_*
    int   mainCursor;
    int   weaponCursor[MAX_PLAYERS];
    int   selectedWeapons[MAX_PLAYERS][WEAPONS_PER_PLAYER];
    int   weaponSlotFilled[MAX_PLAYERS][WEAPONS_PER_PLAYER];
    int   pauseCursor;
} MenuState;

typedef struct {
    int         state;
    int         tickCount;
    int         roundTimer;
    int         winner;
    int         volume;         // 0-10

    World       world;
    Player      players[MAX_PLAYERS];
    Projectile  projectiles[MAX_PROJECTILES];
    int         projectileCount;
    Particle    particles[MAX_PARTICLES];
    int         particleCount;
    InputState  input;
    MenuState   menu;
} GameState;

// ============================================================
//  DECLARATII FUNCTII
// ============================================================

// main.c
void game_init(GameState *gs);
void game_start(GameState *gs);   // porneste runda cu armele alese
void game_tick(GameState *gs);

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

// menu.c
void menu_tick(GameState *gs);
void menu_render(GSGLOBAL *g, GameState *gs);
void pause_tick(GameState *gs);
void pause_render(GSGLOBAL *g, GameState *gs);

#endif
