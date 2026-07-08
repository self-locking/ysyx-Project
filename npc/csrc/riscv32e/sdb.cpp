#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <readline/readline.h>
#include "Vriscv32e___024root.h"
#include "npc.h"
#include "sdb.h"

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
    static char *line_read = NULL;

    if (line_read) {
        free(line_read);
        line_read = NULL;
    }
    line_read = readline("(npc) ");

    return line_read;
}

static int cmd_q(char *args) {
    is_running = 0;
    return 0;
}

static int cmd_c(char *args) {
    if (!cpu_running) {
        printf("Program has terminated.\n");
        return 0;
    }
    cpu_exec(-1);
    return 0;
}

static int cmd_si(char *args) {
    if (!cpu_running) {
        printf("Program has terminated.\n");
        return 0;
    }

    int n = 1;
    if (args != NULL) {
        n = atoi(args);
    }
    cpu_exec(n);

    return 0;
}

static int cmd_pc(char *args) {
    uint32_t pc = top->PC;
    printf("0x%08x: %08x\n", pc, pmem[(pc >> 2) - MBASE]);

    return 0;
}

static int cmd_info(char *args) {
    if (args == NULL) {
        printf("Please input the subcmd\n");
    }
    else if (strcmp(args, "r") == 0) {
        for (int i = 0; i < 16; i++) {
            printf("x%02d = 0x%x\n", i, top->rootp->riscv32e__DOT__gpr7__DOT__rf[i]);
        }
    }
    return 0;
}

static int cmd_x(char *args) {
    int n = 0;
    uint32_t addr = 0;

    if (args == NULL) {
        printf("Unknown subcmd\n");
        return 0;
    }

    // 解析两个参数
    int ret = sscanf(args, "%d %x", &n, &addr);
    if (ret != 2 || n <= 0) {
        printf("Unknown subcmd\n");
        return 0;
    }

    for (int i = 0; i < n; i++) {
        uint32_t cur = addr / 4;
        if (cur < MBASE || cur - MBASE >= MAX_SIZE) {
            printf("out of bound\n");
            continue;
        }

        uint32_t data = pmem[(cur - MBASE)];
        printf("0x%08x: %08x\n", addr, data);
        addr += 4;
    }

    return 0;
}

// sdb指令
static struct {
    const char *name;
    int (*handler) (char *);
} cmd_table [] = {
    {"c", cmd_c},
    {"si", cmd_si},
    {"info", cmd_info},
    {"x", cmd_x},
    {"q", cmd_q},
    {"pc", cmd_pc},
};

void sdb_mainloop() {
    while (is_running) {
        for (char *str; is_running && ((str = rl_gets()) != NULL); ) {
            char *str_end = str + strlen(str);
            char *cmd = strtok(str, " ");

            if (cmd == NULL) { continue; }

            char *args = cmd + strlen(cmd) + 1;
            if (args >= str_end) {
                args = NULL;
            }

            int i;
            for (i = 0; i < 6; i++) {
                if (strcmp(cmd, cmd_table[i].name) == 0) {
                    if (cmd_table[i].handler(args) < 0) { printf("Wrong\n"); return; }
                    break;
                }
            }
            if (i == 6) printf("Unknown command '%s'\n", cmd);
        }
    }
}