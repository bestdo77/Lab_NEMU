#include "cpu/exec/template-start.h"

#define instr cmp

static void do_execute(){
    DATA_TYPE res = op_dest->val - op_src->val;
    if(op_dest->val < op_src->val)cpu.eflags.CF = 1;
    else cpu.eflags.CF = 0;
    cpu.eflags.SF = res >> (DATA_BYTE *8 - 1);
    int n1, n2;
    n1 = op_dest->val >> (DATA_BYTE * 8 - 1);
    n2 = op_src->val >> (DATA_BYTE * 8 - 1);
    cpu.eflags.OF = ((n1 != n2) && (n2 == cpu.eflags.SF));
    cpu.eflags.ZF = !res;
    res ^= res >> 4;
    res ^= res >> 2;
    res ^= res >> 1;
    cpu.eflags.PF = !(res & 1);
    if(((op_dest->val & 0xf) - (op_src->val & 0xf)) >> 4)cpu.eflags.AF = 1;
    else cpu.eflags.AF = 0;
    print_asm_template2();
}

#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif

make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)

#include "cpu/exec/template-end.h"