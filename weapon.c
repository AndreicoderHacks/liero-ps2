#include "liero.h"

typedef struct {
    const char *name;
    int cooldown;
    int damage;
    int explosionRadius;
    int bulletCount;
    int speed;
    int hasGravity;
} WeaponDef;

static const WeaponDef wdefs[WEAPON_COUNT] = {
    { "PISTOL",   15,  15,  0,  1, 80, 0 },
    { "SHOTGUN",  40,  10,  0,  5, 70, 0 },
    { "BAZOOKA",  60,  60, 20,  1, 60, 1 },
    { "GRENADE",  50,  50, 18,  1, 50, 1 },
    { "MINIGUN",   4,   8,  0,  1, 90, 0 },
};

// FIX: sin table globala
static const short w_stab[256] = {
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

void weapon_fire(GameState *gs, int playerIdx) {
    Player *pl = &gs->players[playerIdx];
    if (pl->fireCooldown > 0) return;
    if (!pl->alive) return;

    // FIX: weapon ID = weapons[selectedWeapon], nu selectedWeapon direct
    int weaponId = pl->weapons[pl->selectedWeapon];
    if (weaponId < 0 || weaponId >= WEAPON_COUNT) weaponId = 0;

    const WeaponDef *wd = &wdefs[weaponId];
    pl->fireCooldown = wd->cooldown;

    int cos_val = w_stab[(pl->aimAngle + 64) & 0xFF];
    int sin_val = w_stab[pl->aimAngle & 0xFF];

    int px = pl->x >> 4;
    int py = pl->y >> 4;

    int i;
    for (i = 0; i < wd->bulletCount; i++) {
        int spread = 0;
        if (wd->bulletCount > 1)
            spread = -20 + (i * 40 / (wd->bulletCount - 1));

        int svx = (cos_val * wd->speed) / 256 + spread;
        int svy = (sin_val * wd->speed) / 256;

        projectile_spawn(gs, px, py, svx, svy, playerIdx, weaponId);
    }
}

int  weapon_getCooldown(int w)        { if (w<0||w>=WEAPON_COUNT) return 30; return wdefs[w].cooldown; }
int  weapon_getDamage(int w)          { if (w<0||w>=WEAPON_COUNT) return 10; return wdefs[w].damage; }
int  weapon_getExplosionRadius(int w) { if (w<0||w>=WEAPON_COUNT) return  0; return wdefs[w].explosionRadius; }
const char* weapon_getName(int w)     { if (w<0||w>=WEAPON_COUNT) return "???"; return wdefs[w].name; }
