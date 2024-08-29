#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NUM = 0,
	NOTYPE = 1,
	ADD = 2,
	SUB = 3,
	MUL = 4,
	DIV = 5,
	LEFT = 6 ,
	RIGHT = 7,
	EQ = 8

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{"[0-9]+", NUM},            // 数字
	{" +", NOTYPE},           // 空白字符
    {"\\+", ADD},             // 加号
    {"-", SUB},               // 减号
    {"\\*", MUL},             // 乘号
    {"/", DIV},               // 除号
    {"\\(", LEFT},            // 左括号
    {"\\)", RIGHT},           // 右括号
    {"==", EQ},               // 等于
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
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;//储存匹配结果
	// debug;
	nr_token = 0;//已有的token数量

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {//遍历所有正则表达式规则
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;//匹配成功子串的起始地址
				int substr_len = pmatch.rm_eo;//匹配成功字符串长度

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				//匹配成功的日志
				position += substr_len;
				if(rules[i].token_type==NOTYPE) break;
				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */
				if(nr_token<32){
					strncpy(tokens[nr_token].str,substr_start,substr_len);//复制一段
					tokens[nr_token].str[substr_len-1]='\0';//给字符串结尾
					tokens[nr_token].type=rules[i].token_type;
					nr_token++;
				}else{
					printf("too much tokens\n");
					assert(0);
				}
				switch(rules[i].token_type) {
					case NOTYPE:{
						assert(0);
						break;
					}
					case NUM:break;
					case ADD:break;
					case SUB:break;
					case MUL:break;
					case DIV:break;
					case LEFT:break;
					case RIGHT:break;
					case EQ:break;
					default: panic("please implement me");
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}
	printf("number of token:%d\n",nr_token);
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
	int i;
	for(i=q;i>=p;i--){
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
				if(nowtype==ADD||nowtype==SUB){
					if(tokens[i].type==MUL||tokens[i].type==DIV){
						nowtype=tokens[i].type;
						anspos=i;
					}
				}
			}
		}
	}
	return anspos;
}
uint32_t eval(p, q) {
		debug;
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
			printf("value:%d\n",atoi(tokens[p].str));
			return atoi(tokens[p].str);//直接转成数字
		}
		else if (check_parentheses(p, q) == true) {
			/* The expression is surrounded by a matched pair of parentheses.
			* If that is the case, just throw away the parentheses.
			*/
			return eval(p + 1, q - 1);
		}
		else {
			/* We should do more things here. */
			uint32_t op=find_domanit(p,q);
			printf("op:%d\n",op);
			uint32_t val1=eval(p,op-1),val2=eval(op+1,q);
			switch(tokens[op].type){
				// debug;
				case ADD: return val1+val2;
				case SUB: return val1-val2;
				case MUL: return val1*val2;
				case DIV: return val1/val2;
				default:{
					printf("type of domanit is:%d\n",tokens[op].type);
					assert(0);
				}
			}
		}
	}
uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	/* TODO: Insert codes to evaluate the expression. */
	int i;
	for(i=0;i<nr_token;i++){
		printf("token%d: %s\n",tokens[i].type,tokens[i].str);
	}
	return eval(0,nr_token-1);
	panic("please implement me");
	return 0;
}