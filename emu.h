#ifndef EMU_H
#define EMU_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef uint32_t u32;
typedef uint32_t su20;
typedef uint16_t u16;
typedef uint16_t su12;
typedef uint8_t  u8;
typedef uint8_t  su4;

typedef struct {
    su4 bank;
    u16 addr;
} bigaddr;

#define PAGE(id) ((id) * 4096)
#define MK20(bank, addr) (((su20)(su4)(bank)) << 16 | ((su20)(u16)(addr)))
#define MK16(low, high) (((u16)high) << 8 | ((u16)low))

#endif
