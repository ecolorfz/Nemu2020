#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, NUMBER, HEX,REG,NOTEQ, AND, OR, DEFER, MINUS,EQ
	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	    {" +",  NOTYPE},                                // spaces
        {"\\+", '+'},                                   // plus
        {"\\-", '-'},
        {"\\*", '*'},
        {"\\/", '/'},
        {"\\(", '('},
        {"\\)", ')'},
        {"\\b[0-9]+\\b", NUMBER},
        {"&&", AND},
        {"\\|\\|", OR},
        {"!=", NOTEQ},
		{"\\!", '!'},
        {"\\0[xX][0-9a-fA-F]+", HEX},
        {"\\$[a-zA-Z]+", REG},
        {"==", EQ}                                       // equal

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
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				
				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					default: panic("please implement me");
					         break;
					case NOTYPE:
					    position += substr_len;
					    break;	 
					case '+':
						 tokens[nr_token].type='+';
						 strncpy (tokens[nr_token].str,substr_start,substr_len);
						 tokens[nr_token].str[substr_len]='\0';
						 nr_token++;
						 position += substr_len;
						 break;

				    case '-':
						 tokens[nr_token].type='-';
						strncpy (tokens[nr_token].str,substr_start,substr_len);
                        nr_token++;
						position += substr_len;
						tokens[nr_token].str[substr_len]='\0';
						 break;

			        case '*':
						tokens[nr_token].type='*';
					    strncpy (tokens[nr_token].str,substr_start,substr_len);
                        nr_token++;
						position += substr_len;
						tokens[nr_token].str[substr_len]='\0';
						 break;

			        case '/':
						 tokens[nr_token].type='/';
						strncpy (tokens[nr_token].str,substr_start,substr_len);
                        nr_token++;
							  position += substr_len;
						tokens[nr_token].str[substr_len]='\0';
						 break;

			        case '(':
						 tokens[nr_token].type='(';
					         strncpy (tokens[nr_token].str,substr_start,substr_len);
                          nr_token++;
							  position += substr_len;
						tokens[nr_token].str[substr_len]='\0';
						 break;

			        case ')':
				        tokens[nr_token].type=')';
				        strncpy (tokens[nr_token].str,substr_start,substr_len);
                         nr_token++;
							  position += substr_len;
						tokens[nr_token].str[substr_len]='\0';
				        break;

					case '!':
				        tokens[nr_token].type='!';
			                strncpy (tokens[nr_token].str,substr_start,substr_len);
                            nr_token++;
							position += substr_len;
						tokens[nr_token].str[substr_len]='\0';
				        break;

					case NUMBER:
                        tokens[nr_token].type=NUMBER;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len]='\0';
						nr_token++;
						position += substr_len;
                        assert(strlen(tokens[nr_token].str)<32);
				        break;

					case AND:
						tokens[nr_token].type=AND;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
					    tokens[nr_token].str[substr_len]='\0';
                        nr_token++;
						position += substr_len;
						break;

					case OR:
						tokens[nr_token].type=OR;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len]='\0';
						nr_token++;
						position += substr_len;
					    break;

					case NOTEQ:
					    tokens[nr_token].type=NOTEQ;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len]='\0';
						nr_token++;
						position += substr_len;
						break;

                    case HEX:
					    tokens[nr_token].type=HEX;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
				        tokens[nr_token].str[substr_len]='\0';
				        nr_token++;
						position += substr_len;
                        assert(strlen(tokens[nr_token].str)<32);
				        break;

					case REG:
	                    tokens[nr_token].type=REG;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						nr_token++;
						position += substr_len;
                        assert(strlen(tokens[nr_token].str)<32);
						break;

					case EQ:
				        tokens[nr_token].type=EQ;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						nr_token++;
						position += substr_len;
				        break;		 
				}
				break;
			}
		}
		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}
    
	return true; 
}

bool check_parentheses(int p, int q){
    int i,left=0;
	if(tokens[p].type!='(')return 0;
	for(i=p;i<=q;i++){
		if(tokens[i].type=='(')left++;
		else if(tokens[i].type==')')left--;
		if(left<0)assert(0);
		if(left==0&&i!=q)return 0;
	}
	if(left==0)return 1;
	return 0;
}

int pr(int type){
	if(type=='!'||type==MINUS||type==DEFER)return 1;
	if(type=='*'||type=='/')return 2;
	if(type=='+'||type=='-')return 3;
	if(type==AND)return 4;
	if(type==OR)return 5;
	return 0;
}


uint32_t eval(int p, int q) {

    if(p > q) {
		assert(0);
        /* Bad expression */
    }
    else if(p == q) { 
	        int num;	
		if(tokens[p].type==NUMBER){
		    sscanf(tokens[p].str,"%d",&num);
		    return num;
		}
		else if(tokens[p].type==HEX){
            sscanf(tokens[p].str,"%x",&num);
			return num;
		}
		else if(tokens[p].type==REG){
		return 1;
		}
	
        /* Single token.
         * For now this token should be a number. 
         * Return the value of the number.
         */ 
    }
    else if(check_parentheses(p, q) == true) {
        return eval(p + 1, q - 1); 
    }
    else {
        int op=p,i,left=0;
		int present_pr=0;
		for(i=p;i<=q;i++){
			if(tokens[i].type=='('){
				left++;
				i++;
			}
			while(1){
				if(tokens[i].type=='(')left++;
				else if(tokens[i].type==')')left--;
				i++;
				if(left==0)break;
			}	
			if(i>q)break;
			else if(tokens[i].type==NUMBER||tokens[i].type==REG||tokens[i].type==HEX)continue;
            else if(pr(tokens[i].type)>=present_pr){
					op=i;
					present_pr=pr(tokens[i].type);
				}
			
		}
		int val1;
		if(op==p)val1=0;
		else  val1 = eval(p, op - 1);
        int val2 = eval(op + 1, q);

        switch(tokens[op].type) {
            case '+': return val1 + val2;
            case '-': return val1 - val2; 
            case '*': return val1 * val2; 
            case '/': return val1 / val2;
			case '!': return !val2;
			case AND: return val1 && val2;
			case OR: return val1 || val2;
			case MINUS: return -val2;
			case DEFER: return swaddr_read(val2,4);
            default: assert(0);
        }
    }
	return 0;
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
    int i;
	/* TODO: Insert codes to evaluate the expression. */

	for(i=0;i<nr_token;i++){
        if(tokens[i].type=='-' &&(i==0||tokens[i-1].type=='+'||tokens[i-1].type=='-'||tokens[i-1].type=='*'||tokens[i-1].type=='/')){
			tokens[i].type=MINUS;
		}
	}
	for(i=0;i<nr_token;i++){
        if(tokens[i].type=='*' &&(i==0||tokens[i-1].type=='+'||tokens[i-1].type=='-'||tokens[i-1].type=='*'||tokens[i-1].type=='/')){
			tokens[i].type=DEFER;
		}
	}
	return eval(0, nr_token-1);
	panic("please implement me");
	return 0;
}



