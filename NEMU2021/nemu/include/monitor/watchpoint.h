#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;//监视点序号
	struct watchpoint *next;//链表下一个节点

	/* TODO: Add more members if necessary */
	char args[64];
	int value;
	bool vis;
}WP;
int judge_watchpoints();
void print_watchpoints();
#endif
