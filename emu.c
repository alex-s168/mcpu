#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "asm.h"

static u8* mem;

u8 read(CPU* cpu, u16 addr, su4 bank) {
    if (bank == 0 && addr < 4096 * 2) {
        return mem[addr];
    }
    return 0;
}

void write(CPU* cpu, u16 addr, su4 bank, u8 val) {
    if (bank == 0 && addr < 4096 * 2) {
        mem[addr] = val;
    }
}

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

void print_cpu(const CPU * cpu, int (*out)(const char *)) {
    for (size_t i = 0; i < 256; i ++) {
        const char *name = cpu_reg_names[i];
        if (name == NULL)
            continue;
        static char buf[256];
        u32 val = cpu->regs[i];
        sprintf(buf, "%s = %u = page %u + %u", name, val, val / 4096, val % 4096);
        (void) out(buf);
    }
}

static char src[] =
     "imm.b mmub, b 0"
"\n" "imm.w mmup, w 512"

"\n" "imm.b r0, b 1"
"\n" "sto.b [pt + 1], r0"

"\n" "imm.b mmue, b 1"
"\n" "imm.b r0, b 55"
"\n";

int main() {
    mem = malloc(sizeof(u8) * 4096 * 2);
    if (mem == NULL)
        return 1;

    u8* ptr = mem + PAGE(1);

    size_t line_id = 0;
    int status = 0;
    SPLITERATE(src, "\n", line) {
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
    
    static CPU cpu;
    cpu_reset(&cpu);

    for (size_t i = 0; i < 6; i ++) {
        cpu_step(&cpu);

        print_cpu(&cpu, puts);
        puts("");
    }

    printf("mem[page 0 + 512 + 1] = %u\n", mem[512 + 1]);

    return 0;
}

