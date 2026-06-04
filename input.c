#include "liero.h"

// ============================================================
//  INPUT.C — 2 controllere DualShock 2
// ============================================================

static unsigned char pad_buf[MAX_PLAYERS][256] __attribute__((aligned(64)));
static int pad_ok[MAX_PLAYERS] = {0, 0};

void input_init(void) {
    padInit(0);
    padPortOpen(0, 0, pad_buf[0]);
    padPortOpen(1, 0, pad_buf[1]);
    pad_ok[0] = 0;
    pad_ok[1] = 0;
}

void input_update(GameState *gs) {
    int p;
    for (p = 0; p < MAX_PLAYERS; p++) {
        gs->input.prev[p] = gs->input.current[p];

        int state = padGetState(p, 0);
        if (state == PAD_STATE_STABLE || state == PAD_STATE_FINDCTP1) {
            pad_ok[p] = 1;
        }

        if (!pad_ok[p]) {
            gs->input.current[p] = 0;
            gs->input.analogLX[p] = 0;
            gs->input.analogLY[p] = 0;
            gs->input.analogRX[p] = 0;
            gs->input.analogRY[p] = 0;
            continue;
        }

        struct padButtonStatus buttons;
        if (padRead(p, 0, &buttons) != 0) {
            // buttons.btns e bitmask inversat (0 = apasat)
            gs->input.current[p] = ~buttons.btns & 0xFFFF;

            // Analog sticks: 0-255, centru = 128
            gs->input.analogLX[p] = (int)buttons.ljoy_h - 128;
            gs->input.analogLY[p] = (int)buttons.ljoy_v - 128;
            gs->input.analogRX[p] = (int)buttons.rjoy_h - 128;
            gs->input.analogRY[p] = (int)buttons.rjoy_v - 128;
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
