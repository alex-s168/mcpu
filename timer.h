#include "emu.h"
#include "cpu.h"

typedef void* TimerChip;

void timerchip_init(TimerChip* chip, CPU* cpu);

void timerchip_write(TimerChip* chip, u8 addr, u8 val);

void timerchip_tick(TimerChip* chip);

// PRECISION CHANNEL
// =====================================
// 
// channels loop!
//
// length   1 byte   1 = 1ms; 0 is off; 0 is default
// int      1 byte   interrupt to trigger when done; nothing if 0; 0 is default

// CHANNEL
// =====================================
// 
// channels loop!
//
// length   1 byte   1 = 0.1s; 0 is off; 0 is default
// int      1 byte   interrupt to trigger when done; nothing if 0; 0 is default

// CHANNELS
// =====================================
//
// 0x00 - 0x01   prec0   precision channel; writing restarts channel
// 0x02 - 0x03   prec1   precision channel; writing restarts channel
// 0x04 - 0x05   prec2   precision channel; writing restarts channel
// 0x06 - 0x07   prec3   precision channel; writing restarts channel
// 0x08 - 0x09   ch4     normal channel; writing restarts channel
// 0x0A - 0x0B   ch5     normal channel; writing restarts channel
// 0x0C - 0x0D   ch6     normal channel; writing restarts channel
// 0x0E - 0x0F   ch7     normal channel; writing restarts channel
//
// 0x10          wait0   when accessed holds the cpu until channel done
// 0x11          wait1   when accessed holds the cpu until channel done
// 0x12          wait2   when accessed holds the cpu until channel done
// 0x13          wait3   when accessed holds the cpu until channel done
// 0x14          wait4   when accessed holds the cpu until channel done
// 0x15          wait5   when accessed holds the cpu until channel done
// 0x16          wait6   when accessed holds the cpu until channel done
// 0x17          wait7   when accessed holds the cpu until channel done
