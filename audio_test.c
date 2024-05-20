#include "audio.h"
#include <stdio.h>

int main() {
    SoundChip chip;
    soundchip_init(&chip);

    soundchip_start(&chip);

#define NOISE_CH 0x000 

    soundchip_write(&chip, NOISE_CH + 1, 2);   // rise
    soundchip_write(&chip, NOISE_CH + 2, 255); // vol
    soundchip_write(&chip, NOISE_CH + 3, 0);  // len
    soundchip_write(&chip, NOISE_CH + 4, 3);  // fall

    soundchip_write(&chip, NOISE_CH + 5, 0);   // low
    soundchip_write(&chip, NOISE_CH + 6, 150); // high

// #define VOICE_CH 0x020  // voice2 = tri
#define VOICE_CH 0x00A  // voice0 = square

    soundchip_write(&chip, VOICE_CH + 1, 180);  // freq
    soundchip_write(&chip, VOICE_CH + 2, 20);   // rise
    soundchip_write(&chip, VOICE_CH + 3, 100);  // vol 
    soundchip_write(&chip, VOICE_CH + 4, 1);    // len
    soundchip_write(&chip, VOICE_CH + 5, 40);   // fall
    soundchip_write(&chip, VOICE_CH + 6, 0);    // low
    soundchip_write(&chip, VOICE_CH + 7, 255);  // high
    soundchip_write(&chip, VOICE_CH + 8, 0);    // echo (ignored)
    soundchip_write(&chip, VOICE_CH + 9, 80);   // wl (only for sqr channels

    while (1) {
        (void) getchar();
        //soundchip_write(&chip, NOISE_CH + 0, 0); // trig
        soundchip_write(&chip, VOICE_CH + 0, 0); // trig
    }

    soundchip_stop(&chip);

    return 0;
}
