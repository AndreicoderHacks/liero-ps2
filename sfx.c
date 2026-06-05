#include "liero.h"

// ============================================================
//  SFX.C — Sunete simple via audsrv
//  Daca audsrv nu e disponibil, functiile sunt no-op
// ============================================================

// Verificam daca audsrv e disponibil in ps2sdk
#if defined(__PS2__) || defined(_EE)
  #if __has_include(<audsrv.h>)
    #include <audsrv.h>
    #define HAS_AUDSRV 1
  #else
    #define HAS_AUDSRV 0
  #endif
#else
  #define HAS_AUDSRV 0
#endif

#define SFX_RATE    22050
#define MAX_SFX_SAMPLES 2048

static int sfx_ready = 0;
static short sfx_buffers[SFX_COUNT][MAX_SFX_SAMPLES];
static int   sfx_lengths[SFX_COUNT];

static void generate_tone(int idx, int freq, int duration_ms, int amplitude) {
    int samples = (SFX_RATE * duration_ms) / 1000;
    if (samples > MAX_SFX_SAMPLES) samples = MAX_SFX_SAMPLES;
    sfx_lengths[idx] = samples;

    int i;
    int s_prev2 = 0, s_prev1 = 0;
    // cos(2*PI*freq/22050) * 32767
    int cos_w = 32767 - (int)((long long)freq * freq * 94 / 10000);
    if (cos_w < 0) cos_w = 0;

    for (i = 0; i < samples; i++) {
        int s;
        if (i == 0) {
            s = amplitude / 8;
        } else {
            s = (int)((long long)2 * cos_w * s_prev1 / 32767) - s_prev2;
        }
        // Fade out
        if (i > samples * 3 / 4) {
            s = (int)((long long)s * (samples - i) / (samples / 4));
        }
        if (s > amplitude)  s = amplitude;
        if (s < -amplitude) s = -amplitude;
        sfx_buffers[idx][i] = (short)s;
        s_prev2 = s_prev1;
        s_prev1 = s;
    }
}

void sfx_init(void) {
#if HAS_AUDSRV
    int ret = audsrv_init();
    if (ret != 0) { sfx_ready = 0; return; }

    struct audsrv_fmt_t fmt;
    fmt.bits     = 16;
    fmt.freq     = SFX_RATE;
    fmt.channels = 1;
    ret = audsrv_set_format(&fmt);
    if (ret != 0) { sfx_ready = 0; return; }

    audsrv_set_volume(MAX_VOLUME);
    sfx_ready = 1;
#endif

    // Generam tonurile indiferent (folosite daca audsrv e disponibil)
    generate_tone(SFX_NAVIGATE, 440,  80,  8000);
    generate_tone(SFX_SELECT,   880,  120, 10000);
    generate_tone(SFX_BACK,     220,  100, 8000);
    generate_tone(SFX_SHOOT,    1200, 60,  12000);
    generate_tone(SFX_EXPLODE,  80,   300, 15000);
    generate_tone(SFX_HURT,     300,  150, 10000);
}

void sfx_play(GameState *gs, int sfx_id) {
#if HAS_AUDSRV
    if (!sfx_ready) return;
    if (sfx_id < 0 || sfx_id >= SFX_COUNT) return;
    if (gs->volume == 0) return;
    int vol = gs->volume * (MAX_VOLUME / 9);
    audsrv_set_volume(vol);
    audsrv_play_audio((char *)sfx_buffers[sfx_id],
                      sfx_lengths[sfx_id] * 2);
#else
    (void)gs; (void)sfx_id;
#endif
}

void sfx_set_volume(GameState *gs) {
#if HAS_AUDSRV
    if (!sfx_ready) return;
    int vol = gs->volume * (MAX_VOLUME / 9);
    audsrv_set_volume(vol);
#else
    (void)gs;
#endif
}
