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

### PA1: 
- 发现代码中定义了很多宏，vscode中`ctrl+shift+f`可以全局查找
1. 实现寄存器结构体：
```c
#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)
//断言
assert(reg_w(i) == (sample[i] & 0xffff));
```
- 需要调整寄存器结构才能继续，这个assert就是判断一个32位寄存器的低16位是否跟其16位寄存器一样，如果不一样就不会运行
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
