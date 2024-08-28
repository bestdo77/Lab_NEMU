### PA0: 学会了基本的git使用方式
 - git add <文件名> 表示把文件添加到暂存区，git add .表示把当前目录添加到暂存区
 - git commit -m "" 提供描述性信息
 - git push 上传到远程仓库
 - git branch 看自己处于的分支
 - git checkout -b pa0 建立分支并进入
 - git status 检查状态
 - git push --set-upstream origin pa0 设置pa0的上游分支
 - git log git日志
 - git revert <操作id> 可以回退操作

### PA1: 简易调试器
- 发现代码中定义了很多宏，vscode中`ctrl+shift+f`可以全局查找
#### 思考题1：`opcode_table`数组是什么类型？
- 查找源码有以下定义：
```c
helper_fun opcode_table [256] = {
	......
}
typedef int (*helper_fun)(swaddr_t)
typedef uint32_t swaddr_t;
```
- 查阅资料得知：`helper_fun`是一个函数指针的名字，能接受`swaddr_t`也就是`uint32_t`的参数，返回`int`值。
- opcode_table是一个函数指针数组,里面放的就是对不同内存的操作指令，有些还需要补全。
#### 思考题2：在`cmd_c()`函数中调用`cpu_exec()`时为什么传入参数 $-1$?
 - $-1$ 被传入参数 $n$ 里面，而 $n$ 是个`uint32_t`类型，也就代表着实际上传入的是无符号整数的最大值，来确保程序循环次数足够。
#### 实现寄存器结构体：
```c
#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
//断言
assert(reg_w(i) == (sample[i] & 0xffff));
```
- 需要调整寄存器结构才能继续，这个assert就是判断一个32位寄存器的低16位是否跟其16位寄存器一样，如果不一样就不会运行。
- union中套struct：
```c
union{
	union{
		uint32_t _32;
		uint16_t _16;
		uint8_t _8[2];
	} gpr[8];

	/* Do NOT change the order of the GPRs' definitions. */

	struct{
		uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
	};
};
```
- 每个寄存器（eax，ecx等）与一个union（寄存器对应的地址）组成一个union，而寄存器之间的存储地址是分开的。

#### 添加调试功能：
- 手册中没写去哪里添加，于是开始读源码，程序主要在`ui_mainloop`中运行：
```c
for(i = 0; i < NR_CMD; i ++) {
	if(strcmp(cmd, cmd_table[i].name) == 0) {
		if(cmd_table[i].handler(args) < 0) { return; }
		break;
	}
}
```
- 这段代码表明了每当读入命令时，就要去这个`cmd_table`中寻找是否有命令，意味着应该是到这里添加调试功能。
- cmd_table:
```c
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

};
 
```
1. 单步执行的实现：
	- 这里只提供了`int (*handler) (char *);`的函数指针，而给定的是一个`void cpu_exec(uint32_t);`函数，所以我额外写了个函数调用
	```c
		int cmd_si(char * s){
		if(s==NULL){
			cpu_exec(1);
			return 0;
		}
		int p=0,i=0;
		for(i=0;i<strlen(s);i++){
			p*=10;
			p+=s[i]-'0';
		}
		// printf("%d\n",p);
		// if(p==0) p=1;
		cpu_exec(p);
		return 0;
	}
	```
2. 打印寄存器: