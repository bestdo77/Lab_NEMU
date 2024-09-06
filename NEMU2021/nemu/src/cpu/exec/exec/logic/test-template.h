#include "cpu/exec/template-start.h"

#define instr test
static void do_execute(){
    DATA_TYPE res = op_dest->val &op_src->val;
    int len = (DATA_BYTE << 3) - 1;
    cpu.eflags.CF = 0;
    cpu.eflags.OF = 0;
    cpu.eflags.SF = res >> len;
    cpu.eflags.ZF = !res;
    res ^= res >> 4;
    res ^= res >> 2;
    res ^= res >> 1;
    cpu.eflags.PF = !(res & 1);
    print_asm_template2();
}

make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)

#include "cpu/exec/template-end.h"