#include "liero.h"

// ============================================================
//  INPUT.C — adaptat din Minicraft PS2 (logica care functioneaza)
// ============================================================

static u8 padBuf[MAX_PLAYERS][256] __attribute__((aligned(64)));

void input_init(void) {
    padInit(0);
    padPortOpen(0, 0, padBuf[0]);
    padPortOpen(1, 0, padBuf[1]);

    // Wait 120 iteratii ca in Minicraft — asta e secretul
    int i;
    for (i = 0; i < 120; i++) {
        padGetState(0, 0);
        padGetState(1, 0);
    }
}

void input_update(GameState *gs) {
    int p;
    for (p = 0; p < MAX_PLAYERS; p++) {
        gs->input.prev[p]    = gs->input.current[p];
        gs->input.current[p] = 0;
        gs->input.analogLX[p] = 0;
        gs->input.analogLY[p] = 0;
        gs->input.analogRX[p] = 0;
        gs->input.analogRY[p] = 0;

        int state = padGetState(p, 0);
        if (state == PAD_STATE_STABLE || state == PAD_STATE_FINDCTP1) {
            struct padButtonStatus buttons;
            if (padRead(p, 0, &buttons) != 0) {
                u16 btns = buttons.btns;

                // btns == 0xFFFF = nimic apasat
                if (btns != 0xFFFF) {
                    gs->input.current[p] = (~btns) & 0xFFFF;
                } else {
                    gs->input.current[p] = 0;
                }

                // Analog stang — adaugam si ca D-pad
                int lx = (int)buttons.ljoy_h - 128;
                int ly = (int)buttons.ljoy_v - 128;
                if (lx < -40) { gs->input.current[p] |= PAD_LEFT;  gs->input.analogLX[p] = -1; }
                if (lx >  40) { gs->input.current[p] |= PAD_RIGHT; gs->input.analogLX[p] =  1; }
                if (ly < -40) { gs->input.current[p] |= PAD_UP;    gs->input.analogLY[p] = -1; }
                if (ly >  40) { gs->input.current[p] |= PAD_DOWN;  gs->input.analogLY[p] =  1; }

                // Analog drept — pentru tintire
                int rx = (int)buttons.rjoy_h - 128;
                int ry = (int)buttons.rjoy_v - 128;
                if (rx < -10) gs->input.analogRX[p] = rx;
                if (rx >  10) gs->input.analogRX[p] = rx;
                if (ry < -10) gs->input.analogRY[p] = ry;
                if (ry >  10) gs->input.analogRY[p] = ry;
            }
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
