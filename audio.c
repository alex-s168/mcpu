#include <complex.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "audio.h"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#define DEVICE_SAMPLE_RATE  8000

/* ========================================================================= */

void whitenoise_init() {
    srand(time(NULL));
}

void whitenoise_frame(ma_uint32 frameCount, float* data) {
    for (int i = 0; i < frameCount; i ++) {
        float accf = (((float) (uint8_t) rand()) / 127) - 1;
        data[i] = accf;
    }
}

/* ========================================================================= */

typedef struct {
    complex double state;
    float complex ph;
} SineChannel;

void sine_init(SineChannel* sd) {}

void sine_change_freq(SineChannel* sd, float fq) {
    sd->state = 1 + 0 * I;
    sd->ph = cexpf(-I * 2.0f * M_PI * fq / DEVICE_SAMPLE_RATE);
}

void sine_frame(SineChannel* sd, ma_uint32 frameCount, float* data) {
    for (int i = 0; i < frameCount; ++i) {
        // Phasor multiply
        sd->state *= sd->ph;
        // Take real part for cosine, or imaginary for sine
        data[i] = crealf(sd->state);
    }
    // Amplitude corrections through a taylor expansion around
    // abs(state) very close to 1
    float g = 0.5f * (3.0f - crealf(sd->state) * crealf(sd->state) - cimagf(sd->state) * cimagf(sd->state));
    sd->state *= g;
}

/* ========================================================================= */

typedef struct {
    float freq;
} TriChannel;

void tri_init(TriChannel* tri) {}

void tri_change_freq(TriChannel* tri, float fq) {
    tri->freq = fq;
}

void tri_frame(TriChannel* tr, ma_uint32 frameCount, float* data) {
    float period = DEVICE_SAMPLE_RATE / tr->freq;
    for (int i = 0; i < frameCount; ++i) {
        float t = (float)i / period;
        data[i] = 2.0f * fabsf(2.0f * (t - floorf(t + 0.5f))) - 1.0f;
    }
}

/* ========================================================================= */

typedef struct {
    float freq;
    float active;
} SqrChannel;

void sqr_init(SqrChannel* tri) {
    tri->active = 0;
}

void sqr_change_freq(SqrChannel* tri, float fq) {
    tri->freq = fq;
}

void sqar_change_wavelength(SqrChannel* sqr, float wl) {
    sqr->active = (1 - wl) * 2 - 1;
}

void sqr_frame(SqrChannel* tr, ma_uint32 frameCount, float* data) {
    float period = DEVICE_SAMPLE_RATE / tr->freq;
    for (int i = 0; i < frameCount; ++i) {
        float t = (float)i / period;
        float v = 2.0f * fabsf(2.0f * (t - floorf(t + 0.5f))) - 1.0f;
        if (v < tr->active)
            v = -1;
        if (v > tr->active)
            v = 1;
        data[i] = v;
    }
}

/* ========================================================================= */

typedef struct {
    uint8_t rise;
    float vol;
    uint8_t fall;
    uint8_t len;

    enum {
        STAGE_READY,
        STAGE_RISE,
        STAGE_HOLD,
        STAGE_FALL,
    } stage;

    uint8_t curr_time;
    float curr_vol;
} RiseFall;

// returns current volume
float risefall_tick(RiseFall* rf) {
    switch (rf->stage) {
    case STAGE_READY:
        return 0.0f;

    case STAGE_RISE:
        {
            if (rf->curr_time == rf->rise) {
                rf->curr_time = 0;
                rf->stage = STAGE_HOLD;
                return rf->vol;
            }
            float increase = rf->vol / rf->rise;
            rf->curr_vol += increase;
            rf->curr_time ++;
            return rf->curr_vol;
        }

    case STAGE_HOLD:
        {
            if (rf->curr_time == rf->len) {
                rf->curr_time = 0;
                rf->curr_vol = rf->vol;
                rf->stage = STAGE_FALL;
                return rf->vol;
            }
            rf->curr_time ++;
            return rf->vol;
        }

    case STAGE_FALL:
        {
            if (rf->curr_time == rf->fall) {
                rf->curr_time = 0;
                rf->curr_vol = 0;
                rf->stage = STAGE_READY;
                return 0;
            }
            float decrease = rf->vol / rf->fall;
            rf->curr_vol -= decrease;
            rf->curr_time ++;
            return rf->curr_vol;
        }
    }
}

void risefall_reset(RiseFall* rf) {
    rf->stage = STAGE_READY;
}

void risefall_trigger(RiseFall* rf) {
    rf->curr_time = 0;
    rf->curr_vol = 0;
    rf->stage = STAGE_RISE;
}

void risefall_write(RiseFall* rf, u8 addr, u8 val) {
    if (addr == 0) {
        rf->rise = ((float) val) / 255;
    }
    else if (addr == 1) {
        rf->vol = ((float) val) / 255;
    }
    else if (addr == 2) {
        rf->len = val;
    }
    else if (addr == 3) {
        rf->fall = ((float) val) / 255;
    }
    else {
        assert(false);
    }
}

/* ========================================================================= */

typedef struct {
    RiseFall rf;
    float    fmin;
    float    fmax;
} CWhiteNoiseChannel;

void chwhitenoise_init(CWhiteNoiseChannel* ch) {
    whitenoise_init();
    risefall_reset(&ch->rf);
    ch->fmin = -1;
    ch->fmax = 1;
}

void chwhitenoise_frame(CWhiteNoiseChannel* ch, ma_uint32 frameCount, float* data) {
    float level = risefall_tick(&ch->rf);

    if (level > 0) {
        whitenoise_frame(frameCount, data);
        for (int i = 0; i < frameCount; i ++) {
            if (data[i] < ch->fmin) {
                data[i] = ch->fmin;
            }
            else if (data[i] > ch->fmax) {
                data[i] = ch->fmax;
            }
        }
    }

    for (int i = 0; i < frameCount; i ++)
        data[i] *= level;
}

void chwhitenoise_write(CWhiteNoiseChannel* ch, u8 addr, u8 val) {
    if (addr == 0) {
        risefall_trigger(&ch->rf);
    }
    else if (addr >= 1 && addr <= 4) {
        risefall_write(&ch->rf, addr - 1, val);
    }
    else if (addr == 5) {
        ch->fmin = ((float) val) / 127 - 1;
    }
    else if (addr == 6) {
        ch->fmax = ((float) val) / 127 - 1;
    }
    else if (addr == 7) {
        // reserved
    }
    else {
        assert(false);
    }
}

/* ========================================================================= */

#define NUM_FREQ 210
static float frequencies[NUM_FREQ] = {
9.166667, 9.711745, 10.289235, 10.901066, 11.549275, 12.236032, 12.963624, 13.734482, 13.750000, 14.551177, 14.567617, 15.416434, 15.433853, 16.333143, 16.351599, 17.304361, 17.323914, 18.333334, 18.354048, 19.423491, 19.445436, 20.578470, 20.601723, 21.802132, 21.826765, 23.098551, 23.124651, 24.472063, 24.499714, 25.927248, 25.956543, 27.468964,27.500000, 27.500000, 29.102354, 29.135235, 29.135235, 30.832869, 30.867706, 30.867706, 32.666286, 32.703197, 32.703197, 34.608723, 34.647827, 34.647827, 36.666668, 36.708096, 36.708096, 38.846981, 38.890873, 38.890873, 41.156940, 41.203445, 41.203445, 43.604263, 43.653530, 43.653530, 46.197102, 46.249302, 46.249302, 48.944126, 48.999428, 48.999428, 51.854496, 51.913086, 51.913086, 54.937927, 55.000000, 55.000000, 58.204708, 58.270470, 58.270470, 61.665737, 61.735413, 61.735413, 65.332573, 65.406395, 65.406395, 69.217445, 69.295654, 69.295654, 73.333336, 73.416191, 73.416191, 77.693962, 77.781746, 77.781746, 82.313881, 82.406891, 82.406891, 87.208527, 87.307060, 87.307060, 92.394203, 92.498604, 92.498604, 97.888252, 97.998856, 97.998856, 103.708992, 103.826172, 103.826172, 109.875854, 110.000000, 110.000000, 116.409416, 116.540939, 116.540939, 123.331474, 123.470825, 123.470825, 130.665146, 130.812790, 130.812790, 138.434891, 138.591309, 138.591309, 146.666672, 146.832382, 146.832382, 155.387924, 155.563492, 155.563492, 164.627762, 164.813782, 164.813782, 174.417053, 174.614120, 174.614120, 184.788406, 184.997208, 184.997208, 195.776505, 195.997711, 195.997711, 207.417984, 207.652344, 207.652344, 219.751709, 220.000000, 220.000000, 232.818832, 233.081879, 233.081879, 246.662949, 246.941650, 246.941650, 261.330292, 261.625580, 261.625580, 276.869781, 277.182617, 277.182617, 293.333344, 293.664764, 293.664764, 310.775848, 311.126984, 311.126984, 329.255524, 329.627563, 329.627563, 348.834106, 349.228241, 349.228241, 369.576813, 369.994415, 369.994415, 391.553009, 391.995422, 391.995422, 414.835968, 415.304688, 415.304688, 439.503418, 440.000000, 440.000000, 465.637665, 466.163757, 466.163757, 493.325897, 493.883301, 493.883301, 523.251160, 523.251160, 554.365234, 554.365234, 587.329529, 587.329529, 622.253967, 622.253967, 659.255127, 659.255127, 698.456482, 698.456482, 739.988831, 739.988831, 783.990845, 830.609375, 880.000000, 932.327515, 987.766602, 1046.502319, 1108.730469, 1174.659058, 1244.507935, 1318.510254, 1396.912964, 1479.977661
};

/* ========================================================================= */

typedef struct {
    SqrChannel data;
    RiseFall   rf;
    float      fmin;
    float      fmax;
} CSqrChannel;

void chsqar_init(CSqrChannel* ch) {
    sqr_init(&ch->data);
    risefall_reset(&ch->rf);
    ch->fmin = -1;
    ch->fmax = 1;
}

void chsqr_frame(CSqrChannel* ch, ma_uint32 frameCount, float* data) {
    float level = risefall_tick(&ch->rf);

    if (level > 0) {
        sqr_frame(&ch->data, frameCount, data);
        for (int i = 0; i < frameCount; i ++) {
            if (data[i] < ch->fmin) {
                data[i] = ch->fmin;
            }
            else if (data[i] > ch->fmax) {
                data[i] = ch->fmax;
            }
        }
    }

    for (int i = 0; i < frameCount; i ++)
        data[i] *= level;
}

void chsqr_write(CSqrChannel* ch, u8 addr, u8 val) {
    if (addr == 0) {
        risefall_trigger(&ch->rf);
    }
    else if (addr == 1) {
        if (val >= NUM_FREQ)
            val = NUM_FREQ - 1;
        sqr_change_freq(&ch->data, frequencies[val]);
    }
    else if (addr >= 2 && addr <= 5) {
        risefall_write(&ch->rf, addr - 2, val);
    }
    else if (addr == 6) {
        ch->fmin = ((float) val) / 127 - 1;
    }
    else if (addr == 7) {
        ch->fmax = ((float) val) / 127 - 1;
    }
    else if (addr == 8) {
        // echo
    }
    else if (addr == 9) {
        sqar_change_wavelength(&ch->data, ((float) val) / 255);
    }
    else {
        assert(false);
    }
}

/* ========================================================================= */

typedef struct {
    TriChannel data;
    RiseFall   rf;
    float      fmin;
    float      fmax;
} CTriChannel;

void chtri_init(CTriChannel* ch) {
    tri_init(&ch->data);
    risefall_reset(&ch->rf);
    ch->fmin = -1;
    ch->fmax = 1;
}

void chtri_frame(CTriChannel* ch, ma_uint32 frameCount, float* data) {
    float level = risefall_tick(&ch->rf);

    if (level > 0) {
        tri_frame(&ch->data, frameCount, data);
        for (int i = 0; i < frameCount; i ++) {
            if (data[i] < ch->fmin) {
                data[i] = ch->fmin;
            }
            else if (data[i] > ch->fmax) {
                data[i] = ch->fmax;
            }
        }
    }

    for (int i = 0; i < frameCount; i ++)
        data[i] *= level;
}

void chtri_write(CTriChannel* ch, u8 addr, u8 val) {
    if (addr == 0) {
        risefall_trigger(&ch->rf);
    }
    else if (addr == 1) {
        if (val >= NUM_FREQ)
            val = NUM_FREQ - 1;
        tri_change_freq(&ch->data, frequencies[val]);
    }
    else if (addr >= 2 && addr <= 5) {
        risefall_write(&ch->rf, addr - 2, val);
    }
    else if (addr == 6) {
        ch->fmin = ((float) val) / 127 - 1;
    }
    else if (addr == 7) {
        ch->fmax = ((float) val) / 127 - 1;
    }
    else if (addr == 8) {
        // echo
    }
    else if (addr == 9) {
        // wave length
    }
    else {
        assert(false);
    }
}

/* ========================================================================= */

typedef struct {
    ma_device dev;

    CWhiteNoiseChannel noise0;
    CSqrChannel        voice0;
    CSqrChannel        voice1;
    CTriChannel        voice2;
} SoundData;

void soundchip_init(SoundChip* chip) {
    *chip = malloc(sizeof(SoundData));
    SoundData* d = *chip;

    chwhitenoise_init(&d->noise0);
    chsqar_init(&d->voice0);
    chsqar_init(&d->voice1);
    chtri_init(&d->voice2);
}

float lerpf(float a, float b, float t) {
    return (1.0 - t) * a + t * b;
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    SoundData* sd = pDevice->pUserData;
    float* data = pOutput;

    assert(frameCount == 150); // required for samples

    static float temp[150];

    chwhitenoise_frame(&sd->noise0, frameCount, data);
    
    chsqr_frame(&sd->voice0, frameCount, temp);
    for (int i = 0; i < 150; i ++) {
        data[i] += temp[i];
    }

    chsqr_frame(&sd->voice1, frameCount, temp);
    for (int i = 0; i < 150; i ++) {
        data[i] += temp[i];
    }

    chtri_frame(&sd->voice2, frameCount, temp);
    for (int i = 0; i < 150; i ++) {
        data[i] += temp[i];
    }

    for (int i = 0; i < 150; i ++) {
        float v = data[i];
        if (v > 1)
            data[i] = 1;
        else if (v < -1)
            data[i] = -1;
    }
}

void soundchip_write(SoundChip* chip, su12 addr, u8 val) {
    SoundData* sd = *chip;

    if (addr >= 0x000 && addr <= 0x009) {
        chwhitenoise_write(&sd->noise0, addr - 0x000, val);
    }
    else if (addr >= 0x00A && addr <= 0x014) {
        chsqr_write(&sd->voice0, addr - 0x00A, val);
    }
    else if (addr >= 0x015 && addr <= 0x01F) {
        chsqr_write(&sd->voice1, addr - 0x015, val);
    }
    else if (addr >= 0x020 && addr <= 0x02A) {
        chtri_write(&sd->voice2, addr - 0x020, val);
    }
}

void soundchip_start(SoundChip* chip) {
    SoundData* data = *chip;

    ma_device_config deviceConfig;

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 1;
    deviceConfig.sampleRate        = DEVICE_SAMPLE_RATE;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = data;

    if (ma_device_init(NULL, &deviceConfig, &data->dev) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        return;
    }

    printf("Device Name: %s\n", data->dev.playback.name);

    if (ma_device_start(&data->dev) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&data->dev);
        return;
    }
}

void soundchip_stop(SoundChip* chip) {
    SoundData* data = *chip;

    ma_device_uninit(&data->dev);
}

