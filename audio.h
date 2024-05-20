#include "emu.h"

typedef void* SoundChip;

// setup
void soundchip_init(SoundChip* chip);

// accessing
//  4096 bytes = page
void soundchip_write(SoundChip* chip, su12 addr, u8 val);

// listening
void soundchip_start(SoundChip* chip);
void soundchip_stop(SoundChip* chip);

// RISE & FALL
// ========================================================
//
// rise   1 byte (0-256)   ~53 needed to have 1 second rise
// volume 1 byte (0-256)
// length 1 byte (0-256)   ~53 needed to have 1 second length
// fall   1 byte (0-256)   ~53 needed to have 1 second fall
//      = 4
//
// |   |_________|   |
// |  /|         |\  |
// | / |         | \ |
// |/  |         |  \|
// |   |         |   |
// rise  length  fall

// SAMPLES
// ========================================================
//
// samples are 128 bytes long
// a sample lasts 1 / 53.33 seconds
// (stretched to 150 samples at 8000hz sample rate)

// NOISE CHANNELS
// ========================================================
//
// trig      1 byte  virtual! plays one beat when addressed; cancels current beat
// rise&fall 4 byte
// low       1 byte  minimum allowed
// high      1 byte  maximum allowed
// reserved  1 byte 
//         = 8

// MELODY CHANNELS
// ========================================================
//
// sine, sqare, triangle
//
// trig      1 byte virtual! plays one beat when addressed; cancels current beat
// freq      1 byte
// rise&fall 4 byte
// low       1 byte  minimum allowed
// high      1 byte  maximum allowed
// echo      1 byte  intensity of echo
// wv length 1 byte  only for sqare
//         = 10 

// CHANNELS
// ========================================================
//
// 0x000 - 0x009   noise   white noise 0
// 0x00A - 0x014   voice   square wave 0
// 0x015 - 0x01F   voice   square wave 1
// 0x020 - 0x02A   voice   triangle wave 0
