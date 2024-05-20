#include "cpu.h"

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

    CPU_Intr_Entry entry;
    entry.raw.lo = read(cpu, entry_addr, entry_bank);
    entry.raw.mid = read(cpu, entry_addr + 1, entry_bank);
    entry.raw.hi = read(cpu, entry_addr + 2, entry_bank);

    if (entry.cl_mmu)
        cpu->regs[REG_MMUe] = false;

    if (entry.cl_int)
        cpu->regs[REG_INTl] = false;

    if (entry.addr == 0 && entry.bank == 0) {
        cpu->regs[REG_EXC] = E_NOINTH;
        return cpu_inter(cpu, INTR_EXCEPT);
    } else {
        cpu->regs[REG_PCb] = entry.bank;
        cpu->regs[REG_PC] = entry.addr;
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

static u8 cpu_instr_byte(bool *fail, CPU* cpu) {
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

static u16 cpu_instr_word(bool *fail, CPU *cpu) {
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

static bigaddr cpu_instr_addr(bool *fail, CPU *cpu) {
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

    case INSTR_addi_b: // [opr:  reg],    [val:   8b  imm]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;

            u8 val = cpu_instr_byte(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 opr_val = cpu->regs[opr];
            opr_val += val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_addi_w: // [opr:  reg],    [val:   16b imm]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;

            u16 val = cpu_instr_word(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 opr_val = cpu->regs[opr];
            opr_val += val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_add: // [opr:  reg],    [src:   reg]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;
           
            u8 src = cpu_instr_byte(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 val = cpu->regs[src];

            u16 opr_val = cpu->regs[opr];
            opr_val += val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_subi_b: // [opr:  reg],    [val:   8b  imm]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;
           
            u8 val = cpu_instr_byte(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 opr_val = cpu->regs[opr];
            opr_val -= val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_subi_w: // [opr:  reg],    [val:   16b imm]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;

            u16 val = cpu_instr_word(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 opr_val = cpu->regs[opr];
            opr_val -= val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_sub: // [opr:  reg],    [src:   reg]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;
           
            u8 src = cpu_instr_byte(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 val = cpu->regs[src];

            u16 opr_val = cpu->regs[opr];
            opr_val -= val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_clr: // [dest: reg]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            if (fail || cpu_reg_locked(cpu, opr))
                return;

            cpu->regs[opr] = 0;
        } break;

    case INSTR_sl4: // [opr:  reg]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            if (fail || cpu_reg_locked(cpu, opr))
                return;

            cpu->regs[opr] <<= 4;
        } break;

    case INSTR_sr4: // [opr:  reg]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            if (fail || cpu_reg_locked(cpu, opr))
                return;

            cpu->regs[opr] >>= 4;
        } break;

    case INSTR_sez:
        {
            CPU_Reg_Flag flags;
            flags.byte = cpu->regs[REG_FL];

            flags.zero = true;

            cpu->regs[REG_FL] = flags.byte;
        } break;

    case INSTR_clz:
        {
            CPU_Reg_Flag flags;
            flags.byte = cpu->regs[REG_FL];

            flags.zero = false;

            cpu->regs[REG_FL] = flags.byte;
        } break;

    case INSTR_inz:
        {
            CPU_Reg_Flag flags;
            flags.byte = cpu->regs[REG_FL];

            flags.zero ^= 1;

            cpu->regs[REG_FL] = flags.byte;
        } break;

    case INSTR_not: // [opr:  reg]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            if (fail || cpu_reg_locked(cpu, opr))
                return;

            cpu->regs[opr] = ~ cpu->regs[opr];
        } break;

    case INSTR_and: // [opr:  reg],    [src:   reg]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;
           
            u8 src = cpu_instr_byte(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 val = cpu->regs[src];

            u16 opr_val = cpu->regs[opr];
            opr_val &= val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_andi_b: // [opr:  reg],    [val:   8b  imm]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;

            u8 val = cpu_instr_byte(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 opr_val = cpu->regs[opr];
            opr_val &= val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_andi_w: // [opr:  reg],    [val:   16b imm]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;

            u16 val = cpu_instr_word(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 opr_val = cpu->regs[opr];
            opr_val &= val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_orr: // [opr:  reg],    [src:   reg]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;
           
            u8 src = cpu_instr_byte(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 val = cpu->regs[src];

            u16 opr_val = cpu->regs[opr];
            opr_val |= val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_shl: // [opr:  reg],    [src:   reg]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;
           
            u8 src = cpu_instr_byte(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 val = cpu->regs[src];

            u16 opr_val = cpu->regs[opr];
            opr_val <<= val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_shli_b: // [opr:  reg],    [val:   8b  imm]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;

            u8 val = cpu_instr_byte(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 opr_val = cpu->regs[opr];
            opr_val <<= val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_shr: // [opr:  reg],    [src:   reg]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;

            u8 src = cpu_instr_byte(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 val = cpu->regs[src];

            u16 opr_val = cpu->regs[opr];
            opr_val >>= val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_shri_b: // [opr:  reg],    [val:   8b  imm]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;

            u8 val = cpu_instr_byte(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 opr_val = cpu->regs[opr];
            opr_val >>= val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_xor: // [opr:  reg],    [src:   reg]
        {
            CPU_Reg opr = cpu_instr_byte(&fail, cpu);
            bool pf = fail;
           
            u8 src = cpu_instr_byte(&fail, cpu);

            if (cpu_reg_locked(cpu, opr) || fail || pf)
                return;

            u16 val = cpu->regs[src];

            u16 opr_val = cpu->regs[opr];
            opr_val ^= val;
            cpu->regs[opr] = opr_val;
        } break;

    case INSTR_sxt: // [dest: reg],    [src:   reg]
        {
            fprintf(stderr, "sxt not implemented!\n");
        } break;

    case INSTR_btsi_b: // [src:  reg],    [index: 8b  imm]
        {

        } break;

    case INSTR_bts: // [src:  reg],    [index: reg]
        {

        } break;

    case INSTR_btti_b: // [dst:  reg],    [index: 8b  imm]
        {

        } break;

    case INSTR_btt: // [dst:  reg],    [index: reg]
        {

        } break;

    case INSTR_tst: // [src:  reg]
        {

        } break;

    case INSTR_tstm_b: // [src:  addr]
        {

        } break;

    case INSTR_tstm_w: // [src:  addr]
        {

        } break;

    case INSTR_ceq: // [a:    reg],    [b:     reg]
        {

        } break;

    case INSTR_clt: // [a:    reg],    [b:     reg]
        {

        } break;

    case INSTR_cgt: // [a:    reg],    [b:     reg]
        {

        } break;

    case INSTR_psh_b: // [val:  reg]
        {

        } break;

    case INSTR_pshi_b: // [val:  i8  imm]
        {

        } break;

    case INSTR_psh_w : // [val:  reg]
        {

        } break;

    case INSTR_pshi_w: // [val:  i16 imm]
        {

        } break;

    case INSTR_pll_b: // [dest: reg]
        {

        } break;

    case INSTR_pll_w: // [dest: reg]
        {

        } break;

    case INSTR_jmp: // [addr: addr]
        {

        } break;

    case INSTR_jmz: // [addr: addr]
        {

        } break;

    case INSTR_jnz: // [addr: addr]
        {

        } break;

    case INSTR_cal: // [addr: addr]
        {

        } break;

    case INSTR_ret:
        {

        } break;

    case INSTR_int: // [id:   8b  imm]
        {

        } break;

    case INSTR_rti:
        {

        } break;

    case INSTR_jmf: // [addr: addr],   [bank:  8b  imm]
        {

        } break;
    }
}
