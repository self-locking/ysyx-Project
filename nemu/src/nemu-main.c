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
/* nemu-main.c */

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
// word_t expr(char *e, bool *success);

// void test_expr() {
//   const char *filepath = "./tools/gen-expr/input";
//   FILE *fp = fopen(filepath, "r");
//   assert(fp != NULL);

//   char line_buffer[1024];
//   // 循环逐行读取文件内容
//     while (fgets(line_buffer, sizeof(line_buffer), fp) != NULL) {
//         unsigned expected_result;
//         char expression[1000];

//         // 使用 sscanf 解析每一行
//         // 一个无符号长整型 (%lu)，一个空格，然后是直到行尾的所有字符 (%[^\n])
//         if (sscanf(line_buffer, "%u %999[^\n]", &expected_result, expression) == 2) {
//             bool success = true;
//             word_t actual_result = expr(expression, &success);
            
//             if (!success || actual_result != expected_result) {
//                 printf("Failed: %s, expected %u, but got %u\n", expression, expected_result, actual_result);
//             }
//         } else {
//             // 如果某一行格式不正确，打印一个警告
//             fprintf(stderr, "Could not parse line: %s", line_buffer);
//         }
//     }

//     fclose(fp);
//     printf("Finished.\n");
// }

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  // test_expr();

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
