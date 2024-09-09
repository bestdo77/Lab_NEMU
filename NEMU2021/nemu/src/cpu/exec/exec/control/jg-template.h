#include "cpu/exec/template-start.h"

#define instr jg

static void do_execute(){
    DATA_TYPE_S imm = op_src->val;
    print_asm("jg\t%x", cpu.eip + 1 + DATA_BYTE + imm);
    if(cpu.eflags.ZF == 0 && cpu.eflags.SF == cpu.eflags.OF){
        cpu.eip += imm;
    }
    //if(DATA_BYTE == 2)cpu.eip &= 0x0000ffff;
}

make_instr_helper(i);

#include "cpu/exec/template-end.h"