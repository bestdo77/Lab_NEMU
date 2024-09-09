#include "cpu/exec/template-start.h"

#define instr jbe

static void do_execute(){
    DATA_TYPE_S imm = op_src->val;
    print_asm("jbe %x",cpu.eip + DATA_BYTE+ 1 + imm);
    if(cpu.eflags.CF == 1 || cpu.eflags.ZF == 1) cpu.eip += imm;
}

make_instr_helper(i);

#include "cpu/exec/template-end.h"