#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "asm.h"
#include "timer.h"
#include "audio.h"

#define SPLITERATE(str,split,p) for (char *p = strtok(str, split); p != NULL; p = strtok(NULL, split))

const char * bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

void print_byte(u8 byte) {
    printf("%s%s", bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
}

CPU_Reg watched[] = {
    REG_PC,
    REG_R0,
    REG_R1,
};

void print_cpu(const CPU * cpu, int (*out)(const char *)) {
    for (size_t i = 0; i < sizeof(watched) / sizeof(*watched); i ++) {
        const char *name = cpu_reg_names[watched[i]];
        if (name == NULL)
            continue;
        static char buf[256];
        u32 val = cpu->regs[watched[i]];
        sprintf(buf, "%s = %u = page %u + %u", name, val, val / 4096, val % 4096);
        (void) out(buf);
    }
}

static int assemble_file_into(const char *file, u8 *ptr) {
    FILE* src = fopen(file, "r");

    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    size_t line_id = 0;
    int status = 0;

    while ((read = getline(&line, &len, src)) != -1) {
        printf("%s   ", line);
        u8* old = ptr;
        int s = assemble(line, &ptr);
        status |= s;
        if (s != 0) {
            fprintf(stderr, "error in line %zu\n", line_id + 1);
            puts("???");
        } else {
            while (old < ptr) {
                print_byte(*old);
                printf(" ");
                old ++;
            }
            puts("");
        }
        line_id ++;
    }

    fclose(src);

    return status;
}

static u8* mem;
static SoundChip sc;
static TimerChip tc;

u8 mread(CPU* cpu, u16 addr, su4 bank) {
    if (bank == 0) { 
        if (addr < PAGE(2)) {
            return mem[addr];
        }
        else if (addr < PAGE(3)) {
            return 0;
        }
        else if (addr < PAGE(4)) {
            return 0;
        }
    }
    return 0;
}

void mwrite(CPU* cpu, u16 addr, su4 bank, u8 val) {
    if (bank == 0) { 
        if (addr < PAGE(2)) {
            mem[addr] = val;
        }
        else if (addr < PAGE(3)) {
            soundchip_write(&sc, addr - PAGE(2), val);
        }
        else if (addr < PAGE(4)) {
            timerchip_write(&tc, addr - PAGE(3), val);
        }
    }
}

int main() {
    mem = malloc(sizeof(u8) * PAGE(2));
    if (mem == NULL)
        return 1;

    {
        u8* ptr = mem + PAGE(1);
        int status = assemble_file_into("test.asm", ptr);
        if (status != 0)
            return status;
    }
    
    static CPU cpu;
    cpu_reset(&cpu);

    timerchip_init(&tc, &cpu);
    soundchip_init(&sc);
    soundchip_start(&sc);

    while(true) {
        timerchip_tick(&tc);
        cpu_step(&cpu);

        print_cpu(&cpu, puts);
        puts("");
    }

    soundchip_stop(&sc);

    return 0;
}

