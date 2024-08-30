#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	/* TODO: Add more token types */
	NUM,
	ADR,
	NOTYPE,
	ADD,
	SUB,
	FU,
	MUL,
	DIV,
	LEFT,
	RIGHT,
	EQ,
	NEQ,
	AND,
	OR,
	NOT,
	XING,//解引用
};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{"[0-9]+", NUM},            // 数字
	{"0x[0-9A-Fa-f]+",ADR},   //地址
	{" +", NOTYPE},           // 空白字符
	{"\\+", ADD},             // 加号
	{"-", SUB},               // 减号
	{"-",FU},				  // 负号在减号之后
	{"\\*", MUL},             // 乘号
	{"/", DIV},               // 除号
	{"\\(", LEFT},            // 左括号
	{"\\)", RIGHT},           // 右括号
	{"==", EQ},               // 等于
	{"!=", NEQ},              // 不等于
	{"&&", AND},              // 逻辑与
	{"\\|\\|", OR},           // 逻辑或
	{"!", NOT},               // 逻辑非
	{"\\*", XING},            // 解引用
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	// int fu;
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch; // 储存匹配结果

	nr_token = 0; // 已有的token数量
	bool fu=0;
	while (e[position] != '\0') {
		// tokens[nr_token].fu=0;
		for (i = 0; i < NR_REGEX; i++) {
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position; // 匹配成功子串的起始地址
				int substr_len = pmatch.rm_eo; // 匹配成功字符串长度

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);

				position += substr_len;
				if (rules[i].token_type == NOTYPE) break;

				/* 确保不会超过数组的最大容量。 */
				if (nr_token < 32) {
					/* 确保字符串加上终止符不会超出容量。 */
					if (substr_len < 32) {
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len] = '\0'; // 确保字符串以空字符结束
					}
					tokens[nr_token].type = rules[i].token_type;
					if(tokens[nr_token].type==SUB){
						if(nr_token==0||(tokens[nr_token-1].type!=NUM&&tokens[nr_token-1].type!=ADR&&tokens[nr_token-1].type!=RIGHT)){
							tokens[nr_token].type=FU;
							printf("FU\n");
						}
					}
					nr_token++;
				} else {
					printf("too much tokens\n");
					assert(0);
				}
				// fu=0;
				if(strcmp(tokens[nr_token-1].str,"-")==0&&(nr_token==1||(tokens[nr_token-2].type!=NUM&&tokens[nr_token-2].type!=RIGHT))){//如果当前是负号，且上一个为空或不是数字
					fu=1;
					nr_token--;
				}
				break;
			}
		}

		if (i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}
	// printf("number of token:%d\n", nr_token);
	return true;
}
bool check_parentheses(uint32_t p,uint32_t q){
	if(tokens[p].type != LEFT  || tokens[q].type != RIGHT)
        return false;
    int l = p , r = q;
    while(l < r)
    {
        if(tokens[l].type == LEFT){
            if(tokens[r].type == RIGHT){
                l++ , r--;
                continue;
            }else r--;
        }else if(tokens[l].type == LEFT) return false;
        else l ++;
    }
    return true;
}//看看p，q中间是否都是配对好的括号
uint32_t find_domanit(uint32_t p,uint32_t q){//找主运算符
	uint32_t anspos=0;
	int nowtype=NOTYPE,l=0,r=0;//左右括号的数量
	uint32_t i;
	// debug;
	// printf("p:%d q:%d\n",p,q);
	for(i=q;i>=p+1;i--){
		// debug
		if(tokens[i].type!=NUM&&tokens[i].type!=NOTYPE){
			if(tokens[i].type==LEFT){
				l++;continue;
			}
			if(tokens[i].type==RIGHT){
				r++;continue;
			}
			if(l!=r) continue;
			if(nowtype==NOTYPE){
				nowtype=tokens[i].type;
				anspos=i;
			}else{
				 switch (tokens[i].type) {
						case NOT:
						case XING:
						case FU:
							break;
						case MUL:
                        case DIV:
							if(nowtype == NOT || nowtype == DIV || nowtype == FU){
								nowtype = tokens[i].type;
								anspos = i;
							}
							break;
                        case ADD:
                        case SUB:
                            // 加减运算符的优先级低于乘除
                            if (nowtype == MUL || nowtype == DIV || nowtype == NOT || nowtype == DIV || nowtype == FU) {
                                nowtype = tokens[i].type;
                                anspos = i;
                            }
                            break;
                        case EQ:
                        case NEQ:
                            // 比较运算符的优先级低于逻辑与或
                            if (nowtype == MUL || nowtype == DIV || nowtype==ADD || nowtype==SUB || nowtype == NOT || nowtype == DIV || nowtype ==FU){
                                nowtype = tokens[i].type;
                                anspos = i;
                            }
                            break;
                        case AND:
                        case OR:
                            // 逻辑与或运算符的优先级最低
                            nowtype = tokens[i].type;
                            anspos = i;
                            break;
                        default:
							printf("No operator\n");
							assert(0);
                    }
			}
		}
		// printf("token%d: %s,anspos:%d,fu:%d\n",tokens[i].type,tokens[i].str,anspos,tokens[i].fu);
	}
	return anspos;
}
int eval(p, q) {
		if (p > q) {
			/* Bad expression */
			printf("Bad experssion\n");
			assert(0);
			return -1;
		}
		else if (p == q) {
			/* Single token.
			* For now this token should be a number.
			* Return the value of the number.
			*/
			int t=atoi(tokens[p].str);
			// if(tokens[p].fu==1){
			// 	t=-t;
			// 	tokens[p].fu=0;
			// } 
			printf("value:%d\n",t);
			return t;//直接转成数字
		}
		else if (check_parentheses(p, q) == true) {
			/* The expression is surrounded by a matched pair of parentheses.
			* If that is the case, just throw away the parentheses.
			*/
			return eval(p + 1, q - 1);
		}
		else {
			/* We should do more things here. */
			if(tokens[p].type==NOT){
				return (!eval(p+1,q));
			}else if(tokens[p].type==FU){
				return -eval(p+1,q);
			}
			int op=find_domanit(p,q);
			// printf("op:%d\n",op);
			int val1=eval(p,op-1),val2=eval(op+1,q);
			switch(tokens[op].type){
				// debug;
				case ADD: return val1+val2;
				case SUB: return val1-val2;
				case MUL: return val1*val2;
				case DIV: return val1/val2;
				case EQ: return (int)(val1==val2);
				case NEQ: return (int)(val1!=val2);
				case AND: return (int)(val1&&val2);
				case OR: return (int)(val1||val2);
				default:{
					printf("type of domanit is:%d\n",tokens[op].type);
					assert(0);
				}
			}
		}
	}
int expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	/* TODO: Insert codes to evaluate the expression. */
	// int i;
	// for(i=0;i<nr_token;i++){
	// 	printf("token%d: %s",tokens[i].type,tokens[i].str);
	// }
	// printf("%d\n",(2||0));
	return eval(0,nr_token-1);
	panic("please implement me");
	return 0;
}