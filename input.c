#include "liero.h"

// ============================================================
//  INPUT.C — 2 controllere DualShock 2
// ============================================================

static unsigned char pad_buf[MAX_PLAYERS][256] __attribute__((aligned(64)));

void input_init(void) {
    padInit(0);
    padPortOpen(0, 0, pad_buf[0]);
    padPortOpen(1, 0, pad_buf[1]);
}

void input_update(GameState *gs) {
    int p;
    for (p = 0; p < MAX_PLAYERS; p++) {
        gs->input.prev[p] = gs->input.current[p];

        int state = padGetState(p, 0);

        // Daca pad-ul nu e stabil => reset complet input
        if (state != PAD_STATE_STABLE && state != PAD_STATE_FINDCTP1) {
            gs->input.current[p] = 0;
            gs->input.analogLX[p] = 0;
            gs->input.analogLY[p] = 0;
            gs->input.analogRX[p] = 0;
            gs->input.analogRY[p] = 0;
            continue;
        }

        struct padButtonStatus buttons;
        if (padRead(p, 0, &buttons) != 0) {
            u32 raw = ~buttons.btns & 0xFFFF;

            // Daca toate butoanele par apasate = date corupte, ignoram
            if (raw == 0xFFFF) {
                gs->input.current[p] = 0;
                continue;
            }

            gs->input.current[p]  = raw;
            gs->input.analogLX[p] = (int)buttons.ljoy_h - 128;
            gs->input.analogLY[p] = (int)buttons.ljoy_v - 128;
            gs->input.analogRX[p] = (int)buttons.rjoy_h - 128;
            gs->input.analogRY[p] = (int)buttons.rjoy_v - 128;

            // Analog deadzone — centrul nu e perfect 128
            if (gs->input.analogLX[p] > -10 && gs->input.analogLX[p] < 10)
                gs->input.analogLX[p] = 0;
            if (gs->input.analogLY[p] > -10 && gs->input.analogLY[p] < 10)
                gs->input.analogLY[p] = 0;
            if (gs->input.analogRX[p] > -10 && gs->input.analogRX[p] < 10)
                gs->input.analogRX[p] = 0;
            if (gs->input.analogRY[p] > -10 && gs->input.analogRY[p] < 10)
                gs->input.analogRY[p] = 0;
        } else {
            gs->input.current[p] = 0;
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
