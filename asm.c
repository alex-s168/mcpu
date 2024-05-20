#include <string.h>
#include <stdlib.h>
#include "cpu.h"
#include "asm.h"

static CPU_Reg get_reg(char** src, bool* found) {
    for (size_t i = 0; i < 256; i ++) {
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

static u32 get_const(char** src) {
    return strtol(*src, src, 10);
}

static CPU_Instr_Source_Type get_source(char** src, u8** dest) {
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
    for (size_t i = 0; i < 256; i ++) {
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


