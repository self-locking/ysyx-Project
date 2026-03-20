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
/* sdb.c */

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>
#include <monitor/watchpoint.h>
#include <stdlib.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
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
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Run the program step by step for N instructions", cmd_si },
  { "info", "Print register status or watchpoint information", cmd_info },
  { "x", "Scan memory", cmd_x },
  {"p", "Calculate the value of expression", cmd_p},
  {"w", "Set watchpoint", cmd_w},
  {"d", "Delete watchpoint", cmd_d},

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
	int n = 1;

	if (args != NULL) {
		n = atoi(args);
	}
	cpu_exec(n);

	return 0;
}

static int cmd_info(char *args) {
	if (args == NULL) {
		printf("Please input the subcmd\n");
	}
	else if (strcmp(args, "r") == 0) {
		/* nemu/src/isa/$ISA/reg.c */
    isa_reg_display();
	} 
	else if (strcmp(args, "w") == 0) {
		WP* cur = head;
    if (cur == NULL) {
      printf("No watchpoint\n");
      return 0;
    }
		while (cur != NULL) {
			printf("Watchpoint %d: %s\n", cur->NO, cur->expr);
			cur = cur->next;
		}
	}
	else {
		printf("Unknown subcmd\n");
	}
	return 0;
}

static int cmd_x(char *args) {
  if (args == NULL) {
    printf("Please input the subcmd\n");
    return 0;
  }

  int N;
  vaddr_t addr;
  char *expr_str;
  bool x_success = true;

  expr_str = strtok(args, " ");
  if (expr_str == NULL) {
    printf("Unknown subcmd\n");
    return 0;
  }

  N = atoi(expr_str);            // 取第一个参数为次数
  expr_str = strtok(NULL, "");   // 取剩余全部字符串
  if (expr_str == NULL) {
    printf("Unknown subcmd\n");
    return 0;
  }

  addr = expr(expr_str, &x_success);
  if (!x_success) {
    printf("Wrong expression\n");
    return 0;
  }

  for (int i = 0; i < N; i++) {
    word_t data = vaddr_read(addr, 4);
    printf("0x%x: 0x%x\n", addr, data);
    addr += 4;
  }

  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL) {
    printf("Please input the expression\n");
  }
  else {
    bool success = true;
    word_t value = expr(args, &success);

    if(success == false) {
      printf("Expression is wrong\n");
    }
    else printf("%u\n", value);
  }
  return 0;
}

static int cmd_w(char *args) {
  if (args == NULL) {
    printf("Please input the expression\n");
  }
  else {
    WP* wp = new_wp();
    strncpy(wp->expr, args, sizeof(wp->expr) - 1);
    wp->expr[sizeof(wp->expr) - 1] = '\0';
    bool success = true;
    wp->old_val = expr(wp->expr, &success);
    if(success == false) {
      printf("Expression is wrong\n");
      free_wp(wp);
    }
    else printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
  }
  return 0;
}

static int cmd_d(char *args) {
  if (args == NULL) {
    printf("Please input the watchpoint number\n");
  }
  else {
    int NO = atoi(args); // 没有防御性编程，调用时别乱输
    free_wp_no(NO);      // 用序号索引
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
