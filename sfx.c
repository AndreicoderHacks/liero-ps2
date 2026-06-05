#include "liero.h"

// ============================================================
//  SFX.C — Sunete simple generate cu audsrv + SPU2
//  Generam tonuri PCM sinusoidale scurte pentru meniu si joc
// ============================================================

#include <audsrv.h>

static int sfx_ready = 0;

// Format audio: 22050 Hz, 16bit, mono
#define SFX_RATE    22050
#define SFX_BITS    16
#define SFX_CHAN    1

// Generam un ton simplu ca buffer PCM
// freq = frecventa Hz, duration_ms = durata in ms
#define MAX_SFX_SAMPLES 2048
static short sfx_buffers[SFX_COUNT][MAX_SFX_SAMPLES];
static int   sfx_lengths[SFX_COUNT];  // in samples

static void generate_tone(int idx, int freq, int duration_ms, int amplitude) {
    int samples = (SFX_RATE * duration_ms) / 1000;
    if (samples > MAX_SFX_SAMPLES) samples = MAX_SFX_SAMPLES;
    sfx_lengths[idx] = samples;

    int i;
    for (i = 0; i < samples; i++) {
        // Sin aproximat cu tabel fix (evitam libm pe SPU)
        // val = sin(2*PI*freq*i/rate) * amplitude
        // Folosim: sin(x) ~ x pentru unghiuri mici, dar pentru ton
        // folosim formula de recurenta: s[n] = 2*cos(w)*s[n-1] - s[n-2]
        // Initializare: s[-1]=0, s[0]=sin(w)
        // w = 2*PI*freq/rate
        // Aproximam cos(w) cu tabel
        static int s_prev2 = 0, s_prev1 = 0;
        if (i == 0) { s_prev2 = 0; s_prev1 = 0; }

        // cos(w)*32767 precomputat pentru frecvente comune
        // cos(2*PI*f/22050) ~ 1 - (2*PI*f/22050)^2 / 2
        // Pentru f=440: w=0.1254, cos(w)=0.9921, cos*32767=32512
        int cos_w;
        // Aproximare: cos_w = 32767 - (int)(freq * freq * 94 / 10000)
        cos_w = 32767 - (int)((long long)freq * freq * 94 / 10000);
        if (cos_w < 0) cos_w = 0;

        int s;
        if (i == 0) {
            s = (int)((long long)freq * 2 * 3141592 / 22050 * amplitude / 3141592);
            if (s > amplitude) s = amplitude;
        } else {
            s = (int)((long long)2 * cos_w * s_prev1 / 32767) - s_prev2;
        }

        // Fade out la sfarsit
        int fade = amplitude;
        if (i > samples * 3 / 4) {
            fade = amplitude * (samples - i) / (samples / 4);
        }
        if (s > fade) s = fade;
        if (s < -fade) s = -fade;

        sfx_buffers[idx][i] = (short)s;
        s_prev2 = s_prev1;
        s_prev1 = s;
    }
}

// ------------------------------------------------------------
//  sfx_init
// ------------------------------------------------------------
void sfx_init(void) {
    int ret = audsrv_init();
    if (ret != 0) {
        sfx_ready = 0;
        return;
    }

    struct audsrv_fmt_t fmt;
    fmt.bits     = SFX_BITS;
    fmt.freq     = SFX_RATE;
    fmt.channels = SFX_CHAN;

    ret = audsrv_set_format(&fmt);
    if (ret != 0) {
        sfx_ready = 0;
        return;
    }

    audsrv_set_volume(MAX_VOLUME);

    // Generam tonurile pentru fiecare SFX
    generate_tone(SFX_NAVIGATE, 440,  80,  8000);   // La  - click meniu
    generate_tone(SFX_SELECT,   880,  120, 10000);  // La5 - confirmare
    generate_tone(SFX_BACK,     220,  100, 8000);   // La3 - inapoi
    generate_tone(SFX_SHOOT,    1200, 60,  12000);  // crack impusca
    generate_tone(SFX_EXPLODE,  80,   300, 15000);  // boom explozie
    generate_tone(SFX_HURT,     300,  150, 10000);  // ugh lovit

    sfx_ready = 1;
}

// ------------------------------------------------------------
//  sfx_play
// ------------------------------------------------------------
void sfx_play(GameState *gs, int sfx_id) {
    if (!sfx_ready) return;
    if (sfx_id < 0 || sfx_id >= SFX_COUNT) return;
    if (gs->volume == 0) return;

    // Setam volumul inainte de play
    int vol = gs->volume * (MAX_VOLUME / 9);
    audsrv_set_volume(vol);

    audsrv_play_audio((char *)sfx_buffers[sfx_id],
                      sfx_lengths[sfx_id] * sizeof(short));
}

// ------------------------------------------------------------
//  sfx_set_volume — apelat la schimbare volum din pauza
// ------------------------------------------------------------
void sfx_set_volume(GameState *gs) {
    if (!sfx_ready) return;
    int vol = gs->volume * (MAX_VOLUME / 9);
    audsrv_set_volume(vol);
}
