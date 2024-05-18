#include <stdint.h>
#include <stdbool.h>

typedef uint32_t u32;
typedef uint32_t su20;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef uint8_t  su4;

typedef struct {
    su4 bank;
    u16 addr;
} bigaddr;

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
} CPU_Reg;

const char* cpu_reg_names[] = {
    [REG_PC]   = "pcp",
    [REG_SP]   = "sp",
    [REG_BP]   = "bp",
    [REG_EXC]  = "exc",
    [REG_MMUb] = "mmub",
    [REG_MMUp] = "mmup",
    [REG_MMUe] = "mmue",
    [REG_INTb] = "intb",
    [REG_INTp] = "intp",
    [REG_INTl] = "inte",
    [REG_FL]   = "fl",
    [REG_PCb]  = "pcb",
    [REG_RTIp] = "rtip",
    [REG_RTIb] = "rtib",
    [REG_R0]   = "r0",
    [REG_R1]   = "r1",
    [REG_R2]   = "r2",
    [REG_R3]   = "r3",
    [REG_R4]   = "r4",
    [REG_R5]   = "r5",
    [REG_R6]   = "r6",
    [REG_R7]   = "r7",
};

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
} CPU_Instr_Kind;

const char* cpu_instr_names[] = {
    [INSTR_nop]    = "nop",
    [INSTR_mov]    = "mov",
    [INSTR_imm_b]  = "imm.b",
    [INSTR_imm_w]  = "imm.w",
    [INSTR_lod_b]  = "lod.b",
    [INSTR_lod_w]  = "jmf",
    [INSTR_sto_b]  = "sto.b",
    [INSTR_sto_w]  = "sto.w",
    [INSTR_addi_b] = "jmf",
    [INSTR_addi_w] = "addi.w",
    [INSTR_add]    = "add",
    [INSTR_subi_b] = "subi.b",
    [INSTR_subi_w] = "subi.w",
    [INSTR_sub]    = "sub",
    [INSTR_clr]    = "clr",
    [INSTR_sl4]    = "sl4",
    [INSTR_sr4]    = "sr4",
    [INSTR_sez]    = "sez",
    [INSTR_clz]    = "clz",
    [INSTR_inz]    = "inz",
    [INSTR_not]    = "not",
    [INSTR_and]    = "and",
    [INSTR_andi_b] = "andi.b",
    [INSTR_andi_w] = "andi.w",
    [INSTR_orr]    = "orr",
    [INSTR_shl]    = "shl",
    [INSTR_shli_b] = "shli.b",
    [INSTR_shr]    = "shr",
    [INSTR_shri_b] = "shri.b",
    [INSTR_xor]    = "xor",
    [INSTR_sxt]    = "sxt",
    [INSTR_btsi_b] = "btsi.b",
    [INSTR_bts]    = "bts",
    [INSTR_btti_b] = "btti.b",
    [INSTR_btt]    = "jmf",
    [INSTR_tst]    = "tst",
    [INSTR_tstm_b] = "tstm.b",
    [INSTR_tstm_w] = "tstm.w",
    [INSTR_ceq]    = "ceq",
    [INSTR_clt]    = "clt",
    [INSTR_cgt]    = "cgt",
    [INSTR_psh_b]  = "psh.b",
    [INSTR_pshi_b] = "pshi.b",
    [INSTR_psh_w]  = "psh.w",
    [INSTR_pshi_w] = "pshi.w",
    [INSTR_pll_b]  = "pll.b",
    [INSTR_pll_w]  = "pll.w",
    [INSTR_jmp]    = "jmp",
    [INSTR_jmz]    = "jmz",
    [INSTR_jnz]    = "jnz",
    [INSTR_cal]    = "cal",
    [INSTR_ret]    = "ret",
    [INSTR_int]    = "int",
    [INSTR_rti]    = "rti",
    [INSTR_jmf]    = "jmf",
};

typedef struct {
    void* userdata;

    su20 regs[256];
} CPU;

#define PAGE(id) ((id) * 4096)
#define MK20(bank, addr) (((su20)(su4)(bank)) << 16 | ((su20)(u16)(addr)))
#define MK16(low, high) (((u16)high) << 8 | ((u16)low))

u8 read(CPU* cpu, u16 addr, su4 bank);
u8 readsafe(bool* modified, CPU* cpu, u16 addr, su4 bank);
void write(CPU* cpu, u16 addr, su4 bank, u8 val);
void writesafe(bool* modified, CPU* cpu, u16 addr, su4 bank, u8 val);
void cpu_reset(CPU *cpu);
void cpu_inter(CPU *cpu, CPU_Intr_Kind kind);
void cpu_trig_av(CPU *cpu, u16 addr, su4 bank);
CPU_Page_Entry cpu_page(CPU *cpu, u8 id);
CPU_Page_Entry cpu_page_at(CPU *cpu, u16 addr, su4 bank);
void cpu_step(CPU *cpu);

void cpu_inter(CPU *cpu, CPU_Intr_Kind kind) {
    if (!cpu->regs[REG_INTl]) {
        cpu_reset(cpu);
        cpu->regs[REG_EXC] = E_NOINTH;

        return;
    }

    cpu->regs[REG_RTIb] = cpu->regs[REG_PCb];
    cpu->regs[REG_RTIp] = cpu->regs[REG_PC];

    u16 entry_addr = cpu->regs[REG_INTp] + 3 * kind;
    su4 entry_bank = cpu->regs[REG_INTb];

    static u8 by[3];
    by[0] = read(cpu, entry_addr, entry_bank);
    by[1] = read(cpu, entry_addr + 1, entry_bank);
    by[2] = read(cpu, entry_addr + 2, entry_bank);

    CPU_Intr_Entry* entry = (void*) by;

    if (entry->cl_mmu)
        cpu->regs[REG_MMUe] = false;

    if (entry->cl_int)
        cpu->regs[REG_INTl] = false;

    if (entry->addr == 0 && entry->bank == 0) {
        cpu->regs[REG_EXC] = E_NOINTH;
        return cpu_inter(cpu, INTR_EXCEPT);
    } else {
        cpu->regs[REG_PCb] = entry->bank;
        cpu->regs[REG_PC] = entry->addr;
    }
}

void cpu_reset(CPU *cpu) {
    cpu->regs[REG_PCb]  = 0;
    cpu->regs[REG_PC]   = PAGE(1);
    cpu->regs[REG_EXC]  = 0;
    cpu->regs[REG_MMUe] = false;
    cpu->regs[REG_INTl] = false;
    cpu->regs[REG_FL]   = 0;
}

void cpu_trig_av(CPU *cpu, u16 addr, su4 bank) {
    cpu->regs[REG_EXC] = E_PAGEAV;
    cpu->regs[REG_R0]  = addr;
    cpu->regs[REG_R1]  = bank;
    cpu_inter(cpu, INTR_EXCEPT);
}

CPU_Page_Entry cpu_page(CPU *cpu, u8 id) {
    u16 entry_addr = cpu->regs[REG_MMUp] + id;
    su4 entry_bank = cpu->regs[REG_MMUb];

    u8 byte = read(cpu, entry_addr, entry_bank);
    CPU_Page_Entry entry;
    entry.byte = byte;
    return entry;
}

CPU_Page_Entry cpu_page_at(CPU *cpu, u16 addr, su4 bank) {
    su20 full = MK20(bank, addr);
    u8 id = full / 4096;
    return cpu_page(cpu, id);
}

// if modified true -> cpu state modified -> should return from isntr exec and go to instr exec again
u8 readsafe(bool* modified, CPU* cpu, u16 addr, su4 bank) {
    if (cpu->regs[REG_MMUe]) {
        CPU_Page_Entry page = cpu_page_at(cpu, addr, bank);
        if (!page.read) {
            *modified = true;
            cpu_trig_av(cpu, addr, bank);
            return 69;
        }
    }

    *modified = false;
    return read(cpu, addr, bank);
}

void writesafe(bool* modified, CPU* cpu, u16 addr, su4 bank, u8 val) {
    if (cpu->regs[REG_MMUe]) {
        CPU_Page_Entry page = cpu_page_at(cpu, addr, bank);
        if (!page.write) {
            *modified = true;
            cpu_trig_av(cpu, addr, bank);
            return;
        }
    }

    *modified = false;
    return write(cpu, addr, bank, val);
}

u8 cpu_instr_byte(bool *fail, CPU* cpu) {
    u16 addr = (*((u16*) &cpu->regs[REG_PC])) ++;
    su4 bank = cpu->regs[REG_PCb];

    if (cpu->regs[REG_MMUe]) {
        CPU_Page_Entry page = cpu_page_at(cpu, addr, bank);
        if (!page.exec) {
            *fail = true;
            cpu_trig_av(cpu, addr, bank);
            return 69;
        }
    }

    *fail = false;
    return read(cpu, addr, bank);
}

u16 cpu_instr_word(bool *fail, CPU *cpu) {
    u8 low = cpu_instr_byte(fail, cpu);
    bool pf = *fail;
    u8 high = cpu_instr_byte(fail, cpu);
    if (*fail || pf) {
        *fail = true;
        return 69;
    }

    u16 w = MK16(low, high);
    return w;
}

static u16 cpu_instr_read_src(bool *fail, CPU *cpu, CPU_Instr_Source_Type type) {
    if (type == SRCTY_REGISTER) {
        u8 reg = cpu_instr_byte(fail, cpu);
        return cpu->regs[reg];
    }

    // immediate
    return cpu_instr_word(fail, cpu);
}

bigaddr cpu_instr_addr(bool *fail, CPU *cpu) {
    u8 b0 = cpu_instr_byte(fail, cpu);
    if (*fail) {
        return (bigaddr) {0};
    }

    CPU_Instr_Addr_Header e;
    e.byte = b0;

    u16 src = cpu_instr_read_src(fail, cpu, e.type);

    switch (e.mode) {
    case ADDRMD_ABSOLUTE:
        return (bigaddr) {
            .addr = src,
            .bank = e.bank 
        };

    case ADDRMD_PC_REL: 
        {
            bool neg = e.bank;
            su4 bank = cpu->regs[REG_PCb];
            u16 addr = cpu->regs[REG_PC];
            if (neg)
                addr = addr - src;
            else
                addr = addr + src;
            
            return (bigaddr) {
                .addr = addr,
                .bank = bank,
            };
        }
    
    case ADDRMD_SP_REL:
        {
            bool neg = e.bank;
            u16 addr = cpu->regs[REG_SP];
            if (neg)
                addr = addr - src;
            else
                addr = addr + src;
            
            return (bigaddr) {
                .addr = addr,
                .bank = 0b00, // TODO
            };
        }

    case ADDRMD_PT_REL:
        {
            u16 addr = cpu->regs[REG_MMUp];
            su4 bank = cpu->regs[REG_MMUb];
            addr += src;
            return (bigaddr) {
                .addr = addr,
                .bank = bank,
            };
        }

    case ADDRMD_IT_REL:
        {
            u16 addr = cpu->regs[REG_INTp];
            su4 bank = cpu->regs[REG_INTb];
            addr += src;
            return (bigaddr) {
                .addr = addr,
                .bank = bank,
            };
        }

    case ADDRMD_INDEXED: 
        {
            CPU_Reg reg = 16 + e.bank;
            u16 value = cpu->regs[reg];
            return (bigaddr) {
                .addr = src + value,
                .bank = 0b00, // TODO
            };
        }

    default:
        return (bigaddr) {0};
    }
}

static bool cpu_reg_locked(CPU* cpu, CPU_Reg reg) {
    if (reg == REG_INTl)
        goto locked;

    if ((reg == REG_MMUb || reg == REG_MMUp || reg == REG_MMUe) && cpu->regs[REG_MMUe])
        goto locked;

    if ((reg == REG_INTb || reg == REG_INTp || reg == REG_INTl) && cpu->regs[REG_INTl])
        goto locked;

    return false;

locked:
    // TOOD: fault
    return true;
}

void cpu_step(CPU* cpu) {
    bool fail;
    
    CPU_Instr_Kind opcode = cpu_instr_byte(&fail, cpu);
    if (fail)
        return;

    switch (opcode) {
    case INSTR_nop: // nop
        {

        } break;

    case INSTR_mov: // mov
        {
            u8 dest = cpu_instr_byte(&fail, cpu);
            bool pf = fail;

            u8 src = cpu_instr_byte(&fail, cpu);
            if (fail || pf)
                return;

            if (cpu_reg_locked(cpu, dest))
                return;

            cpu->regs[dest] = cpu->regs[src];
        } break;

    case INSTR_imm_b: // imm.b
        {
            u8 dest = cpu_instr_byte(&fail, cpu);
            bool pf = fail;

            u8 val = cpu_instr_byte(&fail, cpu);
            if (fail || pf)
                return;

            if (cpu_reg_locked(cpu, dest))
               return;

            cpu->regs[dest] = val;
        } break;

    case INSTR_imm_w: // imm.w
        {
            u8 dest = cpu_instr_byte(&fail, cpu);
            bool pf = fail;

            u16 val = cpu_instr_word(&fail, cpu);

            if (cpu_reg_locked(cpu, dest) || fail || pf)
                return;

            cpu->regs[dest] = val;
        } break;

    case INSTR_lod_b: // lod.b
        {
            u8 dest = cpu_instr_byte(&fail, cpu);
            bool pf = fail;

            bigaddr addr = cpu_instr_addr(&fail, cpu);

            if (cpu_reg_locked(cpu, dest) || fail || pf)
                return;

            u8 val = readsafe(&fail, cpu, addr.addr, addr.bank);
            if (fail)
                return;

            cpu->regs[dest] = val;
        } break;

    case INSTR_lod_w: // lod.w
        {
            u8 dest = cpu_instr_byte(&fail, cpu);
            bool pf = fail;

            bigaddr addr = cpu_instr_addr(&fail, cpu);

            if (cpu_reg_locked(cpu, dest) || fail || pf)
                return;

            u8 low = readsafe(&fail, cpu, addr.addr, addr.bank);
            if (fail)
                return;
            u8 high = readsafe(&fail, cpu, addr.addr + 1, addr.bank);
            if (fail)
                return;

            cpu->regs[dest] = MK16(low, high);
        } break;

    case INSTR_sto_b: // sto.b
        {
            bigaddr addr = cpu_instr_addr(&fail, cpu);
            bool pf = fail;

            u8 src = cpu_instr_byte(&fail, cpu);
            if (fail || pf)
                return;

            u8 val = cpu->regs[src];

            writesafe(&fail, cpu, addr.addr, addr.bank, val);
        } break;

    case INSTR_sto_w: // sto.w
        {
            bigaddr addr = cpu_instr_addr(&fail, cpu);
            bool pf = fail;

            u8 src = cpu_instr_byte(&fail, cpu);
            if (fail || pf)
                return;

            u16 val = cpu->regs[src];
            u8 low = val & 0xFF;
            u8 high = (val >> 8) & 0xFF;

            writesafe(&fail, cpu, addr.addr, addr.bank, low);
            if (fail)
                return;

            writesafe(&fail, cpu, addr.addr + 1, addr.bank, high);
        } break;

    default:
        break;
    }
}




#include <string.h>
#include <stdlib.h>

static CPU_Reg get_reg(char** src, bool* found) {
    for (size_t i = 0; i < sizeof(cpu_reg_names) / sizeof(*cpu_reg_names); i ++) {
        if (cpu_reg_names[i] == NULL)
            continue;
        size_t len = strlen(cpu_reg_names[i]);
        if (memcmp(*src, cpu_reg_names[i], len) == 0) {
            *found = true;
            *src += len;
            return i;
        }
    }
    *found = false;
    return 0;
}

u32 get_const(char** src) {
    return strtol(*src, src, 10);
}

CPU_Instr_Source_Type get_source(char** src, u8** dest) {
    bool is_reg;
    CPU_Reg reg = get_reg(src, &is_reg);
    if (is_reg) {
        *(*dest)++ = reg;
        return SRCTY_REGISTER;
    }

    *((u16*)(*dest)) = get_const(src);
    (*dest) += 2;
    return SRCTY_IMMEDIATE;
}

int assemble(char* src, u8** dest) {
    src += strspn(src, " ");

    char* args = strchrnul(src, ' ');
    if (*args != '\0') {
        *args = '\0';
        args ++;
    }

    CPU_Instr_Kind instr;
    bool found = false;
    for (size_t i = 0; i < sizeof(cpu_instr_names) / sizeof(*cpu_instr_names); i ++) {
        if (cpu_instr_names[i] == NULL)
            continue;
        if (strcmp(cpu_instr_names[i], src) == 0) {
            found = true;
            instr = i;
            break;
        }
    }
    if (!found)
        return 1;

    *(*dest)++ = instr;

    while (*args != '\0') {
        args += strspn(args, ", ");
        if (*args == '\0')
            break;

        if (*args == 'w') {
            args ++;
            args += strspn(args, " ");
            u16 v = get_const(&args);
            *((u16*)(*dest)) = v;
            (*dest) += 2;
        }
        else if (*args == 'b') {
            args ++;
            args += strspn(args, " ");
            u8 v = get_const(&args);
            *(*dest)++ = v;
        }
        else if (*args == '[') {
            args ++;
            // valid:
            // [123]
            // [00: 123]
            // [00: r0]
            // [r0]
            // [r0 + 123]
            // [r0 + r0]
            // [pc + r0]
            // [pc - r0]
            // [sp + r0]
            // [sp - r0]
            // [pt + r0]
            // [it + r0]
            // [pc + 123]
            // [pc - 123]
            // [sp + 123]
            // [sp - 123]
            // [pt + 123]
            // [it + 123]
            
            u8 bank = 0;
            char* sep = strchr(args, ':');
            if (sep) {
                *sep = '\0';
                char* ign = args;
                bank = get_const(&ign);
                args = sep + 1;
            }

            if (memcmp(args, "pc", 2) == 0) {
                args += 2;
                args += strspn(args, " ");

                char diff = *args++;
                
                args += strspn(args, " ");
                u8* pheader = (*dest)++;
                    
                CPU_Instr_Addr_Header header;
                header.mode = ADDRMD_PC_REL;
                header.type = get_source(&args, dest); 

                if (diff == '+') 
                    header.bank = 0;
                else if (diff == '-')
                    header.bank = 1;
                else
                    return 1;

                *pheader = header.byte;
            }
            else if (memcmp(args, "sp", 2) == 0) {
                args += 2;
                args += strspn(args, " ");

                char diff = *args++;
                
                args += strspn(args, " ");
                u8* pheader = (*dest)++;
                    
                CPU_Instr_Addr_Header header;
                header.mode = ADDRMD_SP_REL;
                header.type = get_source(&args, dest); 

                if (diff == '+') 
                    header.bank = 0;
                else if (diff == '-')
                    header.bank = 1;
                else
                    return 1;

                *pheader = header.byte;
            }
            else if (memcmp(args, "pt", 2) == 0) {
                args += 2;
                args += strspn(args, " ");
            
                char diff = *args++;
                if (diff != '+')
                    return 1;
                
                args += strspn(args, " ");
                u8* pheader = (*dest)++;
                    
                CPU_Instr_Addr_Header header;
                header.mode = ADDRMD_PT_REL;
                header.type = get_source(&args, dest); 

                *pheader = header.byte;
            }
            else if (memcmp(args, "it", 2) == 0) {
                args += 2;
                args += strspn(args, " ");

                char diff = *args++;
                if (diff != '+')
                    return 1;
                
                args += strspn(args, " ");
                u8* pheader = (*dest)++;
                    
                CPU_Instr_Addr_Header header;
                header.mode = ADDRMD_IT_REL;
                header.type = get_source(&args, dest); 

                *pheader = header.byte;
            }
            else if (*args == 'r') {
                args++;
                int id = (*args ++) - '0';
                args += strspn(args, " ");

                char diff = *args++;
                if (diff != '+')
                    return 1;
                
                args += strspn(args, " ");
                u8* pheader = (*dest)++;
                    
                CPU_Instr_Addr_Header header;
                header.mode = ADDRMD_INDEXED;
                header.type = get_source(&args, dest); 
                header.bank = id;

                *pheader = header.byte;
            }
            else {
                u8* pheader = *dest;
                    
                CPU_Instr_Addr_Header header;
                header.mode = ADDRMD_ABSOLUTE;
                header.type = get_source(&args, dest); 
                header.bank = bank;

                *pheader = header.byte;
            }

            if (*args != ']')
                return 1;
            args ++;
        }
        else {
            CPU_Reg reg = get_reg(&args, &found);
            if (!found)
                return 1;
            *(*dest)++ = reg;
        }
    }

    return 0;
}



#include <stdlib.h>
#include <stdio.h>

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

        printf("pc:  %i = page %i + %i\n", cpu.regs[REG_PC], cpu.regs[REG_PC] / 4096, cpu.regs[REG_PC] % 4096);
        printf("exc: %i\n", cpu.regs[REG_EXC]);
        printf("r0:  %i\n", cpu.regs[REG_R0]);
        puts("");
    }

    printf("mem[page 0 + 512 + 1] = %u\n", mem[512 + 1]);

    return 0;
}
