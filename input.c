#include "liero.h"

static u8 padBuf0[256] __attribute__((aligned(64)));
static u8 padBuf1[256] __attribute__((aligned(64)));

void input_init(void) {
    padInit(0);
    padPortOpen(0, 0, padBuf0);
    padPortOpen(1, 0, padBuf1);
    int i;
    for (i = 0; i < 120; i++) padGetState(0, 0);
}

void input_update(GameState *gs) {
    int p;
    for (p = 0; p < MAX_PLAYERS; p++) {
        gs->input.prev[p] = gs->input.current[p];

        // Reset complet inainte de citire
        gs->input.current[p]  = 0;
        gs->input.analogLX[p] = 0;
        gs->input.analogLY[p] = 0;
        gs->input.analogRX[p] = 0;
        gs->input.analogRY[p] = 0;

        int state = padGetState(p, 0);

        // FIX: orice stare care nu e stabila = reset total, nu citim nimic
        if (state != PAD_STATE_STABLE && state != PAD_STATE_FINDCTP1) {
            gs->input.prev[p] = 0;  // resetam si prev ca sa nu ramana butoane "apasate"
            continue;
        }

        struct padButtonStatus buttons;
        if (padRead(p, 0, &buttons) == 0) continue;

        u16 btns = buttons.btns;

        // 0xFFFF = nimic apasat, 0x0000 = toate apasate (imposibil) = date corupte
        if (btns == 0xFFFF) continue;
        if (btns == 0x0000) continue;

        gs->input.current[p] = (~btns) & 0xFFFF;

        // Analog stang - doar daca modul analog e activ
        // Verificam: daca ambele axe sunt exact 0, analogul e oprit - ignoram
        int lx = (int)buttons.ljoy_h - 128;
        int ly = (int)buttons.ljoy_v - 128;
        int rx = (int)buttons.rjoy_h - 128;
        int ry = (int)buttons.rjoy_v - 128;

        // Deadzone 70 pentru controller batran
        if (lx < -70) { gs->input.current[p] |= PAD_LEFT;  gs->input.analogLX[p] = -1; }
        if (lx >  70) { gs->input.current[p] |= PAD_RIGHT; gs->input.analogLX[p] =  1; }
        if (ly < -70) { gs->input.current[p] |= PAD_UP;    gs->input.analogLY[p] = -1; }
        if (ly >  70) { gs->input.current[p] |= PAD_DOWN;  gs->input.analogLY[p] =  1; }

        if (rx*rx + ry*ry > 70*70) {
            gs->input.analogRX[p] = rx;
            gs->input.analogRY[p] = ry;
        }
    }
}

int input_pressed(GameState *gs, int player, u32 button) {
    return (gs->input.current[player] & button) &&
          !(gs->input.prev[player]    & button);
}

int input_held(GameState *gs, int player, u32 button) {
    return (gs->input.current[player] & button) != 0;
}
