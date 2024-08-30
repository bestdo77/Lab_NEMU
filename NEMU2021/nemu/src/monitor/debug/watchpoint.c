#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#define NR_WP 32

static WP wp_pool[NR_WP]={};
static WP *head, *free_;

void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}//建立链表
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}
uint32_t nr_wp=0;
/* TODO: Implement the functionality of watchpoint */
void new_wp(char* args) {
    if (free_ == NULL) {
        printf("No more watchpoints available.\n");
        assert(0);
    }
	WP *wp = free_; // 获取当前 free_ 指向的节点
    free_ = free_->next; // 更新 free_ 指向下一个节点

    // 复制 args 到 wp->args
    strncpy(wp->args, args, sizeof(wp->args) - 1);
    wp->args[sizeof(wp->args) - 1] = '\0'; // 确保空终止符存在

    // 设置其他成员变量
    wp->vis = 1;
    bool success;
    wp->value = expr(wp->args, &success);

    // 将新的 watchpoint 添加到链表头部
    wp->next = head;
    head = wp;
	printf("watchpoint %d added\n",wp->NO);
}
void free_wp(int n){
	WP *wp=&wp_pool[n];
	wp->vis=false;
	if (head == wp) {
		head = wp->next;
	} else {
		WP *prev = head;
		while (prev->next != NULL && prev->next != wp) {
			prev = prev->next;
		}
		if (prev->next == wp) {
			prev->next = wp->next;
		}
	}
	wp->next = free_;
    free_ = wp;
	printf("watchpoint %d deleted\n",n);
}

void print_watchpoints() {
    WP *current = head;
    while (current != NULL) {
        printf("Watchpoint ID: %d, Args: %s, Value: %d\n",current->NO, current->args, current->value);
        current = current->next;
    }
	printf("all watchpoint showed\n");
}

int judge_watchpoints(){
	WP *current = head;
    while (current != NULL) {
		bool success;
        int value=expr(current->args,&success);
		if(value!=current->value){
			return current->NO;
		}else{
			current->value=value;
		}
    }
	return 0;
}



