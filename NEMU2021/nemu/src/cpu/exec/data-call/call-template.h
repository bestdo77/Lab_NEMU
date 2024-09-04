#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_,SUFFIX)){
    //根据后缀译码
    int len= concat(decode_i_,SUFFIX)(cpu.eip+1);
    //栈指针向下移动一位
    reg_l(R_ESP) -= DATA_BYTE;
    swaddr_write(reg_l(R_ESP),4,cpu.eip+len+1);

    DATA_TYPE_S imm= op_src -> val;
    print_asm("call\t%x",cpu.eip+1+len+imm);
    cpu.eip += imm;
    return len+1;
}

#include "cpu/exec/template-end.h"