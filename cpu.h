#ifndef CPU_H
#define CPU_H

#include "emu.h"

typedef enum {
    REG_PC   = 0b00000000,
    REG_SP   = 0b00000001,
    REG_BP   = 0b00000010,
    REG_EXC  = 0b00000011,
    REG_MMUb = 0b00000100,
    REG_MMUp = 0b00000101,
    REG_MMUe = 0b00000110,
    REG_INTb = 0b00000111,
    REG_INTp = 0b00001000,
    REG_INTl = 0b00001001,
    REG_FL   = 0b00001010,
    REG_PCb  = 0b00001011,
    REG_RTIp = 0b00001100,
    REG_RTIb = 0b00001101,

    REG_R0   = 0b00010000,
    REG_R1   = 0b00010001,
    REG_R2   = 0b00010010,
    REG_R3   = 0b00010011,
    REG_R4   = 0b00010100,
    REG_R5   = 0b00010101,
    REG_R6   = 0b00010110,
    REG_R7   = 0b00010111,

    REG_LEN  = 0b00011000,
} CPU_Reg;

extern const char* cpu_reg_names[REG_LEN];

typedef enum : u8 {
    E_NONE   = 0,
    E_NOINTH = 1,
    E_PAGEAV = 2,
} CPU_Except;

typedef union {
    u8 byte;
    struct {
        bool zero      : 1;
        bool overflow  : 1;
        bool underflow : 1;
        int  reserved  : 5;
    } __attribute__((packed));
} __attribute__((packed))  CPU_Reg_Flag;

typedef union {
    u8 byte;
    struct {
        bool exec     : 1;
        bool write    : 1;
        bool read     : 1;
        int  reserved : 5;
    } __attribute__((packed));
} __attribute__((packed)) CPU_Page_Entry;

typedef union {
    struct {
        u8 lo;
        u8 mid;
        u8 hi;
    } __attribute__((packed)) raw;
    struct {
        // 16b
        u16 addr      : 16;

        // 4b
        su4 bank      : 4;

        // 4b
        bool cl_int   : 1;
        bool cl_mmu   : 1;
        int reserved  : 2;
    } __attribute__((packed));
} __attribute__((packed)) CPU_Intr_Entry;

typedef enum : u8 {
    INTR_EXCEPT = 0,
} CPU_Intr_Kind;

typedef enum {
    SRCTY_REGISTER = 0,
    SRCTY_IMMEDIATE = 1,
} CPU_Instr_Source_Type;

typedef enum {
    ADDRMD_ABSOLUTE = 0b000,
    ADDRMD_PC_REL   = 0b001,
    ADDRMD_SP_REL   = 0b010,
    ADDRMD_PT_REL   = 0b011,
    ADDRMD_IT_REL   = 0b100,
    ADDRMD_INDEXED  = 0b101,
} CPU_Instr_Addr_Mode;

typedef union {
    u8 byte;
    struct {
        su4 bank : 4;
        CPU_Instr_Addr_Mode mode : 3;
        CPU_Instr_Source_Type type : 1;
    } __attribute__((packed));
} __attribute__((packed)) CPU_Instr_Addr_Header;

typedef enum {
    INSTR_nop    = 0b00000000,
    INSTR_mov    = 0b00000001, // [dest: reg],    [src:   reg]
    INSTR_imm_b  = 0b00000010, // [dest: reg],    [val:   8b  imm]
    INSTR_imm_w  = 0b00000011, // [dest: reg],    [val:   16b imm]
    INSTR_lod_b  = 0b00000100, // [dest: reg],    [src:   addr]
    INSTR_lod_w  = 0b00000101, // [dest: reg],    [src:   addr]
    INSTR_sto_b  = 0b00000110, // [dest: addr],   [src:   reg]
    INSTR_sto_w  = 0b00000111, // [dest: addr],   [src:   reg]
    INSTR_addi_b = 0b00001000, // [opr:  reg],    [val:   8b  imm]
    INSTR_addi_w = 0b00001001, // [opr:  reg],    [val:   16b imm]
    INSTR_add    = 0b00001010, // [opr:  reg],    [src:   reg]
    INSTR_subi_b = 0b00001100, // [opr:  reg],    [val:   8b  imm]
    INSTR_subi_w = 0b00001101, // [opr:  reg],    [val:   16b imm]
    INSTR_sub    = 0b00001110, // [opr:  reg],    [src:   reg]
    INSTR_clr    = 0b00010000, // [dest: reg]
    INSTR_sl4    = 0b00010001, // [opr:  reg]
    INSTR_sr4    = 0b00010010, // [opr:  reg]
    INSTR_sez    = 0b00010011,
    INSTR_clz    = 0b00010100,
    INSTR_inz    = 0b00010101,
    INSTR_not    = 0b00100000, // [opr:  reg]
    INSTR_and    = 0b00100001, // [opr:  reg],    [src:   reg]
    INSTR_andi_b = 0b00100010, // [opr:  reg],    [val:   8b  imm]
    INSTR_andi_w = 0b00100011, // [opr:  reg],    [val:   16b imm]
    INSTR_orr    = 0b00100100, // [opr:  reg],    [src:   reg]
    INSTR_shl    = 0b00100111, // [opr:  reg],    [src:   reg]
    INSTR_shli_b = 0b00101000, // [opr:  reg],    [val:   8b  imm]
    INSTR_shr    = 0b00101001, // [opr:  reg],    [src:   reg]
    INSTR_shri_b = 0b00101010, // [opr:  reg],    [val:   8b  imm]
    INSTR_xor    = 0b00101011, // [opr:  reg],    [src:   reg]
    INSTR_sxt    = 0b00101101, // [dest: reg],    [src:   reg]
    INSTR_btsi_b = 0b00110000, // [src:  reg],    [index: 8b  imm]
    INSTR_bts    = 0b00110001, // [src:  reg],    [index: reg]
    INSTR_btti_b = 0b00110010, // [dst:  reg],    [index: 8b  imm]
    INSTR_btt    = 0b00110011, // [dst:  reg],    [index: reg]
    INSTR_tst    = 0b01000000, // [src:  reg]
    INSTR_tstm_b = 0b01000001, // [src:  addr]
    INSTR_tstm_w = 0b01000010, // [src:  addr]
    INSTR_ceq    = 0b01000011, // [a:    reg],    [b:     reg]
    INSTR_clt    = 0b01000101, // [a:    reg],    [b:     reg]
    INSTR_cgt    = 0b01000111, // [a:    reg],    [b:     reg]
    INSTR_psh_b  = 0b01010000, // [val:  reg]
    INSTR_pshi_b = 0b01010001, // [val:  i8  imm]
    INSTR_psh_w  = 0b01010010, // [val:  reg]
    INSTR_pshi_w = 0b01010011, // [val:  i16 imm]
    INSTR_pll_b  = 0b01010100, // [dest: reg]
    INSTR_pll_w  = 0b01010101, // [dest: reg]
    INSTR_jmp    = 0b01100000, // [addr: addr]
    INSTR_jmz    = 0b01100001, // [addr: addr]
    INSTR_jnz    = 0b01100010, // [addr: addr]
    INSTR_cal    = 0b01100011, // [addr: addr]
    INSTR_ret    = 0b01100100,
    INSTR_int    = 0b01100101, // [id:   8b  imm]
    INSTR_rti    = 0b01100110,
    INSTR_jmf    = 0b01100111, // [addr: addr],   [bank:  8b  imm]

    INSTR_LEN    = 0b01101000,
} CPU_Instr_Kind;

extern const char* cpu_instr_names[INSTR_LEN];

typedef struct {
    void* userdata;

    su20 regs[256];
} CPU;

// implemented in emu.c
u8 mread(CPU* cpu, u16 addr, su4 bank);
void mwrite(CPU* cpu, u16 addr, su4 bank, u8 val);

u8 readsafe(bool* modified, CPU* cpu, u16 addr, su4 bank);
void writesafe(bool* modified, CPU* cpu, u16 addr, su4 bank, u8 val);
void cpu_reset(CPU *cpu);
void cpu_inter(CPU *cpu, CPU_Intr_Kind kind);
void cpu_trig_av(CPU *cpu, u16 addr, su4 bank);
CPU_Page_Entry cpu_page(CPU *cpu, u8 id);
CPU_Page_Entry cpu_page_at(CPU *cpu, u16 addr, su4 bank);
void cpu_step(CPU *cpu);

#endif
