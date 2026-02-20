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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
static int len = 0;    // 索引表达式长度 
static int depth = 10; // 控制递归次数

/* 随机生成运算符 */
static char* gen_op() {
        switch(rand() % 4) {
                case 0: return "+";
                case 1: return "-";
                case 2: return "*";
                default: return "/";
        }
}

/* 随机插入空格 */
static void gen_space() {
    if (rand() % 2 && len < (int)sizeof(buf) - 2) {
        len += snprintf(buf + len, sizeof(buf) - len, " ");
    }
}

/* 生成一个 1~99 的数字 */
static void gen_num() {
    if (len < (int)sizeof(buf) - 8) // 留出给数字的空间
        len += snprintf(buf + len, sizeof(buf) - len, "%u", (uint32_t)(rand() % 99 + 1));
}

static void gen_rand_expr(int depth) {
    if (depth <= 0) {
        /* 差不多就停 */
		gen_num();
        gen_space();
        buf[len] = '\0';
        return;
    }

    int choice = rand() % 3;
    if (choice == 0) {
        gen_num();
        gen_space();
    } else if (choice == 1) {
        if (len < (int)sizeof(buf) - 4) {
            len += snprintf(buf + len, sizeof(buf) - len, "(");
            gen_space();
            gen_rand_expr(depth - 1);
            gen_space();
            len += snprintf(buf + len, sizeof(buf) - len, ")");
            gen_space();
        }
    } else {
        gen_rand_expr(depth - 1);

        gen_space();
        char *op = gen_op();
        len += snprintf(buf + len, sizeof(buf) - len, "%s", op);
        gen_space();

        if (op[0] == '/') {
            gen_num(); // 除法右侧生成非0数字
        } else {
            gen_rand_expr(depth - 1);
        }
    }
    buf[len] = '\0';
}


int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
	len = 0;
	buf[0] = '\0';
    gen_rand_expr(depth);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr"); // 用system()调用操作系统的命令行
    if (ret != 0) {
		/* 如果表达式有错则不执行剩余语句 */
		printf("wrong expression\n");
		continue;
	}

    fp = popen("/tmp/.expr", "r"); // popen()用来打开一个管道并执行一个系统命令
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);

  }
  return 0;
}
