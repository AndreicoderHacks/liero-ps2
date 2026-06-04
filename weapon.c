#include "liero.h"

// ============================================================
//  WEAPON.C — Definitii arme si fire logic
// ============================================================

typedef struct {
    const char *name;
    int cooldown;       // ticks intre focuri
    int damage;
    int explosionRadius;
    int bulletCount;    // pentru shotgun
    int speed;          // viteza proiectil (fixed point *16)
    int hasGravity;     // 1 = proiectilul cade (grenade, bazooka)
} WeaponDef;

static const WeaponDef weapons[WEAPON_COUNT] = {
    // name         cool  dmg  expl  bullets  speed  grav
    { "Pistol",      15,   15,   0,    1,      80,    0 },
    { "Shotgun",     40,   10,   0,    5,      70,    0 },
    { "Bazooka",     60,   60,  20,    1,      60,    1 },
    { "Grenade",     50,   50,  18,    1,      50,    1 },
    { "Minigun",      4,    8,   0,    1,      90,    0 },
};

// ------------------------------------------------------------
//  weapon_fire — spawneaza proiectil(e) pentru jucatorul idx
// ------------------------------------------------------------
void weapon_fire(GameState *gs, int playerIdx) {
    Player *pl = &gs->players[playerIdx];
    if (pl->fireCooldown > 0) return;
    if (!pl->alive) return;

    const WeaponDef *wd = &weapons[pl->selectedWeapon];
    pl->fireCooldown = wd->cooldown;

    // Calculam directia din aimAngle (0-255 => 0-360 grade)
    // Unghi 0 = dreapta, creste in sens orar
    int angle256 = pl->aimAngle;

    // Aproximare sin/cos cu tabel fix (evitam float pe PS2 EE)
    // Folosim << 8 pentru precizie
    // sin_tab[i] = sin(i * 2PI / 256) * 256
    static const short sin_tab[256] = {
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
    // cos = sin(angle + 64)
    int cos_val = sin_tab[(angle256 + 64) & 0xFF];
    int sin_val = sin_tab[angle256 & 0xFF];

    int px = pl->x >> 4;
    int py = pl->y >> 4;

    int i;
    for (i = 0; i < wd->bulletCount; i++) {
        int spread = 0;
        if (wd->bulletCount > 1) {
            // Shotgun spread: ±20 unitati din tabel
            spread = -20 + (i * 40 / (wd->bulletCount - 1));
        }

        int svx = (cos_val * wd->speed) / 256 + spread;
        int svy = (sin_val * wd->speed) / 256;

        projectile_spawn(gs, px, py, svx, svy,
                         playerIdx, pl->selectedWeapon);
    }
}

int weapon_getCooldown(int weapon) {
    if (weapon < 0 || weapon >= WEAPON_COUNT) return 30;
    return weapons[weapon].cooldown;
}

int weapon_getDamage(int weapon) {
    if (weapon < 0 || weapon >= WEAPON_COUNT) return 10;
    return weapons[weapon].damage;
}

int weapon_getExplosionRadius(int weapon) {
    if (weapon < 0 || weapon >= WEAPON_COUNT) return 0;
    return weapons[weapon].explosionRadius;
}

const char* weapon_getName(int weapon) {
    if (weapon < 0 || weapon >= WEAPON_COUNT) return "???";
    return weapons[weapon].name;
}
