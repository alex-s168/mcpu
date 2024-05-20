#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "timer.h"

typedef struct {
    uint8_t interrupt;
    float target;
    struct timespec start;
} Channel;

typedef struct {
    CPU* cpu;
    Channel ch[8];
} TimerData;

void timerchip_init(TimerChip* chip, CPU* cpu) {
    TimerData* data = malloc(sizeof(TimerData));
    *chip = data;

    data->cpu = cpu;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);

    for (size_t i = 0; i < 8; i ++) {
        Channel* ch = &data->ch[i];

        ch->start = now;
        ch->interrupt = 0;
        ch->target = 0;
    }
}

void timerchip_write(TimerChip* chip, u8 addr, u8 val) {
    TimerData* data = *chip;

    if (addr <= 0x0F) {
        u8 chid = addr / 2;
        u8 idx = addr % 2;

        Channel* ch = &data->ch[chid];

        if (idx == 0) {
            float millis;
            if (chid < 4) {
                millis = ((float) val) / 255;
            }
            else {
                millis = (((float) val) / 255) * 100; // * 0.5 sec
            }
            ch->target = millis;
        }
        else {
            ch->interrupt = val;
        }

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC_RAW, &now);

        ch->start = now;
    }
    else if (addr <= 0x17) {
        u8 chid = addr - 0x10;
        Channel* ch = &data->ch[chid];

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC_RAW, &now);

        float passed = (now.tv_sec - ch->start.tv_sec) * 1000 + ((float)(now.tv_nsec - ch->start.tv_nsec)) / 1000000;
        
        if (passed < ch->target) {
            float left = ch->target - passed;
            usleep(left * 1000); // TODO: need proper cpu hold because interrupts should be able to happen during hold
        }
    }
}

void timerchip_tick(TimerChip* chip) {
    TimerData* data = *chip;
    
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC_RAW, &now);

    for (size_t i = 0; i < 8; i ++) {
        Channel* ch = &data->ch[i];

        if (ch->target == 0)
            continue;

        float millis = (now.tv_sec - ch->start.tv_sec) * 1000 + ((float)(now.tv_nsec - ch->start.tv_nsec)) / 1000000;
        if (millis >= ch->target) {
            ch->start = now;
            if (ch->interrupt != 0) {
                cpu_inter(data->cpu, ch->interrupt);
                break;
            }
        }
    }
}
