#ifndef __OPERAND_H__
#define __OPERAND_H__

enum { OP_TYPE_REG, OP_TYPE_MEM, OP_TYPE_IMM };

#define OP_STR_SIZE 40

typedef struct {
	uint32_t type;//操作数的类型
	size_t size;
	union {
		uint32_t reg;
		swaddr_t addr;
		uint32_t imm;
		int32_t simm;
	};//操作数的值
	uint32_t val;//抽象成值
	char str[OP_STR_SIZE];//打印调试信息，操作数变成汇编信息。
} Operand;

typedef struct {
	uint32_t opcode;
	bool is_operand_size_16;
	Operand src, dest, src2;
} Operands;//操作数，

#endif
