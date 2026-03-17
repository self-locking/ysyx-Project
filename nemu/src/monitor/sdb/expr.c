/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <memory/vaddr.h>
#include <debug.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
  TK_DEC, TK_HEX, TK_REG,
  TK_NOT_EQ, TK_AND, TK_OR,
  TK_DEREF, TK_NEG,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},             // spaces
  {"==", TK_EQ},                 // equal
  {"!=", TK_NOT_EQ},             // not equal
  {"&&", TK_AND},                // logical and
  {"\\|\\|", TK_OR},             // logical or

  {"\\+", '+'},                  // plus
  {"-", '-'},                    // minus / negative
  {"\\*", '*'},                  // multiply / pointer dereference
  {"/", '/'},                    // divide
  {"\\(", '('},                  // left parenthesis
  {"\\)", ')'},                  // right parenthesis

  {"0[xX][0-9a-fA-F]+",TK_HEX},  // hex num
  {"\\$\\$?[0-9a-z]+", TK_REG},  // register / PC
  {"[0-9]+",TK_DEC},             // decimal num
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[64];
} Token;

static Token tokens[128] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
		/*pmatch储存匹配到的token位置，rm_so是开头，rm_eo是结尾*/
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

		    Assert(substr_len < 64, "token length exceeds 64 characters."); // 不处理overflow

        /* ============================record token================================ */
        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array 'tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
            case TK_NOTYPE:
                break;
            case TK_DEC:
            case TK_HEX:
            case TK_REG:
                tokens[nr_token].type = rules[i].token_type;
                strncpy(tokens[nr_token].str, substr_start, substr_len);
                tokens[nr_token].str[substr_len] = '\0'; // 手动补'\0'
                nr_token++;
                break;
            default:
                tokens[nr_token].type = rules[i].token_type;
                nr_token++;
                break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static bool check_parentheses(int p, int q) {
	/* 检测是否被括号包含 */
	if (tokens[p].type != '(' || tokens[q].type != ')') return false;
  	int balance = 0;

  	for (int i = p; i < q; i++) {
    	if (tokens[i].type == '(') balance++;
    	else if (tokens[i].type == ')') balance--;
    	// 如果在没到最后一个 token 之前 balance 就变成 0，说明首尾括号不是一对
    	if (balance == 0) return false;
	}
	return true; 
}

/* 优先级定义 */
static int get_precedence(int type) {
  switch (type) {
    case TK_EQ: case TK_NOT_EQ: case TK_AND: case TK_OR: return 0; // 二元运算符运算优先级最低
    case '+': case '-': return 1; // 加减优先级相同
    case '*': case '/': return 2; // 乘除优先级高于加减
    case TK_DEREF: case TK_NEG: return 3;      // 指针解引用和负号优先级更高
    default: return 100;
  }
}

long eval(int p, int q, bool *success) {
	if (*success == false || p > q) {
	/* Bad expression */
    *success = false;
    return 0;
  }
  else if (p == q) {
        /* Single token */
        long val = 0;
        switch(tokens[p].type) {
            case TK_DEC:
                sscanf(tokens[p].str, "%ld", &val);
                return val;
            case TK_HEX:
                return (long)strtoul(tokens[p].str, NULL, 0);
            case TK_REG: {
                bool reg_success = true;
                val = (strcmp(tokens[p].str + 1, "pc") == 0) ? cpu.pc : isa_reg_str2val(tokens[p].str + 1, &reg_success); // +1为了去掉$前缀
                if (!reg_success) {
                    printf("Invalid register: %s\n", tokens[p].str);
                    *success = false;
                    return 0;
                }
                return val;
            }
            default:
                *success = false;
                return 0;
        }
    }
  else if (check_parentheses(p, q) == true) {
    return eval(p + 1, q - 1, success);
  }
  /* 处理负号 */
  else if (tokens[p].type == TK_NEG) {
       word_t val = eval(p + 1, q, success);
       if (!*success) return 0;
       return -val;
   }
  else {
	/* Find the main operator */
    int op = -1;
    int op_prec = 100;
    int balance = 0;

    for (int i = p; i <= q; i++) {
      /* 忽略括号内运算符 */
      if (tokens[i].type == '(') balance++;
      else if (tokens[i].type == ')') balance--;

      else if (balance == 0) {
      	int prec = get_precedence(tokens[i].type);
      	if (prec != 100 && tokens[i].type != TK_NEG && prec <= op_prec) {
        	op = i;
        	op_prec = prec;
      	}
      }
    }

    /* 处理指针解引用，op==p确保*左侧没有tokens */
    if (op != -1 && op == p && tokens[op].type == TK_DEREF) {
      word_t val = eval(op + 1, q, success);
      if (!*success) return 0;
      return vaddr_read(val, 4);
    }

    if (op == -1) {
      // 找不到运算符，说明表达式格式有误
      *success = false;
      return 0;
    }

    // 递归求值左右子表达式
    long val1 = eval(p, op - 1, success);
    long val2 = eval(op + 1, q, success);
	if (*success == false) return 0;

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': 
        if (val2 == 0) {
          *success = false;
          return 0;
        }
        return val1 / val2;
        
      /* true返回1，false返回0 */
      case TK_EQ: return val1 == val2;
      case TK_NOT_EQ: return val1 != val2;
      case TK_AND: return val1 && val2;
      case TK_OR: return val1 || val2;
      default: 
        *success = false;
        return 0;
    }
  }
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  else {
    /* 这里判断是不是指针解析和负号 */
    for (int i = 0; i < nr_token; i++) {
      if (tokens[i].type == '*') {
        if (i == 0) {
          tokens[i].type = TK_DEREF;
        } else {
          int prev_type = tokens[i-1].type;
          if (prev_type != TK_DEC && prev_type != TK_HEX && prev_type != TK_REG && prev_type != ')') {
            tokens[i].type = TK_DEREF;
          }
        }
      }
      else if (tokens[i].type == '-') {
        if (i == 0) {
          tokens[i].type = TK_NEG;
        } else {
          int prev_type = tokens[i-1].type;
          if (prev_type != TK_DEC && prev_type != TK_HEX && prev_type != TK_REG && prev_type != ')') {
            tokens[i].type = TK_NEG;
          }
        }
      }
    }

    return (word_t)eval(0, nr_token - 1, success);
  }
}