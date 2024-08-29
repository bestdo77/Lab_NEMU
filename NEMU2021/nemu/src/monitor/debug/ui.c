#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);
static int cmd_si(char * s){
	if(s==NULL){
		cpu_exec(1);
		return 0;
	}
	int p;
	sscanf(s,"%d",&p);
	cpu_exec(p);
	return 0;
}
static int cmd_info(char* s){
	if(strcmp(s,"r")==0){
		int i;
		for(i=R_EAX;i<=R_EDI;i++){
			printf("%s\t0x%.8x\t%d\n",regsl[i],reg_l(i),reg_l(i));
		}
		printf("$eip\t0x%08x\t%d\n", cpu.eip,cpu.eip);
	}else if(strcmp(s,"w")){

	}
	return 0;
}
static int cmd_x(char* s){
	int n,x1;
	sscanf(s,"%d %x",&n,&x1);
	int p;
	for(p=0;p<n;p++){
		if(p%4==0) printf("0x%.8x: ",x1);
		printf("0x%.8x ",swaddr_read(x1,4));
		x1+=4;
		if(p%4==3) printf("\n");
		else if(p==n-1) printf("\n");
	}
	return 0;
}
/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_p(char *args){
	bool success=true;
	bool* p=&success;
	// debug;
	printf("%d\n",expr(args,p));
	// debug;
	return 0;
}
static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help},
	{ "c", "Continue the execution of the program",cmd_c},
	{ "q", "Exit NEMU",cmd_q},
	/* TODO: Add more commands */
	{"si", "Next step",cmd_si},
	{"info","print all reg status",cmd_info},
	{"x","show next 'n' adress",cmd_x},
	{"p","Analyze regular expression",cmd_p},
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}